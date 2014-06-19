#pragma once

#include <opencv/cv.h>
#include <vector>

namespace preprocess
{

typedef std::vector<CvPoint> Frame;

void PreprocessInit();
bool FindBiggestComponent(const IplImage *src, IplImage *dst);
int CountWhiteNeighbor(const IplImage* src, int i, int j);
int CountConnectivity(const IplImage* src, int i, int j);
CvPoint FindEndPoint(const IplImage *src);
CvPoint GetNextPoint(const IplImage *src, const CvPoint &cur_point, const CvPoint &prev_point);
Frame FindPointList(const IplImage *src, const CvPoint &start_point);
Frame GetCoordinates(const IplImage *src);
void Thinning(const IplImage *src, IplImage *dst);


}
