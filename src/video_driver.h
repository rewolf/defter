#ifndef VD_H
#define VD_H
#include <cstdlib>

#include <utility>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <getopt.h>		/* getopt_long() */
#include <fcntl.h>		/* low-level i/o */
#include <unistd.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/mman.h>
#include <sys/ioctl.h>
#include <asm/types.h>		/* for videodev2.h */
#include <linux/videodev2.h>
#define CLEAR(x) memset (&(x), 0, sizeof (x))
#define true 1
#define false 0 //You might have to declaire True and False.
class video_driver{
    public:



        int counter;
        typedef enum
        {
                IO_METHOD_READ,
                IO_METHOD_MMAP,
                IO_METHOD_USERPTR,
        } io_method;

        struct buffer
        {
                void *start;
                size_t length;
        };

        struct l_rec{
            int top,bottom,left,right;
            int * array;
            int num_points;
        };

        char *dev_name;
        io_method io;
        
        int pixel_format;
        int fd;
        struct buffer *buffers;
        unsigned int n_buffers;
        unsigned int width;
        unsigned int height;
        unsigned int count;
        int last_index;
        int * occ_array;
        int * diff_array;


        void errno_exit(const char *s);

        int xioctl(int fd, int request, void *arg);



        int read_frame(void);
        void init_read(unsigned int buffer_size);
        void init_mmap(void);
        void init_userp(unsigned int buffer_size);
        void restart_video();


        void init_device(void);


        void close_device();
        void uninit_device();
        void start_capturing();
        void stop_capturing();
        void open_device(void);


        video_driver(int vw,int vh);
};



#endif
