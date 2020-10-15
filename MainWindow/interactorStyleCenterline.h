#ifndef INTERACTOR_STYLE_CENTERLINE_H
#define INTERACTOR_STYLE_CENTERLINE_H

#include "vtkSmartPointer.h"
#include "vtkInteractorStyleTrackballCamera.h"

class MouseInteractorStyleCenterline : public vtkInteractorStyleTrackballCamera
{
public:
	static MouseInteractorStyleCenterline* New();
	vtkTypeMacro(MouseInteractorStyleCenterline, vtkInteractorStyleTrackballCamera);

	virtual void OnKeyPress();

	void SetPickingPoint(double point[3]);
private:
	double* m_pickingPoint;
};

#endif 