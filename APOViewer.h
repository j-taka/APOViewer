/* APOViewer.h */

#pragma once

#include "ui_APOViewer.h"

#include <AIS_InteractiveContext.hxx>

class APOViewerView;

//! Qt main window which include OpenCASCADE for its central widget.
class APOViewer : public QMainWindow
{
    Q_OBJECT

public:
    //! constructor/destructor.
    APOViewer(QWidget *parent = 0);
    ~APOViewer();

protected:
    //! create all the actions.
    void createActions(void);

    //! create all the menus.
    void createMenus(void);

    //! create the toolbar.
    void createToolBars(void);

private slots:
	//!
	void open(void);

	//! show about box.
    void about(void);

	void goforward(void);
	void gobackward(void);

private:
	int LoadPoseList(const std::string &filename);
	void SetObject();

private:
    Ui::occQtClass ui;

    // wrapped the widget for occ.
    APOViewerView* myAPOViewerView;

	// data
	std::vector<std::string> object_names;
	std::vector<TopoDS_Shape> aTopoObjects;

	size_t time;
	typedef std::vector<gp_Trsf> PosesInEachFrame;
	std::vector<PosesInEachFrame> poses;
};
