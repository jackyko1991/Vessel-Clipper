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

// json
#include <nlohmann/json.hpp>

using json = nlohmann::json;

IO::IO(QObject* parent)
{

}

IO::~IO()
{

}

void IO::SetVoronoiPath(QString path)
{
	m_voronoiFile.setFile(path);
}

void IO::SetReconSurfacePath(QString path)
{
	m_reconstructedSurfaceFile.setFile(path);
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

bool IO::ReadVoronoi()
{
	if (!(m_voronoiFile.isFile() && m_voronoiFile.exists()))
	{
		emit voronoiFileReadStatus(1);
		return 1;
	}

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_voronoiFile.suffix() == "vtp" || m_voronoiFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_voronoiFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit voronoiFileReadStatus(1);
			return 1;
		}
		else
		{
			m_original_voronoiDiagram->DeepCopy(reader->GetOutput());
			m_voronoiDiagram->DeepCopy(reader->GetOutput());
			emit voronoiFileReadStatus(0);
		}
	}
	else if (m_voronoiFile.suffix() == "stl" || m_voronoiFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_voronoiFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit voronoiFileReadStatus(1);
			return 1;
		}
		else
		{
			m_original_voronoiDiagram->DeepCopy(reader->GetOutput());
			m_voronoiDiagram->DeepCopy(reader->GetOutput());
			emit voronoiFileReadStatus(0);
		}
	}
	else if (m_voronoiFile.suffix() == "vtk" || m_voronoiFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
		reader->SetFileName(m_voronoiFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit voronoiFileReadStatus(1);
			return 1;
		}
		else
		{
			m_original_voronoiDiagram->DeepCopy(reader->GetOutput());
			m_voronoiDiagram->DeepCopy(reader->GetOutput());
			emit voronoiFileReadStatus(0);
		}
	}
	return 0;
}

bool IO::ReadReconSurface()
{
	if (!(m_reconstructedSurfaceFile.isFile() && m_reconstructedSurfaceFile.exists()))
	{
		emit reconstructedSurfaceFileReadStatus(1);
		return 1;
	}

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_reconstructedSurfaceFile.suffix() == "vtp" || m_reconstructedSurfaceFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataReader> reader = vtkSmartPointer<vtkXMLPolyDataReader>::New();
		reader->SetFileName(m_reconstructedSurfaceFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit reconstructedSurfaceFileReadStatus(1);
			return 1;
		}
		else
		{
			m_reconSurface->DeepCopy(reader->GetOutput());
			emit reconstructedSurfaceFileReadStatus(0);
		}
	}
	else if (m_reconstructedSurfaceFile.suffix() == "stl" || m_reconstructedSurfaceFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLReader> reader = vtkSmartPointer<vtkSTLReader>::New();
		reader->SetFileName(m_reconstructedSurfaceFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit reconstructedSurfaceFileReadStatus(1);
			return 1;
		}
		else
		{
			m_reconSurface->DeepCopy(reader->GetOutput());
			emit reconstructedSurfaceFileReadStatus(0);
		}
	}
	else if (m_reconstructedSurfaceFile.suffix() == "vtk" || m_reconstructedSurfaceFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataReader> reader = vtkSmartPointer<vtkPolyDataReader>::New();
		reader->SetFileName(m_reconstructedSurfaceFile.absoluteFilePath().toStdString().c_str());
		reader->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		reader->Update();
		if (errorObserver->GetError())
		{
			emit reconstructedSurfaceFileReadStatus(1);
			return 1;
		}
		else
		{
			m_reconSurface->DeepCopy(reader->GetOutput());
			emit reconstructedSurfaceFileReadStatus(0);
		}
	}
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

bool IO::WriteVoronoi(QString path)
{
	QFileInfo m_saveVoronoiFile(path);

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_saveVoronoiFile.suffix() == "vtp" || m_saveVoronoiFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
		writer->SetFileName(m_saveVoronoiFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_voronoiDiagram);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit voronoiFileWriteStatus(1);
			return 1;
		}
		else
		{
			emit voronoiFileWriteStatus(0);
		}
	}

	else if (m_saveVoronoiFile.suffix() == "vtk" || m_saveVoronoiFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
		writer->SetFileName(m_saveVoronoiFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_voronoiDiagram);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit voronoiFileWriteStatus(1);
			return 1;
		}
		else
		{
			emit voronoiFileWriteStatus(0);
		}
	}

	return 0;
}

