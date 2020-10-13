#include <CommonIniEditor.h>
#include <StringHelpers.h>
#include <UnicodeCfgParser.h>
#include <utilities.h>
#include <font.h>
#include <IInput.h>
#include <AggregateGUIElement.h>
#include <BeautifulGUIText.h>
#include <AggregatableGUIElementAdapter.h>
#include <CMBox.h>

#include <IrrlichtDevice.h>
#include <IVideoDriver.h>
#include <IGUIButton.h>
#include <IGUIWindow.h>
#include <IGUICheckBox.h>
#include <IGUIImage.h>
#include <IGUIScrollBar.h>
#include <IGUIFont.h>
#include <IGUIComboBox.h>
#include <IGUIStaticText.h>
#include <IGUIEditBox.h>

using namespace std;
using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

CommonIniEditor::CommonIniEditor(ICommonAppContext* context, std::string Section, int FieldCount, std::string* Keys, std::string* FieldNames, std::string* DefaultValues, ValueType* ValTypes, IIniEditorCustomization* customization, const char* HelpFile):c(context), customization(customization), section(Section), fieldCount(FieldCount), key(Keys), defaultValue(DefaultValues), valType(ValTypes), sel(context, customization->getAlphaBackground(), SColor(255, 255, 255,255)){
	fieldName = new std::wstring[FieldCount];
	for(int i=0; i<FieldCount; i++){
		fieldName[i] = convertUtf8ToWString(FieldNames[i]);
	}
	delete[] FieldNames;
	init(HelpFile);
}

static const std::wstring valueTypeStr[CommonIniEditor::TYPE_COUNT] = {L"INT", L"DOUBLE", L"STRING", L"BOOLEAN", L"NOT_EDITABLE", L"ONE_OF", L"COLOR_RGB", L"COLOR_RGBA"};

CommonIniEditor::CommonIniEditor(ICommonAppContext* context, std::string Section, const std::wstring& guiCode, IIniEditorCustomization* customization, const char* HelpFile):c(context), customization(customization), section(Section), sel(context, customization->getAlphaBackground(), SColor(255, 255, 255,255)){
	UnicodeCfgParser parser(4);
	parser.parse(guiCode);
	const std::list<std::vector<std::wstring>>& result = parser.getResult();
	fieldCount = result.size();
	key = new std::string[fieldCount];
	fieldName = new std::wstring[fieldCount];
	defaultValue = new std::string[fieldCount];
	valType = new ValueType[fieldCount];
	uint32_t i = 0;
	for(auto it = result.begin(); it != result.end(); ++it){
		const std::vector<std::wstring>& line = *it;
		key[i] = convertWStringToUtf8String(line[0]);
		fieldName[i] = line[1];
		defaultValue[i] = convertWStringToUtf8String(line[2]);
		const std::wstring& t = line[3];
		valType[i] = NOT_EDITABLE;
		for(uint32_t j=0; j<TYPE_COUNT; j++){
			if(valueTypeStr[j].compare(t)==0){
				valType[i] = (ValueType)j;
				break;
			}
		}
		i++;
	}
	init(HelpFile);
}

void CommonIniEditor::init(const char* HelpFile){
	lastSucc = false;
	env = c->getIrrlichtDevice()->getGUIEnvironment();
	driver = env->getVideoDriver();
	int w = driver->getScreenSize().Width; int h = driver->getScreenSize().Height;
	wrect = rect<s32>(0, 0, w, h);
	win = env->addWindow(wrect, false, L"");//Edit
	win->getCloseButton()->setVisible(false); win->setDrawTitlebar(false);
	cancel = env->addButton(rect<s32>(3*w/14-c->getRecommendedButtonWidth()/2, h-10-c->getRecommendedButtonHeight(), 3*w/14+c->getRecommendedButtonWidth()/2, h-10), win, customization->getCancelButtonId(), L"Cancel");
	showHelpButton = (HelpFile != NULL);
	help = NULL;
	if(showHelpButton){
		help = env->addButton(rect<s32>(7*w/14-c->getRecommendedButtonWidth()/2, h-10-c->getRecommendedButtonHeight(), 7*w/14+c->getRecommendedButtonWidth()/2, h-10), win, -1, L"Help", NULL);
		helpFile = std::string(HelpFile);
	}
	ok = env->addButton(rect<s32>(11*w/14-c->getRecommendedButtonWidth()/2, h-10-c->getRecommendedButtonHeight(), 11*w/14+c->getRecommendedButtonWidth()/2, h-10), win, customization->getOkButtonId(), L"Ok");
	prect = rect<s32>(10, 10, w-10, h-20-c->getRecommendedButtonHeight());
	pw = prect.getWidth()-15;
	ph = prect.getHeight();
	prect.LowerRightCorner.X -= 15;
	agg = NULL;
	input = new IGUIElement*[fieldCount];
	height = new int[fieldCount];
	for(int i=0; i<fieldCount; i++){//Initialwerte setzen
		input[i] = NULL;
	}
	lastContent = new std::wstring[fieldCount];
	win->setVisible(false);
}

