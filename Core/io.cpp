#include "io.h"

// vtk
#include "vtkXMLPolyDataReader.h"
#include "vtkSTLReader.h"
#include "vtkPolyDataReader.h"
#include "vtkXMLPolyDataWriter.h"
#include "vtkSTLWriter.h"
#include "vtkPolyDataWriter.h"
#include "observe_error.h"
#include "vtkThreshold.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkUnstructuredGrid.h"
#include "vtkKdTreePointLocator.h"
#include "vtkAppendPolyData.h"
#include "vtkCleanPolyData.h"

IO::IO(QObject* parent)
{

}

IO::~IO()
{

}

bool IO::ReadSurface()
{
	if (!(m_surfaceFile.isFile() && m_surfaceFile.exists()))
	{
		emit surfaceFileReadStatus(1);
		return 1;
	}

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_surfaceFile.suffix() == "vtp" || m_surfaceFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_surfaceFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit surfaceFileReadStatus(1);
			return 1;
		}
		else
		{
			m_original_surface->DeepCopy(reader->GetOutput());
			m_surface->DeepCopy(reader->GetOutput());
			emit surfaceFileReadStatus(0);
		}
	}
	else if (m_surfaceFile.suffix() == "stl" || m_surfaceFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_surfaceFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit surfaceFileReadStatus(1);
			return 1;
		}
		else
		{
			m_original_surface->DeepCopy(reader->GetOutput());
			m_surface->DeepCopy(reader->GetOutput());
			emit surfaceFileReadStatus(0);
		}
	}
	else if (m_surfaceFile.suffix() == "vtk" || m_surfaceFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
		reader->SetFileName(m_surfaceFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit surfaceFileReadStatus(1);
			return 1;
		}
		else
		{
			m_original_surface->DeepCopy(reader->GetOutput());
			m_surface->DeepCopy(reader->GetOutput());
			emit surfaceFileReadStatus(0);
		}
	}
	return 0;
}

bool IO::ReadCenterline()
{
	if (!(m_centerlineFile.isFile() && m_centerlineFile.exists()))
	{
		emit centerlineFileReadStatus(1);
		return 1;
	}

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_centerlineFile.suffix() == "vtp" || m_centerlineFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_centerlineFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit centerlineFileReadStatus(1);
			return 1;
		}
		else
		{
			m_original_centerline->DeepCopy(reader->GetOutput());
			m_centerline->DeepCopy(reader->GetOutput());
			emit centerlineFileReadStatus(0);
		}
	}
	else if (m_centerlineFile.suffix() == "stl" || m_centerlineFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_centerlineFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit centerlineFileReadStatus(1);
			return 1;
		}
		else
		{
			m_original_centerline->DeepCopy(reader->GetOutput());
			m_centerline->DeepCopy(reader->GetOutput());
			emit centerlineFileReadStatus(0);
		}
	}
	else if (m_centerlineFile.suffix() == "vtk" || m_centerlineFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
		reader->SetFileName(m_centerlineFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit centerlineFileReadStatus(1);
			return 1;
		}
		else
		{
			m_original_centerline->DeepCopy(reader->GetOutput());
			m_centerline->DeepCopy(reader->GetOutput());
			emit centerlineFileReadStatus(0);
		}
	}

	this->AutoLocateFirstBifurcationPoint();
	return 0;
}

bool IO::WriteSurface(QString path)
{
	QFileInfo m_saveSurfaceFile(path);

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_saveSurfaceFile.suffix() == "vtp" || m_saveSurfaceFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
		writer->SetFileName(m_saveSurfaceFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_surface);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit surfaceFileReadStatus(1);
			return 1;
		}
		else
		{
			emit surfaceFileReadStatus(0);
		}
	}
	else if (m_saveSurfaceFile.suffix() == "stl" || m_saveSurfaceFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
		writer->SetFileName(m_saveSurfaceFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_surface);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit surfaceFileWriteStatus(1);
			return 1;
		}
		else
		{
			emit surfaceFileWriteStatus(0);
		}
	}
	else if (m_saveSurfaceFile.suffix() == "vtk" || m_saveSurfaceFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
		writer->SetFileName(m_saveSurfaceFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_surface);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit surfaceFileWriteStatus(1);
			return 1;
		}
		else
		{
			emit surfaceFileWriteStatus(0);
		}
	}


	return 0;
}

