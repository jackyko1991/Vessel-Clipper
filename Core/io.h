#ifndef IO_H
#define IO_H

// qt
#include <QObject>
#include <QFileInfo>

// vtk
#include "vtkSmartPointer.h"
#include "vtkPolyData.h"

class IO : public QObject
{
	Q_OBJECT

public:
	explicit IO(QObject* parent = 0);
	~IO();
	void SetSurfacePath(QString path);
	void SetCenterlinePath(QString path);
	bool ReadSurface();
	bool ReadCenterline();
	vtkPolyData* GetSurface();
	void SetSurface(vtkPolyData*);
	vtkPolyData* GetCenterline();
	void SetCenterline(vtkPolyData*);
	vtkPolyData* GetOriginalSurface();
	vtkPolyData* GetOriginalCenterline();
	void SetCenterlineFirstBifurcationPointId(int);
	int GetCenterlineFirstBifurcationPointId();
	void AutoLocateFirstBifurcationPoint();

signals:
	// 0 for success, 1 for fail
	void surfaceFileReadStatus(bool);
	void centerlineFileReadStatus(bool);

private:
	QFileInfo m_surfaceFile;
	QFileInfo m_centerlineFile;

	vtkSmartPointer<vtkPolyData> m_original_surface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_original_centerline = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_surface = vtkSmartPointer<vtkPolyData>::New();
	vtkSmartPointer<vtkPolyData> m_centerline = vtkSmartPointer<vtkPolyData>::New();
	
	int m_centerlineFirstBifurcationPointId = 0;
};

#endif // ! IO_H
