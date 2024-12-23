
#include <string>
#include <vector>
#include <cmath>
#include "bvhhelp.h"

using namespace std;

void dumphumanoid_x3d(const vector<hanimjoint>&JOINTS)
{
  const auto NJ=JOINTS.size();
  int merklev=0;
  unsigned nummat=0;
  for (unsigned n=0; n<NJ; n++)
  {
    const hanimjoint&J=JOINTS[n];
    const unsigned t=type(J);
    const int lev=level(J);
    const double*T=offset(J);
    const char*nam=name(J);

    // Upon rising up again in the hierarchy, close open joints.
    bool up=n>0 && lev<merklev;
    if (up)
    {
      for (int d=merklev; d>=lev; d--)
        printf("\n%*s</%s>", 2*d, "", d>0?"Transform":"Transform");
    }

    switch (t)
    {
      // Joints and End Sites
      case 1:
      case 2:
      {
        switch (segmentform)
        {
          case 0: break;
          case 1:
          {
            // Draw a line
            if (T!=nullptr) printf("\n%*s<Shape><LineSet vertexCount='2'><Coordinate point='0 0 0,%g %g %g'/></LineSet></Shape>", 2*lev, "", T[0],T[1],T[2]);
            break;
          }
          case 2:
          {
            // Zeichne einen Zylinder
            if (T!=nullptr)
            {
              const double hei=sqrt(T[0]*T[0]+T[1]*T[1]+T[2]*T[2]);
              const double rad=0.1*sqrt(T[0]*T[0]+T[1]*T[1]+T[2]*T[2]);
              const double xangle=atan2(T[2],T[1]), zangle=atan2(T[0],T[1]);
              if (xangle!=0 || zangle!=0)
              {
                const double ANGLE[2]={xangle,zangle};
                const int DIR[2]={0,2};
                double axis[3], angle;
                AnglesToAxisAngle(axis,&angle,ANGLE,DIR,2);
                printf("\n<Transform translation='%g %g %g' rotation='%g %g %g %g'>"
                , .5*T[0],.5*T[1],.5*T[2]
                , axis[0],axis[1],axis[2], angle);
              }
              else
              {
                printf("\n<Transform translation='%g %g %g'>", .5*T[0],.5*T[1],.5*T[2]);
              }
              if (nummat==0) printf("\n<Shape><Appearance DEF='app1'><Material DEF='mat1' diffuseColor='1 1 1' specularColor='1 1 1' shininess='1.0'/></Appearance>");
              else           printf("\n<Shape><Appearance USE='app1'/>");
              printf("<Cylinder height='%g' radius='%g'/></Shape>", hei, rad);
              printf("</Transform>");
              nummat++;
            }
            break;
          }
        }
        break;
      }
    }

    printf("\n%*s<%s", 2*lev, "", lev>0?"Transform":"Transform");
    if (T!=nullptr) printf(" translation='%g %g %g'", T[0],T[1],T[2]);
    if (nam!=nullptr)
    {
      printf(" DEF='%s'>", nam);
    }
    else
    {
      printf(">");
    }
    const double rad=T==nullptr?1:0.10*sqrt(T[0]*T[0]+T[1]*T[1]+T[2]*T[2]);
    if (nummat==0)
    {
      printf("<Shape><Appearance DEF='app1'><Material DEF='mat1' diffuseColor='1 1 1' specularColor='1 1 1' shininess='1.0'/></Appearance><Sphere radius='%g'/></Shape>", rad);
    }
    else
    {
      printf("<Shape><Appearance USE='app1'/><Sphere radius='%g'/></Shape>", rad);
    }
    nummat++;
    merklev=level(J);
  }
  for (int d=merklev; d>=0; d--) printf("\n%*s</%s>", 2*d, "", d>0?"Transform":"Transform");
}

void dumpmotiontable_x3d(const vector<hanimjoint>&joints, const vector<vector<double>>&Table)
{
    if (Table.size()>0)
    {
        printf("\n<!-- Interpolators -->");
        for (const auto&j: joints) j.dumpmotiontables_x3d(Table);
    }
}

void dumpmotionroutes_x3d(const vector<hanimjoint>&joints)
{
    printf("\n<!-- Routes -->");
    for (const auto&j: joints) j.dumpmotionroutes_x3d();
}
