#include "shapecontext.h"
#include <cmath>
using std::vector;

#define D_HIST_SIZE 6
#define T_HIST_SIZE 12

namespace recognition
{

double CalcChiSquareDist(double hist0[], double hist1[], int size)
{
	// Χ二乗距離を計算する
	double total = 0.0;
	for (int i = 0; i < size; i++) {
		if (hist0[i] + hist1[i] != 0) {
			total += (hist0[i] - hist1[i]) * (hist0[i] - hist1[i]) / (hist0[i] + hist1[i]);
		}
	}
	return total * 0.5;
}

int **ShapeContext(const Signature &point_list)
{
	// 図形のShapeContextを返す
	// [*, *, *, ..., -1, *, *, *, ..., -1] の形で格納され-1が仕切になっている。
	// 前半は距離のヒストグラム、後半は角度のヒストグラム

	int n = point_list.size();
	vector<double> d_mat(n);			// 距離を格納する変数
	vector<double> t_mat(n);			// 角度を格納する変数
	int *d_hist;						// 距離のヒストグラムを格納する一時変数
	int *t_hist;						// 角度のヒストグラムを格納する一時変数
	int **histogram = new int*[n];		// 結果用変数。距離・角度のヒストグラム両方を格納する

	for (int i = 0; i < n; i++)
		histogram[i] = new int[D_HIST_SIZE + T_HIST_SIZE + 2];

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (i == j) {
				// 同じ点同士は計算できないのでパスする
				continue;
			}
			else {
				int x = point_list[j].x - point_list[i].x;
				int y = point_list[j].y - point_list[i].y;
				
				// 距離を計算する
				double dist = sqrt(x * x + y * y);
				double theta;

				// 角度を測る
				if (x != 0)
					theta = atan2(y, x);
				else if (y >= 0)
					theta = M_PI / 2.0;
				else
					theta = -M_PI / 2.0;

				// 距離はlogに変換後格納、角度はそのまま格納(log-polar)
				d_mat[j] = log10(dist);
				t_mat[j] = theta;
			}
		}
		// ヒストグラムを計算する
		d_hist = Histogram(d_mat, D_HIST_SIZE);
		t_hist = Histogram(t_mat, T_HIST_SIZE);

		// ヒストグラムを結果用変数に移し替える
		for (int j = 0; j < D_HIST_SIZE; j++)
			histogram[i][j] = d_hist[j];
		histogram[i][D_HIST_SIZE] = -1;		// 距離のヒストグラムの終わりを示す番兵

		for (int j = D_HIST_SIZE + 1; j < T_HIST_SIZE; j++)
			histogram[i][j] = d_hist[j];
		histogram[i][T_HIST_SIZE] = -1;		// 角度のヒストグラムの終わりを示す番兵

		// 一時変数のメモリ領域を削除
		delete[] d_hist;
		delete[] t_hist;

	}

	return histogram;
}

int *Histogram(const vector<double> array, int n_bin)
{
	// 最大値・最小値を計算する
	int max = array[0];
	int min = array[0];
	int max_id = 0;
	int min_id = 0;

	for (int i = 1; i < array.size(); i++) {
		if (max < array[i]) {
			max = array[i];
			max_id = i;
		}
		if (min > array[i]) {
			min = array[i];
			min_id = i;
		}
	}

	// ヒストグラムを仕切を計算するための変数
	double step = (double)(max - min) / n_bin;
	int *histogram = new int[n_bin];

	// 0で初期化する
	for (int i = 0; i < n_bin; i++) histogram[i] = 0;

	// どのクラスに所属しているかをカウントしていく
	for (int i = 0; i < array.size(); i++) {
		for (int j = 0; j < n_bin; j++) {
			if (min + j * step <= array[i] && array[i] < min + (j + 1) * step) {
				histogram[j]++;
			}
		}
	}

	return histogram;
}



}	// end-namespace