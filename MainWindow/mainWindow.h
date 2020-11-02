#ifndef MainWindow_H
#define MainWindow_H

#include "ui_mainWindow.h"
#include "io.h"
#include "boundaryCaps.h"

// qt
#include <QMainWindow>
#include <QFutureWatcher>

// vtk
#include <vtkSmartPointer.h>
#include <vtkRenderer.h>
#include <vtkPolyDataMapper.h>
#include <vtkActor.h>
#include "vtkSphereSource.h"
#include <vtkCubeSource.h>
#include <vtkCylinderSource.h>
#include <vtkOutlineFilter.h>

namespace Ui {
	class MainWindow;
}

class QProgressBar;

class MainWindow : public QMainWindow
{
	Q_OBJECT
public:

	explicit MainWindow(QMainWindow *parent = nullptr);
	~MainWindow();

public slots:
	virtual void slotExit();
	void slotBrowseSurface();
	void slotBrowseCenterline();
	void slotSliderOpacityChanged();
	void slotSpinBoxOpacityChanged();
	void slotCurrentPickingPoint();
	void slotSetFirstBifurcation();
	void slotAutoLocateFirstBifurcation();
	void slotBoundChanged();
	void slotSetClipper();
	void slotSliderSizeChanged();
	void slotSpinBoxSizeChanged();
	void slotSliderThicknessChanged();
	void slotSpinBoxThicknessChanged();
	void slotSliderRotateChanged();
	void slotSpinBoxRotateChanged();
	void slotClipProximal();
	void slotClipDistal();
	void slotResetSurface();
	void slotResetCenterline();
	void slotSaveSurface();
	void slotSaveCenterline();

	// domain
	void slotSurfaceCapping();
	void slotSaveDomain();
	void slotRemoveCap();
	void slotRemoveAllCaps();
	void slotBoundaryCapTypeChange(int);
	void slotBoundaryCapTableItemChanged(QTableWidgetItem*);
	void slotCurrentBoundaryCap();

	// centerline
	void slotAddCenterlineKeyPoint();
	void slotCenterlineKeyPointTypeChanged(int);
	void slotCenterlineKeyPointUpdated();
	void slotRemoveCenterlineKeyPoint();
	void slotRemoveAllCenterlineKeyPoint();
	void slotComputCenterline();
	void slotCurrentCenterlineKeyPoint();

	// fiducial
	void slotAddFiducial();
	void slotFiducialTypeChanged();
	void slotRemoveFiducial();
	void slotCurrentFiducial();

private slots:
	void readSurfaceFileComplete();
	void readCenterlineFileComplete();

private:
	Ui::MainWindow *ui;
	void enableUI(bool);
	void renderSurface();
	void renderCenterline();
	void renderFirstBifurcationPoint();
	void renderCenterlineKeyPoints();
	void updateCenterlineKeyPointsTable();
	void updateCenterlineDataTable();
	void createFirstBifurationPoint();
	void createCurrentPickingPoint();
	void createOutlineBoundingBox();
	void createClipper();
	void clip(int);
	void renderBoundaryCaps();
	void updateBoundaryCapsTable();
	void renderBoundaryCapsDirection();
	void renderOutlineBoundingBox();
	void updateFiducialTable();
	void renderFiducial();

	IO* m_io;

	// add references to Label and ProgressBar
	QLabel *m_statusLabel;
	QProgressBar *m_statusProgressBar;
	QFutureWatcher<bool>* m_ioWatcher;
	QFutureWatcher<void>* m_executeWatcher;
	QMutex* m_mutex;

	vtkSmartPointer<vtkRenderer> m_renderer = vtkSmartPointer<vtkRenderer>::New();
	vtkSmartPointer<vtkPolyDataMapper> m_surfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> m_centerlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> m_surfaceActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> m_centerlineActor = vtkSmartPointer<vtkActor>::New();

	// first bifurcation point
	vtkSmartPointer<vtkPolyDataMapper> m_firstBifurcationMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> m_firstBifurcationActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkSphereSource> m_firstBifurcationSphereSource = vtkSmartPointer<vtkSphereSource>::New();

	// current picking point
	vtkSmartPointer<vtkPolyDataMapper> m_currentPickingMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> m_currentPickingActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkSphereSource> m_currentPickingSphereSource = vtkSmartPointer<vtkSphereSource>::New();

	// clipping box
	vtkSmartPointer<vtkPolyDataMapper> m_clipperMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> m_clipperActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkPolyData> m_clipper = vtkSmartPointer<vtkPolyData>::New();

	// clip transform
	int m_clppingPointId = 0;
	vtkSmartPointer<vtkTransform> m_clipTransform = vtkSmartPointer<vtkTransform>::New();

	// boundary caps
	QList<vtkActor*> m_boundaryCapActors;
	QList<vtkActor*> m_boundaryCapsDirectionActor;
	vtkSmartPointer<vtkOutlineFilter> m_outlinerFilter = vtkSmartPointer<vtkOutlineFilter>::New();
	vtkSmartPointer<vtkPolyDataMapper> m_outlineMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> m_outlineActor = vtkSmartPointer<vtkActor>::New();

	// centerline keypoints
	QList<vtkActor*> m_centerlineKeyPointActors;

	// fiducials
	QList<vtkActor*> m_fiducialActors;
};

#endif