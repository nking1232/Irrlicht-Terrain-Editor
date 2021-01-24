/* Some Code(Most) from Luiz snippet here:
http://irrlicht.sourceforge.net/forum/viewtopic.php?t=44757
was used which also happens to be based on Katsankat's post here:
http://irrlicht.sourceforge.net/forum/viewtopic.php?f=9&t=32251
I take no credit for the terrain editing code I am simply trying to turn
it into a free easy to use editor A big thanks goes to every from who'm I
borrowed code and the irrlicht tutorials for the gui stuff*/

#include <irrlicht.h>
#include <cmath>
#include <ctime>

#include "driverChoice.h"
#include "Terrain.h"
#include "brushdef.h"
#include "brush.h"
#include "utility.h"
using namespace irr;
using namespace core;
using namespace video;
using namespace scene;
using namespace io;
using namespace quake3;
using namespace gui;


list<Brush*> brushes;

IImage* heightmap = 0;
IImage* brush = 0;
IImage* Check_pat = 0;
ITerrainSceneNode* terrain = 0;
IrrlichtDevice* device = 0;
IVideoDriver* driver = 0;
ISceneManager* smgr = 0;
IGUIEnvironment *env = 0;
ITriangleSelector* selector = 0;
int brushSize = 16;
path openTerrain("basemap.bmp");
f32 step = 2;
int strength = 1;
IGUICheckBox *paintCheck = 0;
IGUICheckBox *texPaintCheck = 0;
s32 red=100, green=100, blue=100;
s32 toolbarState = 1;
Brush *TOOLBAR_1;
Brush *TOOLBAR_2;
Brush *TOOLBAR_3;
Brush *TOOLBAR_4;
Brush *TOOLBAR_5;
Brush *TOOLBAR_6;
Brush *TOOLBAR_7;
Brush *TOOLBAR_8;
Brush *TOOLBAR_9;
brush_t active_brush = BRUSH_RADIAL_GRAD;
enum
{
    GUI_ID_OPEN_FILE_TERRAIN = 100,
    GUI_ID_FAR = 101,
    GUI_ID_SAVE = 102,
    GUI_ID_WIREFRAME = 103,
    GUI_ID_TOOLBOX_WINDOW = 104,
    GUI_ID_TERRAIN_OPEN = 105,
    GUI_ID_TERTEX_OPEN = 106,
    GUI_ID_OPEN_FILE_TEXTURE = 107,
    GUI_ID_QUIT = 108,
    GUI_ID_BRUSHSIZE = 109,
    GUI_ID_BRUSHSTRENGTH = 110,
    GUI_ID_NEWTERRAIN = 111,
    GUI_ID_BRUSH_RADIAL = 112,
    GUI_ID_BRUSH_NOISE = 113,
    GUI_ID_COLOR = 114,
    GUI_ID_PAINT = 115,
    GUI_ID_PAINT_RED = 116,
    GUI_ID_PAINT_GREEN = 117,
    GUI_ID_PAINT_BLUE = 118,
    GUI_ID_BRUSH_TOOLBAR = 119,
    GUI_ID_TOOLBAR_1 = 120,
    GUI_ID_TOOLBAR_2 = 121,
    GUI_ID_TOOLBAR_3 = 122,
    GUI_ID_TOOLBAR_4 = 123,
    GUI_ID_TOOLBAR_5 = 124,
    GUI_ID_TOOLBAR_6 = 125,
    GUI_ID_TOOLBAR_7 = 126,
    GUI_ID_TOOLBAR_8 = 127,
    GUI_ID_TOOLBAR_9 = 128
};
void save(IVideoDriver*);

