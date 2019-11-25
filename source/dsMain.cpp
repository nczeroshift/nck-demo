
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#include "dsTimeline.h"
#include "nckUtils.h"

//#define FULLSCREEN

static bool compareTLItem(const Math::TimelineItem<DS::Stage*> & a, const Math::TimelineItem<DS::Stage*> & b) {
    return a.GetObject()->GetOrder() < b.GetObject()->GetOrder();
}

_DS_BEGIN

dsMain::dsMain(Core::Window * window) : Threadable() {
    wnd = window;
    dev = NULL;
    fontMap = NULL;
    fontTexture = NULL;
    fontTimeTexture = NULL;
    fontTimeMap = NULL;
    mutePlayback = false;
    player = NULL;
    lastTime = 0;
    duration = 3*60e6;
    showUI = false;
    sDensity = Core::Window::GetDisplayDensity();
    hScale = wnd->GetHeight() / (float)REFERENCE_HEIGHT;
    timer = Core::CreateChronometer();
    frTime = Core::CreateChronometer();
    shpRender = NULL;
    server = NULL;

    for (int i = 0; i < 3; i++) {
        reloadShaderFlag[i] = 0;
        reloadTextureCounter[i] = 0;
        reloadCompoundCounter[i] = 0;
        reloadShaderFlag[i] = 0;
    }
    reloadResources = false;
    reloadTimeline = false;
}

dsMain::~dsMain() {
    SafeDelete(timer);
    SafeDelete(frTime);
    std::list<Subtitle*> subs;
    subtitlesTimeline.GetAll(&subs);
    ListFor(Subtitle*, subs, i) {
        delete (*i);
    }
}

void dsMain::AddSub(float start, float duration, const std::string & text) {
    Subtitle * s = subtitlesTimeline.Insert(Math::TimelineItem<Subtitle*>(start, start+duration, new Subtitle(dev, fontTexture, fontMap)));
    s->Set(text, hScale * REFERENCE_SUBTITLE_SIZE);
}
    
