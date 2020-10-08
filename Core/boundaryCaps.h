#ifndef BOUNDARYCAPS_H
#define BOUNDARYCAPS_H

#include "vtkPolyData.h"

enum BoundaryCapType { none, inlet, outlet };

struct BoundaryCap {
	vtkPolyData* polydata;
	BoundaryCapType type;
	QVector<double> center;
	QVector<double> tangent;
	double radius;
};

#endif // !BOUNDARYCAPS_H


