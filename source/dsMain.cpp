
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#include "dsData.h"
#include "dsUtils.h"
#include "dsSettings.h"
#include "dsSubtitles.h"
#include "dsAudioPlayer.h"

#include "scenes/dsSceneLoading.h"
#include "scenes/dsSceneGridTerrain.h"
#include "scenes/dsHUDWindow.h"
#include "scenes/dsHUDWJupiterDiagram.h"
#include "scenes/dsInsideSatellite.h"
#include "scenes/dsOutputStage.h"

#include "tinyxml.h"


// Para correr sem audio usar NULL
#define AUDIO_STREAM        "audio://Ninja Tracks - Stasis.ogg"  //"audio://08_ps_-_wait_while_i_fall_asleep_short.ogg"
#define AUDIO_SAMPLERATE    44100
#define AUDIO_BUFFERS       4
#define AUDIO_FFT           2048

//#define FULLSCREEN

bool compareTLItem(const Math::TimelineItem<DS::Stage*> & a, const Math::TimelineItem<DS::Stage*> & b) {
    return a.GetLayer() < b.GetLayer();
}

_DS_BEGIN

class GraphicRendering : public virtual Core::Threadable
{
public:
    GraphicRendering(Core::Window * window) : Threadable() {
        wnd = window;
        dev = NULL;
        fontMap = NULL;
        fontTexture = NULL;
        fontTimeTexture = NULL;
        fontTimeMap = NULL;
        mutePlayback = false;
        player = NULL;
        lastTime = 0;
        duration = 10e6;
        showUI = false;
        sDensity = Core::Window::GetDisplayDensity();
        hScale = wnd->GetHeight() / (float)REFERENCE_HEIGHT;
        timer = Core::CreateChronometer();
        frTime = Core::CreateChronometer();
        shpRender = NULL;
      
        for (int i = 0; i < 3; i++) {
            reloadShaderFlag[i] = 0;
            reloadTextureCounter[i] = 0;
            reloadCompoundCounter[i] = 0;
            reloadShaderFlag[i] = 0;
        }
        reloadResources = false;
        reloadTimeline = false;
    }

    ~GraphicRendering() {
        SafeDelete(timer);
        SafeDelete(frTime);
        std::list<Subtitle*> subs;
        subtitlesTimeline.GetAll(&subs);
        ListFor(Subtitle*, subs, i) {
            delete (*i);
        }
    }

    void AddSub(float start, float duration, const std::string & text) {
        Subtitle * s = subtitlesTimeline.Insert(Math::TimelineItem<Subtitle*>(start, start+duration, new Subtitle(dev, fontTexture, fontMap)));
        s->Set(text, hScale * REFERENCE_SUBTITLE_SIZE);
    }
    
