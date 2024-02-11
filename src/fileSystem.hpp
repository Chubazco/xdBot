#include <Geode/Geode.hpp>
#include <locale>
#include <codecvt>
#include <string>

#ifdef GEODE_IS_ANDROID
	bool isAndroid = true;
#else
	bool isAndroid = false;
#endif

#ifdef GEODE_IS_ANDROID
    std::string slash = "/";
#else
    std::string slash = "\\";
#endif

#define CCPOINT_CREATE(__X__,__Y__) cocos2d::CCPointMake((float)(__X__), (float)(__Y__))

using namespace geode::prelude;
class macroCell : public CCNode {
public:
    static macroCell* create(std::string name) {
        auto ret = new macroCell();
        if (ret && ret->init(name)) {
            ret->autorelease();
        } else {
            delete ret;
            ret = nullptr;
        }
    return ret;
    }

    void loadMacro(CCObject*);
    void handleLoad(CCObject*);

    void handleDelete(CCObject* btn) {
	    std::string path = Mod::get()->getSaveDir().string()
        +slash+static_cast<CCMenuItemSpriteExtra*>(btn)->getID() + ".xd";

        std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
        std::wstring wideString = converter.from_bytes(path);
	    std::locale utf8_locale(std::locale(), new std::codecvt_utf8<wchar_t>);

        try {
        std::filesystem::remove(wideString);
        } catch (const std::filesystem::filesystem_error& e) {
            FLAlertLayer::create(
    		"Delete Macro",   
    		"There was an <cr>error</c> deleting this macro.",  
    		"OK"      
		    )->show();
            return;
        }
            
        if (this->getParent()->getParent()->getChildren()->count() == 1)  {
            CCLabelBMFont* noMacroLbl = CCLabelBMFont::create("No macros.", "bigFont.fnt");
            noMacroLbl->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
            noMacroLbl->setColor(ccc3(201,170,153));
            noMacroLbl->setScale(0.725f);
            this->getParent()->getParent()->getParent()->addChild(noMacroLbl);
            noMacroLbl->setPosition(this->getParent()->getParent()->getParent()->getContentSize() / 2);
        }

        this->getParent()->removeFromParentAndCleanup(true);
        
        FLAlertLayer::create(
    		"Delete Macro",   
    		"Macro deleted <cg>successfully</c>.",  
    		"OK"      
		)->show();
    }

    void deleteMacro(CCObject* btn) {
        geode::createQuickPopup(
    		"Delete Macro",     
    		"<cr>Delete</c> this macro?", 
    		"Cancel", "Ok",  
    		[this, btn](auto, bool btn2) {
        		if (btn2) this->handleDelete(btn);
    		}); 
    }

    bool init(std::string name) {
        auto lbl = (name.length() > 19) ? (name.substr(0, 19) + "...") : name;
        CCLabelBMFont* macroLabel = CCLabelBMFont::create(lbl.c_str(), "bigFont.fnt");
        macroLabel->setAnchorPoint(ccp(0.0f,0.5f));
        macroLabel->setAlignment(CCTextAlignment::kCCTextAlignmentLeft);
        macroLabel->setPosition({5, 20});
        macroLabel->setScale(0.45f);
        this->addChild(macroLabel);
        auto menu = CCMenu::create();
        menu->setPosition({0,0});

        auto btnSpr1 = ButtonSprite::create("Load");
        btnSpr1->setScale(0.7f);
        auto btn = CCMenuItemSpriteExtra::create(btnSpr1,
   		this,
   		menu_selector(macroCell::loadMacro));
        btn->setPosition({270,20});
        menu->addChild(btn);
        btn->setID(name);
        
        auto btnSpr = CCSprite::createWithSpriteFrameName("GJ_trashBtn_001.png");
        btnSpr->setScale(0.65f);
        btn = CCMenuItemSpriteExtra::create(
        btnSpr,
        this,
        menu_selector(macroCell::deleteMacro)
        );
        btn->setPosition({225,20});
        btn->setID(name);
        menu->addChild(btn);
        this->addChild(menu);
        return true;
    }
};

