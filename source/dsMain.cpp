
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#include "dsData.h"
#include "dsUtils.h"
#include "dsSettings.h"
#include "dsSubtitles.h"
#include "dsAudioPlayer.h"

#include "scenes/dsSceneScratch.h"
#include "scenes/dsSceneLoading.h"
#include "scenes/dsScene80Grid.h"
#include "scenes/dsSceneIcosphere.h"
#include "scenes/dsSceneOutputFilter.h"
#include "scenes/dsSceneMap.h"
#include "scenes/dsSceneSystemInit.h"
#include "scenes/dsSceneFloatPopup.h"
#include "scenes/dsSceneConsoleText.h"
#include "scenes/dsSceneFloatPopup_ScrollCode.h"
#include "scenes/dsSceneFloat_User.h"
#include "scenes/dsSceneFloat_Text.h"
#include "scenes/dsSceneFloat_Text_Attached.h"
#include "scenes/dsSceneSystemShutdown.h"
#include "scenes/dsSceneMetaballs.h"

// Para correr sem audio usar NULL
#define AUDIO_STREAM        "audio://demobit2017.ogg"
#define AUDIO_SAMPLERATE    44100
#define AUDIO_BUFFERS       4
#define AUDIO_FFT           2048

//#define FULLSCREEN

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
            
            iconTexture = data->LoadTexture("texture://iconTexture.png");
            iconShader = data->LoadTexture("texture://iconShader.png");
            iconCompound = data->LoadTexture("texture://iconCompound.png");

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

            renderLoading(10);
            dsSceneOutputFilter * scene = new dsSceneOutputFilter(data);
            scene->Load();


            
            // Init scene
            dsSceneSysInit * sysInit = new dsSceneSysInit(data);
            sysInit->Load();
            scene->AddStage(0e6, 40e6, sysInit);

            std::string texts[] = {
                "62 days to Demobit 2017",
				"Generating demo...",
                "Resource failure",
                "31 days to Demobit 2017",
                "Generating demo...",
                "Resource failure",
                "15 days to Demobit 2017",
                "Connecting to human-machine interfaces...",
                "3 usable interfaces found",
            };

            for (int i = 0; i < 9; i++) {
                int64_t duration = 4e6;
                dsSceneConsoleText * sysText = new dsSceneConsoleText(data);
               
                sysText->SetText(texts[i], Math::Vec2(1920/2, 900), 20);

                int64_t time = 1e6 + i*duration;
                scene->AddStage(time, 1e6 + (i+1)*duration, sysText);

                if (i == 1 || i == 4 || i == 7)
                {
                    for (int j = 0; j < 3; j++) {
                        dsSceneFloatPopup_SC * pop = new dsSceneFloatPopup_SC(data);
                        pop->Load();

                        float x = Math::RandomValue(50, 450);
                        float y = Math::RandomValue(50, 500);
                        float w = Math::RandomValue(300, 440);
                        float h = Math::RandomValue(240, 360);

                        float tOffset = Math::RandomValue(2, 4) * 1e6;

                        pop->SetDimensions(w, h);
                        pop->SetAnimation(Math::Vec2(x, y), Math::Vec2(x, y + 20), 0.3e6, 0.15e6);

                        scene->AddStage(time + 1e6, time + 1e6 + tOffset, pop);
                    }

                    for (int j = 0; j < 3; j++) {
                        dsSceneFloatPopup_SC * pop = new dsSceneFloatPopup_SC(data);
                        pop->Load();

                        float x = Math::RandomValue(1224, 1400);
                        float y = Math::RandomValue(50, 500);
                        float w = Math::RandomValue(300, 440);
                        float h = Math::RandomValue(240, 360);

                        float tOffset = Math::RandomValue(2, 4) * 1e6;

                        pop->SetDimensions(w, h);
                        pop->SetAnimation(Math::Vec2(x, y), Math::Vec2(x, y + 20), 0.3e6, 0.15e6);

                        scene->AddStage(time + 1e6, time + 1e6 + tOffset, pop);
                    }

                }
            }
            
            

            // Map scene
            renderLoading(20);
            dsSceneMap *map = new dsSceneMap(data);
            map->Load();
            renderLoading(30);
            scene->AddStage(41e6, 137e6, map);
            
            {
                dsSceneFloat_User * user_jaerder = new dsSceneFloat_User(data);
                user_jaerder->Load();
                user_jaerder->SetAnimation(Math::Vec2(100, 100), Math::Vec2(100, 150), 0.3e6, 0.15e6);
                user_jaerder->SetDimensions(300, 400);
                scene->AddStage(50e6, 60e6, user_jaerder);


				dsSceneFloat_Text * text = new dsSceneFloat_Text(data);
				text->Load();
				text->SetAnimation(Math::Vec2(250, 356), Math::Vec2(250, 406), 0.3e6, 0.15e6);
				text->SetDimensions(450, 200);
				text->setType(0);
				scene->AddStage(55e6, 605e5, text);
            }

            {
                dsSceneFloat_User * user_zeroshift = new dsSceneFloat_User(data);
                user_zeroshift->setUser(1);
                user_zeroshift->Load();
                user_zeroshift->SetAnimation(Math::Vec2(100, 500), Math::Vec2(100, 550), 0.3e6, 0.15e6);
                user_zeroshift->SetDimensions(300, 400);
                scene->AddStage(62e6, 72e6, user_zeroshift);

				dsSceneFloat_Text * text = new dsSceneFloat_Text(data);
				text->Load();
				text->SetAnimation(Math::Vec2(250, 356), Math::Vec2(250, 406), 0.3e6, 0.15e6);
				text->SetDimensions(400, 200);
				text->setType(1);
				scene->AddStage(67e6, 725e5, text);
            }

            {
				int x_offset = 1000;
				int y_offset = 64;
				dsSceneFloat_User * user_ps = new dsSceneFloat_User(data);
                user_ps->setUser(2);
                user_ps->Load();
                user_ps->SetAnimation(Math::Vec2(420 + x_offset , 500 + y_offset), Math::Vec2(420 + x_offset, 550 + y_offset), 0.3e6, 0.15e6);
                user_ps->SetDimensions(300, 400);
                scene->AddStage(62e6, 72e6, user_ps);

				dsSceneFloat_Text * text = new dsSceneFloat_Text(data);
				text->Load();
				text->SetAnimation(Math::Vec2(0 + x_offset, 356 + (y_offset*3)), Math::Vec2(0 + x_offset, 406 + (y_offset * 3) ), 0.3e6, 0.15e6);
				text->SetDimensions(400, 200);
				text->setType(2);
				scene->AddStage(67e6, 725e5, text);
            }

            {
                dsScene80Grid * gridPopup = new dsScene80Grid(data);
                gridPopup->SetDimensions(400, 400);
                gridPopup->SetAnimation(Math::Vec2(120, 60), Math::Vec2(180, 60), 0.3e6, 0.15e6);
                gridPopup->Load();
                scene->AddStage(80e6, 100e6, gridPopup);

                dsSceneFloat_Text * attached = new dsSceneFloat_Text(data);
                attached->setType(10000);
                attached->SetDimensions(400, 160);
                attached->SetAnimation(Math::Vec2(20, 480), Math::Vec2(50, 480), 0.3e6, 0.15e6);
                attached->Load();
                scene->AddStage(82e6, 100.5e6, attached);
            }

            {
                dsSceneMetaballs *metaballs = new dsSceneMetaballs(data);
                metaballs->Load();
                metaballs->SetDimensions(400, 400);
                metaballs->SetAnimation(Math::Vec2(100, 500), Math::Vec2(100, 500), 0.3e6, 0.15e6);
                scene->AddStage(110e6, 120e6, metaballs);

                dsSceneFloat_Text * attached = new dsSceneFloat_Text(data);
                attached->setType(10001);
                attached->SetDimensions(500, 160);
                attached->SetAnimation(Math::Vec2(300, 850), Math::Vec2(300, 880), 0.3e6, 0.15e6);
                attached->Load();
                scene->AddStage(112e6, 120.5e6, attached);
            }

            {
                dsSceneFloat_Text_Attached * attached = new dsSceneFloat_Text_Attached(data, map);
                attached->setId("Porto");
                attached->setType(3);
                attached->SetDimensions(650, 160);
                attached->SetAnimation(Math::Vec2(1000, 100), Math::Vec2(1000, 150), 0.3e6, 0.15e6);
                attached->Load();
                scene->AddStage(50e6, 60e6, attached);

			}

			{
				dsSceneFloat_Text_Attached * attached_l = new dsSceneFloat_Text_Attached(data, map);
				attached_l->setId("Lisboa");
				attached_l->setType(4);
				attached_l->SetDimensions(400, 200);
				attached_l->SetAnimation(Math::Vec2(1000, 200), Math::Vec2(1000, 250), 0.3e6, 0.15e6);
				attached_l->Load();
				scene->AddStage(61e6, 71e6, attached_l);
			}

			{
				dsSceneFloat_Text_Attached * attached_m = new dsSceneFloat_Text_Attached(data, map);
				attached_m->setId("Madrid");
				attached_m->setType(5);
				attached_m->SetDimensions(650, 160);
				attached_m->SetAnimation(Math::Vec2(1000, 450), Math::Vec2(1000, 500), 0.3e6, 0.15e6);
				attached_m->Load();
				scene->AddStage(83e6, 88e6, attached_m);
			}


			{
				dsSceneFloat_Text_Attached * attached_b = new dsSceneFloat_Text_Attached(data, map);
				attached_b->setId("Bilbao");
				attached_b->setType(6);
				attached_b->SetDimensions(550, 160);
				attached_b->SetAnimation(Math::Vec2(350, 800), Math::Vec2(350, 800), 0.3e6, 0.15e6);
				attached_b->Load();
				scene->AddStage(91e6, 95e6, attached_b);
			} 
			{
				dsSceneFloat_Text_Attached * attached_m = new dsSceneFloat_Text_Attached(data, map);
				attached_m->setId("Marseille");
				attached_m->setType(8);
				attached_m->SetDimensions(600, 110);
				attached_m->SetAnimation(Math::Vec2(1000, 450), Math::Vec2(1000, 500), 0.3e6, 0.15e6);
				attached_m->Load();
				scene->AddStage(100e6, 104e6, attached_m);
			}

			{
				dsSceneFloat_Text_Attached * attached_t = new dsSceneFloat_Text_Attached(data, map);
				attached_t->setId("Thoiseey");
				attached_t->setType(7);
				attached_t->SetDimensions(400, 110);
				attached_t->SetAnimation(Math::Vec2(1000, 300), Math::Vec2(1000, 300), 0.3e6, 0.15e6);
				attached_t->Load();
				scene->AddStage(102e6, 105e6, attached_t);
			}

			{
				dsSceneFloat_Text_Attached * attached_be = new dsSceneFloat_Text_Attached(data, map);
				attached_be->setId("Bern");
				attached_be->setType(10);
				attached_be->SetDimensions(600, 110);
				attached_be->SetAnimation(Math::Vec2(1000, 450), Math::Vec2(1000, 500), 0.3e6, 0.15e6);
				attached_be->Load();
				scene->AddStage(105e6, 109e6, attached_be);
			}

			{
				dsSceneFloat_Text_Attached * attached_mu = new dsSceneFloat_Text_Attached(data, map);
				attached_mu->setId("BezierCurve");
				attached_mu->setType(99);
				attached_mu->SetDimensions(600, 160);
				attached_mu->SetAnimation(Math::Vec2(1000, 450), Math::Vec2(1000, 500), 0.3e6, 0.15e6);
				attached_mu->Load();
				//scene->AddStage(100e6, 101e6, attached_mu);
			}

			{
				dsSceneFloat_Text_Attached * attached_br = new dsSceneFloat_Text_Attached(data, map);
				attached_br->setId("Brno");
				attached_br->setType(12);
				attached_br->SetDimensions(600, 160);
				attached_br->SetAnimation(Math::Vec2(1000, 200), Math::Vec2(1000, 200), 0.3e6, 0.15e6);
				attached_br->Load();
				scene->AddStage(120e6, 124e6, attached_br);
			}

			{
				dsSceneFloat_Text_Attached * attached_tr = new dsSceneFloat_Text_Attached(data, map);
				attached_tr->setId("Trencin");
				attached_tr->setType(13);
				attached_tr->SetDimensions(600, 160);
				attached_tr->SetAnimation(Math::Vec2(1000, 450), Math::Vec2(1000, 500), 0.3e6, 0.15e6);
				attached_tr->Load();
				scene->AddStage(124e6, 130e6, attached_tr);
			}

			{
				dsSceneFloat_Text_Attached * attached_bra = new dsSceneFloat_Text_Attached(data, map);
				attached_bra->setId("Bratislava");
				attached_bra->setType(14);
				attached_bra->SetDimensions(600, 200);
				attached_bra->SetAnimation(Math::Vec2(1000, 450), Math::Vec2(1000, 500), 0.3e6, 0.15e6);
				attached_bra->Load();
				scene->AddStage(130e6, 136e6, attached_bra);
			}

            {
                dsSceneFloat_User * user_party_code = new dsSceneFloat_User(data);
                user_party_code->setUser(3);
                user_party_code->Load();
                user_party_code->SetAnimation(Math::Vec2(50, 100), Math::Vec2(100, 100), 0.3e6, 0.15e6);
                user_party_code->SetDimensions(400, 400);
                scene->AddStage(120e6, 130e6, user_party_code);

                dsSceneFloat_Text * attached = new dsSceneFloat_Text(data);
                attached->setType(10002);
                attached->SetDimensions(400, 160);
                attached->SetAnimation(Math::Vec2(20, 480), Math::Vec2(50, 480), 0.3e6, 0.15e6);
                attached->Load();
                scene->AddStage(121e6, 130.5e6, attached);
            }


            {
                int64_t startOff = 137.5e6;
                dsSceneSysShutdown * shutdown = new dsSceneSysShutdown(data);
                shutdown->Load();
                scene->AddStage(startOff, startOff + 20e6, shutdown);


                std::string texts[] = {
                    "Demo concept implemented successfully",
                    "Thank you lazy humans",
                };

                for (int i = 0; i < 2; i++) {
                    int64_t duration = 4e6;
                    dsSceneConsoleText * sysText = new dsSceneConsoleText(data);

                    sysText->SetText(texts[i], Math::Vec2(1920 / 2, 900), 20);

                    int64_t time = startOff + i*duration;
                    scene->AddStage(time, time + duration, sysText);
                }
            }


            scene->BuildTimeline();

            //timeline.Insert(Math::TimelineItem<DS::Stage*>(0,1e6, loading));
            timeline.Insert(Math::TimelineItem<DS::Stage*>(0e6, 300e6, scene));

            /*dsSceneFloatPopup * p = new dsSceneFloatPopup(data);
            p->Load();
            p->SetDimensions(500, 400);
            p->SetAnimation(Math::Vec2(0, 0), Math::Vec2(0, 0), 0.3e6, 0.15e6);
            timeline.Insert(Math::TimelineItem<DS::Stage*>(0, 100e6, p));
            */
            renderLoading(99.9);
           // Core::Thread::Wait(200);
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
            int64_t time = timer->GetElapsedTime();
            float time_in_secs = time / 1e6;
            
            if (time_in_secs > 150)
                break;

            timeline.Get(time, &items); // Nota: Ir buscar em segundos
            ListFor(Math::TimelineItem<DS::Stage*>, items, i) {
                i->GetObject()->RenderFBO(i->GetStart(), i->GetEnd(), time);

                i->GetObject()->Render(i->GetStart(), i->GetEnd(), time);
            }

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
        Core::Thread::Wait(100);
    }

