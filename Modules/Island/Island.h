#ifndef _ISLAND_H_
#define _ISLAND_H_

//inherits from
#include <Scene/RenderNode.h>
#include <Core/IListener.h>

//templated classes
#include <Math/Vector.h>

//forward reference
class HeightMap;
namespace OpenEngine {
    namespace Core {
        class InitializeEventArg;
    }
    namespace Geometry {
        class Face;
    }
    namespace Renderers {
        class IRenderingView;
    }
    namespace Scene {
      class TransformationNode;
      class RenderStateNode;
    }
}

using OpenEngine::Math::Vector;
using OpenEngine::Scene::RenderNode;
using OpenEngine::Scene::RenderStateNode;
using OpenEngine::Renderers::RenderingEventArg;
using OpenEngine::Scene::TransformationNode;
using OpenEngine::Scene::ISceneNodeVisitor;
using namespace std;

class Island : public RenderNode, 
    public OpenEngine::Core::IListener<OpenEngine::Core::InitializeEventArg> {
private:
    HeightMap* heightMap;
    TransformationNode* trees;
    RenderStateNode* rsn;

    bool enabled; //original inherited from MTree - Module
    bool bRender; /* Polygon Flag Set To TRUE By Default */

    int numberOfTrees;
    bool renderTrees;
    bool enableTexture;
    int renderState, numberOfRenderStates;

public:
    Island(HeightMap* heightMap);
    ~Island();

    virtual void Apply(RenderingEventArg arg, ISceneNodeVisitor& v);
    void Handle(OpenEngine::Core::InitializeEventArg arg);

    void toggleRenderState();
    void toggleRenderStateOnTrees();
};

#endif
