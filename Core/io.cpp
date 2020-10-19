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
	QFileInfo cappedSurfaceFile(this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName() + "_capped.stl"));
	writer->SetFileName(cappedSurfaceFile.absoluteFilePath().toStdString().c_str());
	writer->SetInputData(cleaner->GetOutput());
	writer->Update();

	// boundary caps
	QFileInfoList boundaryCapFileList;
	for (int i = 0; i < m_boundaryCaps.size(); i++)
	{
		QFileInfo boundaryCapFile(this->addUniqueSuffix(m_domainFile.absolutePath() + "/" + m_domainFile.baseName() + "_" + m_boundaryCaps.at(i).name + ".stl"));
		boundaryCapFileList.append(boundaryCapFile);
		writer->SetFileName(boundaryCapFile.absoluteFilePath().toStdString().c_str());
		writer->SetInputData(m_boundaryCaps.at(i).polydata);
		writer->Update();
	}

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
	//std::cout << "IO::SetCenterlineKeyPoint input : " << idx << " " << m_centerlineKeyPoints.at(idx).first[0] << std::endl;
	//
	//std::cout << "IO::SetCenterlineKeyPoint input before replace " << pair.first << std::endl;

	//for (int i = 0; i < m_centerlineKeyPoints.size(); i++)
	//{
	//	std::cout << i << ": " << "(" << m_centerlineKeyPoints.at(i).first << ")" <<
	//		m_centerlineKeyPoints.at(i).first[0] << ", " <<
	//		m_centerlineKeyPoints.at(i).first[1] << ", " <<
	//		m_centerlineKeyPoints.at(i).first[2] << std::endl;
	//}
	
	m_centerlineKeyPoints.replace(idx, pair);
	 
	/*std::cout << "IO::SetCenterlineKeyPoint input after replace " << pair.first <<std::endl;

	for (int i = 0; i < m_centerlineKeyPoints.size(); i++)
	{
		std::cout << i << ": " << "("<< m_centerlineKeyPoints.at(i).first <<")"<<
			m_centerlineKeyPoints.at(i).first[0] << ", " << 
			m_centerlineKeyPoints.at(i).first[1] << ", " << 
			m_centerlineKeyPoints.at(i).first[2] <<std::endl;
	}*/

	emit centerlineKeyPointUpdated();
}

void IO::RemoveCenterlineKeyPoint(int index)
{
	m_centerlineKeyPoints.removeAt(index);
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