private:
    int reloadTextureCounter[3];
    int reloadCompoundCounter[3];
    int reloadShaderFlag[3];

    bool reloadTimeline;
    bool reloadResources;
    
    bool showUI;
    Data * data;
    int64_t lastTime;               // Onde começou a tocar 
    std::list<float>    fpsQueue;   // Queue para filtrar os fps
    Core::Chronometer * frTime;     // Timer para a framerate
    Core::Chronometer * timer;      // Tempo passado desde começou a reproduzir
    int64_t duration;               // Duração total
    Math::TimelineNode<Subtitle*> subtitlesTimeline; // Suporte de desenho de legendas
    bool mutePlayback;              // Modo de reprodução sem pista de som
    AudioPlayer * player;           // Dispositivo de som e FFT
    float sDensity;                 // Densidade do ecrã
    float hScale;                   // Factor de escala vertical
    Gui::ShapeRenderer * shpRender;
    Graph::Texture	* fontTexture, * fontTimeTexture;   // Textura da fonte
    Gui::FontMap    * fontMap, * fontTimeMap;           // Mapa de caracteres da font
    Graph::Device   * dev;          // Dispositivo de gráfico
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
    
    bool runDemo = conf->Run(&width, &height, &fullscreen);

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


/*AddSub(2, 4, "Demoscene\n2016!");
AddSub(8, 4, "Scene-pt\nYo yo, toca a levantar a mini!");
AddSub(14, 4, "E toca a codar uma demo!");
AddSub(20, 4, "Efeito por\nxernobyl");
AddSub(26, 4, "Arruinado por\nzeroshift");
AddSub(32, 4, "Ye");*/

//Graph::Program * particle = NULL;
//Graph::Program * pCanvas = NULL;
//Graph::RTManager * rtManager = NULL;
//Graph::Texture2D * rtTexture = NULL;

//particle = dev->LoadProgram("shader://particle.cpp");
//pCanvas = dev->LoadProgram("shader://canvas.cpp");
//pCanvas->SetVariable1i("texture_0", 0);
//rtManager = dev->CreateRTManager(wnd->GetWidth(), wnd->GetHeight());
//rtTexture = dynamic_cast<Graph::Texture2D*>(dev->CreateTexture(Graph::TEXTURE_2D, wnd->GetWidth(), wnd->GetHeight()));
//rtManager->Attach(0, rtTexture);


//scene->AddStage(1, 10000, map);

//scene->AddStage(1, 2000, grid80);
//scene->AddStage(1, 2000, ico);
/*dsSceneScratch *scratch = new dsSceneScratch(data);
scratch->Load();

dsScene80Grid *grid80 = new dsScene80Grid(data);
grid80->Load();

dsSceneIcosphere * ico = new dsSceneIcosphere(data);
ico->Load();*/