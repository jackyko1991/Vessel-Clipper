#include "interactorStyleCenterline.h"
#include <vtkObjectFactory.h>
#include "vtkRenderWindowInteractor.h"
#include "vtkAppendPolyData.h"
#include "vtkPolyData.h"
#include <vtkPointPicker.h>
#include "vtkRenderWindow.h"
#include "vtkRendererCollection.h"
#include <memory>

void MouseInteractorStyleCenterline::OnKeyPress()
{
	vtkRenderWindowInteractor* rwi = this->Interactor;
	std::string key = rwi->GetKeySym();

	if (key == "g" || key=="G")
	{
		// try to get surface file
		if (m_io->GetSurface()->GetNumberOfPoints() == 0)
			return;

		vtkSmartPointer<vtkPolyData> surface = vtkSmartPointer<vtkPolyData>::New();
		surface->DeepCopy(m_io->GetSurface());

		// append boundary cap files
		if (m_io->GetBoundaryCaps().size() > 0)
		{
			vtkSmartPointer<vtkAppendPolyData> appendFilter = vtkSmartPointer<vtkAppendPolyData>::New();
			appendFilter->SetInputData(surface);

			for (int i = 0; i < m_io->GetBoundaryCaps().size(); i++)
			{
				appendFilter->AddInputData(m_io->GetBoundaryCaps().at(i).polydata);
			}
			appendFilter->Update();

			surface->DeepCopy(appendFilter->GetOutput());
		}

		// pick point from render window
		this->Interactor->GetPicker()->Pick(this->Interactor->GetEventPosition()[0],
			this->Interactor->GetEventPosition()[1],
			0,  // always zero.
			this->Interactor->GetRenderWindow()->GetRenderers()->GetFirstRenderer());

		double picked[3];
		this->Interactor->GetPicker()->GetPickPosition(picked);

		if (picked[0] == 0 && picked[1] == 0 && picked[2] == 0)
			return;

		// check if centerline keypoints has things
		if (m_io->GetCenterlineKeyPoints().size() == 0)
			return;

		QPair<QVector<double>, bool> keyPoint;
		keyPoint.first.append(picked[0]);
		keyPoint.first.append(picked[1]);
		keyPoint.first.append(picked[2]);
		keyPoint.second = m_io->GetCenterlineKeyPoints().at(m_io->GetCenterlineKeyPoints().size()-1).second;

		m_io->SetCenterlineKeyPoint(m_io->GetCenterlineKeyPoints().size() - 1, keyPoint);
	}
	else if (key == "Tab")
	{
		QPair<QVector<double>, bool> keyPoint;
		keyPoint.first.append(m_io->GetCenterlineKeyPoints().at(m_io->GetCenterlineKeyPoints().size() - 1).first[0]);
		keyPoint.first.append(m_io->GetCenterlineKeyPoints().at(m_io->GetCenterlineKeyPoints().size() - 1).first[1]);
		keyPoint.first.append(m_io->GetCenterlineKeyPoints().at(m_io->GetCenterlineKeyPoints().size() - 1).first[2]);
		keyPoint.second = !m_io->GetCenterlineKeyPoints().at(m_io->GetCenterlineKeyPoints().size() - 1).second;

		m_io->SetCenterlineKeyPoint(m_io->GetCenterlineKeyPoints().size() - 1, keyPoint);
	}
	else if (key == "n" || key == "N")
	{
		if (m_io->GetSurface()->GetNumberOfPoints() == 0)
			return;

		QPair<QVector<double>, bool> keyPoint;
		keyPoint.first.append(m_io->GetSurface()->GetPoint(0)[0]);
		keyPoint.first.append(m_io->GetSurface()->GetPoint(0)[1]);
		keyPoint.first.append(m_io->GetSurface()->GetPoint(0)[2]);

		if (m_io->GetCenterlineKeyPoints().size() > 0)
			keyPoint.second = m_io->GetCenterlineKeyPoints().at(m_io->GetCenterlineKeyPoints().size() - 1).second;
		else
			keyPoint.second = 0;

		m_io->AddCenterlineKeyPoint(keyPoint.first, keyPoint.second);
	}
}

void MouseInteractorStyleCenterline::SetPickingPoint(int index)
{
	m_index = -1;
}

void MouseInteractorStyleCenterline::SetDataIo(IO *io)
{
	m_io = io;
}

vtkStandardNewMacro(MouseInteractorStyleCenterline);