
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#include "dsData.h"
#include "dsUtils.h"
#include "dsSettings.h"
#include "dsSubtitles.h"
#include "dsAudioPlayer.h"
#include "dsStage.h"
#include "tinyxml.h"
#include "dsSceneLoading.h"
#include "dsServer.h"

_DS_BEGIN

class dsMain : public virtual Core::Threadable
{
public:
    dsMain(Core::Window * window);
    virtual ~dsMain();

    void AddSub(float start, float duration, const std::string & text);

    void Run();

    virtual void LoadTimeline(Math::TimelineNode<DS::Stage*> * timeline) = 0;
    virtual bool LoadMusic(std::string * filename, int * sampleRate, int * buffers, int * fftSize) = 0;
    virtual float GetMaxDuration() = 0; // Max demo duration in seconds.

    double UpdateFPS();
    
    void ReleaseStuff();

    int64_t GetTime();

    int64_t GetDuration();

    void DrawText(float x, float y, float size, bool bold, const std::string & str, const Math::Color4ub & col = Math::Color4ub(255, 255, 255, 255));
    
    void DrawTimeText(float y);

    void DrawFrameRate(float x, float y, float fps);

    void DrawSeekbar(float y, float height);

    void DrawFFT(float x, float y, float width, float height);

    void DrawFPS(float x, float y, float width, float height);

    void PlayPauseToggle();

    void SeekTo(int64_t pos);

    void ReloadTimeline();

    void ReloadResources();

    void ToggleUI();

    void UpdateWndInput();

    void renderLoading(float progress);

protected:
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
    Server * server;
};

_DS_END


