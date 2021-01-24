#ifndef UTILITY_H_INCLUDED
#define UTILITY_H_INCLUDED
#include <irrlicht.h>
using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace io;
using namespace quake3;
using namespace gui;

int remainder(s32 a, s32 b)
{
    s32 ret=0;
    for(ret = a; ret >= b; ret -= b)
    {

    }
    return ret;
}
s32 blend(s32 c1, s32 c2, s32 t)
{
    s32 ret = 0;
    if(c1 >= c2){
        ret = (c1-c2) * (t*0.01) + c2;
    }else{
        ret = (c2-c1) * ((100-t)*0.01) + c1;
    }
    return ret;
}
#endif // UTILITY_H_INCLUDED
