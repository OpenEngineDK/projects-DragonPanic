#ifndef _TARGET_H_
#define _TARGET_H_

// using templates
#include <Math/Vector.h>

// inherits from
#include <Core/IListener.h>
#include <Core/EngineEvents.h>
#include <Scene/RenderNode.h>

#include <Scene/TransformationNode.h>
#include <Renderers/IRenderer.h>
#include <Scene/ISceneNodeVisitor.h>

class HeightMap;
using OpenEngine::Core::IListener;
using OpenEngine::Core::ProcessEventArg;
using OpenEngine::Math::Vector;
using OpenEngine::Scene::RenderNode;
using OpenEngine::Scene::ISceneNodeVisitor;
using OpenEngine::Renderers::RenderingEventArg;

using OpenEngine::Scene::TransformationNode;

class Target : public IListener<OpenEngine::Core::ProcessEventArg> {
private:

    class TargetRenderNode : public RenderNode {
        Target* target;
    public:
        bool render;
        TargetRenderNode();
        virtual ~TargetRenderNode() {}
        void SetTarget(Target* target);
        virtual void Apply(RenderingEventArg arg, ISceneNodeVisitor& v);
    };

    bool active;
    Vector<3,float> target;
    HeightMap& heightMap;
    TargetRenderNode* rnode;
    TransformationNode* tnode;

public:
    Target(HeightMap& heightMap);
    virtual ~Target();

    void SetActive(bool active);
    void Handle(OpenEngine::Core::ProcessEventArg arg);
    
    TransformationNode* GetTargetNode();
  
    Vector<3,float> getTarget();
    void setTarget(float x, float y, float z);
    void setTarget( Vector<3,float> v );
    void printTarget();
    void toggleRenderState();
};

#endif
