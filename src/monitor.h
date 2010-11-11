#include <SDL/SDL.h>
#ifndef MONITOR_H
#define MONITOR_H
/*
 * FOV calc, and stuff
 */

class monitor{
    public:
        SDL_Surface *outs;
        int out_width, out_height;
        monitor(int outw,int outh);
        /*void draw_orientation();
        void fill(Uint32 col,int width,int height);
        void render(int x,int y);
        void event_loop();
        */
};
#endif