    void Run() {
      
        Graph::Texture * iconTexture;
        Graph::Texture * iconCompound;
        Graph::Texture * iconShader;

        Math::TimelineNode<DS::Stage*> timeline;

        try {
            dev = Graph::CreateDevice(wnd, Graph::DEVICE_AUTO, wnd->GetWidth(), wnd->GetHeight());
            dev->ClearColor(1.0, 1.0, 1.0, 1.0);
            dev->ClearFlags(Graph::BUFFER_COLOR_BIT | Graph::BUFFER_DEPTH_BIT);

            RenderLoading(wnd, dev);

            data = new Data(wnd, dev);
            shpRender = data->GetShapeRenderer();

            dev->Enable(Graph::STATE_BLEND);
            dev->BlendFunc(Graph::BLEND_SRC_ALPHA, Graph::BLEND_INV_SRC_ALPHA);

            fontTexture = data->LoadTexture("texture://tex2d_sans_serif.png");
            fontTexture->SetFilter(Graph::FILTER_MIPMAPPING, Graph::FILTER_NEAREST);
            fontTexture->SetFilter(Graph::FILTER_MAGNIFICATION, Graph::FILTER_LINEAR);
            fontTexture->SetFilter(Graph::FILTER_MINIFICATION, Graph::FILTER_LINEAR);

            iconTexture = data->LoadTexture("texture://ui/iconTexture.png");
            iconShader = data->LoadTexture("texture://ui/iconShader.png");
            iconCompound = data->LoadTexture("texture://ui/iconCompound.png");

            fontMap = data->LoadFontMap("script://sans_serif.txt");
            fontMap->SetPositionAccuracy(Gui::FONT_POSITION_ACCURACY_INTEGER);

            loading = new dsSceneLoading(data);
            loading->Load();

            renderLoading(0);

            if (AUDIO_STREAM != NULL) {
                player = new AudioPlayer(AUDIO_SAMPLERATE, AUDIO_FFT, AUDIO_BUFFERS);
                player->Start();
                player->LoadBG(AUDIO_STREAM);
                duration = player->GetDuration();
            }
            else
                mutePlayback = true;

            renderLoading(0);
            
            data->LoadTimeline();

            //dsSceneGridTerrain * gt = new dsSceneGridTerrain(data);
            //gt->Load();
            //dsInsideSatellite * inside = new dsInsideSatellite(data);
            //inside->Load();

            dsOutputStage * inside = new dsOutputStage(data);
            inside->Load();


            {
                auto w = data->GetWidth();
                auto h = data->GetHeight();

                auto l = w * 0.2;
                auto cw = w * 0.6;
                auto t = h * 0.1;

                dsHUDWindow * h1 = new dsHUDWindow(data);
                h1->Add(0.0, dsHUDWindow::Point(l, t, cw, 0, 0.0));
                h1->Add(0.3, dsHUDWindow::Point(l, t, cw, h * 0.2, 1.0));

                dsHUDWindow * h2 = new dsHUDWindow(data);
                h2->Add(1.0, dsHUDWindow::Point(l+cw*0.5, t + h * 0.2 + h*0.1, cw*0.5, 0, 0.0));
                h2->Add(1.3, dsHUDWindow::Point(l+cw*0.5, t + h * 0.2 + h*0.1, cw*0.5, h * 0.2, 1.0));
                h2->Add(10.0, dsHUDWindow::Point(l + cw*0.5, t + h * 0.2 + h*0.1, cw*0.5, h * 0.25, 1.0));


                dsHUDWindow * h3 = new dsHUDWindow(data);
                h3->Add(3.0, dsHUDWindow::Point(l + cw*0.5, t + h * 0.2 + 2*h*0.1 + h * 0.2, cw*0.5, 0, 0.0));
                h3->Add(3.3, dsHUDWindow::Point(l + cw*0.5, t + h * 0.2 + 2*h*0.1 + h * 0.2, cw*0.5, h * 0.2, 1.0));
                h3->Add(10.0, dsHUDWindow::Point(l + cw*0.5, t + h * 0.2 + 2 * h*0.1 + h * 0.2, cw*0.5, h * 0.25, 1.0));


                inside->AddStage(0e6, 60e6, h1);
                inside->AddStage(0e6, 60e6, h2);
                inside->AddStage(0e6, 60e6, h3);

                dsHUDWJupiterDiagram * d = new dsHUDWJupiterDiagram(data);
                {
                    float w2 = cw * 0.5 - cw*0.1;
                    float h2 = w2 * 9 / 16.0;
                    d->Load();
                    d->Add(2.0, dsHUDWindow::Point(l, t + h * 0.2 + h*0.1, w2, 0, 0.0));
                    d->Add(2.3, dsHUDWindow::Point(l, t + h * 0.2 + h*0.1, w2, h2, 1.0));
                    inside->AddStage(0e6, 60e6, d);
                }

            }

            timeline.Insert(Math::TimelineItem<DS::Stage*>(0e6, 60e6, inside));
            
            renderLoading(99.9);
        }
        catch (const Core::Exception & ex) {
            ex.PrintStackTrace();
            ReleaseStuff();
            Teardown();
            return;
        }
               

        timeline.Build();

        if(player)
            player->Play();

        timer->Start();
        frTime->Start();

        ReloadResult rCompounds, rShaders, rTextures;
        dev->ClearColor(0.3, 0.3, 0.3, 1.0);
        
        while (!IsTearingDown())
        {
            dev->ClearColor(0, 0, 0, 1);
            dev->Clear();
            dev->Viewport(0, 0, wnd->GetWidth(), wnd->GetHeight());
        

            if (reloadResources) {
                rShaders = data->ReloadShaders();
                rCompounds = data->ReloadCompounds();
                rTextures = data->ReloadTextures();
                reloadResources = false;
            }

            std::list<Math::TimelineItem<DS::Stage*>> items;
            int64_t time = GetTime();
         
            float time_in_secs = time / 1e6;
            
            if (time_in_secs > 150)
                break;

            timeline.Get(time, &items); 
            items.sort(compareTLItem);

            ListFor(Math::TimelineItem<DS::Stage*>, items, i) {
                i->GetObject()->RenderFBO(i->GetStart(), i->GetEnd(), time);
            }

            ListFor(Math::TimelineItem<DS::Stage*>, items, i) {
                i->GetObject()->Render(i->GetStart(), i->GetEnd(), time);
            }
        

            dev->Viewport(0, 0, wnd->GetWidth(), wnd->GetHeight());

            const double fps = UpdateFPS();
            if (showUI) {
                dev->Disable(Graph::STATE_DEPTH_TEST);
                dev->MatrixMode(Graph::MATRIX_PROJECTION);
                dev->Identity();
                dev->Ortho2D(wnd->GetWidth(), wnd->GetHeight());

                dev->MatrixMode(Graph::MATRIX_VIEW);
                dev->Identity();

                dev->MatrixMode(Graph::MATRIX_MODEL);
                dev->Identity();

                dev->Enable(Graph::STATE_BLEND);
                dev->Disable(Graph::STATE_DEPTH_TEST);
                dev->BlendFunc(Graph::BLEND_SRC_ALPHA, Graph::BLEND_INV_SRC_ALPHA);
          
                DrawFrameRate(10, 20, fps);
                DrawText(10, 45, 24, false, "space - play/pause");
                DrawText(10, 65, 24, false, "z - reload timeline");
                DrawText(10, 85, 24, false, "x - reload resources");
                DrawText(10, 105, 24, false, "c - hide/show UI");
                DrawText(10, 125, 24, false, "esc - exit");

                DrawFPS(10, 140, 200, 100);

                if (!mutePlayback) {
                    DrawFFT(10, 260, 200, 100);
                }

                DrawSeekbar(wnd->GetHeight() - 20, 20);
                DrawTimeText(wnd->GetHeight() - 45);

                Gui::ShapeRenderer * shp = data->GetShapeRenderer();

                if (rCompounds.GetTotal() > 0) {
                    iconCompound->Enable();
                    shp->Square(wnd->GetWidth() - 140, 10, 24, 24, Math::Color4ub());
                    iconCompound->Disable();

                    DrawText(wnd->GetWidth() - 100, 20, 24, false,Math::IntToString(rCompounds.GetTotal()));
                    DrawText(wnd->GetWidth() - 65, 20, 24, false, Math::IntToString(rCompounds.GetSuccess()),Math::Color4ub(0, 255, 0));
                    DrawText(wnd->GetWidth() - 30, 20, 24, false, Math::IntToString(rCompounds.GetFailure()),Math::Color4ub(255,0,0));
                }

                if (rShaders.GetTotal() > 0) {
                    iconShader->Enable();
                    shp->Square(wnd->GetWidth() - 140, 38, 24, 24, Math::Color4ub());
                    iconShader->Disable();

                    DrawText(wnd->GetWidth() - 100, 50, 24, false, Math::IntToString(rShaders.GetTotal()));
                    DrawText(wnd->GetWidth() - 65, 50, 24, false, Math::IntToString(rShaders.GetSuccess()), Math::Color4ub(0, 255, 0));
                    DrawText(wnd->GetWidth() - 30, 50, 24, false, Math::IntToString(rShaders.GetFailure()), Math::Color4ub(255, 0, 0));
                }

                if (rTextures.GetTotal() > 0) {
                    iconTexture->Enable();
                    shp->Square(wnd->GetWidth() - 140, 68, 24, 24, Math::Color4ub());
                    iconTexture->Disable();

                    DrawText(wnd->GetWidth() - 100, 80, 24, false, Math::IntToString(rTextures.GetTotal()));
                    DrawText(wnd->GetWidth() - 65, 80, 24, false, Math::IntToString(rTextures.GetSuccess()), Math::Color4ub(0, 255, 0));
                    DrawText(wnd->GetWidth() - 30, 80, 24, false, Math::IntToString(rTextures.GetFailure()), Math::Color4ub(255, 0, 0));
                }
            }

            dev->PresentAll();
        }
               
        if(player)
            player->Stop();

        ReleaseStuff();
    }

