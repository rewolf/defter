#ifndef FRAME_H
#define FRAME_H
#include <vector>
#include <sys/types.h>
#include <string.h>
#include <bits/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <algorithm>
#include "v_util.h"
/*
 *Will do stuff like colour conversion and such;
 as well as access to modified versions (light reduce )
 * */

struct line{
    float r,theta;
    int accum;
};

struct pair2{
    int i;
    int j;
};
struct rect{
    int x,y;
    int w,h;
};
class frame{
    public:
        std::vector<struct line *> line_accum;
        frame(char * b,int w,int h);
        int y_thresh;
        inline int get_off(int x,int y);
        bool score_func(float,float,float,float);
        unsigned int get_y(int x,int y);
        unsigned int get_u(int x,int y);
        unsigned int get_v(int x,int y);
        unsigned int get_r(int x,int y);
        unsigned int get_g(int x,int y);
        unsigned int get_b(int x,int y);
        
        int * accum;
        int * op;
        int * blurred;
        int * sort_buf;
        float * angles;
        float drho,dtheta;
        int numangle;
        int numrho;
        int max;
        int edge_max;
        float * tabsin;
        float * tabcos;
        int widthclip(int x);
        int n_r;
        int n_theta;
        int heightclip(int y);
        int * compute_sobel(int x,int y,int w,int h,bool initialise);
        int get_normalised_edge(int x,int y);
        void whole_search();
        void search_box(int x,int y,int w,bool init);
        int * compute_canny();
        void mark_r(int x,int y);
        void mark_g(int x,int y);

        void set_r(int x,int y,char val);
        void set_g(int x,int y,char val);
        void set_b(int x,int y,char val);
        bool in_bounds(int x, int y);
        char * buffer;
        int width,height;
};

#endif
