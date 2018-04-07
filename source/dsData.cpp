
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#include "dsData.h"
#include "nckUtils.h"

_DS_BEGIN

Data::Data(Core::Window * wnd, Graph::Device * gDev) {
    window = wnd;
    gDevice = gDev;
    shapeRenderer = NULL;
    widgetRenderer = NULL;

    Graph::Texture * fontTexture = this->LoadTexture("texture://tex2d_sans_serif.png");
    fontTexture->SetFilter(Graph::FILTER_MIPMAPPING, Graph::FILTER_NEAREST);
    fontTexture->SetFilter(Graph::FILTER_MAGNIFICATION, Graph::FILTER_LINEAR);
    fontTexture->SetFilter(Graph::FILTER_MINIFICATION, Graph::FILTER_LINEAR);

    Gui::FontMap * fontMap = this->LoadFontMap("script://sans_serif.txt");
    fontMap->SetPositionAccuracy(Gui::FONT_POSITION_ACCURACY_INTEGER);

    m_Textures.insert(std::pair<std::string, Graph::Texture*>("texture://tex2d_sans_serif.png", fontTexture));
    m_FontMap.insert(std::pair<std::string, Gui::FontMap*>("script://sans_serif.txt", fontMap));

    shapeRenderer = new Gui::ShapeRenderer(gDev);
    widgetRenderer = new Gui::BlenderWidgetRenderer(gDevice, shapeRenderer, fontMap, fontTexture);
    lastTimelineChange = 0;
}

Data::~Data() {
    MapFor(std::string, Compound*, m_Compounds, i) {
        delete i->second;
    }
    m_Compounds.clear();
    
    MapFor(std::string, Graph::Texture*, m_Textures, i) {
        delete i->second;
    }
    m_Textures.clear();

    MapFor(std::string, Graph::Program*, m_Programs, i) {
        delete i->second;
    }
    m_Programs.clear();

    MapFor(std::string, Gui::FontMap*, m_FontMap, i) {
        delete i->second;
    }
    m_FontMap.clear();

    SafeDelete(widgetRenderer);
    SafeDelete(shapeRenderer);
    
}

ReloadResult Data::ReloadTextures() {
    int total = 0, success = 0;
    std::list<std::string> errors;
    total = gDevice->ReloadPrograms(&success, &errors);
    return ReloadResult(total, success, 0);
}

ReloadResult Data::ReloadCompounds() {
    int total = 0, success = 0;
    std::list<std::string> errors;
    MapFor(std::string, Compound * , m_Compounds, i) {
        int flag;
        
        try {
            flag = i->second->Reload();
            if (flag == 1)
                success++;
        }
        catch (const Core::Exception & e) {
            errors.push_back(e.ToString());
        }
        total++;
    }
    return ReloadResult(total, success, errors.size());
}

ReloadResult Data::ReloadShaders() {
    int total = 0, success = 0;
    std::list<std::string> errors;
    total = gDevice->ReloadPrograms(&success, &errors);
    return ReloadResult(total, success, errors.size());
}

ReloadResult Data::ReloadTimeline() {
    std::string timelineSrc = "script://timeline.cpp";
    if (Core::FileReader::Exists(timelineSrc)) {
        int64_t t = Core::GetFileLastModified(timelineSrc);
        if (t > lastTimelineChange) {

        }
    }
    return ReloadResult(0, 0, 0);
}

Graph::Device * Data::GetGraphicsDevice() {
    return gDevice;
}

Gui::ShapeRenderer * Data::GetShapeRenderer() {
    return shapeRenderer;
}

Gui::WidgetRenderer * Data::GetWidgetRenderer() {
    return widgetRenderer;
}

Compound * Data::LoadCompound(const std::string & filename) {
    std::map<std::string, Compound*>::iterator i = m_Compounds.find(filename);
    if (i != m_Compounds.end())
        return i->second;
    Compound * ret = new Compound(gDevice);
    ret->Load(filename);
    m_Compounds.insert(std::pair<std::string, Compound*>(filename,ret));
    return ret;
}

