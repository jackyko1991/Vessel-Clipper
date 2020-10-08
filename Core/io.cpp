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

	std::cout << "before update id: " <<
		m_firstBifurcationPoint[0] << ", " <<
		m_firstBifurcationPoint[1] << ", " <<
		m_firstBifurcationPoint[2] << ", " <<
		std::endl;
	m_centerlineFirstBifurcationPointId = id;

	std::cout <<"after update id: "<< 
		m_firstBifurcationPoint[0] << ", " <<
		m_firstBifurcationPoint[1] << ", " <<
		m_firstBifurcationPoint[2] << ", " <<
		std::endl;
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


void IO::SetSurfacePath(QString path)
{
	m_surfaceFile.setFile(path);
}

void IO::SetCenterlinePath(QString path)
{
	m_centerlineFile.setFile(path);
}