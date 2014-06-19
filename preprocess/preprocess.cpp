#include "preprocess.h"
using namespace std;


namespace preprocess
{



static Frame PrevFrame = Frame();

void PreprocessInit()
{
	PrevFrame.clear();
}



/*--------------------------------------------------------------------------------
 * Find Biggest Component from a video frame
 *
 */

bool FindBiggestComponent(const IplImage *src, IplImage *dst)
{
    int color = 1;
    vector<CvConnectedComp> cc_list;
    
    for(int i = 0; i < src->height; i++) {
        for(int j = 0; j < src->width; j++) {
            uchar pixel = CV_IMAGE_ELEM(src, uchar, i, j);
            if (pixel < 125) 
                continue;
            if (color > 255) 
                return false;

            CvPoint seed_point = cvPoint(j, i);
            CvConnectedComp cc;
            CvScalar new_val =cvRealScalar(color);
            cvFloodFill(dst, seed_point, new_val, cvScalarAll(0), cvScalarAll(0), &cc);
            cc_list.push_back(cc);

            color++;
        }
    }

    if(cc_list.size() <= 0) 
        return false;

    double max_area = 0;
    int max_color = -1;
    for(int i = 0;i < cc_list.size(); i++) {
        if(cc_list[i].area > max_area){
            max_area = cc_list[i].area;
            max_color = i + 1;
        }
    }

    for(int i = 0; i < dst->height; i++) {
        for(int j = 0; j < dst->width; j++) {
            uchar pixel = CV_IMAGE_ELEM(dst, uchar, i, j);
            if( (int)pixel == max_color ) {
                CV_IMAGE_ELEM(dst, uchar, i, j) = 255;
            }
            else{
                CV_IMAGE_ELEM(dst, uchar, i, j) = 0;
            }
        }
    }

    return true; 
}



/*--------------------------------------------------------------------------------
 * Count Function
 *
 */

int CountWhiteNeighbor(const IplImage *src, int i, int j)
{
    int count = 0;
    
    if (CV_IMAGE_ELEM(src, uchar, i - 1, j - 1) == 255) count++;
    if (CV_IMAGE_ELEM(src, uchar, i - 1, j) == 255)       count++;
    if (CV_IMAGE_ELEM(src, uchar, i - 1, j + 1) == 255) count++;
    if (CV_IMAGE_ELEM(src, uchar, i, j - 1) == 255)       count++;
    if (CV_IMAGE_ELEM(src, uchar, i, j + 1) == 255)	      count++;
    if (CV_IMAGE_ELEM(src, uchar, i + 1, j - 1) == 255) count++;
    if (CV_IMAGE_ELEM(src, uchar, i + 1, j) == 255)       count++;
    if (CV_IMAGE_ELEM(src, uchar, i + 1, j + 1) == 255) count++;

    return count;
}


int CountConnectivity(const IplImage *src, int i, int j)
{
    int count = 0;
    
    if ((CV_IMAGE_ELEM(src, uchar, i - 1, j - 1) - CV_IMAGE_ELEM(src, uchar, i, j - 1)) > 0)       count++;
    if ((CV_IMAGE_ELEM(src, uchar, i, j - 1) - CV_IMAGE_ELEM(src, uchar, i + 1, j - 1)) > 0)       count++;
    if ((CV_IMAGE_ELEM(src, uchar, i + 1, j - 1) - CV_IMAGE_ELEM(src, uchar, i + 1, j)) > 0)       count++;
    if ((CV_IMAGE_ELEM(src, uchar, i + 1, j) - CV_IMAGE_ELEM(src, uchar, i + 1, j + 1)) > 0)       count++;
    if ((CV_IMAGE_ELEM(src, uchar, i, j + 1) - CV_IMAGE_ELEM(src, uchar, i, j + 1)) > 0)             count++;
    if ((CV_IMAGE_ELEM(src, uchar, i - 1, j + 1) - CV_IMAGE_ELEM(src, uchar, i - 1, j + 1)) > 0) count++;
    if ((CV_IMAGE_ELEM(src, uchar, i - 1, j + 1) - CV_IMAGE_ELEM(src, uchar, i - 1, j)) > 0)       count++;
    if ((CV_IMAGE_ELEM(src, uchar, i - 1, j) - CV_IMAGE_ELEM(src, uchar, i - 1, j - 1)) > 0)       count++;

    return count;
}



/*--------------------------------------------------------------------------------
 *
 */

CvPoint FindEndPoint(const IplImage* src)
{
    CvPoint point = {-1, -1};
    for (int i = 0; i < src->height; i++) {
        for (int j = 0; j < src->width; j++) {

            if (CV_IMAGE_ELEM(src, uchar, i, j) == 255) {
                int n_white_neibor = CountWhiteNeighbor(src, i, j);
                if (n_white_neibor == 1) {
                    point.x = j;
                    point.y = i;
                    return point;
                }
            }
        }
    }

    return point;
    
}

/*--------------------------------------------------------------------------------
 *
 */

CvPoint GetNextPoint(const IplImage *src, const CvPoint &cur_point, const CvPoint &prev_point)
{
    int width = src->width;
    int height = src->height;

    CvPoint ret = {-1, -1};

    // cur_point の8近傍の中で (色が白) かつ (prev_pointでない点)

    if(cur_point.y - 1 >= 0 && cur_point.x - 1 >= 0 &&
       CV_IMAGE_ELEM(src, uchar, cur_point.y - 1, cur_point.x - 1) == 255 &&
       (prev_point.x == -1 || (prev_point.y != cur_point.y - 1 || prev_point.x != cur_point.x - 1))) {
        ret.y = cur_point.y - 1;
        ret.x = cur_point.x - 1;
    }
    else if(cur_point.y-1>=0 &&
            CV_IMAGE_ELEM(src, uchar, cur_point.y - 1, cur_point.x) == 255 &&
            (prev_point.x == -1 || (prev_point.y != cur_point.y - 1 || prev_point.x != cur_point.x))) {
        ret.y = cur_point.y - 1;
        ret.x = cur_point.x;
    }
    else if(cur_point.y - 1 >= 0 && cur_point.x + 1 < width &&
            CV_IMAGE_ELEM(src, uchar, cur_point.y - 1, cur_point.x + 1) == 255 &&
            (prev_point.x == -1 || (prev_point.y != cur_point.y - 1 || prev_point.x != cur_point.x + 1))) {
        ret.y = cur_point.y - 1;
        ret.x = cur_point.x + 1;
    }
    else if(cur_point.x - 1 >= 0 &&
            CV_IMAGE_ELEM(src, uchar, cur_point.y, cur_point.x - 1) == 255  &&
            (prev_point.x == -1 || (prev_point.y != cur_point.y || prev_point.x != cur_point.x-1))) {
        ret.y = cur_point.y;
        ret.x = cur_point.x - 1;
    }
    else if(cur_point.x + 1 < width &&
            CV_IMAGE_ELEM(src, uchar, cur_point.y, cur_point.x+1) == 255 &&
            (prev_point.x == -1 || (prev_point.y != cur_point.y || prev_point.x != cur_point.x + 1))) {
        ret.y = cur_point.y;
        ret.x = cur_point.x + 1;
    }
    else if(cur_point.y + 1 < height && cur_point.x - 1 >= 0 &&
            CV_IMAGE_ELEM(src, uchar, cur_point.y + 1, cur_point.x - 1) == 255 &&
            (prev_point.x == -1 || (prev_point.y != cur_point.y + 1 || prev_point.x != cur_point.x - 1))) {
        ret.y = cur_point.y + 1;
        ret.x = cur_point.x - 1;
    }
    else if(cur_point.y + 1 < height &&
            CV_IMAGE_ELEM(src, uchar, cur_point.y + 1, cur_point.x) == 255  &&
            (prev_point.x == -1 || (prev_point.y != cur_point.y + 1 || prev_point.x != cur_point.x))) {
        ret.y = cur_point.y + 1;
        ret.x = cur_point.x;
    }
    else if(cur_point.y + 1 < height && cur_point.x + 1 < width &&
            CV_IMAGE_ELEM(src, uchar, cur_point.y + 1, cur_point.x + 1) == 255 &&
            (prev_point.x == -1 || (prev_point.y != cur_point.y + 1 || prev_point.x != cur_point.x + 1))) {
        ret.y = cur_point.y + 1;
        ret.x = cur_point.x + 1;
    }

    return ret;
}





/*--------------------------------------------------------------------------------
 *
 */
Frame FindPointList(const IplImage *src, const CvPoint &start_point)
{
    Frame cur_frame;
    
	CvPoint prev_point = cvPoint(-1, -1);
	CvPoint cur_point = start_point;

	int count = 0;
	while (cur_point.x >= 0 && cur_point.y >= 0) {
		cur_frame.push_back(cur_point);
		CvPoint tmp = cur_point;
		cur_point = GetNextPoint(src, cur_point, prev_point);
		prev_point = tmp;

		if (++count >= 20) {
            break;
        }
	}
	
	if (PrevFrame.size() == 0) {
        PrevFrame = cur_frame;
		return cur_frame;
    }

    // cur_frame == PrevFrame ならば
	if ((cur_frame[0].x == PrevFrame[0].x && cur_frame[0].y == PrevFrame[0].y) &&
		(cur_frame[cur_frame.size() - 1].x == PrevFrame[PrevFrame.size() - 1].x &&
         cur_frame[cur_frame.size() - 1].y == PrevFrame[PrevFrame.size() - 1].y)) {
			 return Frame();
	}

	double dist0 = sqrt(pow(PrevFrame[0].x - cur_frame[0].x, 2) + pow(PrevFrame[0].y - cur_frame[0].y, 2));
    double dist1 = sqrt(pow(PrevFrame[0].x - cur_frame[cur_frame.size() - 1].x, 2) + pow(PrevFrame[0].y - cur_frame[cur_frame.size() - 1].y, 2));

	if (dist0 > dist1) {
		Frame tmp_list;
		for (int i = 0; i < cur_frame.size(); i++) {
			tmp_list.push_back(cur_frame[cur_frame.size() - 1 - i]);
		}
		cur_frame.assign(tmp_list.begin(), tmp_list.end());
	}

    PrevFrame = cur_frame;
    return cur_frame;
}


/*--------------------------------------------------------------------------------
 *
 */
Frame GetCoordinates(const IplImage *src)
{
	CvPoint end_point = FindEndPoint(src);

	if (end_point.x == -1 && end_point.y == -1)
		return Frame();
	else
		return FindPointList(src, end_point);
}







} // end-namespace
