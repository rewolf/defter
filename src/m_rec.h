#ifndef M_REC_H
#define M_REC_H
#include <vector>
#include "frame.h"
#include "pose.h"
#include "v_util.h"
#include <stdlib.h>
#include <opencv/cv.h>

struct point {
    float x,y,z;
};


struct out_run{
    int x0, y0;
    int x1,y1;
};

struct screen_obstruction{
    int N_out; // number of pixels out
    int total_N;
    int centre_x;
    int centre_y;
    float directionx;
    float directiony;
};

typedef struct point point;
class m_rec{
    public:
        vec2f corner_ai,corner_aj,corner_bi,corner_bj;
        vec2f lcorner_ai,lcorner_aj,lcorner_bi,lcorner_bj;
        vec2f ptl,ptr,pbl,pbr;
        bool detected;
        bool hand_detected;
        int handx,handy;
        pose * pose_im;
        double homography_matrix[9];
        double screen_corners[4][2];// = {{0.,0.},{1920.,0.},{1920.,1200.},{0.,1200.}};
        double image_corners[4][2];// = {{0.,0.},{1920.,0.},{1920.,1200.},{0.,1200.}};
        CvMat  _display_points;// = cvMat(4,2,CV_64F,screen_corners);
        CvMat  _h;// = cvMat(3,3,CV_64F,homography_matrix);
        CvMat  _src_points;
        int top_left[2];
        int top_right[2];
        int bottom_left[2];
        int bottom_right[2];
        float lastdr1,lastdr2;
        int transform_xy(int x,int y,int * ox,int * oy);

        struct line  old_a;
        struct line  old_b;
        struct line  old_i;
        struct line  old_j;
        
       
        m_rec(frame & f);
        frame & l_frame;
        bool orient();
        void set_found(struct line * a,struct line * b, struct line * i,struct line * j);
        bool find_monitor();
        bool check_quadrangle(struct line * a,struct line * b, struct line * i,struct line * j,struct screen_obstruction * scr_ob);
        bool check_mag(struct line * a,struct line * b, struct line * i,struct line * j);
        int validate_hand(struct screen_obstruction * sob, int *fx,int *fy); // return -1 on fail
        float dist (point &p1,point &p2);
        bool in_bounds(int x,int y);
        bool in_border_bounds(int x,int y);
        bool border_check(int x1,int y1,int x2,int y2,struct screen_obstruction * so,bool tolerate_out);
};
#endif