//Generates white noise for the random brush(very slow).
void genWhiteNoise(int bsize)
{
    IImage *tempBrush = 0;
    tempBrush = driver->createImage(ECF_A8R8G8B8, dimension2d<u32>(bsize, bsize));
    tempBrush->fill(SColor(255,0,0,0));
    SColor color(255,255,255,255);
    SColor color2(255,0,0,0);
    for(int w = 0; w <= bsize; w++)
    {
        for(int h = 1; h <= bsize; h++)
        {
            srand(device->getTimer()->getTime() % (rand()+ w% 10000)/2 + (rand()+ h% 10000)/2);
            int x = rand() % 2;
            color.setRed(x? 255 : 0);
            color.setGreen(x? 255 : 0);
            color.setBlue(x? 255 : 0);
            tempBrush->setPixel(w, h, color, false);


        }
    }
    if(brush)
        brush->drop();
    brush = tempBrush;
    //driver->writeImageToFile(tempBrush, "test.png", 0);
}
//Generates brushes for use with the editor
IImage *generateBrush(int radius)
{
    IImage* tempBrush = 0;
    tempBrush = driver->createImage(ECF_A8R8G8B8, dimension2d<u32>(radius*2,radius*2));
    tempBrush->fill(SColor(255,0,0,0));
    position2di center((radius*2)/2,(radius*2)/2);
    const double PI = 3.14159;
    double loss = 255 / radius;
    SColor color(255,255,255,255);
    for(double r = 0; r <= radius; r++)
    {
        for(double angle=0; angle <= 2*PI; angle +=0.001)
        {
            tempBrush->setPixel(center.X + r*cos(angle), center.Y + r*sin(angle), color, false);
        }
        if(color.getRed() > 0)
        color.setRed(color.getRed() - loss);
        if(color.getGreen() > 0)
        color.setGreen(color.getGreen() - loss);
        if(color.getBlue() > 0)
        color.setBlue(color.getBlue() - loss);
    }

    return (tempBrush);
    //driver->writeImageToFile(tempBrush, "test.png", 0);
}
//Loads a terrain although the save function just outputs to basemap.bmp
void load(const c8* fn, IGUIFileOpenDialog* dialog)
{
    path filename(fn);

    path extension;
    getFileNameExtension(extension, filename);
    extension.make_lower();

    if(extension == ".bmp" || extension == ".jpg" ||
        extension == ".png")
    {
        device->getLogger()->log(fn, ELL_INFORMATION);
        s32 id = dialog->getID();
        switch(id)
        {
        case GUI_ID_TERRAIN_OPEN:
            //Open our terrain
            openTerrain = filename;
            if(terrain)
            {
                terrain->remove();
                terrain = smgr->addTerrainSceneNode(openTerrain, 0, -1, core::vector3df(0, 0, 0));
                terrain->setScale(core::vector3df(32, 5, 32));
                terrain->setMaterialFlag(video::EMF_LIGHTING, false);

                terrain->setPosition(terrain->getPosition());
                selector->drop();
                selector = smgr->createTerrainTriangleSelector(terrain, 0);
            }
            break;
        case GUI_ID_TERTEX_OPEN:
            //Open the terrains texture
            terrain->setMaterialTexture(0, driver->getTexture(filename));
            break;
        default:
            break;
        }
    }

}
//Handler for menu items
void OnMenuItemSelected(IGUIContextMenu* menu)
{
    s32 id = menu->getItemCommandId(menu->getSelectedItem());
    switch (id)
    {
    case GUI_ID_OPEN_FILE_TERRAIN:
        env->addFileOpenDialog(L"Please select a terrain file",true,0,GUI_ID_TERRAIN_OPEN);
        break;
    case GUI_ID_OPEN_FILE_TEXTURE:
        env->addFileOpenDialog(L"Please select a texture for your terrain", true, 0, GUI_ID_TERTEX_OPEN);
        break;
    case GUI_ID_FAR:
            break;
    case GUI_ID_SAVE:
        save(driver);
        break;
    case GUI_ID_QUIT:
        device->closeDevice();
        break;
    case GUI_ID_WIREFRAME:
        terrain->setMaterialFlag(EMF_WIREFRAME, !terrain->getMaterial(0).Wireframe);
    case GUI_ID_NEWTERRAIN:
         IImage* tempBrush;
        tempBrush = driver->createImage(ECF_A8R8G8B8, dimension2d<u32>(512,512));
        tempBrush->fill(SColor(255,0,0,0));
        driver->writeImageToFile(tempBrush, "basemap.bmp", 0);
        openTerrain = "basemap.bmp";
        terrain->remove();
        terrain = smgr->addTerrainSceneNode(openTerrain, 0, -1, core::vector3df(0, 0, 0));
        terrain->setScale(core::vector3df(32, 5, 32));
        terrain->setMaterialFlag(video::EMF_LIGHTING, false);
        terrain->setPosition(terrain->getPosition());
        selector->drop();
        selector = smgr->createTerrainTriangleSelector(terrain, 0);

        break;
    default:
        break;
    }
}
/*==============================================================================
  Receiver class
==============================================================================*/
class MyEventReceiver : public IEventReceiver
{
    bool KeyIsDown[KEY_KEY_CODES_COUNT];
    bool LEFTBUTTONCLICKED;
    bool RIGHTBUTTONCLICKED;