bool IO::WriteReconSurface(QString path)
{
	QFileInfo m_saveReconSurfaceFile(path);

	vtkSmartPointer<ErrorObserver> errorObserver = vtkSmartPointer<ErrorObserver>::New();

	if (m_saveReconSurfaceFile.suffix() == "vtp" || m_saveReconSurfaceFile.suffix() == "VTP")
	{
		vtkSmartPointer<vtkXMLPolyDataWriter> writer = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
		writer->SetFileName(m_saveReconSurfaceFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_reconSurface);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit reconstructedSurfaceFileWriteStatus(1);
			return 1;
		}
		else
		{
			emit reconstructedSurfaceFileWriteStatus(0);
		}
	}
	else if (m_saveReconSurfaceFile.suffix() == "stl" || m_saveReconSurfaceFile.suffix() == "STL")
	{
		vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();
		writer->SetFileName(m_saveReconSurfaceFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_reconSurface);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit reconstructedSurfaceFileWriteStatus(1);
			return 1;
		}
		else
		{
			emit reconstructedSurfaceFileWriteStatus(0);
		}
	}
	else if (m_saveReconSurfaceFile.suffix() == "vtk" || m_saveReconSurfaceFile.suffix() == "VTK")
	{
		vtkSmartPointer<vtkPolyDataWriter> writer = vtkSmartPointer<vtkPolyDataWriter>::New();
		writer->SetFileName(m_saveReconSurfaceFile.absoluteFilePath().toStdString().c_str());
		writer->AddObserver(vtkCommand::ErrorEvent, errorObserver);
		writer->SetInputData(m_reconSurface);
		writer->Update();
		if (errorObserver->GetError())
		{
			emit reconstructedSurfaceFileWriteStatus(1);
			return 1;
		}
		else
		{
			emit reconstructedSurfaceFileWriteStatus(0);
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

vtkPolyData * IO::GetVoronoiDiagram()
{
	return m_voronoiDiagram;
}

void IO::SetVornoiDiagram(vtkPolyData* polydata)
{
	m_voronoiDiagram->DeepCopy(polydata);
}

vtkPolyData * IO::GetReconstructedSurface()
{
	return m_reconSurface;
}

void IO::SetReconstructedSurface(vtkPolyData * polydata)
{
	m_reconSurface->DeepCopy(polydata);
}

//vtkPolyData * IO::GetInterpolatedCenterline()
//{
//	return m_interpolatedCenterline;
//}
//
//void IO::SetInterpolatedCenterline(vtkPolyData *polydata)
//{
//	m_interpolatedCenterline->DeepCopy(polydata);
//}

vtkPolyData * IO::GetOriginalSurface()
{
	return m_original_surface;
}

vtkPolyData * IO::GetOriginalCenterline()
{
	return m_original_centerline;
}

vtkPolyData * IO::GetOriginalVoronoiDiagram()
{
	return m_original_voronoiDiagram;
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
	if (m_centerline->GetNumberOfPoints() == 0)
		return;

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

	vtkSmartPointer<vtkSTLWriter> writer = vtkSmartPointer<vtkSTLWriter>::New();

	// boundary caps
	QFileInfoList boundaryCapFileList;
	for (int i = 0; i < m_boundaryCaps.size(); i++)
	{
		QFileInfo boundaryCapFile(this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName() + "_" + m_boundaryCaps.at(i).name + ".stl"));
		boundaryCapFileList.append(boundaryCapFile);
		writer->SetFileName(boundaryCapFile.absoluteFilePath().toStdString().c_str());

		//vtkSmartPointer<vtkTriangleFilter> triangleFilter = vtkSmartPointer<vtkTriangleFilter>::New();
		//triangleFilter->SetInputData(m_boundaryCaps.at(i).polydata);
		//triangleFilter->Update();

		//// make triangles into same size
		//vtkSmartPointer<vtkvmtkPolyDataSurfaceRemeshing> surfaceRemeshing = vtkSmartPointer<vtkvmtkPolyDataSurfaceRemeshing>::New();
		//surfaceRemeshing->SetInputData(triangleFilter->GetOutput());
		//surfaceRemeshing->SetMinArea(1e-3);
		//surfaceRemeshing->SetMaxArea(1e-2);
		//surfaceRemeshing->SetNumberOfIterations(10);
		//surfaceRemeshing->Update();

		//writer->SetInputData(surfaceRemeshing->GetOutput());
		writer->SetInputData(m_boundaryCaps.at(i).polydata);
		writer->Update();

		//appendFilter->AddInputData(triangleFilter->GetOutput());
		appendFilter->AddInputData(m_boundaryCaps.at(i).polydata);
	}

	appendFilter->Update();
	vtkSmartPointer<vtkCleanPolyData> cleaner = vtkSmartPointer<vtkCleanPolyData>::New();
	cleaner->SetInputData(appendFilter->GetOutput());
	cleaner->Update();

	QFileInfo cappedSurfaceFile(this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName() + "_capped.stl"));
	writer->SetFileName(cappedSurfaceFile.absoluteFilePath().toStdString().c_str());
	writer->SetInputData(cleaner->GetOutput());
	writer->Update();

	// vessel surface
	QFileInfo vesselFile(this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName() + "_vessel.stl"));
	writer->SetFileName(vesselFile.absoluteFilePath().toStdString().c_str());
	writer->SetInputData(m_surface);
	writer->Update();

	// centerline
	vtkSmartPointer<vtkXMLPolyDataWriter> vtpWriter = vtkSmartPointer<vtkXMLPolyDataWriter>::New();
	QFileInfo centerlineFile(this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName() + "_centerline.vtp"));
	vtpWriter->SetFileName(centerlineFile.absoluteFilePath().toStdString().c_str());
	vtpWriter->SetInputData(m_centerline);
	vtpWriter->Update();

	// domain json
	json j; 
	j["domain"]["filename"] = cappedSurfaceFile.fileName().toStdString();
	j["vessel"]["filename"] = vesselFile.fileName().toStdString();
	j["centerline"]["filename"] = centerlineFile.fileName().toStdString();
	j["bifurcation_point"]["coordinate"] = { this->m_firstBifurcationPoint[0], this->m_firstBifurcationPoint[1], this->m_firstBifurcationPoint[2] };
	for (int i = 0; i < boundaryCapFileList.size(); i++)
	{
		j["boundary_" + std::to_string(i)]["filename"] = boundaryCapFileList.at(i).fileName().toStdString();
		switch (m_boundaryCaps.at(i).type)
		{
			case BoundaryCapType::none:
				j["boundary_" + std::to_string(i)]["type"] = "none";
				break;
			case BoundaryCapType::inlet:
				j["boundary_" + std::to_string(i)]["type"] = "inlet";
				break;
			case BoundaryCapType::outlet:
				j["boundary_" + std::to_string(i)]["type"] = "outlet";
				break;
		}
		
		j["boundary_" + std::to_string(i)]["coordinate"] = {m_boundaryCaps.at(i).center[0],m_boundaryCaps.at(i).center[1],m_boundaryCaps.at(i).center[2] };
		j["boundary_" + std::to_string(i)]["tangent"] = { m_boundaryCaps.at(i).tangent[0],m_boundaryCaps.at(i).tangent[1],m_boundaryCaps.at(i).tangent[2] };
		j["boundary_" + std::to_string(i)]["radius"] = m_boundaryCaps.at(i).radius;
	}
	for (int i = 0; i < m_fiducial.size(); i++)
	{
		j["fiducial_" + std::to_string(i)]["coordinate"] = { m_fiducial.at(i).first[0], m_fiducial.at(i).first[1], m_fiducial.at(i).first[2] };
		switch (m_fiducial.at(i).second)
		{
		case FiducialType::Stenosis:
			j["fiducial_" + std::to_string(i)]["type"] = "Stenosis";
			break;
		case FiducialType::Bifurcation:
			j["fiducial_" + std::to_string(i)]["type"] = "Bifurcation";
			break;
		case FiducialType::DoS_Ref:
			j["fiducial_" + std::to_string(i)]["type"] = "DoS_Ref";
			break;
		case FiducialType::Others:
			j["fiducial_" + std::to_string(i)]["type"] = "Others";
			break;
		}
	}

	
	std::ofstream o(m_domainFile.absoluteFilePath().toStdString());
	o << std::setw(4) << j << std::endl;
}