class loadMacroPopup : public geode::Popup<std::string const&> {
public:
    void importMacro(CCObject*) {
        file::FilePickOptions fileOptions;
        if (isAndroid) {
            fileOptions = {
                std::nullopt,
                {}
            };
        } else {
            file::FilePickOptions::Filter textFilter;
            textFilter.description = "Macro Files";
            textFilter.files = {"*.xd"};
            fileOptions.filters.push_back(textFilter);
        }
        
        file::pickFile(file::PickMode::OpenFile , fileOptions, [this](ghc::filesystem::path result) {
            auto path = std::filesystem::path(result.c_str());
            

        if (path.empty()) {
        FLAlertLayer::create("Import Macro", "error 1.", "Ok")->show();
        return;
    }
    if (!std::filesystem::exists(path)) {
        FLAlertLayer::create("Import Macro", "error 2", "Ok")->show();
        return;
    }

    if (std::filesystem::is_directory(path)) {
        FLAlertLayer::create("Import Macro", "Error 3", "Ok")->show();
        return;
    }
    if (path.extension().string() != ".xd") {
        FLAlertLayer::create("Import macro", "The selected file must be xd.", "Ok")->show();
        return;
    }
		
    std::string copyPath = Mod::get()->getSaveDir().string()
            + slash + path.filename().string();

        std::filesystem::copy_file(path, copyPath);

            refresh();
            FLAlertLayer::create(
    		    "Import Macro",   
    		    "Macro imported <cg>successfully</c>.",  
    		    "OK"      
			)->show();
        });
    }

    bool setup(std::string const& value) override {
        CCArray* macroList = CCArray::create();
        auto winSize = CCDirector::sharedDirector()->getWinSize();
        Result<std::vector<ghc::filesystem::path>> macros;
        std::string titleLol;
        std::string noBUMessage;
        CCPoint corner = winSize/2.f-CCPOINT_CREATE(m_size.width/2.f,m_size.height/2.f);
        macros = file::readDirectory(Mod::get()->getSaveDir());

        auto emptyBtn = CCSprite::createWithSpriteFrameName("GJ_plainBtn_001.png");
        emptyBtn->setScale(0.75f);
        auto folderIcon = CCSprite::createWithSpriteFrameName("folderIcon_001.png");
        folderIcon->setPosition(emptyBtn->getContentSize() / 2);
        folderIcon->setScale(0.8f);
        emptyBtn->addChild(folderIcon);
        auto menu = CCMenu::create();
        menu->setPosition({0,0});

        CCSprite* importMacroSprite = CCSprite::createWithSpriteFrameName("GJ_plusBtn_001.png");
        importMacroSprite->setScale(0.8f);
        auto importMacroButton = CCMenuItemSpriteExtra::create(
            importMacroSprite,
            this,
            menu_selector(loadMacroPopup::importMacro)
        );
        importMacroButton->setPosition(corner + CCPOINT_CREATE(380,65));
        menu->addChild(importMacroButton);

        m_mainLayer->addChild(menu);
        auto openFolderBtn = CCMenuItemSpriteExtra::create(
            emptyBtn,
            this,
            menu_selector(loadMacroPopup::openMacrosFolder)
        );
        openFolderBtn->setPosition(corner + CCPOINT_CREATE(380,20));
        menu->addChild(openFolderBtn);

        for (int i = 0; i < macros.value().size(); ++i) {

            std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
            std::wstring wideString = converter.from_bytes(macros.value()[i].string());
            std::locale utf8_locale(std::locale(), new std::codecvt_utf8<wchar_t>);

            std::wifstream file;

            if (macros.value()[i].extension() == ".xd") {
                file.open(wideString);
                file.imbue(utf8_locale);
                if (file){
                    macroCell* cell = macroCell::create(macros.value()[i].filename().string().substr(0, macros.value()[i].filename().string().find_last_of('.')));
                    macroList->addObject(cell);
                }
            }
        }

        ListView* mcrList = ListView::create(macroList, 40, 308, 200);

        mcrList->setScaleY(0.875f);
        mcrList->setScaleX(0.89f);
        mcrList->setPosition({-14, -9.5f});

        auto list = GJListLayer::create(mcrList, "Load Macro", {0, 0, 0, 0}, 280, 180, 2);

        m_mainLayer->addChild(list);

        CCArray* listChildren = list->getChildren();

        static_cast<CCSprite*>(listChildren->objectAtIndex(0))->setScale(0.8f);
        CCSprite* topS = static_cast<CCSprite*>(listChildren->objectAtIndex(1));
        topS->setScale(0.8f);
        topS->setPosition({140, 191});

        CCLabelBMFont* label = static_cast<CCLabelBMFont*>(listChildren->objectAtIndex(4));
        label->setScale(0.7f);
        label->setPosition({140, 194});
        list->setPosition(corner + CCPOINT_CREATE(38.8,35));

        if (macroList->count() == 0) {
            CCLabelBMFont* noMacroLbl = CCLabelBMFont::create("No macros.", "bigFont.fnt");
            noMacroLbl->setAlignment(CCTextAlignment::kCCTextAlignmentCenter);
            noMacroLbl->setColor(ccc3(201,170,153));
            noMacroLbl->setScale(0.725f);

            list->addChild(noMacroLbl);
            noMacroLbl->setPosition(list->getContentSize() / 2);
        }
        return true;
    }

