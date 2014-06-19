#include "DP.h"
#include <math.h>
#include <float.h>
using namespace std;

#define INF 99999999
#define N_POINTS 20

namespace recognition
{


/********************************************************************************
 *
 ********************************************************************************/
DP::DP()
{
    this->DPDistanceList = NULL;
}


DP::~DP()
{
    delete[] this->DPDistanceList;
}



/********************************************************************************
 *
 ********************************************************************************/
double DP::Match(const Signature &ref, const Signature &test)
{

    // DP Tableを初期化する
	double **dp_table = new double*[ref.size()];
	if (dp_table == NULL) {
		throw runtime_error("[ERROR] Memory for DP Matching cannot be allocated.");
	}

    for (int i = 0; i < ref.size(); i++) {
		dp_table[i] = new double[test.size()];

		if (dp_table[i] == NULL) {
			throw runtime_error("[ERROR] Memory for DP Matching cannot be allocated.");
		}
	}

	for (int i = 0; i < ref.size(); i++) {
		for (int j = 0; j < test.size(); j++) {
			dp_table[i][j] = INF;
		}
	}


    // DPマッチング
    dp_table[0][0] = 0;
    int prev_match[2] = { 0, 0 };

    for (int i = 1; i < ref.size(); i++) {
        for (int j = 0; j < test.size(); j++) {
            double min = dp_table[i - 1][j];

			if (j == 0) {
				prev_match[0] = i - 1;
				prev_match[1] = 0;
			}
			else {
				prev_match[0] = i - 1;
				prev_match[1] = j - 1;
			}

            if (j - 1 >= 0 && dp_table[i - 1][j - 1] < min) {
                min = dp_table[i - 1][j - 1];
                prev_match[0] = i - 1;
                prev_match[1] = j - 1;
            }
            if (j - 2 >= 0 && dp_table[i - 1][j - 2] < min) {
                min = dp_table[i - 1][j - 2];
                prev_match[0] = i - 1;
                prev_match[1] = j - 2;
            }

            dp_table[i][j] = min + LocalDistance(ref, test, i, j, prev_match);
        }
    }
    
    double dp_dist = dp_table[ref.size() - 1][test.size() - 1];
    

    // メモリ開放を行う
	for (int i = 0; i < ref.size(); i++) {
		delete[] dp_table[i];
    }
	
	delete[] dp_table;

	return dp_dist;

}



/********************************************************************************
 *
 ********************************************************************************/
void DP::LoadRefFiles(const vector<string> &path_list)
{
    this->RefList.clear();
    
	for (int i = 0; i < path_list.size(); i++) {
		Signature s = SignFromFile(path_list[i]);
		const char *basename = strrchr(path_list[i].c_str(), '/') + 1;
		LinerInterpolate(&s);
		//FeatureSample(&s, N_POINTS);
		UniformSample(&s, N_POINTS);
		//SaveSign(s, basename);
		this->RefList.push_back(s);
	}

    if (this->DPDistanceList != NULL) {
        delete[] this->DPDistanceList;
    }

    this->DPDistanceList = new double[this->RefList.size()];
}


/********************************************************************************
 *
 ********************************************************************************/
void DP::LoadTest(Signature test)
{
	//SaveSign(test, "test1.sdt");
	LinerInterpolate(&test);
	//SaveSign(test, "test2.sdt");
	//FeatureSample(&test, N_POINTS);
	UniformSample(&test, N_POINTS);
	//SaveSign(test, "test3.sdt");

	this->Test = test;
}



/********************************************************************************
 *
 ********************************************************************************/
void DP::MatchAll()
{
    int min_id = -1;
    double min_dist;

	for (int refdata_id = 0; refdata_id < this->RefList.size(); refdata_id++) {
		Scale(&this->RefList[refdata_id], this->Test);
		Translate(&this->RefList[refdata_id], this->Test[0]);
	}

    for (int refdata_id = 0; refdata_id < this->RefList.size(); refdata_id++) {
        Signature &sign = this->Test;
		Signature &ref = this->RefList[refdata_id];

        double dp_dist = Match(ref, sign);
        this->DPDistanceList[refdata_id] = dp_dist;

        if (min_id == -1 || min_dist > dp_dist) {
            min_id = refdata_id;
            min_dist = dp_dist;
        }
    }

    this->MinID = min_id;
}



/********************************************************************************
 *
 ********************************************************************************/
int DP::GetMinID()
{
    return this->MinID;
}



/********************************************************************************
 *
 ********************************************************************************/
double DP::GetDPDistance(int refdata_id)
{
    return this->DPDistanceList[refdata_id];
}







/********************************************************************************
 *
 ********************************************************************************/




double DP::LocalDistance(const Signature &ref, const Signature &test, int i, int j, const int prev_match[])
{
    double ed = LocalDistance_ED(ref, test, i, j);
    double dv = LocalDistance_DV(ref, test, i, j);
    double dpw = LocalDistance_DPW(ref, test, i, j, prev_match);
    //double sc = LocalDistance_SC(ref, test, i, j);

	return (ed + dv) / 2.0;
}

double DP::LocalDistance_ED(const Signature &ref, const Signature &test, int i, int j)
{
    int dx = ref[i].x - test[j].x;
    int dy = ref[i].y - test[j].y;

    return sqrt(dx * dx + dy * dy);
}

double DP::LocalDistance_DV(const Signature &ref, const Signature &test, int i, int j)
{
    if (i + 1 >= ref.size() || j + 1 >= test.size()) {
        int dx = ref[i].x - test[j].x;
        int dy = ref[i].y - test[j].y;

        return sqrt(dx * dx + dy * dy);
    }
    else {
        int ref_dx = ref[i + 1].x - ref[i].x;
        int ref_dy = ref[i + 1].y - ref[i].y;
        int test_dx = test[j + 1].x - test[j].x;
        int test_dy = test[j + 1].y - test[j].y;

        int dx = ref_dx - test_dx;
        int dy = ref_dy - test_dy;

        return sqrt(dx * dx + dy * dy);
    }
}

double DP::LocalDistance_DPW(const Signature &ref, const Signature &test, int i, int j, const int prev_match[])
{
	int dx = ref[prev_match[0]].x - test[prev_match[1]].x;
    int dy = ref[prev_match[0]].y - test[prev_match[1]].y;

    dx = (test[j].x + dx) - ref[i].x;
    dy = (test[j].y + dy) - ref[i].y;

    return sqrt(dx * dx + dy * dy);
}

double DP::LocalDistance_SC(const Signature &ref, const Signature &test, int i, int j)
{
    return 0;
}




} // end-namespace
