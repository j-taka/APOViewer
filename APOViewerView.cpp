/* APOViewerView.cpp */

#include <OpenGl_GraphicDriver.hxx>

#include "APOViewerView.h"

#include <QMenu>
#include <QMouseEvent>
#include <QRubberBand>
#include <QStyleFactory>

#include <V3d_View.hxx>

#include <Aspect_Handle.hxx>
#include <Aspect_DisplayConnection.hxx>

#ifdef WNT
    #include <WNT_Window.hxx>
#elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
    #include <Cocoa_Window.hxx>
#else
    #undef Bool
    #undef CursorShape
    #undef None
    #undef KeyPress
    #undef KeyRelease
    #undef FocusIn
    #undef FocusOut
    #undef FontChange
    #undef Expose
    #include <Xw_Window.hxx>
#endif


static Handle(Graphic3d_GraphicDriver)& GetGraphicDriver()
{
  static Handle(Graphic3d_GraphicDriver) aGraphicDriver;
  return aGraphicDriver;
}

APOViewerView::APOViewerView(QWidget* parent )
    : QGLWidget(parent),
    myXmin(0),
    myYmin(0),
    myXmax(0),
    myYmax(0),    
    myCurrentMode(CurAction3d_DynamicRotation),
    myDegenerateModeIsOn(Standard_True),
    myRectBand(NULL)
{
    // No Background
    setBackgroundRole( QPalette::NoRole );

    // Enable the mouse tracking, by default the mouse tracking is disabled.
    setMouseTracking( true );

	// needed so that this widget will process keyboard event
	// setFocusPolicy(Qt::StrongFocus);

	// set default keyboard focus on this widget
	// setFocus();	
	
	// installEventFilter(this);

}

#if 0
bool APOViewerView::eventFilter(QObject *, QEvent *event)
{
	bool bRtn = false;

	std::ofstream ofs("hoge2.txt");
	ofs << event->type() << std::endl;
	ofs.close();

	if (event->type() == QEvent::KeyPress) {
		bRtn = eventKeyPress(static_cast<QKeyEvent *>(event));
	}

	return bRtn;
}
#endif

void APOViewerView::init()
{
    // Create Aspect_DisplayConnection
    Handle(Aspect_DisplayConnection) aDisplayConnection =
            new Aspect_DisplayConnection();

    // Get graphic driver if it exists, otherwise initialise it
    if (GetGraphicDriver().IsNull())
    {
        GetGraphicDriver() = new OpenGl_GraphicDriver(aDisplayConnection);
    }

    // Get window handle. This returns something suitable for all platforms.
    WId window_handle = (WId) winId();

    // Create appropriate window for platform
    #ifdef WNT
        Handle(WNT_Window) wind = new WNT_Window((Aspect_Handle) window_handle);
    #elif defined(__APPLE__) && !defined(MACOSX_USE_GLX)
        Handle(Cocoa_Window) wind = new Cocoa_Window((NSView *) window_handle);
    #else
        Handle(Xw_Window) wind = new Xw_Window(aDisplayConnection, (Window) window_handle);
    #endif

    // Create V3dViewer and V3d_View
    myViewer = new V3d_Viewer(GetGraphicDriver(), Standard_ExtString("viewer3d"));

    myView = myViewer->CreateView();

    myView->SetWindow(wind);
    if (!wind->IsMapped()) wind->Map();

    // Create AISInteractiveContext
    myContext = new AIS_InteractiveContext(myViewer);

    // Set up lights etc
    myViewer->SetDefaultLights();
    myViewer->SetLightOn();

    myView->SetBackgroundColor(Quantity_NOC_BLACK);
    myView->MustBeResized();
    myView->TriedronDisplay(Aspect_TOTP_LEFT_LOWER, Quantity_NOC_GOLD, 0.08, V3d_ZBUFFER);

    myContext->SetDisplayMode(AIS_Shaded, Standard_True);
}

const Handle(AIS_InteractiveContext)& APOViewerView::getContext() const
{
    return myContext;
}

void APOViewerView::paintEvent( QPaintEvent* /*theEvent*/ )
{
    if (myContext.IsNull())
    {
        init();
    }

    myView->Redraw();
}

