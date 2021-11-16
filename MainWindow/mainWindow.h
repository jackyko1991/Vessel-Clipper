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
#include <vtkBillboardTextActor3D.h>

namespace Ui {
	class MainWindow;
}

class QProgressBar;
class BranchOperation;
class Preferences;

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
	void slotDisplayWireframe(int);

	// voronoi
	void slotBrowseVoronoi();
	void slotComputeVoronoi();
	void slotSliderVoronoiOpacityChanged();
	void slotSpinBoxVoronoiOpacityChanged();
	void slotSaveVoronoi();

	// recon
	void slotReconSmoothValueSliderChanged();
	void slotReconSmoothValueSpinBoxChanged();
	void slotReconAddAll();
	void slotReconAddCurrent();
	void slotReconRemoveAll();
	void slotReconRemoveCurrent();
	void slotReconClip();
	void slotReconInterpolate();
	void slotResetRecon();
	void slotReconstruct();

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
	void slotComputeCenterline();
	void slotCurrentCenterlineKeyPoint();
	void slotSetCenterlineIdsArray(QString);
	void slotSetAbscissasArray(QString);
	void slotCenterlineConfigUpdate();

	// fiducial
	void slotAddFiducial();
	void slotFiducialTypeChanged();
	void slotRemoveFiducial();
	void slotCurrentFiducial();

	// stenosis and normal planes
	void slotSetStenosisPoint();
	void slotSetProximalNormalPoint();
	void slotSetDistalNormalPoint();

	// action
	void slotActionBranch();
	void slotActionPreferences();
	void slotCenterlineConfigure();

private slots:
	void readSurfaceFileComplete();
	void readCenterlineFileComplete();
	void readVoronoiFileComplete();

private:
	Ui::MainWindow *ui;
	void enableUI(bool);
	void renderSurface();
	void renderCenterline();
	void renderVoronoi();
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
	void updateCenterlinesInfoWidget();
	void ExtractCylindericInterpolationVoronoiDiagram(vtkPolyData* clippedCenterline, vtkPolyData* outputData, int idx, bool dir, float len);

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

	// vornoi diagram
	vtkSmartPointer<vtkPolyDataMapper> m_voronoiMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> m_vornoiActor = vtkSmartPointer<vtkActor>::New();

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

	// normal planes
	vtkSmartPointer<vtkPolyDataMapper> m_proximalNormalMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkPolyDataMapper> m_distalNormalMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> m_proximalNormalActor = vtkSmartPointer<vtkActor>::New();
	vtkSmartPointer<vtkActor> m_distalNormalActor = vtkSmartPointer<vtkActor>::New();

	// centerline ids
	QList<vtkBillboardTextActor3D*> m_centerlineIdsActors;

	// reconstructed surface
	vtkSmartPointer<vtkPolyDataMapper> m_reconSurfaceMapper = vtkSmartPointer<vtkPolyDataMapper>::New();
	vtkSmartPointer<vtkActor> m_reconSurfaceActor = vtkSmartPointer<vtkActor>::New();

	// utils
	BranchOperation* m_branchOperation;

	// settings
	Preferences* m_preferences;
};

#endif