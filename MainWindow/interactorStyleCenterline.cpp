#include "interactorStyleCenterline.h"
#include <vtkObjectFactory.h>
#include "vtkRenderWindowInteractor.h"

void MouseInteractorStyleCenterline::OnKeyPress()
{
	vtkRenderWindowInteractor* rwi = this->Interactor;
	std::string key = rwi->GetKeySym();

	if (key == "space")
	{
		std::cout << "space pressed" << std::endl;
		std::cout << m_pickingPoint[0] << " " << m_pickingPoint[1] << " " << m_pickingPoint[2] << std::endl;
	}
}

void MouseInteractorStyleCenterline::SetPickingPoint(double point[3])
{
	m_pickingPoint = point;
}

vtkStandardNewMacro(MouseInteractorStyleCenterline);