void dsMain::Run() {
      
    Graph::Texture * iconTexture;
    Graph::Texture * iconCompound;
    Graph::Texture * iconShader;

    Math::TimelineNode<DS::Stage*> timeline;

    try {
        dev = Graph::CreateDevice(wnd, Graph::DEVICE_AUTO, wnd->GetWidth(), wnd->GetHeight());
        dev->ClearColor(1.0, 1.0, 1.0, 1.0);
        dev->ClearFlags(Graph::BUFFER_COLOR_BIT | Graph::BUFFER_DEPTH_BIT);

        dev->Enable(Graph::STATE_ZBUFFER_WRITE);

        // OpenGL basic stuff init.
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

        // Load demo music
        renderLoading(0);
        std::string audioFilename;
        int audioSampleRate;
        int audioBuffers;
        int audioFFT;
        if (LoadMusic(&audioFilename, &audioSampleRate, &audioBuffers, &audioFFT)) {
            player = new AudioPlayer(audioSampleRate, audioFFT, audioBuffers);
            player->Start();
            player->LoadBG(audioFilename);
            duration = player->GetDuration();
        }
        else
            mutePlayback = true;

        renderLoading(5);
        
        // Load hardcoded timeline.
        LoadTimeline(&timeline);

        // Load XML timeline.
        //data->LoadTimeline(&timeline);

        renderLoading(99.9);

        server = new Server(data);
        server->Start();
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
        /*if (server) {
            if (!server->FreeRun()) {
                Core::Thread::Wait(32);
            }
        }*/

        dev->ClearColor(0, 0, 0, 1);
        dev->Clear();       

		if (server != NULL) {
			ServerOperation * op = NULL;
			if (server->FetchOperation(SERVER_OP_RELOAD_SHADER, &op)){
				rShaders = data->ReloadShaders();

				if (rShaders.GetFailure()) {
					std::list<std::string> shadersErrors;
					data->GetShaderErrors(&shadersErrors);
					std::string shdErrorString = "";

					ListFor(std::string, shadersErrors, j) {
						if (shdErrorString.empty())
							shdErrorString += (*j);
						else
							shdErrorString += "\n" + (*j);
					}

					op->SetError(shdErrorString);
				}

				op->Finish();
			}

		}

        /*bool serverReload = false;
        if (server) {
            serverReload = server->BeginReload();
        }

        if (reloadResources || serverReload) 
		{
			data->LockAccess();
            rShaders = data->ReloadShaders();
            rCompounds = data->ReloadCompounds();
            rTextures = data->ReloadTextures();
            reloadResources = false;
			data->UnlockAccess();

			std::list<std::string> shadersErrors;
			data->GetShaderErrors(&shadersErrors);
			std::string shdErrorString = "";

			ListFor(std::string, shadersErrors, j) {
				if (shdErrorString.empty())
					shdErrorString += (*j);
				else
					shdErrorString += "\n" + (*j);
			}

			if (server) server->FinishReload(shdErrorString);
        }*/

        std::list<Math::TimelineItem<DS::Stage*>> items;
        int64_t time = GetTime();
         
        float time_in_secs = time / 1e6;
            
        if (time_in_secs > this->GetMaxDuration())
            break;

		Graph::Program::SetGlobalTime(time_in_secs);

        timeline.Get(time, &items); 
        items.sort(compareTLItem);

        ListFor(Math::TimelineItem<DS::Stage*>, items, i) {
            i->GetObject()->Update(i->GetStart(), i->GetEnd(), time);
        }

        ListFor(Math::TimelineItem<DS::Stage*>, items, i) {
            i->GetObject()->RenderFBO(i->GetStart(), i->GetEnd(), time);
        }

        dev->Viewport(0, 0, wnd->GetWidth(), wnd->GetHeight());

        ListFor(Math::TimelineItem<DS::Stage*>, items, i) {
            i->GetObject()->Render(i->GetStart(), i->GetEnd(), time);
        }
        
        if (server) {
            server->FetchImage(dev, wnd->GetWidth(), wnd->GetHeight());
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

double dsMain::UpdateFPS() {
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
    
void dsMain::ReleaseStuff() {
    SafeDelete(data);
    SafeDelete(player);
    SafeDelete(dev);
}

int64_t dsMain::GetTime() {
    if (mutePlayback) {
        return lastTime + timer->GetElapsedTime();
    }
    return player->GetTime();
}

int64_t dsMain::GetDuration() {
    return duration;
}

void dsMain::DrawText(float x, float y, float size, bool bold, const std::string & str, const Math::Color4ub & col) {
    fontTexture->Enable();
    dev->Color(0, 0, 0, col.GetA()*0.8);
    fontMap->Draw(x+1, y+1, size, str);

    dev->Color(col.GetR(), col.GetG(), col.GetB(), col.GetA());
    fontMap->Draw(x, y, size, str);
    fontTexture->Disable();
}
    
void dsMain::DrawTimeText(float y) {
    dev->Color(255, 255, 255, 255);
    DrawText(10, y, 50,true, Math::TimeToString(GetTime()));
    dev->Color(255, 0, 0, 255);
    DrawText(210, y + 7, 24,false, Math::TimeToString(GetDuration()));
}

void dsMain::DrawFrameRate(float x, float y, float fps) {
    DrawText(x, y, 32, true, "FPS: "+Math::FloatToString(fps,3));
}

void dsMain::DrawSeekbar(float y, float height) {
    int64_t duration = GetDuration();
    int64_t audioTime = GetTime();
    float margin = 8;
    shpRender->Square(0, y, wnd->GetWidth(), height, Math::Color4ub(100, 100, 100,150));
    shpRender->Square(margin / 2, wnd->GetHeight() - (height-margin / 2), (wnd->GetWidth() - margin) * (audioTime / (float)duration), height-margin, Math::Color4ub(255, 255, 255,200));
}

void dsMain::DrawFFT(float x, float y, float width, float height) {
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
        DrawText(x + width + 10, yP, 16, false, Math::FloatToString(maxDB - (i * diffDB / steps), 1) + "dB");
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

void dsMain::DrawFPS(float x, float y, float width, float height) {
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
        DrawText(x + width + 10, yP, 16, false, Math::FloatToString(bestFPS - i*(bestFPS / steps), 1) + "fps");
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

void dsMain::PlayPauseToggle() {
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

void dsMain::SeekTo(int64_t pos) {
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

void dsMain::ReloadTimeline() {

}

void dsMain::ReloadResources() {
    reloadResources = true;
}

void dsMain::ToggleUI() {
    showUI = !showUI;
}

void dsMain::UpdateWndInput() {
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

void dsMain::renderLoading(float progress) {
    dev->ClearColor(0, 0, 0, 1);
    dev->Clear();
    dev->Viewport(0, 0, wnd->GetWidth(), wnd->GetHeight());
    loading->Render(0, 10e6, (progress/100)*10e6);
    dev->PresentAll();
    //Core::Thread::Wait(100);
}

_DS_END


void Core::Application_Main(const std::vector<std::string> & CmdLine)
{
    DS::DemoSettings * conf = new DS::DemoSettings();
    int width = 1920, height = 1080;
    float scale = 0.6;
 
    bool fullscreen = false;

#ifdef FULLSCREEN
    scale = 1.0;
    fullscreen = true;
#endif

    width *= scale;
    height *= scale;
    
    bool runDemo = true;// conf->Run(&width, &height, &fullscreen);

    SafeDelete(conf);

    if (runDemo) 
    {
        Core::Window * wnd = Core::CreateWindow("coprocessor", width, height, fullscreen);

        DS::dsMain * grThread = new DS::dsTimeline(wnd);

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

