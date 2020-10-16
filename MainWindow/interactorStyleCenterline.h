#ifndef INTERACTOR_STYLE_CENTERLINE_H
#define INTERACTOR_STYLE_CENTERLINE_H

#include "io.h"

#include "vtkSmartPointer.h"
#include "vtkInteractorStyleTrackballCamera.h"

class MouseInteractorStyleCenterline : public vtkInteractorStyleTrackballCamera
{
public:
	static MouseInteractorStyleCenterline* New();
	vtkTypeMacro(MouseInteractorStyleCenterline, vtkInteractorStyleTrackballCamera);

	virtual void OnKeyPress();

	void SetPickingPoint(int);
	void SetDataIo(IO*);

private:
	int m_index = -1;
	IO* m_io;
};

#endif 