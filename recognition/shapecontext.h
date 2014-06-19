#pragma once

#include <vector>
#include "Signature.h"

namespace recognition 
{

double CalcChiSquareDist(double hist0[], double hist1[], int size);
int **ShapeContext(const Signature &point_list);
int *Histogram(const std::vector<double> array[], int bin);

}