    static loadMacroPopup* create() {
        auto ret = new loadMacroPopup();
        if (ret && ret->init(355, 255, "")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

	void openLoadMenu(CCObject*) {
        auto layer = create();
        layer->m_noElasticity = (static_cast<float>(Mod::get()->getSettingValue<double>("Speedhack")) < 1) ? true : false;
		layer->show();
	}

    void refresh() {
        this->keyBackClicked();
        openLoadMenu(nullptr);
    }

    void openMacrosFolder(CCObject*) {
        file::openFolder(Mod::get()->getSaveDir());
    }
};


class saveMacroPopup : public geode::Popup<std::string const&> {
public:
    CCTextInputNode* macroNameInput = nullptr;

    bool setup(std::string const& value) override {
        auto winSize = CCDirector::sharedDirector()->getWinSize();

        auto inputBg = CCScale9Sprite::create("square02b_001.png", {0, 0, 80, 80});

        inputBg->setScale(0.7f);
        inputBg->setColor({ 0, 0, 0 });
        inputBg->setOpacity(75);
        inputBg->setPosition(winSize/2 + CCPOINT_CREATE(0,7));
        inputBg->setContentSize({ 235, 54 });

        macroNameInput = CCTextInputNode::create(150, 30, "Macro Name", "bigFont.fnt");
        macroNameInput->m_textField->setAnchorPoint({ 0.5f, 0.5f });
        macroNameInput->ignoreAnchorPointForPosition(true);
        macroNameInput->m_placeholderLabel->setAnchorPoint({ 0.5f, 0.5f });
        macroNameInput->setPosition(winSize/2 + CCPOINT_CREATE(0,7));
        macroNameInput->setMaxLabelScale(0.7f);
        macroNameInput->setLabelPlaceholderColor(ccc3(163, 135, 121));
        macroNameInput->setMouseEnabled(true);
        macroNameInput->setTouchEnabled(true);

        auto title = CCLabelBMFont::create("Save Macro", "bigFont.fnt");
        title->setPosition(winSize/2 + CCPOINT_CREATE(0,50));
        title->setScale(0.6f);
        m_mainLayer->addChild(title);

        auto btnSpr = ButtonSprite::create("Save");
        btnSpr->setScale(0.9f);
        auto menu = CCMenu::create();
        menu->setPosition({0,0});
        auto btn = CCMenuItemSpriteExtra::create(
            btnSpr,
            this,
            menu_selector(saveMacroPopup::saveMacro)
        );

        btn->setPosition(winSize/2 + CCPOINT_CREATE(0,-40));
        menu->addChild(btn);
        m_mainLayer->addChild(menu);

        m_mainLayer->addChild(inputBg);
        m_mainLayer->addChild(macroNameInput);
        return true;
    }

    static saveMacroPopup* create() {
        auto ret = new saveMacroPopup();
        if (ret && ret->init(220, 140, "")) {
            ret->autorelease();
            return ret;
        }
        CC_SAFE_DELETE(ret);
        return nullptr;
    }

	void openSaveMacro(CCObject*);

    void saveMacro(CCObject*);
   
};