void APOViewerView::resizeEvent( QResizeEvent* /*theEvent*/ )
{
    if( !myView.IsNull() )
    {
        myView->MustBeResized();
    }
}

void APOViewerView::fitAll( void )
{
    myView->FitAll();
    myView->ZFitAll();
    myView->Redraw();
}

void APOViewerView::reset( void )
{
    myView->Reset();
}

void APOViewerView::pan( void )
{
    myCurrentMode = CurAction3d_DynamicPanning;
}

void APOViewerView::zoom( void )
{
    myCurrentMode = CurAction3d_DynamicZooming;
}

void APOViewerView::rotate( void )
{
    myCurrentMode = CurAction3d_DynamicRotation;
}

void APOViewerView::mousePressEvent( QMouseEvent* theEvent )
{
    if (theEvent->button() == Qt::LeftButton)
    {
        onLButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
    else if (theEvent->button() == Qt::MidButton)
    {
        onMButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
    else if (theEvent->button() == Qt::RightButton)
    {
        onRButtonDown((theEvent->buttons() | theEvent->modifiers()), theEvent->pos());
    }
}

void APOViewerView::mouseReleaseEvent( QMouseEvent* theEvent )
{
    if (theEvent->button() == Qt::LeftButton)
    {
        onLButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
    else if (theEvent->button() == Qt::MidButton)
    {
        onMButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
    else if (theEvent->button() == Qt::RightButton)
    {
        onRButtonUp(theEvent->buttons() | theEvent->modifiers(), theEvent->pos());
    }
}

void APOViewerView::mouseMoveEvent( QMouseEvent * theEvent )
{
    onMouseMove(theEvent->buttons(), theEvent->pos());
}

void APOViewerView::wheelEvent( QWheelEvent * theEvent )
{
    onMouseWheel(theEvent->buttons(), theEvent->delta(), theEvent->pos());
}

void APOViewerView::onLButtonDown( const int /*theFlags*/, const QPoint thePoint )
{
    // Save the current mouse coordinate in min.
    myXmin = thePoint.x();
    myYmin = thePoint.y();
    myXmax = thePoint.x();
    myYmax = thePoint.y();

}

void APOViewerView::onMButtonDown( const int /*theFlags*/, const QPoint /* thePoint */)
{
}

void APOViewerView::onRButtonDown( const int /*theFlags*/, const QPoint thePoint )
{
	// Save the current mouse coordinate in min.
	myXmin = thePoint.x();
	myYmin = thePoint.y();
	myXmax = thePoint.x();
	myYmax = thePoint.y();

	if (myCurrentMode == CurAction3d_DynamicRotation)
	{
		myView->StartRotation(thePoint.x(), thePoint.y());
	}
}

void APOViewerView::onMouseWheel( const int /*theFlags*/, const int theDelta, const QPoint thePoint )
{
    Standard_Integer aFactor = 16;

    Standard_Integer aX = thePoint.x();
    Standard_Integer aY = thePoint.y();

    if (theDelta > 0)
    {
        aX += aFactor;
        aY += aFactor;
    }
    else
    {
        aX -= aFactor;
        aY -= aFactor;
    }

    myView->Zoom(thePoint.x(), thePoint.y(), aX, aY);
}

void APOViewerView::addItemInPopup( QMenu* /*theMenu*/ )
{
}

void APOViewerView::popup( const int /*x*/, const int /*y*/ )
{
}

void APOViewerView::onLButtonUp( const int theFlags, const QPoint thePoint )
{
    // Hide the QRubberBand
    if (myRectBand)
    {
        myRectBand->hide();
    }

    // Ctrl for multi selection.
    if (thePoint.x() == myXmin && thePoint.y() == myYmin)
    {
        if (theFlags & Qt::ControlModifier)
        {
            multiInputEvent(thePoint.x(), thePoint.y());
        }
        else
        {
            inputEvent(thePoint.x(), thePoint.y());
        }
    }

}

void APOViewerView::onRButtonUp( const int /*theFlags*/, const QPoint thePoint )
{
    if (thePoint.x() == myXmin && thePoint.y() == myYmin)
    {
        panByMiddleButton(thePoint);
    }
}

void APOViewerView::onMButtonUp( const int /*theFlags*/, const QPoint thePoint )
{
    popup(thePoint.x(), thePoint.y());
}

void APOViewerView::onMouseMove( const int theFlags, const QPoint thePoint )
{
    // Draw the rubber band.
    if (theFlags & Qt::LeftButton)
    {
        drawRubberBand(myXmin, myYmin, thePoint.x(), thePoint.y());

        dragEvent(thePoint.x(), thePoint.y());
    }

    // Ctrl for multi selection.
    if (theFlags & Qt::ControlModifier)
    {
        multiMoveEvent(thePoint.x(), thePoint.y());
    }
    else
    {
        moveEvent(thePoint.x(), thePoint.y());
    }

    // Middle button.
    if (theFlags & Qt::RightButton)
    {
        switch (myCurrentMode)
        {
        case CurAction3d_DynamicRotation:
            myView->Rotation(thePoint.x(), thePoint.y());
            break;

        case CurAction3d_DynamicZooming:
            myView->Zoom(myXmin, myYmin, thePoint.x(), thePoint.y());
            break;

        case CurAction3d_DynamicPanning:
            myView->Pan(thePoint.x() - myXmax, myYmax - thePoint.y());
            myXmax = thePoint.x();
            myYmax = thePoint.y();
            break;

         default:
            break;
        }
    }

}

void APOViewerView::dragEvent( const int x, const int y )
{
    myContext->Select(myXmin, myYmin, x, y, myView, Standard_True);

    emit selectionChanged();
}

void APOViewerView::multiDragEvent( const int x, const int y )
{
    myContext->ShiftSelect(myXmin, myYmin, x, y, myView, Standard_True);

    emit selectionChanged();

}

void APOViewerView::inputEvent( const int x, const int y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    myContext->Select(Standard_True);

    emit selectionChanged();
}

void APOViewerView::multiInputEvent( const int x, const int y )
{
    Q_UNUSED(x);
    Q_UNUSED(y);

    myContext->ShiftSelect(Standard_True);

    emit selectionChanged();
}

void APOViewerView::moveEvent( const int x, const int y )
{
    myContext->MoveTo(x, y, myView, Standard_True);
}

void APOViewerView::multiMoveEvent( const int x, const int y )
{
    myContext->MoveTo(x, y, myView, Standard_True);
}

void APOViewerView::drawRubberBand( const int minX, const int minY, const int maxX, const int maxY )
{
    QRect aRect;

    // Set the rectangle correctly.
    (minX < maxX) ? (aRect.setX(minX)) : (aRect.setX(maxX));
    (minY < maxY) ? (aRect.setY(minY)) : (aRect.setY(maxY));

    aRect.setWidth(abs(maxX - minX));
    aRect.setHeight(abs(maxY - minY));

    if (!myRectBand)
    {
        myRectBand = new QRubberBand(QRubberBand::Rectangle, this);

        // setStyle is important, set to windows style will just draw
        // rectangle frame, otherwise will draw a solid rectangle.
        myRectBand->setStyle(QStyleFactory::create("windows"));
    }

    myRectBand->setGeometry(aRect);
    myRectBand->show();
}

void APOViewerView::panByMiddleButton( const QPoint& thePoint )
{
    Standard_Integer aCenterX = 0;
    Standard_Integer aCenterY = 0;

    QSize aSize = size();

    aCenterX = aSize.width() / 2;
    aCenterY = aSize.height() / 2;

    myView->Pan(aCenterX - thePoint.x(), thePoint.y() - aCenterY);
}

#if 0
bool APOViewerView::eventKeyPress(QKeyEvent *event)
{
	std::ofstream ofs("hoge.txt");
	switch (event->key()) {
	case Qt::Key_Left:
		ofs << "left key" << std::endl;
		break;
	case Qt::Key_Right:
		ofs << "right key" << std::endl;
		break;
	default:
		ofs << event->key() << std::endl;
		break;
	}
	ofs.close();
	return false;
}
#endif