#include "io.h"

// vtk
#include "vtkXMLPolyDataReader.h"
#include "vtkSTLReader.h"
#include "vtkPolyDataReader.h"
#include "observe_error.h"
#include "vtkThreshold.h"
#include "vtkPointData.h"
#include "vtkCellData.h"
#include "vtkDataArray.h"
#include "vtkUnstructuredGrid.h"

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

vtkPolyData * IO::GetSurface()
{
	return m_surface;
}

vtkPolyData * IO::GetCenterline()
{
	return m_centerline;
}

void IO::SetCenterlineFirstBifurcationPointId(int id)
{
	m_centerlineFirstBifurcationPointId = id;
}

int IO::GetCenterlineFirstBifurcationPointId()
{
	return m_centerlineFirstBifurcationPointId;
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

	std::cout << "Auto locate first bifucation point ok" << std::endl;
}


void IO::SetSurfacePath(QString path)
{
	m_surfaceFile.setFile(path);
}

void IO::SetCenterlinePath(QString path)
{
	m_centerlineFile.setFile(path);
}