#ifndef BOUNDARYCAPS_H
#define BOUNDARYCAPS_H

#include "vtkPolyData.h"
#include "vtkSmartPointer.h"

enum BoundaryCapType { none, inlet, outlet };

struct BoundaryCap {
	QString name;
	vtkSmartPointer<vtkPolyData> polydata = vtkSmartPointer<vtkPolyData>::New();
	BoundaryCapType type = BoundaryCapType::none;
	QVector<double> center;
	QVector<double> tangent;
	double radius;
};

#endif // !BOUNDARYCAPS_H


