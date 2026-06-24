/*
 *  camera.h
 */

#ifndef _CAMERA_H
#define _CAMERA_H 1

#include "vector.h"

struct Camera {
    Vec2 position;
    Vec2 velocity;
    double zoom_speed;
    double scale;
};

struct Mat33 {
    float e[3*3];
};

Mat33 CreateBasisMatrix();
Mat33 CreateCameraMatrix(Camera*);
Mat33 multiply(Mat33, Mat33);
Vec2 multiply(Mat33, Vec2);
void PrintMatrix(Mat33);

#endif

