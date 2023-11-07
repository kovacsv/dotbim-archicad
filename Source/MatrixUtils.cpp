#include <MatrixUtils.hpp>

Quaternion::Quaternion (double qx, double qy, double qz, double qw) :
    qx (qx),
    qy (qy),
    qz (qz),
    qw (qw)
{

}

// Without scale
Geometry::Matrix34 ComposeMatrix (const Vector3D& translation, const Quaternion& rotation)
{
    double tx = translation.x;
    double ty = translation.y;
    double tz = translation.z;
    double qx = rotation.qx;
    double qy = rotation.qy;
    double qz = rotation.qz;
    double qw = rotation.qw;

    double x2 = qx + qx;
    double y2 = qy + qy;
    double z2 = qz + qz;
    double xx = qx * x2;
    double xy = qx * y2;
    double xz = qx * z2;
    double yy = qy * y2;
    double yz = qy * z2;
    double zz = qz * z2;
    double wx = qw * x2;
    double wy = qw * y2;
    double wz = qw * z2;

    return Geometry::Matrix34 (
        { (1.0 - (yy + zz)), (xy + wz), (xz - wy) },
        { (xy - wz), (1.0 - (xx + zz)), (yz + wx) },
        { (xz + wy), (yz - wx), (1.0 - (xx + yy)) },
        { tx, ty, tz }
    );
}

// Without scale
// http://www.euclideanspace.com/maths/geometry/rotations/conversions/matrixToQuaternion/index.htm
void DecomposeMatrix (const Geometry::Matrix34& matrix, Vector3D& translation, Quaternion& rotation)
{
    translation = Vector3D (
        matrix.Get (0, 3),
        matrix.Get (1, 3),
        matrix.Get (2, 3)
    );

    double m00 = matrix.Get (0, 0);
    double m01 = matrix.Get (0, 1);
    double m02 = matrix.Get (0, 2);
    double m10 = matrix.Get (1, 0);
    double m11 = matrix.Get (1, 1);
    double m12 = matrix.Get (1, 2);
    double m20 = matrix.Get (2, 0);
    double m21 = matrix.Get (2, 1);
    double m22 = matrix.Get (2, 2);

    double tr = m00 + m11 + m22;
    if (tr > 0.0) {
        double s = sqrt (tr + 1.0) * 2.0;
        rotation = Quaternion (
            (m21 - m12) / s,
            (m02 - m20) / s,
            (m10 - m01) / s,
            0.25 * s
        );
    } else if ((m00 > m11) && (m00 > m22)) {
        double s = sqrt (1.0 + m00 - m11 - m22) * 2.0;
        rotation = Quaternion (
            0.25 * s,
            (m01 + m10) / s,
            (m02 + m20) / s,
            (m21 - m12) / s
        );
    } else if (m11 > m22) {
        double s = sqrt (1.0 + m11 - m00 - m22) * 2.0;
        rotation = Quaternion (
            (m01 + m10) / s,
            0.25 * s,
            (m12 + m21) / s,
            (m02 - m20) / s
        );
    } else {
        double s = sqrt (1.0 + m22 - m00 - m11) * 2.0;
        rotation = Quaternion (
            (m02 + m20) / s,
            (m12 + m21) / s,
            0.25 * s,
            (m10 - m01) / s
        );
    }
}
