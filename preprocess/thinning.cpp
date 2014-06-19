#include "preprocess.h"

namespace preprocess {

// 田村の方法
// 参考URL: http://imagingsolution.blog107.fc2.com/blog-entry-138.html

#define N_PATTERN 2
#define N_GPATTERN 12

static char pattern1_remove[N_PATTERN][3][3] = {
    {
        '*', '0', '*',
        '*', '1',  '*',
        '*', '*', '*',
    },

    {
        '*', '*', '*',
        '*', '1', '0',
        '*', '*', '*',
    },
};

static char pattern1_noremove[N_PATTERN][3][3] = {
    {
        '*', '0', '*',
        '*', '1', '1',
        '*', '1', '0',
    },

    {
        '0', '1', '*',
        '1', '1', '0',
        '*', '*', '*',
    },
};

static char pattern2_remove[N_PATTERN][3][3] = {
    {
        '*', '*', '*',
        '*', '1', '*',
        '*', '0', '*',
    },

    {
        '*', '*', '*',
        '0', '1', '*',
        '*', '*', '*',
    },
};

static char pattern2_noremove[N_PATTERN][3][3] = {
    {
        '0', '1', '*',
        '1', '1', '*',
        '*', '0', '*',
    },

    {
        '*', '*', '*',
        '0', '1', '1',
        '*', '1', '0',
    }
};

static char pattern_noremove[N_GPATTERN][3][3] = {
    {
        '*', '*', '*',
        '0', '1', '0',
        '*', '1', '*',
    },

    {
        '*', '0', '*',
        '1', '1', '0',
        '*', '0', '*',
    },

    {
        '*', '1', '*',
        '0', '1', '0',
        '*', '*', '*',
    },

    {
        '*', '0', '*',
        '*', '1', '1',
        '*', '0', '*',
    },

    {
        '*', '*', '*',
        '0', '1', '*',
        '1', '0', '*',
    },

    {
        '1', '0', '*',
        '0', '1', '*',
        '*', '*', '*',
    },

    //

    {
        '*', '0', '1',
        '*', '1', '0',
        '*', '*', '*',
    },

    {
        '*', '*', '*',
        '*', '1', '0',
        '*', '0', '1',
    },

    {
        '0', '1', '0',
        '1', '1', '1',
        '0', '*', '0',
    },

    {
        '0', '1', '0',
        '*', '1', '1',
        '0', '1', '0',
    },

    {
        '0', '*', '0',
        '1', '1', '1',
        '0', '1', '0',
    },

    {
        '0', '1', '0',
        '1', '1', '*',
        '0', '1', '0',
    },
};


inline bool check(const IplImage *src, int i, int j, char pattern[3][3])
{
    for (int m = -1; m <= 1; m++) {
        for (int n = -1; n <= 1; n++) {
            uchar p0 = CV_IMAGE_ELEM(src, uchar, i + m, j + n);
            char p1_c = pattern[m + 1][n + 1];

            if (p1_c == '*')
                continue;
			
            uchar p1 = 255 * (p1_c - '0');
            if (p0 != p1)
                return false;
        }
    }

    return true;
}


void Thinning(const IplImage *src, IplImage *dst)
{
    IplImage *tmp1 = cvCreateImage(cvSize(src->width + 2, src->height + 2), IPL_DEPTH_8U, 1);

    /* 黒い枠で囲う */
    for (int i = 0;i < tmp1->height; i++) {
        for (int j = 0; j < tmp1->width; j++) {
            if (i == 0 || j == 0 || i == tmp1->height - 1 || j == tmp1->width - 1) {
                CV_IMAGE_ELEM(tmp1, uchar, i, j) = 0;
            }
            else {
                uchar pixel = CV_IMAGE_ELEM(src, uchar, i - 1, j - 1);
                CV_IMAGE_ELEM(tmp1, uchar, i, j) = pixel;
            }
        }
    }

    IplImage *tmp2 = cvCreateImage(cvSize(tmp1->width, tmp1->height), IPL_DEPTH_8U, 1);
    while (true) {
        // Pattern1
        bool flag = false;
        cvCopy(tmp1, tmp2);

        for (int i = 1; i < tmp1->height - 1; i++) {
            for (int j = 1; j < tmp1->width - 1; j++) {
                if (CV_IMAGE_ELEM(tmp1, uchar, i, j) != 255)
                    continue;

                bool remove_flag = false;

                for (int k = 0; k < N_PATTERN; k++) {
                    if (check(tmp1, i, j, pattern1_remove[k])) {
                        remove_flag = true;
                        break;
                    }
                }

                if (remove_flag) {
                    for (int k = 0; k < N_PATTERN; k++) {
                        if (check(tmp1, i, j, pattern1_noremove[k])) {
                            remove_flag = false;
                            break;
                        }
                    }

                    if (!remove_flag) continue;

                    for (int k = 0; k < N_GPATTERN; k++) {
                        if (check(tmp1, i, j, pattern_noremove[k])) {
                            remove_flag = false;
                            break;
                        }
                    }

                    if (!remove_flag) continue;

                    flag = true;
                    CV_IMAGE_ELEM(tmp2, uchar, i, j) = 0;
                }
            }
        }

        if (!flag) break;

        // Pattern2
        flag = false;
        cvCopy(tmp2, tmp1);

        for (int i = 1; i < tmp1->height - 1; i++) {
            for (int j = 1; j < tmp1->width - 1; j++) {
                if (CV_IMAGE_ELEM(tmp1, uchar, i, j) != 255)
                    continue;

                bool remove_flag = false;

                for (int k = 0; k < N_PATTERN; k++) {
                    if (check(tmp1, i, j, pattern1_remove[k])) {
                        remove_flag = true;
                        break;
                    }
                }

                if (remove_flag) {
                    for (int k = 0; k < N_PATTERN; k++) {
                        if (check(tmp1, i, j, pattern1_noremove[k])) {
                            remove_flag = false;
                            break;
                        }
                    }

                    if (!remove_flag) continue;

                    for (int k = 0; k < N_GPATTERN; k++) {
                        if (check(tmp1, i, j, pattern_noremove[k])) {
                            remove_flag = false;
                            break;
                        }
                    }

                    if (!remove_flag) continue;			

                    flag = true;
                    CV_IMAGE_ELEM(tmp2, uchar, i, j) = 0;
                }
            }
        }

        if (!flag) break;
        cvCopy(tmp2, tmp1);
    }

    for (int i = 0; i < src->height; i++) {
        for (int j = 0; j < src->width; j++) {
            CV_IMAGE_ELEM(src, uchar, i, j) = CV_IMAGE_ELEM(tmp1, uchar, i + 1, j + 1);
        }
    }

}


} // end-namespace
