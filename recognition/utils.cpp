#include "utils.h"
#include <sstream>
#include <exception>
using namespace std;

namespace recognition
{

void Translate(Signature *sign, CvPoint origin)
{
    int dx = origin.x - (*sign)[0].x;
    int dy = origin.y - (*sign)[0].y;
    
    for (int i = 0; i < (*sign).size(); i++) {
        (*sign)[i].x += dx;
        (*sign)[i].y += dy;
    }
}


void Scale(Signature *sign, const Signature &ref)
{
	CvScalar s = cvScalar(-1, -1, -1, -1);
	for (int i = 0; i < ref.size(); i++) {
		if (s.val[0] == -1 || s.val[0] < ref[i].x) 
			s.val[0] = ref[i].x;
		if (s.val[1] == -1 || s.val[1] < ref[i].y)
			s.val[1] = ref[i].y;
		if (s.val[2] == -1 || s.val[2] > ref[i].x)
			s.val[2] = ref[i].x;
		if (s.val[3] == -1 || s.val[3] > ref[i].y)
			s.val[3] = ref[i].y;
	}

	CvRect ref_rect = cvRect(s.val[0], s.val[1], s.val[2] - s.val[0], s.val[3] - s.val[1]);


	s = cvScalar(-1, -1, -1, -1);
	for (int i = 0; i < sign->size(); i++) {
		if (s.val[0] == -1 || s.val[0] < (*sign)[i].x) 
			s.val[0] = (*sign)[i].x;
		if (s.val[1] == -1 || s.val[1] < (*sign)[i].y)
			s.val[1] = (*sign)[i].y;
		if (s.val[2] == -1 || s.val[2] > (*sign)[i].x)
			s.val[2] = (*sign)[i].x;
		if (s.val[3] == -1 || s.val[3] > (*sign)[i].y)
			s.val[3] = (*sign)[i].y;
	}

	CvRect sign_rect = cvRect(s.val[0], s.val[1], s.val[2] - s.val[0], s.val[3] - s.val[1]);

	
	double w_rate = (double)ref_rect.width / sign_rect.width;
	double h_rate = (double)ref_rect.height / sign_rect.height;
	
	for (int i = 0; i < (*sign).size(); i++) {
		(*sign)[i].x *= h_rate;	// w_rate;
		(*sign)[i].y *= h_rate;
	}
}
    

    
void UniformSample(Signature *sign, int n_points)
{
	Signature res;
	
	int step = sign->size() / n_points;

	if (step == 0) step = 1;

	for (int i = 0; i < sign->size(); i += step) {
		res.push_back((*sign)[i]);
	}

    *sign = res;
}


void FeatureSample(Signature *sign, int n_points)
{
	bool *is_sampled = new bool[sign->size()];
	for (int i = 0; i < sign->size(); i++)
		is_sampled[i] = false;

	is_sampled[0] = true;
	is_sampled[sign->size() - 1] = true;
	int n_sampled = 2;

	bool is_finished = false;

	while (!is_finished) {
		int index1;
		int index2 = 0;

		// サンプリングする点を一つ選ぶ
		while (true) {
			int x1, y1;
			int x2, y2;
			x1 = y1 = x2 = y2 = -1;

			// サンプリングの起点となる2点を選ぶ
			index1 = index2;

			for (int i = index2 + 1; i < sign->size(); i++) {
				if (is_sampled[i]) {
					if (x2 == -1 && y2 == -1) {
						index2 = i;
						break;
					}
				}
			}

			if (index1 == index2) 
				break;

			x1 = (*sign)[index1].x;
			y1 = (*sign)[index1].y;
			x2 = (*sign)[index2].x;
			y2 = (*sign)[index2].y;


			// 2点を通る直線から最大の距離を持つ点を選ぶ
			int max_id = -1;
			double max_dist = 0;

			if (x2 - x1 != 0) {
				double m = (double)(y2 - y1) / (x2 - x1);
				double a = m;
				double b = -1;
				double c = y1 - m * x1;

				for (int i = index1; i < index2; i++) {
					int x = (*sign)[i].x;
					int y = (*sign)[i].y;
					double d = fabs(a * x + b * y + c) / sqrt(a * a + b * b);

					if (max_dist < d) {
						max_id = i;
						max_dist = d;
					}
				}

			}
			else {
				for (int i = 0; i < sign->size(); i++) {
					double d = abs((*sign)[i].x - x1);
					if (max_dist < d) {
						max_id = i;
						max_dist = d;
					}
				}
			}

			// サンプリングのフラグを立てる	
			is_sampled[max_id] = true;
			n_sampled++;

			// 十分サンプリングできたら終了
			if (n_sampled >= n_points || n_sampled >= sign->size()) {
				is_finished = true;
				break;
			}
		}

	}

	Signature res;
	for (int i = 0; i < (*sign).size(); i++) {
		if (is_sampled[i]) {
			res.push_back((*sign)[i]);
		}
	}

	delete[] is_sampled;

	*sign = res;
}



void FeatureSample(Signature *sign, double threshold_dist)
{
	bool *is_sampled = new bool[sign->size()];
	for (int i = 0; i < sign->size(); i++)
		is_sampled[i] = false;

	is_sampled[0] = true;
	is_sampled[sign->size() - 1] = true;
	int n_sampled = 2;

	bool is_finished = false;

	while (!is_finished) {
		int index1;
		int index2 = 0;

		// サンプリングする点を一つ選ぶ
		while (true) {
			int x1, y1;
			int x2, y2;
			x1 = y1 = x2 = y2 = -1;

			// サンプリングの起点となる2点を選ぶ
			index1 = index2;

			for (int i = index2 + 1; i < sign->size(); i++) {
				if (is_sampled[i]) {
					if (x2 == -1 && y2 == -1) {
						index2 = i;
						break;
					}
				}
			}

			if (index1 == index2) 
				break;

			x1 = (*sign)[index1].x;
			y1 = (*sign)[index1].y;
			x2 = (*sign)[index2].x;
			y2 = (*sign)[index2].y;


			// 2点を通る直線から最大の距離を持つ点を選ぶ
			int max_id = -1;
			double max_dist = 0;

			if (x2 - x1 != 0) {
				double m = (double)(y2 - y1) / (x2 - x1);
				double a = m;
				double b = -1;
				double c = y1 - m * x1;

				for (int i = index1; i < index2; i++) {
					int x = (*sign)[i].x;
					int y = (*sign)[i].y;
					double d = fabs(a * x + b * y + c) / sqrt(a * a + b * b);

					if (max_dist < d) {
						max_id = i;
						max_dist = d;
					}
				}

			}
			else {
				for (int i = 0; i < sign->size(); i++) {
					double d = abs((*sign)[i].x - x1);
					if (max_dist < d) {
						max_id = i;
						max_dist = d;
					}
				}
			}

			// サンプリングのフラグを立てる	
			is_sampled[max_id] = true;
			n_sampled++;

			// 十分サンプリングできたら終了
			if (max_dist < threshold_dist || n_sampled >= sign->size()) {
				is_finished = true;
				break;
			}
		}

	}

	Signature res;
	for (int i = 0; i < (*sign).size(); i++) {
		if (is_sampled[i]) {
			res.push_back((*sign)[i]);
		}
	}

	delete[] is_sampled;

	*sign = res;
}



void LinerInterpolate(Signature *sign, double step)
{
	if (step <= 0) {
		step = sqrt(2);
	}

	Signature res;

	for (int i = 0; i < sign->size() - 1; i++) {
		int dx = (*sign)[i + 1].x - (*sign)[i].x;
		int dy = (*sign)[i + 1].y - (*sign)[i].y;
		double dist = sqrt(dx * dx + dy * dy);
		double e  = 0.0;

		while (e < dist) {
			int x = (*sign)[i].x + (int)(e * dx / dist);
			int y = (*sign)[i].y + (int)(e * dy / dist);
			CvPoint xy = cvPoint(x, y);
			res.push_back(xy);
			e += step;
		}
	}

	RepetationRemove(&res);

	*sign = res;
}



Signature SignFromFile(string file_path)
{
	ostringstream stream;

    FILE *fp = fopen(file_path.c_str(), "r");
	if (fp == NULL) {
		stream << "[ERROR] ファイル " << file_path << " が開けません。";
		throw runtime_error(stream.str());
	}

	Signature point_list;

	int n;
	if (fscanf(fp, "%d", &n) != 1) {
		stream << "[ERROR] ファイル " << file_path << " の座標数が読み込めません。";
		throw runtime_error(stream.str());
	}

	for (int i = 0; i < n; i++) {
		int x, y;
		int status = fscanf(fp, "%d %d", &x, &y);
		if (status == EOF) {
			break;
		}
		else if (status != 2) {
			stream << "[ERROR] ファイル " << file_path << " のフォーマットが変です。";
			throw runtime_error(stream.str());
		}

		CvPoint p = cvPoint(x, y);
		point_list.push_back(p);
	}

	return point_list;
}




    
vector<string> ListDir(string dir)
{
	vector<string> list;

#ifdef __linux__
	struct dirent *dirst;

	DIR *dp = opendir(dir.c_str());
	while ((dirst = readdir(dp)) != NULL) {
		const char *ext = strstr(dirst->d_name, ".");

		if (strcmp(ext, ".sdt") == 0) {
			string file = dirst->d_name;
			string path = dir + "/" + file;
			list.push_back(path);
		}
	}


#elif _WIN32
	HANDLE handle;
	WIN32_FIND_DATA find_data;

	handle = FindFirstFile((dir + "/" + "*.sdt").c_str(), &find_data);
	if (handle != INVALID_HANDLE_VALUE) {
		while (true) {
			string file = find_data.cFileName;
			string path = dir + "/" + file;
			list.push_back(path);

			if (!FindNextFile(handle, &find_data))
				break;
		}
	}

	FindClose(handle);


#endif

	return list;

}




void SaveSign(const Signature &sign, string save_name)
{
	FILE *fp = fopen(save_name.c_str(), "w");

	fprintf(fp, "%d\n", (int)sign.size());
	for (int i = 0; i < sign.size(); i++) {
		fprintf(fp, "%d\t%d\n", sign[i].x, sign[i].y);
	}

	fclose(fp);
}




void RepetationRemove(Signature *sign)
{
	CvPoint prev = cvPoint(-1, -1);
	Signature res;

	for (int i = 0; i < sign->size(); i++) {
		CvPoint cur = (*sign)[i];

		if (prev.x == cur.x && prev.y == cur.y) {
			continue;
		}
		else {
			prev = cur;
			res.push_back(cur);
		}
	}

	*sign = res;
}





} // end-namespace