    public:
        virtual bool OnEvent(const SEvent& event)
        {
            if (event.EventType == EET_KEY_INPUT_EVENT)
                KeyIsDown[event.KeyInput.Key] = event.KeyInput.PressedDown;

            if(event.EventType == EET_MOUSE_INPUT_EVENT){
                if(event.MouseInput.Event==EMIE_LMOUSE_PRESSED_DOWN) LEFTBUTTONCLICKED = true;
                else if(event.MouseInput.Event==EMIE_LMOUSE_LEFT_UP) LEFTBUTTONCLICKED = false;
                else if(event.MouseInput.Event==EMIE_RMOUSE_PRESSED_DOWN) RIGHTBUTTONCLICKED = true;
                else if(event.MouseInput.Event==EMIE_RMOUSE_LEFT_UP) RIGHTBUTTONCLICKED = false;
            }

            if(event.EventType == EET_GUI_EVENT)
            {
                s32 id = event.GUIEvent.Caller->getID();
                switch(event.GUIEvent.EventType)
                {
                case EGET_MENU_ITEM_SELECTED:
                    OnMenuItemSelected((IGUIContextMenu*)event.GUIEvent.Caller);
                    break;
                case EGET_FILE_SELECTED:
                    load(core::stringc(((IGUIFileOpenDialog*)event.GUIEvent.Caller)->getFileName()).c_str(), (IGUIFileOpenDialog*)event.GUIEvent.Caller);
                    break;
                case EGET_SCROLL_BAR_CHANGED:
                    if(id == GUI_ID_FAR)
                    {
                         const s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        smgr->getActiveCamera()->setFarValue(pos * 100);
                    }else if(id == GUI_ID_BRUSHSIZE){
                        const s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();

                        if(pos/2 > 0)
                            brushSize = pos;
                        if(active_brush == BRUSH_RADIAL_GRAD)
                            generateBrush(pos/2);
                        else if(active_brush == BRUSH_WHITE_NOISE)
                            genWhiteNoise(pos/2);
                    }else if(id == GUI_ID_BRUSHSTRENGTH){
                        const s32 pos = ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                        step = pos;
                    }else if(id == GUI_ID_PAINT_RED){
                        red = 2 * ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                    }else if(id == GUI_ID_PAINT_GREEN){
                        green = 2 * ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                    }else if(id == GUI_ID_PAINT_BLUE){
                        blue = 2 * ((IGUIScrollBar*)event.GUIEvent.Caller)->getPos();
                    }
                    break;
                case EGET_BUTTON_CLICKED:
                    switch(id)
                    {
                    case GUI_ID_BRUSH_RADIAL:
                        active_brush = BRUSH_RADIAL_GRAD;
                        generateBrush(brushSize);
                        break;
                    case GUI_ID_BRUSH_NOISE:
                        active_brush = BRUSH_WHITE_NOISE;
                        genWhiteNoise(brushSize);
                        break;
                    default:
                        break;
                    }
                    break;
                default:
                    break;
                }
            }

            return false;
        }

        virtual bool IsKeyDown(EKEY_CODE keyCode) const { return KeyIsDown[keyCode]; }
        virtual bool IsLMBDown() const { return LEFTBUTTONCLICKED; }
        virtual bool IsRMBDown() const { return RIGHTBUTTONCLICKED; }

        MyEventReceiver(){
            for (u32 i=0; i<KEY_KEY_CODES_COUNT; ++i)
                KeyIsDown[i] = false;

            LEFTBUTTONCLICKED = RIGHTBUTTONCLICKED = false;
        }
};