    double UpdateFPS() {
        const double ttime = frTime->GetElapsedTime();
        double fps = 1.00f / (ttime / 1.0e6);
        if (fps > 200)
            fps = 200;
        fpsQueue.push_front(fps);
        if (fpsQueue.size() > 50)
            fpsQueue.pop_back();
        double meanFps = 0.0;
        ListFor(float, fpsQueue, i)
            meanFps += (*i);
        meanFps /= fpsQueue.size();
        frTime->Start(); // restart
        return meanFps;
    }
    
    void ReleaseStuff() {
        SafeDelete(data);
        SafeDelete(player);
        SafeDelete(dev);
    }

    int64_t GetTime() {
        if (mutePlayback) {
            return lastTime + timer->GetElapsedTime();
        }
        return player->GetTime();
    }

    int64_t GetDuration() {
        return duration;
    }

    void DrawText(float x, float y, float size, bool bold, const std::string & str, const Math::Color4ub & col = Math::Color4ub(255,255,255,255)) {
        fontTexture->Enable();
        dev->Color(0, 0, 0, col.GetA()*0.8);
        fontMap->Draw(x+1, y+1, size, str);

        dev->Color(col.GetR(), col.GetG(), col.GetB(), col.GetA());
        fontMap->Draw(x, y, size, str);
        fontTexture->Disable();
    }
    
