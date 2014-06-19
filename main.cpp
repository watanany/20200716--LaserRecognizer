#include "main.h"
using namespace std;

using preprocess::Frame;
using preprocess::PreprocessInit;
using preprocess::FindBiggestComponent;
using preprocess::Thinning;
using preprocess::GetCoordinates;

using recognition::Signature;
using recognition::UniformSample;
using recognition::FeatureSample;
using recognition::LinerInterpolate;
using recognition::DP;
using recognition::ListDir;
using recognition::SaveSign;




int main(int argc, char *argv[])
{
	//CvCapture *video_capture = cvCreateCameraCapture( 0 );
	//CvCapture *video_capture = cvCreateFileCapture("C:\\Users\\m5171146\\Dropbox\\m5171146\\Research\\Program\\LaserRecognizer2\\data_ipod4\\writer2\\0-9.avi");
	CvCapture *video_capture = cvCreateFileCapture("C:\\Users\\m5171146\\Dropbox\\m5171146\\Research\\Program\\LaserRecognizer2\\data_c200\\0-9[1].avi");


	if (video_capture == NULL) {
		return EXIT_SUCCESS;
	}

	// ウィンドウの設定
	const char *input_window = "入力画像";
	const char *output_window = "出力画像";

	cvNamedWindow(input_window, CV_WINDOW_AUTOSIZE);
	cvNamedWindow(output_window, CV_WINDOW_AUTOSIZE);

	// ウィンドウをいい感じの位置に移動させる
	cvMoveWindow(input_window, 250, 50);
	cvMoveWindow(output_window, 250 + 700, 50);

	// カメラからの画像を処理するためのバッファなど
	IplImage *frame_orig = cvQueryFrame(video_capture);
	IplImage *frame_one = cvCreateImage(cvSize(frame_orig->width, frame_orig->height), frame_orig->depth, frame_orig->nChannels);
	IplImage *frame_gray = cvCreateImage(cvSize(frame_orig->width, frame_orig->height), frame_orig->depth, 1);
	IplImage *frame_bin = cvCreateImage(cvSize(frame_orig->width, frame_orig->height), frame_orig->depth, 1);
	IplImage *frame_out = cvCreateImage(cvSize(frame_orig->width, frame_orig->height), frame_orig->depth, frame_orig->nChannels);

	// クロージング処理用のカーネル
	IplConvKernel *closing_kernel = cvCreateStructuringElementEx(5, 5, 2, 2, CV_SHAPE_ELLIPSE);

	// DPマッチングのためのリファレンス画像の変数
	vector<string> path_list = ListDir(REFDIR);
	//vector<string> path_list2 = ListDir(REFDIR2);
	//path_list.insert(path_list.end(), path_list2.begin(), path_list2.end());

	printf("[Loading Reference Files] \n");
	for (int i = 0; i < path_list.size(); i++) {
		//printf("\tpath_list[%d]: %s\n", i, path_list[i].c_str());
		printf("\t%s\n", path_list[i].c_str());
	}
	printf("\n\n");


    DP dp;
    dp.LoadRefFiles(path_list);
	bool shift_on = false;
	bool x_flag = false;

	while (true) {
		Signature sign;
		int key;
		int recog_flag = 0;

		/* 前処理フェイズ */
		// ビデオフレームから一文字抽出
		PreprocessInit();

		while ((key = cvWaitKey(33)) != K_ESC) {
			// カメラから1フレーム取得
			frame_orig = cvQueryFrame(video_capture);
			if (frame_orig == NULL) {
				key = K_ESC;
				break;
			}
			cvCopy(frame_orig, frame_one);

			// RB要素を0にする(レーザーは緑色を使用しているため)
			for (int i = 0; i < frame_one->height; i++) {
				for (int j = 0; j < frame_one->width; j++) {
					CV_IMAGE_ELEM(frame_one, uchar, i, j * 3 + 0) = 0;
					CV_IMAGE_ELEM(frame_one, uchar, i, j * 3 + 2) = 0;
				}
			}

			// グレースケールに変換
			cvCvtColor(frame_one, frame_gray, CV_BGR2GRAY);

			// 二値画像に変換
			cvThreshold(frame_gray, frame_bin, 128, 255, CV_THRESH_BINARY);

			// クロージング処理を施す
			cvMorphologyEx(frame_bin, frame_bin, NULL, closing_kernel, CV_MOP_CLOSE);

			// 大きな領域を見つける
			bool found = FindBiggestComponent(frame_bin, frame_bin);

			// 細線化処理を施す
			Thinning(frame_bin, frame_bin);

			// グレースケール(1チャンネル)をBGR(3チャンネル)に変換
			cvCvtColor(frame_bin, frame_out, CV_GRAY2BGR);

			// レーザーポインターを見つけたら抽出開始
			// レーザーポインターを見失ったら抽出終了
			if (found && recog_flag == 0) {
				recog_flag++;
				printf("[NOW RECOGNIZING...]\n");
			}
			else if (!found && recog_flag == 1) {
				recog_flag++;
			}

			if (recog_flag == 1) {
				Frame coords = GetCoordinates(frame_bin);	// FIX: GetCoordinates has bug
				for (int i = 0; i < coords.size(); i++) {
					sign.push_back(coords[i]);
				}
			}
			else if (recog_flag == 2) {
				break;
			}

			if (sign.size() >= POINT_LIMIT) {
				fprintf(stderr, "[WARNING] Too Many Points: Continue\n");
				sign.clear();
				break;
			}

			
			// スペースキーで一時停止
			if (key == K_SPACE) {
				printf("[PAUSE]\n");
				if (cvWaitKey(-1) == K_SPACE) {
					break;
				}
			}

			// ウィンドウ表示
			cvShowImage(input_window, frame_orig);
			cvShowImage(output_window, frame_out);
		}


		// ESCが押されていたら終了
		if (key == K_ESC) {
			break;
		}

		/* 認識フェイズ */
		// 座標を抽出できたら認識する
		if (sign.size() != 0) {
			dp.LoadTest(sign);
            dp.MatchAll();
			int min_id = dp.GetMinID();

			printf("DP DISTANCE: \n");
			for (int i = 0; i < path_list.size(); i++) {
				const char *b = strrchr(path_list[i].c_str(), '/') + 1;
				//printf("\t%c: %.2f\n", b[0], dp.GetDPDistance(i));
				printf("\t%s: %.2f\n", b, dp.GetDPDistance(i));
			}

			if (min_id < 0) {
				continue;
			}

			string name;
			const char *basename = strrchr(path_list[min_id].c_str(), '/') + 1;
			for (const char *p = basename; *p != '.'; p++) 
				name += *p;

			// X と CapsLockだけ二画必要なので別途処理する
			// Shift, CapsLock入力モード
			if (name == "shift" && !shift_on) {
				shift_on = true;
			}
			else if (name == "shift" && shift_on) {
				shift_on = false;
				name = "capslock";
			}
			else if (shift_on) {
				shift_on = false;
				if (isalpha(name[0])) {
					name = (char)toupper(name[0]) + string(name.c_str() + 1);
				}
			}

			// X入力モード
			if (!x_flag && name == "X1") {
				x_flag = true;
			}
			else if (x_flag && name == "X2" || name == "return") {
				x_flag = false;
				name = "X";
			}
			else if (!x_flag && name == "X2") {
				name = "return";
			}
			else if (x_flag) {
				x_flag = false;
			}

			
			printf("DETECTED: %s\n", name.c_str());
		}
		printf("\n");

	}
	


	// メモリ開放
	cvDestroyAllWindows();
	cvReleaseCapture(&video_capture);
	cvReleaseImage(&frame_gray);
	cvReleaseImage(&frame_bin);
	cvReleaseImage(&frame_out);

	// ウィンドウ破棄
	cvDestroyWindow(input_window);
	cvDestroyWindow(output_window);

	printf("続行します。何かキーを押してください： ");
	getchar();

	return EXIT_SUCCESS;


}




