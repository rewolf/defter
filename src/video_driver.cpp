#include "video_driver.h"
#include <poll.h>
void video_driver::errno_exit(const char *s)
{
	fprintf(stderr, "%s error %d, %s\n", s, errno, strerror(errno));
	exit(EXIT_FAILURE);
}

/*
ioctl wrapper
*/
int video_driver::xioctl(int fd, int request, void *arg)
{
	int r;

	do
		r = ioctl(fd, request, arg);
	while (-1 == r && EINTR == errno);

	return r;
}




int video_driver::read_frame(void)
{
	struct v4l2_buffer buf;
	unsigned int i;
	ssize_t read_bytes;
	unsigned int total_read_bytes;

//	case IO_METHOD_MMAP:
		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
               struct pollfd pfd;
              pfd.fd = fd;
             pfd.events = POLLIN;
             poll(&pfd,1,-1);
		if (-1 == xioctl(fd, VIDIOC_DQBUF, &buf))
		{
			switch (errno)
			{
			case EAGAIN:
				return 0;

			case EIO:

			default:
				errno_exit("VIDIOC_DQBUF");
			}
		}

		assert(buf.index < n_buffers);
                //printf("Index is %i\n",buf.index);
                last_index=buf.index;


		if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
			errno_exit("VIDIOC_QBUF");

	return 1;
}
void video_driver::init_read(unsigned int buffer_size)
{
	buffers = (buffer *)calloc(1, sizeof(*buffers));

	if (!buffers)
	{
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	buffers[0].length = buffer_size;
	buffers[0].start = malloc(buffer_size);

	if (!buffers[0].start)
	{
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}
}

void video_driver::init_mmap(void)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 3;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_MMAP;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			fprintf(stderr, "%s does not support "
					"memory mapping\n", dev_name);
			exit(EXIT_FAILURE);
		}
		else
		{
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	if (req.count < 2)
	{
		fprintf(stderr, "Insufficient buffer memory on %s\n", dev_name);
		exit(EXIT_FAILURE);
	}

	buffers = (buffer *)calloc(req.count, sizeof(*buffers));

	if (!buffers)
	{
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < req.count; ++n_buffers)
	{
		struct v4l2_buffer buf;

		CLEAR(buf);

		buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
		buf.memory = V4L2_MEMORY_MMAP;
		buf.index = n_buffers;

		if (-1 == xioctl(fd, VIDIOC_QUERYBUF, &buf))
			errno_exit("VIDIOC_QUERYBUF");

                printf ("MMAPing buffer\n");

		buffers[n_buffers].length = buf.length;
		buffers[n_buffers].start = mmap(NULL  ,
				buf.length,
				PROT_READ | PROT_WRITE  ,
				MAP_SHARED  ,
				fd, buf.m.offset);
                printf("Addr for %i is %p\n",n_buffers,buffers[n_buffers].start);

		if (MAP_FAILED == buffers[n_buffers].start)
			errno_exit("mmap");
	}
}

void video_driver::init_userp(unsigned int buffer_size)
{
	struct v4l2_requestbuffers req;

	CLEAR(req);

	req.count = 4;
	req.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	req.memory = V4L2_MEMORY_USERPTR;

	if (-1 == xioctl(fd, VIDIOC_REQBUFS, &req))
	{
		if (EINVAL == errno)
		{
			fprintf(stderr, "%s does not support "
					"user pointer i/o\n", dev_name);
			exit(EXIT_FAILURE);
		}
		else
		{
			errno_exit("VIDIOC_REQBUFS");
		}
	}

	buffers = (buffer *)calloc(4, sizeof(*buffers));

	if (!buffers)
	{
		fprintf(stderr, "Out of memory\n");
		exit(EXIT_FAILURE);
	}

	for (n_buffers = 0; n_buffers < 4; ++n_buffers)
	{
		buffers[n_buffers].length = buffer_size;
		buffers[n_buffers].start = malloc(buffer_size);

		if (!buffers[n_buffers].start)
		{
			fprintf(stderr, "Out of memory\n");
			exit(EXIT_FAILURE);
		}
	}
}