void CommonIniEditor::createGUI(){
	IGUIFont* font = env->getSkin()->getFont();
	SColor textColor = env->getSkin()->getColor(EGDC_BUTTON_TEXT);
	AggregateGUIElement* content = new AggregateGUIElement(env, .95f, 1.f, .95f, 1.f, false, false, true, {}, {}, false, customization->getAggregationID());
	ScrollBar* sb = new ScrollBar(env, .025f, false, customization->getScrollBarID());
	agg = new AggregateGUIElement(env, 1.f, 1.f, 1.f, 1.f, false, true, false, {
		new EmptyGUIElement(env, .025f, 1.f, false, false, customization->getAggregatableID()),
		content,
		sb
	}, {}, false, customization->getAggregationID(), NULL, win, prect);
	sb->linkToScrollable(content);
	f32 labelSpace = 0.5f;
	int wholeWidth = prect.getWidth()*.95f;
	int labelWidth = wholeWidth*labelSpace;
	f32 lineSpace = (f32)c->getRecommendedButtonHeight()/(f32)prect.getHeight();
	for(int i=0; i<fieldCount; i++){
		content->addSubElement(new EmptyGUIElement(env, .5f*lineSpace, 1.f, false, false, customization->getAggregatableID()));
		IAggregatableGUIElement* finalLine = NULL;
		if(valType[i]==INT || valType[i]==DOUBLE || valType[i]==STRING){
			AggregateGUIElement* line = new AggregateGUIElement(env, lineSpace, 1.f, lineSpace, 1.f, false, true, false, {
				new BeautifulGUIText(makeWordWrappedText(fieldName[i], labelWidth, font).c_str(), textColor, 0.f, NULL, false, true, env, labelSpace),
				addAggregatableEditBox(env, L"", 1.f-labelSpace, true, valType[i]==INT?INT_EDIT:(valType[i]==DOUBLE?DOUBLE_EDIT:STRING_EDIT))
			}, {}, false, customization->getInvisibleAggregationID());
			input[i] = getFirstGUIElementChild(*(line->getChildren().begin()+1));
			finalLine = line;
		}else if(valType[i]==NOT_EDITABLE){
			finalLine = new BeautifulGUIText(convertUtf8ToWString(convertWStringToUtf8String(fieldName[i]).append(defaultValue[i])).c_str(), textColor, 0.f, NULL, true, true, env, lineSpace);
			input[i] = finalLine;
		}else if(valType[i]==BOOLEAN){
			finalLine = addAggregatableCheckBox(env, makeWordWrappedText(fieldName[i], wholeWidth, font).c_str(), false, lineSpace);
			input[i] = getFirstGUIElementChild(finalLine);
		}else if(valType[i]==ONE_OF){
			AggregateGUIElement* line = new AggregateGUIElement(env, lineSpace, 1.f, lineSpace, 1.f, false, true, false, {
				new BeautifulGUIText(makeWordWrappedText(fieldName[i], labelWidth, font).c_str(), textColor, 0.f, NULL, false, true, env, labelSpace),
				addAggregatableComboBox(env, 1.f-labelSpace)
			}, {}, false, customization->getInvisibleAggregationID());
			input[i] = getFirstGUIElementChild(*(line->getChildren().begin()+1));
			finalLine = line;
			((IGUIComboBox*)input[i])->setItemHeight(c->getRecommendedButtonHeight());
			std::wstringstream ss;
			for(uint32_t ci = 0; ci<defaultValue[i].size(); ci++){
				char c = defaultValue[i][ci];
				if(c==';'){
					((IGUIComboBox*)input[i])->addItem(ss.str().c_str());
					ss.str(L"");
				}else{
					ss << c;
				}
			}
			((IGUIComboBox*)input[i])->addItem(ss.str().c_str());
		}else if(valType[i]==COLOR_RGB || valType[i]==COLOR_RGBA){
			AggregateGUIElement* line = new AggregateGUIElement(env, lineSpace, 1.f, lineSpace, 1.f, false, true, false, {
				new EmptyGUIElement(env, .33, 1.f, false, false, customization->getAggregatableID()),
				addAggregatableButton(env, fieldName[i].c_str(), .34f),
				new EmptyGUIElement(env, .33, 1.f, false, false, customization->getAggregatableID())
			}, {}, false, customization->getInvisibleAggregationID());
			finalLine = line;
			input[i] = getFirstGUIElementChild(*(line->getChildren().begin()+1));
		}
		if(finalLine){content->addSubElement(finalLine);}
	}
	content->addSubElement(new EmptyGUIElement(env, .5f*lineSpace, 1.f, false, false, customization->getAggregatableID()));
}