    void DrawTimeText(float y) {
        dev->Color(255, 255, 255, 255);
        DrawText(10, y, 50,true, Math::TimeToString(GetTime()));
        dev->Color(255, 0, 0, 255);
        DrawText(210, y + 7, 24,false, Math::TimeToString(GetDuration()));
    }

    void DrawFrameRate(float x, float y, float fps) {
        DrawText(x, y, 32, true, "FPS: "+Math::FloatToString(fps,3));
    }

    void DrawSeekbar(float y, float height) {
        int64_t duration = GetDuration();
        int64_t audioTime = GetTime();
        float margin = 8;
        shpRender->Square(0, y, wnd->GetWidth(), height, Math::Color4ub(100, 100, 100,150));
        shpRender->Square(margin / 2, wnd->GetHeight() - (height-margin / 2), (wnd->GetWidth() - margin) * (audioTime / (float)duration), height-margin, Math::Color4ub(255, 255, 255,200));
    }

    void DrawFFT(float x, float y, float width, float height) {
        float groupWidth = 2;
        const float diffDB = player->GetFFT()->GetMax_dB() - player->GetFFT()->GetMin_dB();
        const float maxDB = player->GetFFT()->GetMax_dB();

        float freq_start = 40;
        float freq_max = 4000;
        float freq_step = 40;

        groupWidth =floor(width/((freq_max - freq_start) / freq_step));

        shpRender->Square(x, y, width, height, Math::Color4ub(100, 100, 100,155));
        
        int steps = diffDB / 10;
        for (int i = 0; i <= steps; i++)
        {
            float yP = y + i * (height / steps);
            shpRender->Square(x, yP, width, 1, Math::Color4ub(0, 0, 0, 50));
            DrawText(x + width + 10, yP, 16, false, Math::FloatToString(maxDB - (i * diffDB / steps), 3) + "dB");
        }

        for (int i = 0, f = freq_start; f <= freq_max; f += freq_step, i++)
        {
            double dB = player->GetdB(f);
            dB = (dB / diffDB) * height;
            shpRender->Square(x+groupWidth * i, y - dB + height, groupWidth, dB, Math::Color4ub(255, 255, 255,200));
        }

        shpRender->Square(x, y, 1, height, Math::Color4ub(0, 0, 0, 50));
        shpRender->Square(x + width, y, 1, height, Math::Color4ub(0, 0, 0, 50));
    }