bool IO::WriteCenterline(QString path)
{
	QFileInfo m_saveCenterlineFile(path);

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_saveCenterlineFile.suffix() == "vtp" || m_saveCenterlineFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
		writer->SetFileName(m_saveCenterlineFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_centerline);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit centerlineFileWriteStatus(1);
			return 1;
		}
		else
		{
			emit centerlineFileWriteStatus(0);
		}
	}
	else if (m_saveCenterlineFile.suffix() == "vtk" || m_saveCenterlineFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
		writer->SetFileName(m_saveCenterlineFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_centerline);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit centerlineFileWriteStatus(1);
			return 1;
		}
		else
		{
			emit centerlineFileWriteStatus(0);
		}
	}

	return 0;
}

vtkPolyData * IO::GetSurface()
{
	return m_surface;
}

void IO::SetSurface(vtkPolyData * polydata)
{
	m_surface->DeepCopy(polydata);
}

vtkPolyData * IO::GetCenterline()
{
	return m_centerline;
}

void IO::SetCenterline(vtkPolyData *polydata)
{
	m_centerline->DeepCopy(polydata);
}

vtkPolyData * IO::GetOriginalSurface()
{
	return m_original_surface;
}

vtkPolyData * IO::GetOriginalCenterline()
{
	return m_original_centerline;
}

void IO::SetCenterlineFirstBifurcationPoint(QVector<double> point)
{
	m_firstBifurcationPoint[0] = point[0];
	m_firstBifurcationPoint[1] = point[1];
	m_firstBifurcationPoint[2] = point[2];

	updateCenterlineFirstBifurcationPointId();
}

QVector<double> IO::GetCenterlineFirstBifurcationPoint()
{
	return m_firstBifurcationPoint;
}

void IO::updateCenterlineFirstBifurcationPointId()
{
	// update first bifurcation point id
	vtkSmartPointer<vtkKdTreePointLocator> kdTree = vtkSmartPointer<vtkKdTreePointLocator>::New();
	kdTree->SetDataSet(m_centerline);
	kdTree->Update();

	double point[3] = { m_firstBifurcationPoint[0], m_firstBifurcationPoint[1], m_firstBifurcationPoint[2] };
	int id = kdTree->FindClosestPoint(point);

	m_centerlineFirstBifurcationPointId = id;
}

void IO::AutoLocateFirstBifurcationPoint()
{
	vtkDataArray* groupids = m_centerline->GetCellData()->GetArray("GroupIds");

	if (m_centerline->GetNumberOfPoints() == 0 || groupids==nullptr)
		return;

	vtkSmartPointer<vtkThreshold> splitter = vtkSmartPointer<vtkThreshold>::New();
	splitter->SetInputData(m_centerline);
	splitter->ThresholdBetween(0, 0);
	splitter->SetInputArrayToProcess(0, 0, 0, vtkDataObject::FIELD_ASSOCIATION_CELLS, "GroupIds");
	splitter->Update();
	vtkSmartPointer<vtkUnstructuredGrid> mainBranch = splitter->GetOutput();

	double maxAbsc = 0;

	for (int i = 0; i < mainBranch->GetNumberOfPoints(); i++)
	{
		vtkDataArray* absc_array = mainBranch->GetPointData()->GetArray("Abscissas");
		if (absc_array == nullptr)
			return;
		double absc = absc_array->GetComponent(i, 0);
		if (absc > maxAbsc)
		{
			maxAbsc = absc;
			m_centerlineFirstBifurcationPointId = i;
		}
	}

	m_firstBifurcationPoint[0] = m_centerline->GetPoint(m_centerlineFirstBifurcationPointId)[0];
	m_firstBifurcationPoint[1] = m_centerline->GetPoint(m_centerlineFirstBifurcationPointId)[1];
	m_firstBifurcationPoint[2] = m_centerline->GetPoint(m_centerlineFirstBifurcationPointId)[2];
}

