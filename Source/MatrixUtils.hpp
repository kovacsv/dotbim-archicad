#pragma once

#include <Matrix34.hpp>

class Quaternion
{
public:
    Quaternion (double qx, double qy, double qz, double qw);

    double qx;
    double qy;
    double qz;
    double qw;
};

Geometry::Matrix34 ComposeMatrix (const Vector3D& translation, const Quaternion& rotation);
void DecomposeMatrix (const Geometry::Matrix34& tranmat, Vector3D& translation, Quaternion& rotation);