void video_driver::init_device(void)
{
	struct v4l2_capability cap;
	struct v4l2_cropcap cropcap;
	struct v4l2_crop crop;
	struct v4l2_format fmt;


	if (-1 == xioctl(fd, VIDIOC_QUERYCAP, &cap))
	{
		if (EINVAL == errno)
		{
			fprintf(stderr, "%s is no V4L2 device\n", dev_name);
			exit(EXIT_FAILURE);
		}
		else
		{
			errno_exit("VIDIOC_QUERYCAP");
		}
	}

	if (!(cap.capabilities & V4L2_CAP_VIDEO_CAPTURE))
	{
		fprintf(stderr, "%s is no video capture device\n", dev_name);
		exit(EXIT_FAILURE);
	}

	switch (io)
	{
	case IO_METHOD_READ:
		if (!(cap.capabilities & V4L2_CAP_READWRITE))
		{
			fprintf(stderr, "%s does not support read i/o\n", dev_name);
			exit(EXIT_FAILURE);
		}

		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		if (!(cap.capabilities & V4L2_CAP_STREAMING))
		{
			fprintf(stderr, "%s does not support streaming i/o\n", dev_name);
			exit(EXIT_FAILURE);
		}

		break;
	}



	cropcap.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

	if (-1 == xioctl(fd, VIDIOC_CROPCAP, &cropcap))
	{

	}

	crop.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	crop.c = cropcap.defrect;

	if (-1 == xioctl(fd, VIDIOC_S_CROP, &crop))
	{
		switch (errno)
		{
		case EINVAL:

			break;
		default:

			break;
		}
	}

	CLEAR(fmt);

	fmt.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
	fmt.fmt.pix.width = width;
	fmt.fmt.pix.height = height;
	fmt.fmt.pix.pixelformat = pixel_format;
	fmt.fmt.pix.field = V4L2_FIELD_INTERLACED;

	if (-1 == xioctl(fd, VIDIOC_S_FMT, &fmt))
		errno_exit("VIDIOC_S_FMT");

        /*struct v4l2_frmivalenum frameinterval;
	width = fmt.fmt.pix.width;
	height = fmt.fmt.pix.height;
        frameinterval.index =0; 
        frameinterval.pixel_format = pixel_format;
        frameinterval.width = width;
        frameinterval.height = height;
        
        frameinterval.discrete.numerator=1;
        frameinterval.discrete.denominator=5;

        
        frameinterval.type = V4L2_FRMIVAL_TYPE_DISCRETE;
        if (-1 == xioctl(fd,VIDIOC_ENUM_FRAMEINTERVALS,&frameinterval)){
            printf("Setting frame interval failed\n");
            exit(0);
        }*/

        struct v4l2_streamparm sp;
        sp.type=V4L2_BUF_TYPE_VIDEO_CAPTURE;
        sp.parm.capture.timeperframe.denominator=30;
        sp.parm.capture.timeperframe.numerator=1;
        if (-1 == xioctl(fd,VIDIOC_S_PARM,&sp)){
            printf("Setting stream parameters failed\n");
            exit(0);
        }


	switch (io)
	{
	case IO_METHOD_READ:
		init_read(fmt.fmt.pix.sizeimage);
		break;

	case IO_METHOD_MMAP:
		init_mmap();
		break;

	case IO_METHOD_USERPTR:
		init_userp(fmt.fmt.pix.sizeimage);
		break;
	}
}
















void video_driver::stop_capturing(void)
{
	enum v4l2_buf_type type;

	switch (io)
	{
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
	case IO_METHOD_USERPTR:
		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMOFF, &type))
			errno_exit("VIDIOC_STREAMOFF");

		break;
	}
}
void video_driver::close_device(void)
{
	if (-1 == close(fd))
		errno_exit("close");

	fd = -1;
}
void video_driver::start_capturing(void)
{
	unsigned int i;
	enum v4l2_buf_type type;

	switch (io)
	{
	case IO_METHOD_READ:
		/* Nothing to do. */
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i)
		{
			struct v4l2_buffer buf;

			CLEAR(buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_MMAP;
			buf.index = i;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}

		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");

		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i)
		{
			struct v4l2_buffer buf;

			CLEAR(buf);

			buf.type = V4L2_BUF_TYPE_VIDEO_CAPTURE;
			buf.memory = V4L2_MEMORY_USERPTR;
			buf.m.userptr = (unsigned long) buffers[i].start;
			buf.length = buffers[i].length;

			if (-1 == xioctl(fd, VIDIOC_QBUF, &buf))
				errno_exit("VIDIOC_QBUF");
		}


		type = V4L2_BUF_TYPE_VIDEO_CAPTURE;

		if (-1 == xioctl(fd, VIDIOC_STREAMON, &type))
			errno_exit("VIDIOC_STREAMON");

		break;
	}
}

void video_driver::uninit_device(void)
{
	unsigned int i;

	switch (io)
	{
	case IO_METHOD_READ:
		free(buffers[0].start);
		break;

	case IO_METHOD_MMAP:
		for (i = 0; i < n_buffers; ++i)
			if (-1 == munmap(buffers[i].start, buffers[i].length))
				errno_exit("munmap");
		break;

	case IO_METHOD_USERPTR:
		for (i = 0; i < n_buffers; ++i)
			free(buffers[i].start);
		break;
	}

	free(buffers);
}




void video_driver::restart_video(){
    stop_capturing();
    uninit_device();
    close_device();

    open_device();
    init_device();
    start_capturing();

}



void video_driver::open_device(void)
{
	struct stat st;

	if (-1 == stat(dev_name, &st))
	{
		fprintf(stderr, "Cannot identify '%s': %d, %s\n",
				dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}

	if (!S_ISCHR(st.st_mode))
	{
		fprintf(stderr, "%s is no device\n", dev_name);
		exit(EXIT_FAILURE);
	}

	fd = open(dev_name, O_RDWR /* required */  | O_NONBLOCK, 0);

	if (-1 == fd)
	{
		fprintf(stderr, "Cannot open '%s': %d, %s\n",
				dev_name, errno, strerror(errno));
		exit(EXIT_FAILURE);
	}
}



video_driver::video_driver(int vw,int vh){
	pixel_format = V4L2_PIX_FMT_YUYV;//V4L2_PIX_FMT_RGB332;//V4L2_PIX_FMT_MJPEG;//V4L2_PIX_FMT_YUYV;
	fd = -1;
	buffers = NULL;
	n_buffers = 1;
	width=vw;
	height=vh;
	dev_name = (char *)"/dev/video0";
        io=IO_METHOD_MMAP;
        




}
