#ifndef BRUSH_H_INCLUDED
#define BRUSH_H_INCLUDED
#include "brushdef.h"
#include "utility.h"
#include <irrlicht.h>
using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace io;
using namespace quake3;
using namespace gui;
class Brush
{
    public:
    brush(){ibrush_t = 0;}
    bool verify(){if(ibrush_t == 0){return false;}}
    //Draw Texture onto terrain
    void draw(ITerrainSceneNode *terrain, IImage *a, ITexture *b, s32 brushSize,s32 x, s32 z, s32 t, IrrlichtDevice *device, IVideoDriver *driver ){
        //TODO: Paint.
        const double PI = 3.14159;
        double loss = 255 / brushSize;
        SColor color(0,0,0,0);
        SColor color2(0,0,0,0);
        stringw twi=L"Width: ", the=L"Height: ";
        twi += ibrush_t->getDimension().Width;
        the += ibrush_t->getDimension().Height;
        device->getLogger()->log(twi.c_str(), ELL_INFORMATION);
        device->getLogger()->log(the.c_str(), ELL_INFORMATION);
        for(double r = 0; r <= brushSize/20; r++){
            for(double angle=0; angle <= 2*PI; angle +=0.001){
                s32 bx = a->getDimension().Width - x + r*cos(angle);
                s32 bz = z + r*sin(angle);
                //stringw bxs = "X: ";
                //bxs += bx;
                //device->getLogger()->log(bxs.c_str(), ELL_INFORMATION);
                color = ibrush_t->getPixel(remainder(bx, (int)ibrush_t->getDimension().Width),remainder(bz, (int)ibrush_t->getDimension().Height));
                color2 = a->getPixel(bx, bz);
                //tempBrush->setPixel(center.X + r*cos(angle), center.Y + r*sin(angle), color, false);
                SColor color3(0,0,0,0);




                    a->setPixel(bx, bz, color);
                }

            }
            //Iter->setPixel(512 - x, z, SColor(255,red,green,blue));
            ITexture *old = b;
            // old->drop(); will cause the program to throw an exception.
            driver->removeTexture(old);
            b = driver->addTexture("terrain", a);
            terrain->setMaterialTexture(0,b);


    }
    //Paint Terrain
    void draw(ITerrainSceneNode *terrain, IImage *a, SColor color, ITexture *b,s32 x, s32 z, s32 brushSize, IrrlichtDevice *device, IVideoDriver *driver ){
            //TODO: Paint.
            const double PI = 3.14159;
            double loss = 255 / brushSize;

            for(double r = 0; r <= brushSize/20; r++){
                for(double angle=0; angle <= 2*PI; angle +=0.001){
                    s32 bx = a->getDimension().Width - x + r*cos(angle);
                    s32 bz = z + r*sin(angle);

                    //tempBrush->setPixel(center.X + r*cos(angle), center.Y + r*sin(angle), color, false);
                    a->setPixel(bx, bz, color);
                }

            }
            //Iter->setPixel(512 - x, z, SColor(255,red,green,blue));
            ITexture *old = b;
            // old->drop(); will cause the program to throw an exception.
            driver->removeTexture(old);
            b = driver->addTexture("terrain", a);
            terrain->setMaterialTexture(0,b);
    }
    //Raise/lower vertex
    void draw(ITerrainSceneNode *terrain, s32 index, f32 step, bool up, s32 heightmapWidth, s32 heightmapHeight, s32 brushSize, s32 strength, IrrlichtDevice *device){
        scene::IMesh* pMesh = terrain->getMesh();
       if(!up){
            device->getLogger()->log("NOT UP", ELL_INFORMATION);
       }
        //s32 heightmapWidth = heightmap->getDimension().Width;
        //s32 heightmapHeight = heightmap->getDimension().Height;

        s32 b;
        for (b=0; b<pMesh->getMeshBufferCount(); ++b){
            IMeshBuffer* pMeshBuffer = pMesh->getMeshBuffer(b);
            // skip mesh buffers that are not the right type
            if (pMeshBuffer->getVertexType() != video::EVT_2TCOORDS) continue;

            S3DVertex2TCoords* pVertices = (S3DVertex2TCoords*)pMeshBuffer->getVertices();

            s32 brushWidth = ibrush_t->getDimension().Width;
            s32 brushHeight = ibrush_t->getDimension().Height;

            for(int y = 0; y < brushHeight; y++){
                for(int x = 0; x < brushWidth; x++){

                    SColor brushPixel = ibrush_t->getPixel(x, y);

                    if((index-(brushWidth/2)-((brushWidth/2)*heightmapWidth) + (x+(y*heightmapWidth))) >= 0){
                        f32 hy = pVertices[index-(brushWidth/2)-((brushWidth/2)*heightmapWidth) + (x+(y*heightmapWidth))].Pos.Y;
                        f32 bp = brushPixel.getRed()/255.0*step;
                        bp = (up)?bp:-bp;
                        if(up)
                        {
                            if(bp > 0 && hy+bp+strength <= 255)
                            pVertices[index-(brushWidth/2)-((brushWidth/2)*heightmapWidth) + (x+(y*heightmapWidth))].Pos.Y = hy+bp;
                        }
                        if(!up)
                        {
                            if(hy+bp+strength >=0 && hy+bp+strength <= 255)
                            pVertices[index-(brushWidth/2)-((brushWidth/2)*heightmapWidth) + (x+(y*heightmapWidth))].Pos.Y = hy+bp;
                        }
                }
            }
            }
        }

        // force terrain render buffer to reload
        terrain->setPosition(terrain->getPosition());
    }
    bool setBrush(IImage *a)
    {
        if(a != 0)
        {
            ibrush_t = a;
        }
    }
    void setType(brush_t a)
    {

        type = a;
    }
    brush_t getType(){return type;}
    IImage *ibrush_t;
    brush_t  type;
    s32 lx;
    s32 lz;
};
#endif // BRUSH_H_INCLUDED
