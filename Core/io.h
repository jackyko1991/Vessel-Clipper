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

class IO : public QObject
{
	Q_OBJECT

public:
	explicit IO(QObject* parent = 0);
	~IO();
	void SetSurfacePath(QString path);
	void SetCenterlinePath(QString path);
	void SetWriteSurfacePath(QString path);
	void SetWriteCenterlinePath(QString path);
	bool ReadSurface();
	bool ReadCenterline();
	bool WriteSurface(QString);
	bool WriteCenterline(QString);
	vtkPolyData* GetSurface();
	void SetSurface(vtkPolyData*);
	vtkPolyData* GetCenterline();
	void SetCenterline(vtkPolyData*);
	vtkPolyData* GetOriginalSurface();
	vtkPolyData* GetOriginalCenterline();
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
	void AddCenterlineKeyPoint(double*, bool);
	QList<QPair<double*, bool>> GetCenterlineKeyPoints();

	static QString addUniqueSuffix(const QString &fileName);

signals:
	// 0 for success, 1 for fail
	void surfaceFileReadStatus(bool);
	void centerlineFileReadStatus(bool);
	void surfaceFileWriteStatus(bool);
	void centerlineFileWriteStatus(bool);

private:
	QFileInfo m_surfaceFile;
	QFileInfo m_centerlineFile;
	QFileInfo m_domainFile;
	void updateCenterlineFirstBifurcationPointId();
	QVector<double> m_firstBifurcationPoint = QVector<double>(3);
	QList<BoundaryCap> m_boundaryCaps;

	vtkSmartPointer<vtkPolyData> m_original_surface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_original_centerline = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_surface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_centerline = vtkSmartPointer<vtkPolyData>::New();

	int m_centerlineFirstBifurcationPointId = 0;
	QList<QPair<double*, bool>> m_centerlineKeyPoints;
};

#endif // ! IO_H