Compound * Data::LoadCompound(const std::string & filename, Scene::Processor * processor) {
    std::map<std::string, Compound*>::iterator i = m_Compounds.find(filename);
    if (i != m_Compounds.end())
        return i->second;
    Compound * ret = new Compound(gDevice);
    ret->Load(filename,processor);
    m_Compounds.insert(std::pair<std::string, Compound*>(filename, ret));
    return ret;
}

Graph::Texture * Data::LoadTexture(const std::string & filename) {
    std::map<std::string, Graph::Texture*>::iterator i = m_Textures.find(filename);
    if (i != m_Textures.end())
        return i->second;
    Graph::Texture * ret = gDevice->LoadTexture(filename);
    m_Textures.insert(std::pair<std::string, Graph::Texture*>(filename, ret));
    return ret;
}

Gui::FontMap * Data::LoadFontMap(const std::string & filename) {
    std::map<std::string, Gui::FontMap*>::iterator i = m_FontMap.find(filename);
    if (i != m_FontMap.end())
        return i->second;
    Gui::FontMap * ret = new Gui::FontMap(gDevice);
    ret->Load(filename);
    m_FontMap.insert(std::pair<std::string, Gui::FontMap*>(filename, ret));
    return ret;
}

Graph::Program * Data::LoadProgram(const std::string & filename) {
    std::map<std::string, Graph::Program*>::iterator i = m_Programs.find(filename);
    if (i != m_Programs.end())
        return i->second;
    Graph::Program * ret = gDevice->LoadProgram(filename);
    m_Programs.insert(std::pair<std::string, Graph::Program*>(filename, ret));
    return ret;
}

Graph::Texture * Data::GetTexture(const std::string & filename) {
    if (m_Textures.find(filename) != m_Textures.end())
        return m_Textures.find(filename)->second;
    return NULL;
}

Compound * Data::GetCompound(const std::string & filename) {
    if (m_Compounds.find(filename) != m_Compounds.end())
        return m_Compounds.find(filename)->second;
    return NULL;
}

Gui::FontMap * Data::GetFontMap(const std::string & filename) {
    if (m_FontMap.find(filename) != m_FontMap.end())
        return m_FontMap.find(filename)->second;
    return NULL;
}

Graph::Program * Data::GetProgram(const std::string & filename) {
    if (m_Programs.find(filename) != m_Programs.end())
        return m_Programs.find(filename)->second;
    return NULL;
}


float Data::GetAspect() {
    return window->GetWidth() / (float)window->GetHeight();
}

float Data::GetHeight() {
    return window->GetHeight();
}

float Data::GetWidth() {
    return window->GetWidth();
}

void Data::GetProgramKeys(std::vector<std::string> * names) {
    names->reserve(m_Programs.size());
    MapFor(std::string, Graph::Program *, m_Programs, i) {
        names->push_back(i->first);
    }
}

void Data::GetTextureKeys(std::vector<std::string> * names) {
    names->reserve(m_Programs.size());
    MapFor(std::string, Graph::Texture *, m_Textures, i) {
        names->push_back(i->first);
    }
}

void Data::GetCompoundsKeys(std::vector<std::string> * names) {
    names->reserve(m_Programs.size());
    MapFor(std::string, Graph::Texture *, m_Textures, i) {
        names->push_back(i->first);
    }
}

/*
void Data::LoadTimeline(Math::TimelineNode<DS::Stage*> * timeline) {
    std::string timelineSrc = "script://timeline.cpp";
    if (Core::FileReader::Exists(timelineSrc)) {
        std::string fullPath = Core::ResolveFilename(timelineSrc);
        sceneTimeline.LoadFile(timelineSrc.c_str());
        lastTimelineChange = Core::GetFileLastModified(timelineSrc);
    }
}
*/

_DS_END
