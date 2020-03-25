/* APOViewer.cpp */

#include "APOViewer.h"
#include "APOViewerView.h"

#include <QToolBar>
#include <QTreeView>
#include <QMessageBox>
#include <QFileDialog>
#include <QDockWidget>

#include <gp_Quaternion.hxx>

#include <gp_Circ.hxx>
#include <gp_Elips.hxx>
#include <gp_Pln.hxx>

#include <gp_Lin2d.hxx>

#include <Geom_ConicalSurface.hxx>
#include <Geom_ToroidalSurface.hxx>
#include <Geom_CylindricalSurface.hxx>

#include <GCE2d_MakeSegment.hxx>

#include <TopoDS.hxx>
#include <TopExp.hxx>
#include <TopExp_Explorer.hxx>
#include <TColgp_Array1OfPnt2d.hxx>

#include <BRepLib.hxx>

#include <BRepBuilderAPI_MakeVertex.hxx>
#include <BRepBuilderAPI_MakeEdge.hxx>
#include <BRepBuilderAPI_MakeWire.hxx>
#include <BRepBuilderAPI_MakeFace.hxx>
#include <BRepBuilderAPI_Transform.hxx>
#include <BRepBuilderAPI_MakePolygon.hxx>

#include <BRepPrimAPI_MakeBox.hxx>
#include <BRepPrimAPI_MakeCone.hxx>
#include <BRepPrimAPI_MakeSphere.hxx>
#include <BRepPrimAPI_MakeCylinder.hxx>
#include <BRepPrimAPI_MakeTorus.hxx>
#include <BRepPrimAPI_MakePrism.hxx>
#include <BRepPrimAPI_MakeRevol.hxx>

#include <BRepFilletAPI_MakeFillet.hxx>
#include <BRepFilletAPI_MakeChamfer.hxx>

#include <BRepOffsetAPI_MakePipe.hxx>
#include <BRepOffsetAPI_ThruSections.hxx>

#include <BRepAlgoAPI_Cut.hxx>
#include <BRepAlgoAPI_Fuse.hxx>
#include <BRepAlgoAPI_Common.hxx>

#include <STEPControl_Reader.hxx>

#include <AIS_Shape.hxx>

APOViewer::APOViewer(QWidget *parent)
    : QMainWindow(parent)
{
    ui.setupUi(this);

    myAPOViewerView = new APOViewerView(this);

    setCentralWidget(myAPOViewerView);

    createActions();
    createMenus();
    createToolBars();
}

APOViewer::~APOViewer()
{

}

void APOViewer::createActions( void )
{
    // File
	connect(ui.actionOpen, SIGNAL(triggered()), this, SLOT(open()));
	connect(ui.actionExit, SIGNAL(triggered()), this, SLOT(close()));

    // View
    connect(ui.actionZoom, SIGNAL(triggered()), myAPOViewerView, SLOT(zoom()));
    connect(ui.actionPan, SIGNAL(triggered()), myAPOViewerView, SLOT(pan()));
    connect(ui.actionRotate, SIGNAL(triggered()), myAPOViewerView, SLOT(rotate()));

    connect(ui.actionReset, SIGNAL(triggered()), myAPOViewerView, SLOT(reset()));
    connect(ui.actionFitAll, SIGNAL(triggered()), myAPOViewerView, SLOT(fitAll()));

	connect(ui.actionLeft, SIGNAL(triggered()), this, SLOT(gobackward()));
	connect(ui.actionRight, SIGNAL(triggered()), this, SLOT(goforward()));

    // Primitive
    connect(ui.actionBox, SIGNAL(triggered()), this, SLOT(makeBox()));
    connect(ui.actionCone, SIGNAL(triggered()), this, SLOT(makeCone()));
    connect(ui.actionSphere, SIGNAL(triggered()), this, SLOT(makeSphere()));
    connect(ui.actionCylinder, SIGNAL(triggered()), this, SLOT(makeCylinder()));
    connect(ui.actionTorus, SIGNAL(triggered()), this, SLOT(makeTorus()));

    // Modeling
    connect(ui.actionFillet, SIGNAL(triggered()), this, SLOT(makeFillet()));
    connect(ui.actionChamfer, SIGNAL(triggered()), this, SLOT(makeChamfer()));
    connect(ui.actionExtrude, SIGNAL(triggered()), this, SLOT(makeExtrude()));
    connect(ui.actionRevolve, SIGNAL(triggered()), this, SLOT(makeRevol()));
    connect(ui.actionLoft, SIGNAL(triggered()), this, SLOT(makeLoft()));

    connect(ui.actionCut, SIGNAL(triggered()), this, SLOT(testCut()));
    connect(ui.actionFuse, SIGNAL(triggered()), this, SLOT(testFuse()));
    connect(ui.actionCommon, SIGNAL(triggered()), this, SLOT(testCommon()));

    connect(ui.actionHelix, SIGNAL(triggered()), this, SLOT(testHelix()));

    // Help
    connect(ui.actionAbout, SIGNAL(triggered()), this, SLOT(about()));
}