    void DrawFPS(float x, float y, float width, float height) {
        const float fpsLimits[] = {30,60,120,180,240};

        shpRender->Square(x, y, width, height, Math::Color4ub(100, 100, 100, 155));
   
        float maxFPS = 10.0;
        ListFor(float, fpsQueue, i) {
            if (*i > maxFPS)
                maxFPS = *i;
        }

        float bestFPS = maxFPS;
        for (int i = 4; i >= 0; i--) {
            if (fpsLimits[i] - maxFPS > 10) {
                bestFPS = fpsLimits[i];
            }
        }
        
        int steps = 4;
        for (int i = 0; i <= steps; i++)
        {
            float yP = y + i * (height / steps);
            shpRender->Square(x, yP, width, 1, Math::Color4ub(0, 0, 0, 50));
            DrawText(x + width + 10, yP, 16, false, Math::FloatToString(bestFPS - i*(bestFPS / steps), 3) + "fps");
        }

        dev->Color(255, 255, 255, 200);
        dev->Begin(Graph::PRIMITIVE_LINES);
        float rx = x + width;
        Math::Vec2 lastPos;
        ListFor(float, fpsQueue, i) {
            float ry = y+height  - height * *i / bestFPS;

            if (lastPos.GetX() > 0) {
                dev->Vertex(rx, ry);
                dev->Vertex(lastPos.GetX(), lastPos.GetY());
            }
            lastPos = Math::Vec2(rx, ry);
            rx -= (width / (fpsQueue.size() -1));
        }
        dev->End();
     
        shpRender->Square(x, y, 1, height, Math::Color4ub(0, 0, 0, 50));
        shpRender->Square(x + width, y, 1, height, Math::Color4ub(0, 0, 0, 50));
    }

    void PlayPauseToggle() {
        if (mutePlayback) {
            if (timer->IsRunning()) {
                timer->Stop();
                lastTime = timer->GetElapsedTime();
            }
            else
                timer->Start();
        }
        else {
            if (player->isPlaying())
                player->Pause();
            else
                player->Play();
        }
    }

    void SeekTo(int64_t pos) {
        if (mutePlayback) {
            timer->Clear();
            timer->Start();
            lastTime = pos;
        }
        else {
            player->Seek(pos);
        }

        Core::DebugLog(Math::IntToString(pos));
    }

    void ReloadTimeline() {

    }

    void ReloadResources() {
        reloadResources = true;
    }

    void ToggleUI() {
        showUI = !showUI;
    }

