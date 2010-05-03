#ifndef _OSC_SURFACE_H_
#define _OSC_SURFACE_H_

// using templates
#include <Math/Vector.h>

// inherits from
#include <Core/IListener.h>
#include <Core/EngineEvents.h>
#include <Scene/RenderNode.h>

#include <Utils/Timer.h>

class HeightMap;
namespace OpenEngine {
    namespace Core {
      class ProcessEventArg;
    }
    namespace Renderers {
        class IRenderingView;
    }
}

using OpenEngine::Core::IListener;
using OpenEngine::Core::ProcessEventArg;
using OpenEngine::Core::InitializeEventArg;
using OpenEngine::Core::DeinitializeEventArg;
using OpenEngine::Scene::RenderNode;
using OpenEngine::Scene::ISceneNodeVisitor;
using OpenEngine::Renderers::RenderingEventArg;
using OpenEngine::Math::Vector;
using OpenEngine::Utils::Timer;

class OscSurface : public IListener<OpenEngine::Core::ProcessEventArg>, public RenderNode {
private:
  HeightMap* heightMap;

  float   *z_norm;
  double runningTime;

  // rubberPlane
  float   *zCur, *zOld, *zNew;
  double  t;  // time stamp for 'z'
  int     M, N;
  float   aspect;
  float   c;
  double  dt;  // integration time step
  float   taux2, tauy2, tauxy2;

  Vector<3,float> translate;
  float scale;

  Timer timer;
  Vector<4,float> color;

  inline float GaussPeak(float my_x, float my_y,
			 float sigma_x, float sigma_y,
			 float x, float y);

  inline void PreFrame(double time);

public:
  OscSurface(HeightMap* heightMap, Vector<4,float> color);
  virtual ~OscSurface();

  void Handle(OpenEngine::Core::ProcessEventArg arg);
  void Apply(RenderingEventArg arg, ISceneNodeVisitor& v);

  void createRipple(float x, float z, float width, float height);  
};

#endif
