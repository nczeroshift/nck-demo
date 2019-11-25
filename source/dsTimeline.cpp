
/**
* NCK-DEMO SCENE
* https://github.com/nczeroshift/nck-demo
*/

#include "dsTimeline.h"
#include "scenes/dsOutputPostProcess.h"

#include "scenes/dsSolidColor.h"
#include "scenes/dsFloatText.h"
#include "scenes/dsConsoleText.h"

#include "scenes/dsTestScene.h"

// Comentar/Descomentar o audiostream desactiva/activa o suporte de audio.
//#define AUDIO_STREAM        "audio://soundtrack.ogg" 
//#define AUDIO_SAMPLERATE    44100
//#define AUDIO_BUFFERS       4
//#define AUDIO_FFT           2048

#define T_S(s) s*1e6
#define TM_S(m,s) (m*60.0+s)*1e6

_DS_BEGIN

dsTimeline::dsTimeline(Core::Window * window) : dsMain(window) {

}

dsTimeline::~dsTimeline() {

}

float dsTimeline::GetMaxDuration() {
    return 30*60.0;
}

void dsTimeline::LoadTimeline(Math::TimelineNode<DS::Stage*> * timeline) 
{    
    const float width = data->GetWidth();
    const float height = data->GetHeight();

    dsOutputPostProcess * output = new dsOutputPostProcess(data);
  
    renderLoading(20);
    output->Load();
    renderLoading(100);
  
    // Fade In
    /*
    {
        dsSolidColor * fadeIn = new dsSolidColor(data);
        fadeIn->SetColors(Math::Color4ub(0, 0, 0, 255), Math::Color4ub(0, 0, 0, 0), true);
        fadeIn->SetOrder(9999);
        output->AddStage(0, T_S(5), fadeIn);
    }
    */

	/*{
		dsIdentify * test = new dsIdentify(data);
		test->Load();
		timeline->Insert(Math::TimelineItem<DS::Stage*>(0e6, GetMaxDuration()*1e6, new StageProxy(test)));
		//output->AddStage(0, GetMaxDuration()*1e6, test);
	}*/

    {
        dsTestScene * test = new dsTestScene(data);
        test->Load();
		timeline->Insert(Math::TimelineItem<DS::Stage*>(0e6, GetMaxDuration()*1e6, new StageProxy(test))); 
    }

   // timeline->Insert(Math::TimelineItem<DS::Stage*>(0e6, GetMaxDuration()*1e6, new StageProxy(output)));
	//timeline->Insert(Math::TimelineItem<DS::Stage*>(0e6, GetMaxDuration()*1e6, new StageProxy(output)));
}

bool dsTimeline::LoadMusic(std::string * filename, int * sampleRate, int * buffers, int * fftSize) {
#ifdef AUDIO_STREAM
        *filename = AUDIO_STREAM;
        *sampleRate = AUDIO_SAMPLERATE;
        *buffers = AUDIO_BUFFERS;
        *fftSize = AUDIO_FFT;
        return true;
#else
    return false;
#endif
}

_DS_END