void IO::AddCenterlineKeyPoint(QVector<double> point, bool type)
{
	// type = 0 for source, type = 1 for target
	QPair<QVector<double>, bool> keyPoint;
	keyPoint.first.append(point[0]);
	keyPoint.first.append(point[1]);
	keyPoint.first.append(point[2]);
	keyPoint.second = type;
	
	m_centerlineKeyPoints.append(keyPoint);

	emit centerlineKeyPointUpdated();
}

QList<QPair<QVector<double>, bool>> IO::GetCenterlineKeyPoints()
{
	return m_centerlineKeyPoints;
}

void IO::SetCenterlineKeyPoint(int idx , QPair<QVector<double>, bool> pair)
{
	m_centerlineKeyPoints.replace(idx, pair);

	emit centerlineKeyPointUpdated();
}

void IO::RemoveCenterlineKeyPoint(int index)
{
	m_centerlineKeyPoints.removeAt(index);
}

void IO::AddFiducial(QVector<double> point, FiducialType type)
{
	QPair<QVector<double>, FiducialType> fiducial;
	fiducial.first.append(point[0]);
	fiducial.first.append(point[1]);
	fiducial.first.append(point[2]);
	fiducial.second = type;

	m_fiducial.append(fiducial);
}

void IO::RemoveFiducial(int index)
{
	m_fiducial.removeAt(index);
}

void IO::SetFiducial(int idx, QPair<QVector<double>, FiducialType> pair)
{
	m_fiducial.replace(idx, pair);
}

QList<QPair<QVector<double>, FiducialType>> IO::GetFiducial()
{
	return m_fiducial;
}

void IO::SetStenosisPoint(double x, double y, double z)
{
	m_stenosisPoint[0] = x;
	m_stenosisPoint[1] = y;
	m_stenosisPoint[2] = z;

	std::cout << "m_io set stenosis point: " << x << "," << y << "," << z << std::endl;
}

void IO::SetProximalNormalPoint(double x, double y, double z)
{
	m_proximalNormalPoint[0] = x;
	m_proximalNormalPoint[1] = y;
	m_proximalNormalPoint[2] = z;
}

void IO::SetDistalNormalPoint(double x, double y, double z)
{
	m_distalNormalPoint[0] = x;
	m_distalNormalPoint[1] = y;
	m_distalNormalPoint[2] = z;
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
		ret = QString("%1_%2%3").arg(firstPart).arg(ii).arg(secondPart);
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