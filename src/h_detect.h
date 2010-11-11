#include "frame.h"
#include "m_rec.h"
#ifndef H_DETECT_H
#define H_DETECT_H

struct fingertip{
    int x,y;
    float cval;
    int contour_index;
};
class h_detect{
    public:
        frame * l_frame;
        int * edge_buffer;
        int * recog_buffer;
        int init_y;
        int buffer_size;
        m_rec * m;
        int max_x,max_y;
        int min_x,min_y;
        int ebw,ebh;
        int lastx,lasty;
        int palmx,palmy;
        int p_startx,p_starty;
        bool do_palm_thresh_check;
        int num_points;
        h_detect(frame * f,m_rec * mr);
        std::vector<struct fingertip *> fingertips;
        std::vector<int> ptsx;
        std::vector<int> ptsy;
        void do_moore_neighbourhood(int startx,int starty);

        std::vector<int> hand_ptsx;
        std::vector<int> hand_ptsy;

        int tmpcnt;

        void flood_fill(int x,int y);
        inline int get_recog_val(int x,int y);

        float curvature(int i, int l); // curvature at P_i with param l
        int  pts_off(int i);
        void next_clockwise(int px,int py,int * cx,int *cy);
        void flood_fill_rec(int x,int y);
        void flood_fill_rec2(int x,int y,int yval);
        bool hand_search(int x,int y,int * eb);
        bool accept (int x,int y);
        void add_fingertip(int x,int y,float cval,int c_index);
        bool accept (int x,int y,int old_x,int old_y);

};

#endif