/*==============================================================================
  Save file
==============================================================================*/
void save (IVideoDriver* driver){
    //We get the dimensions of the height map
    s32 heightmapWidth = heightmap->getDimension().Width;
    s32 heightmapHeight = heightmap->getDimension().Height;
    //Then we generate a image from those dimensions
    const dimension2d<u32> dim (heightmapWidth, heightmapHeight);
    IImage *img = driver->createImage(ECF_A8R8G8B8, dim);
    //Next we get all of the Vertices from the terrain
    S3DVertex2TCoords* verts = (S3DVertex2TCoords*)terrain->getMesh()->getMeshBuffer(0)->getVertices();
    //Then translate them into a 2d image.
    for (u32 y= 0, i = 0; y < heightmapHeight; y++){
        for(u32 x = 0; x < heightmapWidth; i++, x++){
            u8 py = (u8)verts[i].Pos.Y;
            img->setPixel((heightmapHeight-1)-y, x, SColor(0, py, py, py));
        }
    }
    //Next we save our image and clear it from memory.
    driver->writeImageToFile(img, openTerrain, 0);
    img->drop();
}
//Builds our gui so things look pretty ^p^
void buildGUI()
{
    //We set up our Menu here.
    IGUIContextMenu* menu = env->addMenu();
    menu->addItem(L"File", -1, true, true);
    menu->addItem(L"View", -1, true, true);
    menu->getSubMenu(0)->addItem(L"New", GUI_ID_NEWTERRAIN);
    menu->getSubMenu(0)->addItem(L"Open Terrain File", GUI_ID_OPEN_FILE_TERRAIN);
    menu->getSubMenu(0)->addItem(L"Open Terrain Texture", GUI_ID_OPEN_FILE_TEXTURE);
    menu->getSubMenu(0)->addItem(L"Save", GUI_ID_SAVE);
    menu->getSubMenu(0)->addItem(L"Quit", GUI_ID_QUIT);
    menu->getSubMenu(1)->addItem(L"Wireframe", GUI_ID_WIREFRAME);
    IGUIElement* root = env ->getRootGUIElement();
    IGUIElement* e = root->getElementFromId(GUI_ID_TOOLBOX_WINDOW, true);
    if(e)
    {
        e->remove();
    }
    IGUIWindow* wnd = env->addWindow(rect<s32>(600,45,800,480),false,L"Toolbox",0,GUI_ID_TOOLBOX_WINDOW);

    wnd->setDraggable(false);

    ITexture* icon = driver->getTexture("Radial_Gradient_Icon.png");
    IGUIButton* button = env->addButton(rect<s32>(10,115,45,150),wnd,GUI_ID_BRUSH_RADIAL,0,L"Radial Gradient Brush(Fading Circle");
    button->setImage(icon);
    icon = driver->getTexture("Noise_Icon.png");
    button = env->addButton(rect<s32>(50,115,85,150),wnd,GUI_ID_BRUSH_NOISE,0,L"Noise Brush");
    button->setImage(icon);
    IGUIScrollBar* scrollbar = env->addScrollBar(true, rect<s32>(10,30,190,50),wnd, GUI_ID_FAR);
    env->addStaticText(L"Camera Far Value:", rect<s32>(10,20,80,45), false, false, wnd);
    IGUIScrollBar* brushSize = env->addScrollBar(true, rect<s32>(10,60,190,80),wnd, GUI_ID_BRUSHSIZE);
    env->addStaticText(L"Brush Size:", rect<s32>(10,50,80,90), false, false, wnd);
    IGUIScrollBar* brushStrength = env->addScrollBar(true, rect<s32>(10,90,190,110),wnd, GUI_ID_BRUSHSTRENGTH);
    env->addStaticText(L"Brush Strength:", rect<s32>(10,80,80,120), false, false, wnd);
    paintCheck = env->addCheckBox(false, rect<s32>(10, 160, 80, 180), wnd, GUI_ID_PAINT,L"Paint Terrain");
    IGUIScrollBar* brushRed = env->addScrollBar(true, rect<s32>(10,180,190,200),wnd, GUI_ID_PAINT_RED);
    IGUIScrollBar* brushGreen = env->addScrollBar(true, rect<s32>(10,200,190,220),wnd, GUI_ID_PAINT_GREEN);
    IGUIScrollBar* brushBlue = env->addScrollBar(true, rect<s32>(10,220,190,240),wnd, GUI_ID_PAINT_BLUE);
    scrollbar->setMax(200);
    scrollbar->setPos(100);
    brushSize->setMax(128);
    brushSize->setMin(16);
    brushSize->setPos(16);
    brushStrength->setMax(50);
    brushStrength->setMin(1);
    brushStrength->setPos(1);
    IGUIWindow* brushBar = env->addWindow(rect<s32>(0,500,800,600),false,L"Toolbar",0,GUI_ID_BRUSH_TOOLBAR);
    brushBar->setDrawTitlebar(0);
    brushBar->setDraggable(false);
    brushBar->getCloseButton()->remove();
    env->addButton(rect<s32>(10,10,100,100),brushBar,GUI_ID_TOOLBAR_1,0,L"Slot1");
}
MyEventReceiver receiver;


