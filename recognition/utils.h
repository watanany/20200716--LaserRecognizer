#pragma once

#include <iostream>
#include <vector>
#include <string>
#include <opencv/cv.h>

#ifdef __linux__
#include <dirent.h>
#include <unistd.h>
#elif defined _WIN32
#include <windows.h>
#endif


#include "Signature.h"

namespace recognition 
{

void Translate(Signature *sign, CvPoint origin);
void Scale(Signature *sign, const Signature &ref);
void UniformSample(Signature *sign, int n_points);
void FeatureSample(Signature *sign, int n_points);
void FeatureSample(Signature *sign, double threshold_dist);
void LinerInterpolate(Signature *sign, double step=-1);

Signature SignFromFile(std::string file_path);
std::vector<std::string> ListDir(std::string dir);
void SaveSign(const Signature &sign, std::string save_name);
void RepetationRemove(Signature *sign);


}
