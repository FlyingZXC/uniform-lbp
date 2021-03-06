#include "SpatialHistogramReco.h"

#include <iostream>
#include <limits>

using namespace cv;

// this one uses only the Ternary pattern structure from : 
//
// Xiaoyang Tan and Bill Triggs
// Enhanced Local Texture Feature Sets for Face Recognition Under Difﬁcult Lighting Conditions
//
// i actually only calculate the 2 outer structures,
//   since the 'dc'/'middle' channel is already determined by <>.
// 2 rings of neighbours applied to patched rois
// patched uniform histograms, L1 distance are used.
//

class LTPH : public SpatialHistogramReco
{
private:
    int _t; // distance to center , t in formula (2)

protected:
    virtual void oper(const Mat & src, Mat & hist) const ;
    virtual double distance(const Mat & hist_a, Mat & hist_b) const 
    {
        return cv::norm(hist_a,hist_b,NORM_L1); 
    }

public:

    LTPH( int t = 5, int gridx=8, int gridy=8, double threshold = DBL_MAX) 
        : SpatialHistogramReco(gridx,gridy,threshold,4*60,CV_8U)
        , _t(t)
    {}
};




void LTPH::oper(const Mat & src, Mat & hist) const {
    
    //
    //! precalculated 8-neighbours case:
    //
    static int uniform[256] = {
        0,1,2,3,4,58,5,6,7,58,58,58,8,58,9,10,11,58,58,58,58,58,58,58,12,58,58,58,13,58,
        14,15,16,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,17,58,58,58,58,58,58,58,18,
        58,58,58,19,58,20,21,22,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
        58,58,58,58,58,58,58,58,58,58,58,58,23,58,58,58,58,58,58,58,58,58,58,58,58,58,
        58,58,24,58,58,58,58,58,58,58,25,58,58,58,26,58,27,28,29,30,58,31,58,58,58,32,58,
        58,58,58,58,58,58,33,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,34,58,58,58,58,
        58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
        58,35,36,37,58,38,58,58,58,39,58,58,58,58,58,58,58,40,58,58,58,58,58,58,58,58,58,
        58,58,58,58,58,58,41,42,43,58,44,58,58,58,45,58,58,58,58,58,58,58,46,47,48,58,49,
        58,58,58,50,51,52,58,53,54,55,56,57
    };

    // calculate patterns
    int radius = 2;
    for(int i=radius;i<src.rows-radius;i++) 
    {
        for(int j=radius;j<src.cols-radius;j++) 
        {
            //
            // 7 0 1
            // 6 c 2
            // 5 4 3
            //
            // center and 2 rings of neighbours:
            uchar c   = src.at<uchar>(i,j);
            uchar n[2][8]= {
                {
                    src.at<uchar>(i-1,j),
                    src.at<uchar>(i-1,j+1),
                    src.at<uchar>(i,j+1),
                    src.at<uchar>(i+1,j+1),
                    src.at<uchar>(i+1,j),
                    src.at<uchar>(i+1,j-1),
                    src.at<uchar>(i,j-1),
                    src.at<uchar>(i-1,j-1) 
                },
                {
                    src.at<uchar>(i-2,j),
                    src.at<uchar>(i-2,j+2),
                    src.at<uchar>(i,  j+2),
                    src.at<uchar>(i+2,j+2),
                    src.at<uchar>(i+2,j),
                    src.at<uchar>(i+2,j-2),
                    src.at<uchar>(i,  j-2),
                    src.at<uchar>(i-2,j-2) 
                }
            };
            unsigned code1 = 0;
            unsigned code2 = 0;
            unsigned code3 = 0;
            unsigned code4 = 0;
            for ( int i=0; i<8; i++ )
            {
                code1 += (n[0][i]>(c+_t))<<i;
                code2 += (n[0][i]<(c-_t))<<i;
                code3 += (n[1][i]>(c+_t))<<i;
                code4 += (n[1][i]<(c-_t))<<i;
            }
            hist.at<uchar>(uniform[code1])      += 1;
            hist.at<uchar>(uniform[code2]+60)   += 1;
            hist.at<uchar>(uniform[code3]+60*2) += 1;
            hist.at<uchar>(uniform[code4]+60*3) += 1;
        }
    }
    distanceTransform(hist, hist, DIST_C, 3);
}


Ptr<FaceRecognizer> createLTPHFaceRecognizer(int t, int grid_x, int grid_y, double threshold)
{
    return makePtr<LTPH>(t, grid_x, grid_y, threshold);
}