void APOViewer::createMenus( void )
{
}

void APOViewer::createToolBars( void )
{
    QToolBar* aToolBar = addToolBar(tr("&Navigate"));
    aToolBar->addAction(ui.actionZoom);
    aToolBar->addAction(ui.actionPan);
    aToolBar->addAction(ui.actionRotate);

    aToolBar = addToolBar(tr("&View"));
    aToolBar->addAction(ui.actionReset);
    aToolBar->addAction(ui.actionFitAll);
	aToolBar->addAction(ui.actionLeft);
	aToolBar->addAction(ui.actionRight);
}

void APOViewer::about()
{
    QMessageBox::about(this, tr("About APOViewer"),
        tr("<h2>APOViewer</h2>"
        "<p>APOViewer is viewer for pose.list of Assembly Plan from Observation (APO)"));
}

int APOViewer::LoadPoseList(const std::string &file)
{
	std::ifstream ifs(file);
	if (!ifs) {
		QString message("Cannot open: ");
		message += file.c_str();
		QMessageBox::critical(this, tr("File not open"), message);
		return -1;
	}
	try {
		std::string dummy;
		size_t num;
		ifs >> dummy >> dummy >> dummy >> num;
		object_names.resize(num);
		for (size_t i(0); i < object_names.size(); ++i) {
			ifs >> object_names[i];
		}
		ifs >> dummy >> dummy >> dummy >> num;
		poses.resize(num);
		for (size_t i(0); i < poses.size(); ++i) {
			poses[i].resize(object_names.size());
			for (size_t j(0); j < poses[i].size(); ++j) {
				Standard_Real x, y, z, w;
				// location
				ifs >> dummy >> x >> y >> z;
				gp_Vec v(x, y, z);
				// vector-angle 
				ifs >> x >> y >> z >> w;
				gp_Quaternion q;
				q.SetVectorAndAngle(gp_Vec(x, y, z), w);
				poses[i][j].SetTransformation(q, v);
			}
		}
		return 0;
	}
	catch (...) {
		QMessageBox::critical(this, tr("Format error"), tr("Format error"));
		object_names.clear();
		poses.clear();
		return -1;
	}
}

void APOViewer::open()
{
	QString fileName = QFileDialog::getOpenFileName(this, tr("model file"), ".", tr("pose list file(*.list)"));
	if (!fileName.isEmpty()) {
		if (LoadPoseList(fileName.toStdString()) == 0) {
			aTopoObjects.resize(object_names.size());
			for (size_t i(0); i < object_names.size(); ++i) {
				STEPControl_Reader reader;
				IFSelect_ReturnStatus stat = reader.ReadFile(object_names[i].c_str());
				if (stat != IFSelect_ReturnStatus::IFSelect_RetDone) {
					QString message("Cannot open: ");
					message += object_names[i].c_str();
					QMessageBox::critical(this, tr("Cannot open"), message);
					aTopoObjects.clear();
					return;
				}
				reader.TransferRoot();
				aTopoObjects[i] = reader.Shape();
			}
		}
		time = 0;
		SetObject();
	}
}

void APOViewer::SetObject()
{
	// draw
	myAPOViewerView->getContext()->EraseAll(Standard_False);
	for (size_t i(0); i < aTopoObjects.size(); ++i) {
		// transform
		BRepBuilderAPI_Transform transform(poses[time][i]);
		transform.Perform(aTopoObjects[i]);
		Handle(AIS_Shape) anAisModel = new AIS_Shape(transform.Shape());
		anAisModel->SetColor(Quantity_NOC_AZURE);
		myAPOViewerView->getContext()->Display(anAisModel, (i == aTopoObjects.size() - 1));
	}
}

void APOViewer::goforward()
{
	if (time < poses.size() - 1) {
		time++;
		SetObject();
	}
}

void APOViewer::gobackward()
{
	if (time > 0) {
		time--;
		SetObject();
	}
}

