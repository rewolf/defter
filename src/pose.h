/*
 * An implementation of the POSIT algorithm for pose estimation
 * */

#ifndef POSIT_H
#define POSIT_H
class pose{
    public:
        float * local_image_points; // 4 
        float * local_image_vectors; // 4 
        float * local_object_vectors;
        float * local_object_points; // 4
        float focal_length;
        pose();
        void pos(float *ip,float *op,float *iv,float * r,float * tr);
        void posit(float * image_points,float * object_points,float * image_vectors,float * object_vectors,float *r,float *t);
        void compute_object_vectors(float *ov,float *op);
        void compute_pose(float *ip);
};
#endif