void CommonIniEditor::removeGUI(){
	agg->remove();
	agg = NULL;
}

CommonIniEditor::~CommonIniEditor(){
	cancel->remove();
	ok->remove();
	win->remove();
	delete[] key;
	delete[] fieldName;
	delete[] defaultValue;
	delete[] valType;
	delete[] input;
	delete[] height;
	delete[] lastContent;
}

void CommonIniEditor::edit(IniFile* Ini){
	createGUI();
	colorState = -1;
	lastSucc = false;
	ini = Ini;
	win->setVisible(true);
	win->setRelativePosition(wrect);
	env->setFocus(win);
	std::string def;
	for(int i=0; i<fieldCount; i++){
		if(ini->isAvailable(section, key[i])){
			def = ini->get(section, key[i]);
		}else{
			def = defaultValue[i];
		}
		lastContent[i] = convertUtf8ToWString(def);
		if(valType[i]==INT || valType[i]==DOUBLE || valType[i]==STRING){
			input[i]->setText(lastContent[i].c_str());
		}else if(valType[i]==BOOLEAN){
			((IGUICheckBox*)input[i])->setChecked((bool)atoi(def.c_str()));
		}else if(valType[i]==ONE_OF){
			IGUIComboBox* combo = (IGUIComboBox*)input[i];
			for(uint32_t it=0; it<combo->getItemCount(); it++){
				std::string item = convertWStringToUtf8String(std::wstring(combo->getItem(it)));
				if(item.compare(def)==0){
					combo->setSelected(it);
					break;
				}
			}
		}
	}
}

bool CommonIniEditor::isCorrectType(std::wstring content, ValueType type){
	if(type==STRING){
		return true;
	}else if(type==INT){
		std::string c = convertWStringToUtf8String(content);
		if(c.size()>=1){
			if((c[0]<'0' || c[0]>'9') && c[0]!='-'){return false;}
		}
		for(uint32_t i=1; i<c.size(); i++){
			if(c[i]<'0' || c[i]>'9'){return false;}//error state
		}
		return true;
	}else if(type==DOUBLE){
		std::string c = convertWStringToUtf8String(content);
		bool dotRead = false;
		if(c.size()>=1){
			if(c[0]=='.'){
				dotRead = true;
			}else if((c[0]<'0' || c[0]>'9') && c[0]!='-'){
				return false;
			}
		}
		for(uint32_t i=1; i<c.size(); i++){
			if(c[i]=='.'){
				if(dotRead){return false;}
				dotRead = true;
			}else if(c[i]<'0' || c[i]>'9'){
				return false;
			}
		}
		return true;
	}else{
		return false;
	}
}

void CommonIniEditor::render(){
	if(sel.isVisible()){
		sel.render();
	}
}

