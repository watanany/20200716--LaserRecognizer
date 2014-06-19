#pragma once

#include <vector>
#include <opencv/cv.h>

namespace recognition 
{

#if 0
struct Point {
	int X;
	int Y;
	int Pressure;
	int Direction;
	int Altitude;
	int Time;
};
#endif

typedef std::vector<CvPoint> Signature;

}
