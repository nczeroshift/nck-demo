
#include "dsSceneFloat_Text.h"
#include "nckCamera.h"
#include "../dsUtils.h"


dsSceneFloat_Text::dsSceneFloat_Text(DS::Data * data) : dsSceneFloatPopup(data){
    usage.reserve(10);
    setType(0);
}

dsSceneFloat_Text::~dsSceneFloat_Text() {
  
}

void dsSceneFloat_Text::Load() {
   
}


void dsSceneFloat_Text::RenderFBO(int64_t start, int64_t end, int64_t time) {
    Graph::Device * dev = m_Data->GetGraphicsDevice();

}

void dsSceneFloat_Text::Render(int64_t start, int64_t end, int64_t time) {

    Graph::Device * dev = m_Data->GetGraphicsDevice();
    Gui::ShapeRenderer * shp = m_Data->GetShapeRenderer();

    prepare(start, end, time);

    Math::Vec2 pos = Math::LinearInterpolation(showPosA, showPosB, alpha);

    dev->Disable(Graph::STATE_DEPTH_TEST);

    dev->PushMatrix();
    dev->Translate(pos.GetX(), pos.GetY(), 0);

    dev->Color(255, 255, 255, 240);
  
    DS::RenderSquare(dev, width, alpha * height, false);

    Graph::Texture * fontTex = m_Data->GetTexture("texture://tex2d_sans_serif.png");
    Gui::FontMap * fontMap = m_Data->GetFontMap("script://sans_serif.txt");

    float speed = 20.0;
    int charsCount = speed * (time - start) / 1e6;

    float fontSize = 42;


    int secs = 10 * (time - start) / 1e6;
    int totalSize = 0;
    for(int i = 0;i < usage.size(); i++){
        std::string txt = usage[i];

        std::string tmp = txt.substr(0, MIN(charsCount - totalSize, txt.length()));
                
        float len = fontMap->GetLength(fontSize, tmp, true);

        totalSize += txt.length();

        Math::Vec2 position = Math::Vec2(10, fontSize / 2);

        dev->PushMatrix();
        dev->Translate(position.GetX()+10, fontSize*i+ position.GetY() + 20, 0);

        if (tmp.size() != txt.size() || (i == usage.size()-1)) {
            if (secs % 2 == 0) {
                dev->Color(0, 0, 0, 255);
                dev->PushMatrix();
                dev->Translate(len + fontSize*0.5, -fontSize*0.5, 0);
                DS::RenderSquare(dev, fontSize*0.5, fontSize, false);
                dev->PopMatrix();
            }
        }

        dev->Color(0, 0, 0);
        fontTex->Enable();
        fontMap->Draw(0,0 , fontSize, tmp, true, Gui::FONT_ALIGNMENT_LEFT);
        fontTex->Disable();
        dev->PopMatrix();

        if (tmp.size() != txt.size())
            break;
    }

    drawBorders();
}

void dsSceneFloat_Text::setType(int usage) {
    this->usage.clear();
    if (usage == 0) {
        this->usage.push_back("jae686/code");
        this->usage.push_back("$groups");
        this->usage.push_back(" >CoProcessor");
        this->usage.push_back(" >Volumetric Illusions [VI]");
    }
    else if (usage == 1) {
        this->usage.push_back("zeroshift/code");
        this->usage.push_back("$groups");
        this->usage.push_back(" >CoProcessor");
        this->usage.push_back(" >Napalm Core [NC]");
  
    }
    else if (usage == 2) {
        this->usage.push_back("ps/music");
        this->usage.push_back("$groups");
        this->usage.push_back(" >TPOLM");
		this->usage.push_back(" >Enough Records");
    }
    else if (usage == 3) {
        this->usage.push_back("... and don't forget to come");
        this->usage.push_back("to Inércia Demoparty 2005!");
    }
    else if (usage == 4) {
        this->usage.push_back("Breeze! Vamos à Breeze!");
        this->usage.push_back("Hi Xrl, Mr.Hugo, Jeenio!");
    }
    else if (usage == 5) {
        // Madrid
        this->usage.push_back("Playboy Mansion Party");
        this->usage.push_back("Sushi food poisoning hangover is so nice");
    }
    else if (usage == 6) {
        // Bilbau
        this->usage.push_back("Euskal");
        this->usage.push_back("Hi Ham, Stage 7!");
        this->usage.push_back("Greetz to Navis/ASD");
    }
    else if (usage == 7) {
        // Thoiseey
        this->usage.push_back("V.I.P.");
        this->usage.push_back("Forsaken by destiny!");
    }
    else if (usage == 8) {
        // Merseille / Arles
        this->usage.push_back("Main Party");
        this->usage.push_back("1500m^2 screen with mosquitoes!");
    }
    else if (usage == 10) {
        // Berne
        this->usage.push_back("Buenzli/Demodays");
        this->usage.push_back("Coweees for everyone!");
    }
    else if (usage == 12) {
        // Brno
        this->usage.push_back("ResetKani");
    }
    else if (usage == 13) {
        // Trencin
        this->usage.push_back("Forever");
    } else if (usage == 14) {
        // Trencin
        this->usage.push_back("Demobit 2017");
        this->usage.push_back("Hi everyone!");
        this->usage.push_back("Awesome party!");
    } else if (usage == 99) {
		// Trencin
		this->usage.push_back("??");
		this->usage.push_back("Becasue I C^C ^C");
	} else if (usage == 10000) {
 
        this->usage.push_back("80's grid");
        this->usage.push_back("and menger sponge");
    }
    else if (usage == 10001) {
        this->usage.push_back("Fuckings to last minute bugs");
		this->usage.push_back("Greetings to Acession only");
		this->usage.push_back("... and you!");
    }
    else if (usage == 10002) {
        this->usage.push_back("Partycoding");
        this->usage.push_back("like a boss");
    }
}