    void UpdateWndInput() {
        static bool wasLeftClickDown = false;
        static bool wasPausePressed = false;
        static bool wasZPressed = false;
        static bool wasXPressed = false;
        static bool wasCPressed = false;

        if (wnd->GetMouseButtonStatus(Core::BUTTON_MOUSE_LEFT) == Core::BUTTON_STATUS_DOWN) {
            if(!wasLeftClickDown && showUI){
                Core::Point p = wnd->GetMousePosition();
                float off = (p.GetX() - 4) / (float)(wnd->GetWidth() - 8);
                if (p.GetY() > wnd->GetHeight() - 40) {
                    SeekTo(off * GetDuration());
                }
            }
            wasLeftClickDown = true;
        }
        else
            wasLeftClickDown = false;

        if (wnd->GetKeyStatus(Core::KeyboardButton::BUTTON_SPACE) == Core::ButtonStatus::BUTTON_STATUS_DOWN) {
            if (!wasPausePressed) PlayPauseToggle();
            wasPausePressed = true;
        }
        else
            wasPausePressed = false;

        if (wnd->GetKeyStatus(Core::KeyboardButton::BUTTON_Z) == Core::ButtonStatus::BUTTON_STATUS_DOWN) {
            if(!wasZPressed) ReloadTimeline();
            wasZPressed = true;
        }
        else
            wasZPressed = false;

        if (wnd->GetKeyStatus(Core::KeyboardButton::BUTTON_X) == Core::ButtonStatus::BUTTON_STATUS_DOWN) {
            if(!wasXPressed) ReloadResources();
            wasXPressed = true;
        }
        else
            wasXPressed = false;

        if (wnd->GetKeyStatus(Core::KeyboardButton::BUTTON_C) == Core::ButtonStatus::BUTTON_STATUS_DOWN) {
            if(!wasCPressed) ToggleUI();
            wasCPressed = true;
        }
        else
            wasCPressed = false;


        if (wnd->GetKeyStatus(Core::BUTTON_ESCAPE) != Core::BUTTON_STATUS_UP)
            Teardown();
    }

    void renderLoading(float progress) {
        dev->ClearColor(0, 0, 0, 1);
        dev->Clear();
        dev->Viewport(0, 0, wnd->GetWidth(), wnd->GetHeight());
        loading->Render(0, 10e6, (progress/100)*10e6);
        dev->PresentAll();
        //Core::Thread::Wait(100);
    }

private:
    int reloadTextureCounter[3];
    int reloadCompoundCounter[3];
    int reloadShaderFlag[3];

    bool reloadTimeline;
    bool reloadResources;
    
    bool showUI;
    Data * data;
    int64_t lastTime;               // Onde come�ou a tocar 
    std::list<float>    fpsQueue;   // Queue para filtrar os fps
    Core::Chronometer * frTime;     // Timer para a framerate
    Core::Chronometer * timer;      // Tempo passado desde come�ou a reproduzir
    int64_t duration;               // Dura��o total
    Math::TimelineNode<Subtitle*> subtitlesTimeline; // Suporte de desenho de legendas
    bool mutePlayback;              // Modo de reprodu��o sem pista de som
    AudioPlayer * player;           // Dispositivo de som e FFT
    float sDensity;                 // Densidade do ecr�
    float hScale;                   // Factor de escala vertical
    Gui::ShapeRenderer * shpRender;
    Graph::Texture	* fontTexture, * fontTimeTexture;   // Textura da fonte
    Gui::FontMap    * fontMap, * fontTimeMap;           // Mapa de caracteres da font
    Graph::Device   * dev;          // Dispositivo de gr�fico
    Core::Window    * wnd;
    dsSceneLoading * loading;
};

_DS_END



void Core::Application_Main(const std::vector<std::string> & CmdLine)
{
    DS::DemoSettings * conf = new DS::DemoSettings();
    int width = 1920, height = 1080;
    float scale =0.6;
 
    bool fullscreen = false;

#ifdef FULLSCREEN
    scale = 1.0;
    fullscreen = true;
#endif

    width *= scale;
    height *= scale;
    
    bool runDemo = true;//conf->Run(&width, &height, &fullscreen);

    SafeDelete(conf);

    if (runDemo) 
    {
        Core::Window * wnd = Core::CreateWindow("coprocessor", width, height, fullscreen);

        DS::GraphicRendering * grThread = new DS::GraphicRendering(wnd);

        grThread->Start();
        while (wnd->Peek(true)) 
        {
            if(fullscreen)
                wnd->SetCursorVisiblity(false);

            if (!grThread->IsRunning())
                break;
            
            grThread->UpdateWndInput();
        }
        grThread->TearDownAndJoin();

        SafeDelete(grThread);
        SafeDelete(wnd);
    }

}

