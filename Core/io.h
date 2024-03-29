#ifndef IO_H
#define IO_H

// qt
#include <QObject>
#include <QFileInfo>
#include <QVector>
#include <QPair>

// vtk
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"

#include "boundaryCaps.h"

enum FiducialType {
	Stenosis, 
	Bifurcation, 
	DoS_Ref, 
	Proximal_Normal,
	Distal_Normal,
	Others};

class IO : public QObject
{
	Q_OBJECT

public:
	explicit IO(QObject* parent = 0);
	~IO();
	void SetSurfacePath(QString path);
	void SetCenterlinePath(QString path);
	void SetVoronoiPath(QString path);
	void SetReconCenterlinePath(QString path);
	void SetReconSurfacePath(QString path);
	//void SetWriteSurfacePath(QString path);
	//void SetWriteCenterlinePath(QString path);
	//void SetWriteVoronoiPath(QString path);
	//void SetWriteReconSurfacePath(QString path);
	bool ReadSurface();
	bool ReadCenterline();
	bool ReadVoronoi();
	bool ReadReconCenterline();
	bool ReadReconSurface();
	bool WriteSurface(QString);
	bool WriteCenterline(QString);
	bool WriteVoronoi(QString);
	bool WriteReconCenterline(QString);
	bool WriteReconSurface(QString);
	vtkPolyData* GetSurface();
	void SetSurface(vtkPolyData*);
	vtkPolyData* GetCenterline();
	void SetCenterline(vtkPolyData*);
	vtkPolyData* GetVoronoiDiagram();
	void SetVornoiDiagram(vtkPolyData*);
	vtkPolyData* GetReconstructedSurface();
	void SetReconstructedSurface(vtkPolyData*);
	vtkPolyData* GetReconstructedCenterline();
	void SetReconstructedCenterline(vtkPolyData*);
	vtkPolyData* GetOriginalSurface();
	vtkPolyData* GetOriginalCenterline();
	vtkPolyData* GetOriginalVoronoiDiagram();
	void SetCenterlineFirstBifurcationPoint(QVector<double>);
	QVector<double> GetCenterlineFirstBifurcationPoint();
	void AutoLocateFirstBifurcationPoint();
	int GetCenterlineFirstBifurcationPointId();
	void AddBoundaryCap(BoundaryCap);
	void RemoveBoundaryCap(int);
	void RemoveAllBoundaryCaps();
	QList<BoundaryCap> GetBoundaryCaps();
	void SetBoundaryCap(int, BoundaryCap);
	void SetWriteDomainPath(QString dir);
	void WriteDomain();
	void AddCenterlineKeyPoint(QVector<double>, bool);
	QList<QPair<QVector<double>, bool>> GetCenterlineKeyPoints();
	void SetCenterlineKeyPoint(int, QPair<QVector<double>, bool>);
	void RemoveCenterlineKeyPoint(int);
	void AddFiducial(QVector<double>, FiducialType);
	void RemoveFiducial(int);
	void SetFiducial(int, QPair<QVector<double>, FiducialType>);
	QList<QPair<QVector<double>, FiducialType>> GetFiducial();
	void SetStenosisPoint(double x, double y, double z);
	void SetProximalNormalPoint(double x, double y, double z);
	void SetDistalNormalPoint(double x, double y, double z);

	static QString addUniqueSuffix(const QString &fileName);

signals:
	// 0 for success, 1 for fail
	void surfaceFileReadStatus(bool);
	void centerlineFileReadStatus(bool);
	void voronoiFileReadStatus(bool);
	void reconstructedCenterlineFileReadStatus(bool);
	void reconstructedSurfaceFileReadStatus(bool);
	void surfaceFileWriteStatus(bool);
	void centerlineFileWriteStatus(bool);
	void voronoiFileWriteStatus(bool);
	void reconstructedCenterlineFileWriteStatus(bool);
	void reconstructedSurfaceFileWriteStatus(bool);
	void centerlineKeyPointUpdated();

	// data updated
	void signalSurfaceUpdated();
	void signalCenterlineUpdated();
	void signalReconCenterlineUpdated();
	void signalReconSurfaceUpdated();
	void signalVoronoiUpdated();

private:
	QFileInfo m_surfaceFile;
	QFileInfo m_centerlineFile;
	QFileInfo m_domainFile;
	QFileInfo m_voronoiFile;
	QFileInfo m_reconstructedCenterlineFile;
	QFileInfo m_reconstructedSurfaceFile;
	void updateCenterlineFirstBifurcationPointId();
	QVector<double> m_firstBifurcationPoint = QVector<double>(3);
	int m_centerlineFirstBifurcationPointId = 0;

	QVector<double> m_stenosisPoint = QVector<double>(3);
	QVector<double> m_proximalNormalPoint = QVector<double>(3);
	QVector<double> m_distalNormalPoint = QVector<double>(3);

	QList<BoundaryCap> m_boundaryCaps;

	vtkSmartPointer<vtkPolyData> m_original_surface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_original_centerline = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_original_voronoiDiagram = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_surface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_centerline = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_reconSurface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_reconCenterline = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_voronoiDiagram = vtkSmartPointer<vtkPolyData>::New();

	QList<QPair<QVector<double>, bool>> m_centerlineKeyPoints;
	QList<QPair<QVector<double>, FiducialType>> m_fiducial;

};

#endif // ! IO_H
