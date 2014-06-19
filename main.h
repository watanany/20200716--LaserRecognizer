#pragma once

#include <iostream>
#include <cstring>
#include <opencv/cv.h>
#include <opencv/highgui.h>
#include <string>
#include <vector>
#include <map>


#include "preprocess/preprocess.h"
#include "recognition/recognition.h"


#define REFDIR0 "Graffiti/number"
#define REFDIR1 "Graffiti/alphabet"
#define REFDIR2 "Graffiti/specialkey"
#define REFDIR REFDIR0


#define STEP 15
#define K_ESC 27
#define K_SPACE 32
#define THRESHOLD_G 128
#define POINT_LIMIT 10000


#if 0

inline void Sleepy(unsigned int mill_seconds)
{
#if __linux__
	struct timespec t;
	t.tv_sec = mill_seconds / 1000;
	t.tv_nsec = (mill_seconds % 1000) * 1000;
	nanosleep(&t, NULL);
#elif defined _WIN32
	Sleep(mill_seconds);
#endif
}

#endif



