
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

dvec4 toaxisangle(const dmat4&M)
{
    dvec3 myaxis {0,0,1};
    double myangle=0;
    axisAngle(M, myaxis, myangle);
    return {myaxis.x,myaxis.y,myaxis.z,myangle};
}
