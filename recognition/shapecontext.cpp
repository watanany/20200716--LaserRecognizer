#include "shapecontext.h"
#include <cmath>
using std::vector;

#define D_HIST_SIZE 6
#define T_HIST_SIZE 12

namespace recognition
{

double CalcChiSquareDist(double hist0[], double hist1[], int size)
{
	// ����拗�����v�Z����
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
	// �}�`��ShapeContext��Ԃ�
	// [*, *, *, ..., -1, *, *, *, ..., -1] �̌`�Ŋi�[����-1���d�؂ɂȂ��Ă���B
	// �O���͋����̃q�X�g�O�����A�㔼�͊p�x�̃q�X�g�O����

	int n = point_list.size();
	vector<double> d_mat(n);			// �������i�[����ϐ�
	vector<double> t_mat(n);			// �p�x���i�[����ϐ�
	int *d_hist;						// �����̃q�X�g�O�������i�[����ꎞ�ϐ�
	int *t_hist;						// �p�x�̃q�X�g�O�������i�[����ꎞ�ϐ�
	int **histogram = new int*[n];		// ���ʗp�ϐ��B�����E�p�x�̃q�X�g�O�����������i�[����

	for (int i = 0; i < n; i++)
		histogram[i] = new int[D_HIST_SIZE + T_HIST_SIZE + 2];

	for (int i = 0; i < n; i++) {
		for (int j = 0; j < n; j++) {
			if (i == j) {
				// �����_���m�͌v�Z�ł��Ȃ��̂Ńp�X����
				continue;
			}
			else {
				int x = point_list[j].x - point_list[i].x;
				int y = point_list[j].y - point_list[i].y;
				
				// �������v�Z����
				double dist = sqrt(x * x + y * y);
				double theta;

				// �p�x�𑪂�
				if (x != 0)
					theta = atan2(y, x);
				else if (y >= 0)
					theta = M_PI / 2.0;
				else
					theta = -M_PI / 2.0;

				// ������log�ɕϊ���i�[�A�p�x�͂��̂܂܊i�[(log-polar)
				d_mat[j] = log10(dist);
				t_mat[j] = theta;
			}
		}
		// �q�X�g�O�������v�Z����
		d_hist = Histogram(d_mat, D_HIST_SIZE);
		t_hist = Histogram(t_mat, T_HIST_SIZE);

		// �q�X�g�O���������ʗp�ϐ��Ɉڂ��ւ���
		for (int j = 0; j < D_HIST_SIZE; j++)
			histogram[i][j] = d_hist[j];
		histogram[i][D_HIST_SIZE] = -1;		// �����̃q�X�g�O�����̏I���������ԕ�

		for (int j = D_HIST_SIZE + 1; j < T_HIST_SIZE; j++)
			histogram[i][j] = d_hist[j];
		histogram[i][T_HIST_SIZE] = -1;		// �p�x�̃q�X�g�O�����̏I���������ԕ�

		// �ꎞ�ϐ��̃������̈���폜
		delete[] d_hist;
		delete[] t_hist;

	}

	return histogram;
}

int *Histogram(const vector<double> array, int n_bin)
{
	// �ő�l�E�ŏ��l���v�Z����
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

	// �q�X�g�O�������d�؂��v�Z���邽�߂̕ϐ�
	double step = (double)(max - min) / n_bin;
	int *histogram = new int[n_bin];

	// 0�ŏ���������
	for (int i = 0; i < n_bin; i++) histogram[i] = 0;

	// �ǂ̃N���X�ɏ������Ă��邩���J�E���g���Ă���
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