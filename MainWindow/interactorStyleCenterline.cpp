#include "interactorStyleCenterline.h"
#include <vtkObjectFactory.h>
#include "vtkRenderWindowInteractor.h"
#include "vtkAppendPolyData.h"
#include "vtkPolyData.h"
#include <vtkPointPicker.h>
#include "vtkRenderWindow.h"
#include "vtkRendererCollection.h"

void MouseInteractorStyleCenterline::OnKeyPress()
{
	vtkRenderWindowInteractor* rwi = this->Interactor;
	std::string key = rwi->GetKeySym();

	if (key == "g")
	{
		std::cout << "key pressed" << std::endl;

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

		std::cout << "picked point: " << picked[0] << ", " << picked[1] << ", "<< picked[2] << std::endl;

		double picked2[3];
		picked2[0] = picked[0];
		picked2[1] = picked[1];
		picked2[2] = picked[2];

		QPair<double*, bool> keyPoint;
		keyPoint.first = picked2;
		keyPoint.second = m_io->GetCenterlineKeyPoints().at(m_io->GetCenterlineKeyPoints().size()-1).second;

		m_io->SetCenterlineKeyPoint(m_io->GetCenterlineKeyPoints().size() - 1, keyPoint);
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