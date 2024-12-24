
#include <glm/glm.hpp>
#include <glm/gtc/type_precision.hpp>
#include <glm/gtx/euler_angles.hpp>
// #include<glm/gtc/quaternion.hpp>
// #include<glm/gtx/quaternion.hpp>
// #include<glm/gtx/constants.hpp>
// #include<glm/gtx/euler_angles.hpp>
#include <glm/gtx/matrix_interpolation.hpp>

using namespace glm;

#if 0
void ZXYAngleToAxisAngle(double axis[], double*angle, double xangle, double yangle, double zangle)
{
    const dmat4 MX=eulerAngleX(xangle), MY=eulerAngleY(-yangle), MZ=eulerAngleZ(zangle);
    const dmat4 M=MZ*MX*MY;
    dvec3 myaxis;
    double myangle;
    axisAngle(M, myaxis, myangle);
    axis[0]=myaxis.x;
    axis[1]=myaxis.y;
    axis[2]=myaxis.z;
    *angle=myangle;
}

void ZYXAngleToAxisAngle(double axis[], double*angle, double xangle, double yangle, double zangle)
{
    const dmat4 MX=eulerAngleX(xangle), MY=eulerAngleY(-yangle), MZ=eulerAngleZ(zangle);
    const dmat4 M=MZ*MY*MX;
    dvec3 axis2;
    double angle2;
    axisAngle(M, axis2, angle2);
    axis[0]=axis2.x;
    axis[1]=axis2.y;
    axis[2]=axis2.z;
    *angle=angle2;
}
#endif

dvec4 AnglesToAxisAngle(const double A[], const int D[], unsigned num)
{
    dvec3 myaxis {0,0,1};
    double myangle=0;
    switch (num)
    {
        case 1:
        {
            const dmat4 M=D[0]==0?eulerAngleX(A[0]):D[0]==1?eulerAngleY(A[0]):eulerAngleZ(A[0]);
            axisAngle(M, myaxis, myangle);
            break;
        }
        case 2:
        {
            const dmat4 M1=D[0]==0?eulerAngleX(A[0]):D[0]==1?eulerAngleY(A[0]):eulerAngleZ(A[0]);
            const dmat4 M2=D[1]==0?eulerAngleX(A[1]):D[1]==1?eulerAngleY(A[1]):eulerAngleZ(A[1]);
            axisAngle(M1*M2, myaxis, myangle);
            break;
        }
        case 3:
        {
            const dmat4 M1=D[0]==0?eulerAngleX(A[0]):D[0]==1?eulerAngleY(A[0]):eulerAngleZ(A[0]);
            const dmat4 M2=D[1]==0?eulerAngleX(A[1]):D[1]==1?eulerAngleY(A[1]):eulerAngleZ(A[1]);
            const dmat4 M3=D[2]==0?eulerAngleX(A[2]):D[2]==1?eulerAngleY(A[2]):eulerAngleZ(A[2]);
            axisAngle(M1*M2*M3, myaxis, myangle);
            break;
        }
    }
    return {myaxis.x,myaxis.y,myaxis.z,myangle};
}

dvec4 toaxisangle(const dmat4&M)
{
    dvec3 myaxis {0,0,1};
    double myangle=0;
    axisAngle(M, myaxis, myangle);
    return {myaxis.x,myaxis.y,myaxis.z,myangle};
}