int IO::GetCenterlineFirstBifurcationPointId()
{
	return m_centerlineFirstBifurcationPointId;
}

void IO::AddBoundaryCap(BoundaryCap boundaryCap)
{
	m_boundaryCaps.append(boundaryCap);
}

void IO::RemoveBoundaryCap(int index)
{
	m_boundaryCaps.removeAt(index);
}

void IO::RemoveAllBoundaryCaps()
{
	this->m_boundaryCaps.clear();
}

void IO::SetBoundaryCap(int index, BoundaryCap bc)
{
	if (m_boundaryCaps.size() <= index)
		return;

	m_boundaryCaps.replace(index, bc);
}

void IO::SetWriteDomainPath(QString dir)
{
	m_domainFile.setFile(dir);
}

void IO::WriteDomain()
{
	// capped surface
	vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
	appendFilter->SetInputData(m_surface);

	for (int i = 0; i < m_boundaryCaps.size(); i++)
	{
		appendFilter->AddInputData(m_boundaryCaps.at(i).polydata);
	}
	appendFilter->Update();
	vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner->SetInputData(appendFilter->GetOutput());
	cleaner->Update();

	vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
	QString cappedSurfacePath = this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName()+"_capped.stl");

	writer->SetFileName(cappedSurfacePath.toStdString().c_str());
	writer->SetInputData(cleaner->GetOutput());
	writer->Update();

	// boundary caps
	QStringList boundaryCapsFilenameList;
	for (int i = 0; i < m_boundaryCaps.size(); i++)
	{
		QString boundaryCapPath = this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName() +"_" + m_boundaryCaps.at(i).name +".stl");
		boundaryCapsFilenameList.append(boundaryCapPath);
		writer->SetFileName(boundaryCapPath.toStdString().c_str());
		writer->SetInputData(m_boundaryCaps.at(i).polydata);
		writer->Update();
	}

	// surface wall
	writer->SetFileName(this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName() + "_wall.stl").toStdString().c_str());
	writer->SetInputData(m_surface);
	writer->Update();

	// centerline
	writer->SetFileName(this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName() + "_centerline.stl").toStdString().c_str());
	writer->SetInputData(m_centerline);
	writer->Update();

	// domain json
}

QString IO::addUniqueSuffix(const QString & fileName)
{
	// If the file doesn't exist return the same name.
	if (!QFile::exists(fileName)) {
		return fileName;
	}

	QFileInfo fileInfo(fileName);
	QString ret;

	// Split the file into 2 parts - dot+extension, and everything else. For
	// example, "path/file.tar.gz" becomes "path/file"+".tar.gz", while
	// "path/file" (note lack of extension) becomes "path/file"+"".
	QString secondPart = fileInfo.completeSuffix();
	QString firstPart;
	if (!secondPart.isEmpty()) {
		secondPart = "." + secondPart;
		firstPart = fileName.left(fileName.size() - secondPart.size());
	}
	else {
		firstPart = fileName;
	}

	// Try with an ever-increasing number suffix, until we've reached a file
	// that does not yet exist.
	for (int ii = 1; ; ii++) {
		// Construct the new file name by adding the unique number between the
		// first and second part.
		ret = QString("%1 (%2)%3").arg(firstPart).arg(ii).arg(secondPart);
		// If no file exists with the new name, return it.
		if (!QFile::exists(ret)) {
			return ret;
		}
	}
}

QList<BoundaryCap> IO::GetBoundaryCaps()
{
	return m_boundaryCaps;
}

void IO::SetSurfacePath(QString path)
{
	m_surfaceFile.setFile(path);
}

void IO::SetCenterlinePath(QString path)
{
	m_centerlineFile.setFile(path);
}