void CommonIniEditor::processEvent(irr::SEvent event){
	win->setRelativePosition(wrect);
	//ColorSelector Events
	if(colorState>-1){
		sel.processEvent(event);
		if(!sel.isVisible()){
			if(sel.lastSuccess()){
				SColor color = sel.getColor();
				std::string keyname = key[colorState]; keyname.append(".R");
				ini->set(section, keyname, convertToString(color.getRed()));
				keyname = key[colorState]; keyname.append(".G");
				ini->set(section, keyname, convertToString(color.getGreen()));
				keyname = key[colorState]; keyname.append(".B");
				ini->set(section, keyname, convertToString(color.getBlue()));
				keyname = key[colorState]; keyname.append(".A");
				ini->set(section, keyname, convertToString(color.getAlpha()));
			}
			for(int i=0; i<fieldCount; i++){
				if(input[i]){input[i]->setEnabled(true);}
			}
			colorState = -1;
		}
	}else{//own Events
		if(event.EventType == EET_GUI_EVENT){
			irr::SEvent::SGUIEvent guievent = event.GUIEvent;
			if(guievent.EventType == EGET_EDITBOX_CHANGED){
				for(int i=0; i<fieldCount; i++){
					//Type Checking:
					if(input[i] && (guievent.Caller==input[i]) && (valType[i]==INT || valType[i]==DOUBLE || valType[i]==STRING)){
						std::wstring thisContent = std::wstring(input[i]->getText());
						if(!isCorrectType(thisContent, valType[i])){
							input[i]->setText(lastContent[i].c_str());
						}else{
							lastContent[i] = thisContent;
						}
					}
				}
			}else if(guievent.EventType==EGET_ELEMENT_HOVERED || guievent.EventType==EGET_ELEMENT_FOCUSED){
				for(int32_t i=0; i<fieldCount; i++){
						bringToFrontRecursive(guievent.Caller);
				}
			}else if(guievent.EventType == EGET_BUTTON_CLICKED){
				for(int i=0; i<fieldCount; i++){
					if((valType[i]==COLOR_RGB || valType[i]==COLOR_RGBA) && guievent.Caller==input[i]){
						colorState = i;
						std::string def[4]; int di=0;
						for(uint32_t ci=0; ci<defaultValue[i].size(); ci++){
							if(di<4 && defaultValue[i][ci] == ','){
								di++;
							}else{
								def[di].append(std::string(&(defaultValue[i][ci]),1));
							}
						}
						std::string keyname = key[i]; keyname.append(".R");
						if(ini->isAvailable(section, keyname)){
							def[0] = ini->get(section, keyname);
						}
						keyname = key[i]; keyname.append(".G");
						if(ini->isAvailable(section, keyname)){
							def[1] = ini->get(section, keyname);
						}
						keyname = key[i]; keyname.append(".B");
						if(ini->isAvailable(section, keyname)){
							def[2] = ini->get(section, keyname);
						}
						keyname = key[i]; keyname.append(".A");
						if(ini->isAvailable(section, keyname)){
							def[3] = ini->get(section, keyname);
						}
						sel.setColor(SColor(atoi(def[3].c_str()), atoi(def[0].c_str()), atoi(def[1].c_str()), atoi(def[2].c_str())));
						sel.select(valType[i]==COLOR_RGBA);
						colorState = i;
						for(int i=0; i<fieldCount; i++){
							if(input[i]){input[i]->setEnabled(false);}
						}
						break;
					}
				}
				if(guievent.Caller==ok){
					lastSucc = true;
					for(int i=0; i<fieldCount; i++){
						std::string value;
						if(valType[i]==INT || valType[i]==DOUBLE || valType[i]==STRING){
							value = convertWStringToUtf8String(std::wstring(input[i]->getText()));
						}else if(valType[i]==BOOLEAN){
							value = convertToString(((IGUICheckBox*)input[i])->isChecked());
						}else if(valType[i]==NOT_EDITABLE){
							value = defaultValue[i];
						}else if(valType[i]==ONE_OF){
							int idx = ((IGUIComboBox*)input[i])->getSelected();
							if(idx==-1){idx=0;}
							value = convertWStringToUtf8String(std::wstring(((IGUIComboBox*)input[i])->getItem(idx)));
						}else if(valType[i]==COLOR_RGB || valType[i]==COLOR_RGBA){
							std::string def[4]; int di=0;
							for(uint32_t ci=0; ci<defaultValue[i].size(); ci++){
								if(di<4 && defaultValue[i][ci] == ','){
									di++;
								}else{
									def[di].append(std::string(&(defaultValue[i][ci]),1));
								}
							}
							std::string keyname = key[i]; keyname.append(".R");
							if(!ini->isAvailable(section, keyname)){
								ini->set(section, keyname, def[0]);
							}
							keyname = key[i]; keyname.append(".G");
							if(!ini->isAvailable(section, keyname)){
								ini->set(section, keyname, def[1]);
							}
							keyname = key[i]; keyname.append(".B");
							if(!ini->isAvailable(section, keyname)){
								ini->set(section, keyname, def[2]);
							}
							if(valType[i]==COLOR_RGBA){
								keyname = key[i]; keyname.append(".A");
								if(!ini->isAvailable(section, keyname)){
									ini->set(section, keyname, def[3]);
								}
							}
						}
						if(valType[i]!=COLOR_RGB && valType[i]!=COLOR_RGBA){
							ini->set(section, key[i], value);
						}
					}
					win->setVisible(false);
					removeGUI();
				}else if(guievent.Caller==cancel){
					cancelEdit();
				}else if(guievent.Caller==help){
					customization->OnHelpButtonPressed(helpFile.c_str());
				}
			}
		}
	}
}

void CommonIniEditor::cancelEdit(){
	win->setVisible(false);
	removeGUI();
}

bool CommonIniEditor::isVisible(){
	return win->isVisible();
}