int main()
{
    device = createDevice(EDT_OPENGL, dimension2d<u32>(800, 600), 32, false, true, false, &receiver);
    if(!device)
        return 1;
    driver = device->getVideoDriver();
    if(!driver)
        return 1;
    smgr = device->getSceneManager();
    if(!smgr)
        return 1;
    env = device->getGUIEnvironment();
    if(!smgr)
        return 1;
    device->getCursorControl()->setVisible(false);

    env->addStaticText(
        L"Press F1 to save\nPress W for wireframe\n Click anywhere to raise the terrain",
        rect<s32>(10,421,250,475), true, true, 0, -1, true);
    heightmap = driver->createImageFromFile(openTerrain);
    if(!heightmap)
        return 1;

    terrain = smgr->addTerrainSceneNode(openTerrain, 0, -1, core::vector3df(0, 0, 0));
    if(!terrain)
        return 1;
    terrain->setScale(core::vector3df(32, 5, 32));
    terrain->setMaterialFlag(video::EMF_LIGHTING, false);

    terrain->setPosition(terrain->getPosition());
    IImage *Iter = driver->createImageFromFile("rockwall.jpg");
    Check_pat = driver->createImageFromFile("natfl190.jpg");
    generateBrush(32);
    TOOLBAR_1 = new Brush();
    TOOLBAR_1->setBrush(driver->createImageFromFile("rock006.jpg"));
    TOOLBAR_1->setType(BRUSH_TEXTURE);
    TOOLBAR_2 = new Brush();
    TOOLBAR_2->setType(BRUSH_PAINT);
    TOOLBAR_3 = new Brush();
    TOOLBAR_3->setBrush(generateBrush(32));
    TOOLBAR_3->setType(BRUSH_TERRAIN);
    ITexture *ter = driver->addTexture("terrain",Iter);
    terrain->setMaterialTexture(0, ter);
    selector = smgr->createTerrainTriangleSelector(terrain, 0);

    // Arrow
    ISceneNode* arrow = smgr->addAnimatedMeshSceneNode(smgr->addArrowMesh("arrow", SColor(255, 255, 0, 0), SColor(255, 0, 255, 0)), NULL);
    arrow->setMaterialFlag(video::EMF_LIGHTING, false);
    arrow->setScale(vector3df(100, 100, 100));
    arrow->setRotation(vector3df(0,0,180));

    ICameraSceneNode* cam = smgr->addCameraSceneNodeFPS(0, 100.0f, 3.0f);
    cam->setPosition(vector3df(-100,500,100));
    cam->setTarget(vector3df(0,0,0));
    cam->setFarValue(10000);
    generateBrush(32);
    buildGUI();
    ITimer* irrTimer = device->getTimer();
    u32 then = 0, then30 = 0;
    char c[24];
    bool wireframe = false;
    int lastFPS = -1;
    while(device->run()){
        if(device->isWindowActive()){
            u32 now = irrTimer->getTime();

            if (then30 < now){
                //We need to break the while loop if we press escape which will in turn close the program
                if(receiver.IsKeyDown(KEY_ESCAPE)) break;

                if (receiver.IsKeyDown(KEY_KEY_W) && then < now){
                    device->getLogger()->log("KEY_W", ELL_INFORMATION);
                    wireframe = !wireframe;
                    terrain->setMaterialFlag(EMF_WIREFRAME, wireframe);

                    then = now + 300;
                }
                //We adjust the brush speed if F4 or F5 is pressed down(Not used)
                if (receiver.IsKeyDown(KEY_F4) && then < now){
                    device->getLogger()->log("KEY_F4", ELL_INFORMATION);
                    step += 1.f;

                    then = now + 100;
                } else if (receiver.IsKeyDown(KEY_F5) && then < now && step > 0){
                    device->getLogger()->log("KEY_F5", ELL_INFORMATION);
                    step -= 1.f;

                    then = now + 100;
                }
                //We check if the save button(F1) is pressed down
                if(receiver.IsKeyDown(KEY_F1))
                {
                    device->getLogger()->log("KEY_F1", ELL_INFORMATION);
                    save (driver);
                }
                if(receiver.IsKeyDown(KEY_KEY_T))
                {
                    //Disable the input receiver for our fps camera scene node which releases the mouse.
                    cam->setInputReceiverEnabled( !cam->isInputReceiverEnabled() );
                    //We set our cursor to visible again.
                    device->getCursorControl()->setVisible( !device->getCursorControl()->isVisible() );
                }
                if(receiver.IsKeyDown(KEY_KEY_1)){
                    toolbarState = 1;
                }else if(receiver.IsKeyDown(KEY_KEY_2)){
                    toolbarState = 2;
                }else if(receiver.IsKeyDown(KEY_KEY_3)){
                    toolbarState = 3;
                }else if(receiver.IsKeyDown(KEY_KEY_4)){
                    toolbarState = 4;
                }else if (receiver.IsKeyDown(KEY_KEY_5)){
                    toolbarState = 5;
                }else if(receiver.IsKeyDown(KEY_KEY_6)){
                    toolbarState = 6;
                }else if(receiver.IsKeyDown(KEY_KEY_7)){
                    toolbarState = 7;
                }else if(receiver.IsKeyDown(KEY_KEY_8)){
                    toolbarState = 8;
                }else if(receiver.IsKeyDown(KEY_KEY_9)){
                    toolbarState = 9;
                }
                // move the arrow to the nearest vertex ...
                //400, 300 si la résolution utilisée est 800x600
                const position2di clickPosition = position2di(400, 300);
                const line3d<float> ray = smgr->getSceneCollisionManager()->getRayFromScreenCoordinates(clickPosition, cam);
                vector3df pos;
                triangle3df Tri;

                ISceneNode* out;
                if (smgr->getSceneCollisionManager()->getCollisionPoint(ray, selector, pos, Tri, out)){
                    //arrow->setPosition(pos);
                    static const s32 scale = 32; // terrain is scaled 32X
                    static const s32 size = 512; // heightmap is 512x512 pixels
                    s32 x = (s32)(pos.X / scale);
                    s32 z = (s32)(pos.Z / scale);
                    s32 index = x * size + z;

                    // ... Move it if clicked
                    if(receiver.IsLMBDown() || receiver.IsRMBDown() && then < now){
                            if(device->getCursorControl()->isVisible())
                            {
                                /*We do nothing if or mouse is visible
                                this helps prevent changes from being made while using the menus*/
                            }
                            else
                            {
                                if(receiver.IsLMBDown())
                                    device->getLogger()->log("Left mouse button down", ELL_INFORMATION);
                                    if(receiver.IsRMBDown()){
                                        device->getLogger()->log("Right mouse button down", ELL_INFORMATION);
                                    }
                                switch (toolbarState)
                                {
                                case 1:
                                    switch(TOOLBAR_1->getType()){

                                    case BRUSH_TEXTURE:
                                        TOOLBAR_1->draw(terrain,Iter,ter,brushSize ,x, z, 12, device, driver);
                                        break;
                                    case BRUSH_PAINT:
                                        TOOLBAR_1->draw(terrain, Iter, SColor(255, red, green, blue), ter, x, z, brushSize, device, driver );
                                        break;
                                    case BRUSH_TERRAIN:
                                        TOOLBAR_1->draw(terrain, index, step, receiver.IsLMBDown(), heightmap->getDimension().Width, heightmap->getDimension().Height, brushSize, strength, device);
                                    default:
                                        break;
                                    }
                                    break;
                                case 2:
                                    switch(TOOLBAR_2->getType()){
                                    case BRUSH_TEXTURE:
                                        TOOLBAR_2->draw(terrain,Iter,ter,brushSize ,x, z, 12, device, driver);
                                        break;
                                    case BRUSH_PAINT:
                                        TOOLBAR_2->draw(terrain, Iter, SColor(255, red, green, blue), ter, x, z, brushSize, device, driver );
                                        break;
                                    case BRUSH_TERRAIN:
                                        TOOLBAR_2->draw(terrain, index, step, receiver.IsLMBDown(), heightmap->getDimension().Width, heightmap->getDimension().Height, brushSize, strength, device);
                                        break;
                                    default:
                                        break;
                                    }
                                    break;
                                case 3:
                                     switch(TOOLBAR_3->getType()){
                                    case BRUSH_TEXTURE:
                                        TOOLBAR_3->draw(terrain,Iter,ter,brushSize ,x, z, 12, device, driver);
                                        break;
                                    case BRUSH_PAINT:
                                        TOOLBAR_3->draw(terrain, Iter, SColor(255, red, green, blue), ter, x, z, brushSize, device, driver );
                                        break;
                                    case BRUSH_TERRAIN:
                                        TOOLBAR_3->draw(terrain, index, step, receiver.IsLMBDown(), heightmap->getDimension().Width, heightmap->getDimension().Height, brushSize, strength, device);
                                        break;
                                    default:
                                        break;
                                    }
                                    break;
                                case 4:
                                     switch(TOOLBAR_2->getType()){
                                    case BRUSH_TEXTURE:
                                        TOOLBAR_2->draw(terrain,Iter,ter,brushSize ,x, z, 12, device, driver);
                                        break;
                                    case BRUSH_PAINT:
                                        TOOLBAR_2->draw(terrain, Iter, SColor(255, red, green, blue), ter, x, z, brushSize, device, driver );
                                        break;
                                    case BRUSH_TERRAIN:
                                        TOOLBAR_2->draw(terrain, index, step, receiver.IsLMBDown(), heightmap->getDimension().Width, heightmap->getDimension().Height, brushSize, strength, device);
                                        break;
                                    default:
                                        break;
                                    }
                                    break;
                                case 5:
                                     switch(TOOLBAR_2->getType()){
                                    case BRUSH_TEXTURE:
                                        TOOLBAR_2->draw(terrain,Iter,ter,brushSize ,x, z, 12, device, driver);
                                        break;
                                    case BRUSH_PAINT:
                                        TOOLBAR_2->draw(terrain, Iter, SColor(255, red, green, blue), ter, x, z, brushSize, device, driver );
                                        break;
                                    case BRUSH_TERRAIN:
                                        TOOLBAR_2->draw(terrain, index, step, receiver.IsLMBDown(), heightmap->getDimension().Width, heightmap->getDimension().Height, brushSize, strength, device);
                                        break;
                                    default:
                                        break;
                                    }
                                    break;
                                case 6:
                                     switch(TOOLBAR_2->getType()){
                                    case BRUSH_TEXTURE:
                                        TOOLBAR_2->draw(terrain,Iter,ter,brushSize ,x, z, 12, device, driver);
                                        break;
                                    case BRUSH_PAINT:
                                        TOOLBAR_2->draw(terrain, Iter, SColor(255, red, green, blue), ter, x, z, brushSize, device, driver );
                                        break;
                                    case BRUSH_TERRAIN:
                                        TOOLBAR_2->draw(terrain, index, step, receiver.IsLMBDown(), heightmap->getDimension().Width, heightmap->getDimension().Height, brushSize, strength, device);
                                        break;
                                    default:
                                        break;
                                    }
                                    break;
                                case 7:
                                     switch(TOOLBAR_2->getType()){
                                    case BRUSH_TEXTURE:
                                        TOOLBAR_2->draw(terrain,Iter,ter,brushSize ,x, z, 12, device, driver);
                                        break;
                                    case BRUSH_PAINT:
                                        TOOLBAR_2->draw(terrain, Iter, SColor(255, red, green, blue), ter, x, z, brushSize, device, driver );
                                        break;
                                    case BRUSH_TERRAIN:
                                        TOOLBAR_2->draw(terrain, index, step, receiver.IsLMBDown(), heightmap->getDimension().Width, heightmap->getDimension().Height, brushSize, strength, device);
                                        break;
                                    default:
                                        break;
                                    }
                                    break;
                                case 8:
                                     switch(TOOLBAR_2->getType()){
                                    case BRUSH_TEXTURE:
                                        TOOLBAR_2->draw(terrain,Iter,ter,brushSize ,x, z, 12, device, driver);
                                        break;
                                    case BRUSH_PAINT:
                                        TOOLBAR_2->draw(terrain, Iter, SColor(255, red, green, blue), ter, x, z, brushSize, device, driver );
                                        break;
                                    case BRUSH_TERRAIN:
                                        TOOLBAR_2->draw(terrain, index, step, receiver.IsLMBDown(), heightmap->getDimension().Width, heightmap->getDimension().Height, brushSize, strength, device);
                                        break;
                                    default:
                                        break;
                                    }
                                    break;
                                case 9 :
                                     switch(TOOLBAR_2->getType()){
                                    case BRUSH_TEXTURE:
                                        TOOLBAR_2->draw(terrain,Iter,ter,brushSize ,x, z, 12, device, driver);
                                        break;
                                    case BRUSH_PAINT:
                                        TOOLBAR_2->draw(terrain, Iter, SColor(255, red, green, blue), ter, x, z, brushSize, device, driver );
                                        break;
                                    case BRUSH_TERRAIN:
                                        TOOLBAR_2->draw(terrain, index, step, receiver.IsLMBDown(), heightmap->getDimension().Width, heightmap->getDimension().Height, brushSize, strength, device);
                                        break;
                                    default:
                                        break;
                                    }
                                    break;
                                default:
                                    break;
                                }
                               /* if(paintCheck->isChecked())
                                {
                                    stringw xc = "X:";
                                    xc += 512 - x;
                                    device->getLogger()->log(xc.c_str(), ELL_INFORMATION);
                                    stringw Zc = "Z:";
                                    Zc += z;
                                    device->getLogger()->log(Zc.c_str(), ELL_INFORMATION);
                                    //TODO: Paint.
                                    const double PI = 3.14159;
                                    double loss = 255 / brushSize;
                                    SColor color(255,red,green,blue);
                                    for(double r = 0; r <= brushSize/20; r++)
                                    {
                                        for(double angle=0; angle <= 2*PI; angle +=0.001)
                                        {
                                            s32 bx = 512 - x + r*cos(angle);
                                            s32 bz = z + r*sin(angle);
                                            //color = Check_pat->getPixel(remainder(bx+60, 128),remainder(bz, 128));
                                            //tempBrush->setPixel(center.X + r*cos(angle), center.Y + r*sin(angle), color, false);
                                            Iter->setPixel(512 - x + r*cos(angle), z + r*sin(angle), color);
                                        }

                                    }
                                    //Iter->setPixel(512 - x, z, SColor(255,red,green,blue));
                                    ITexture *old = ter;
                                    // old->drop(); will cause the program to throw an exception.
                                    driver->removeTexture(old);
                                    ter = driver->addTexture("terrain", Iter);
                                    terrain->setMaterialTexture(0, ter);

                                }else{
                                    RaiseTerrainVertex(index, step, receiver.IsLMBDown());
                                    then = now + 100;
                                }*/

                            }
                    }

                    x *= scale;
                    z *= scale;

                    arrow->setPosition(vector3df(x, terrain->getHeight(x, z) + 100, z));
                }
                //Here we do all of the rendering
                driver->beginScene(true, true, 0);
                smgr->drawAll();
                env->drawAll();
                driver->endScene();
                int fps = driver->getFPS();
                if(lastFPS != fps)
                {
                    stringw str = "FPS: ";
                    str += fps;
                    str += " File: ";
                    str += openTerrain;
                    device->setWindowCaption(str.c_str());
                }
                then30 = now + 30;
            }
        }
    }
    //We do general cleanup stuff before we close the program.
    heightmap->drop();
    brush->drop();

    device->closeDevice();
    device->drop();

    return 0;
}
