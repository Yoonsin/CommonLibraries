#include <ItemSelectElement.h>
#include <FlexibleFont.h>
#include <FileSystemItemOrganizer.h>
#include <IExtendableSkin.h>
#include <ScrollBarSkinExtension.h>
#include <AggregateSkinExtension.h>
#include <Drawer2D.h>

#include <irrlicht.h>

#include <sstream>
#include <iostream>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class AppSkin : public IExtendableSkin {

	public:
	
	enum SkinIDs{
		DEFAULT_AGGREGATABLE,
		REGULAR_SCROLLBAR,
		REGULAR_AGGREGATION,
		NO_HIGHLIGHT_AGGREGATION,
		INVISIBLE_AGGREGATION,
		LIST_ELE_AGGREGATION,
		ID_COUNT
	};

	AppSkin(irr::IrrlichtDevice* device):
		IExtendableSkin(device->getGUIEnvironment()->createSkin(gui::EGST_WINDOWS_CLASSIC), device){
		registerExtension(new ScrollBarSkinExtension(this, {SColor(255,255,255,255), SColor(255,196,198,201)}, .1f, .3f), REGULAR_SCROLLBAR);
		registerExtension(new AggregateSkinExtension(this, true, true), REGULAR_AGGREGATION);
		registerExtension(new AggregateSkinExtension(this, false, true), NO_HIGHLIGHT_AGGREGATION);
		registerExtension(new AggregateSkinExtension(this, false, false), INVISIBLE_AGGREGATION);
		registerExtension(new AggregateSkinExtension(this, true, false), LIST_ELE_AGGREGATION);
		registerExtension(new DefaultAggregatableSkin(this, true), DEFAULT_AGGREGATABLE);
	}
		
	~AppSkin(){
		parent->drop();//drop required here since parent is created in this constructor
	}
		
};

class IconSource : public IItemSelectIconSource{

	private:
	
	irr::video::ITexture* ascending;
	irr::video::ITexture* descending;
	irr::video::ITexture* mkdir;
	irr::video::ITexture* file;
	irr::video::ITexture* folder;
	
	public:
	
	IconSource(IVideoDriver* driver){
		ascending = driver->getTexture("ascending.png");
		descending = driver->getTexture("descending.png");
		mkdir = driver->getTexture("mkdir.png");
		file = driver->getTexture("file.png");
		folder = driver->getTexture("folder.png");
	}
	
	irr::video::ITexture* getAscendingIcon(){
		return ascending;
	}
	
	irr::video::ITexture* getDescendingIcon(){
		return descending;
	}
	
	irr::video::ITexture* getMkdirIcon(){
		return mkdir;
	}
	
	irr::video::ITexture* getItemIcon(const IItemOrganizer::Item& item){
		return file;
	}
	
	irr::video::ITexture* getFolderIcon(){
		return folder;
	}
	
};

int main(int argc, char *argv[]){
	IrrlichtDevice* nulldevice = createDevice(EDT_NULL);
	core::dimension2d<u32> deskres = nulldevice->getVideoModeList()->getDesktopResolution();
	nulldevice->drop();
	
	SIrrlichtCreationParameters param;
	param.Bits = 32;
   param.DriverType = EDT_OPENGL;
	rect<irr::s32> winRect(0,0,9*deskres.Width/10,9*deskres.Height/10);
	param.WindowSize = winRect.getSize();
	
	IrrlichtDevice* device = createDeviceEx(param);
	IVideoDriver* driver = device->getVideoDriver();
	IGUIEnvironment* env = device->getGUIEnvironment();
	ISceneManager* smgr = device->getSceneManager();
	
	device->setWindowCaption(L"Manual Tests for ItemSelect Stuff");
	
	Drawer2D* drawer = new Drawer2D(device);
	
	FlexibleFontManager* fmgr = new FlexibleFontManager(device);
	BMFontLoader* bmFontLoader = new BMFontLoader(fmgr);
	fmgr->addFontLoader(bmFontLoader);
	bmFontLoader->drop();
	
	AppSkin* skin = new AppSkin(device);
	assert(isExtendableSkin(skin));
	env->setSkin(skin);
	skin->drop();
	
	FlexibleFont* ff = fmgr->getFont("../FontTest/roboto-medium.fnt");
	ff->setDefaultMaterialType(fmgr->getSDFFMaterialType());
	skin->setFont(ff);
	
	IItemOrganizer* fso = new FileSystemItemOrganizer(device->getFileSystem());
	
	IconSource is(driver);
	
	ItemSelectElement* savediag = new ItemSelectElement(device, drawer, L"test.save", fso, AppSkin::REGULAR_AGGREGATION, AppSkin::LIST_ELE_AGGREGATION, -1, true, &is);
	
	while(device->run()){
		if(device->isWindowActive()){
			driver->beginScene(true, true, SColor(255,240,240,240));//SColor(0,200,200,200));
			smgr->drawAll();
			env->drawAll();
			driver->endScene();
		}
	}
	
	device->drop();
	
	delete drawer;

	return 0;
}
