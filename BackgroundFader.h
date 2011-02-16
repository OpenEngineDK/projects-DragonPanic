#ifndef _BACKGROUND_FADER_
#define _BACKGROUND_FADER_

#include <Core/IListener.h>
#include <Core/EngineEvents.h>
#include <Renderers/IRenderer.h>

using OpenEngine::Core::IListener;
using OpenEngine::Core::ProcessEventArg;
using OpenEngine::Renderers::IRenderer;

class BackgroundFader : public IListener<OpenEngine::Core::ProcessEventArg> {
private:
  float timeSpend,time;
  bool done;
  Timer timer;
  IRenderer& renderer;
public:
  BackgroundFader(float time, IRenderer& renderer) : time(time), renderer(renderer) {
      timeSpend = 0.0;
      done = false;
      timer.Start();
  }
  
  void Handle(OpenEngine::Core::ProcessEventArg arg) {
      unsigned int dt = timer.GetElapsedTimeAndReset().AsInt();
      float deltaTime = ((float)dt)/1000.0;
      if (done) return;
      
      if (timeSpend >= time) {
          timeSpend = time;
          done = true;
      }
      else
          timeSpend += deltaTime;

      if (!done) {
          double pctDone = timeSpend/time;
          float pFade = 1.4 * pctDone;
          Vector<4,float> color( 0.39*pFade, 0.45*pFade, 1.0*pFade, 1.0 );
          //Vector<4,float> color( 0.6*pFade, 0.6*pFade, 0.6*pFade, 1.0 );
          renderer.SetBackgroundColor(color);
      }
  }
};

#endif // _BACKGROUND_FADER_
