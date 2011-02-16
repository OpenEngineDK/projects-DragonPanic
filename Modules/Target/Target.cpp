#include "Target.h"

#include "../Island/HeightMap.h"
#include "../../Common/OpenGLUtil.h"

#include <Meta/OpenGL.h>
#include <Logging/Logger.h>

using namespace std;

Target::Target(HeightMap& heightMap)
    : heightMap(heightMap) {
    target = Vector<3,float>(0,0,0);
    active = false;
    rnode = new TargetRenderNode();
    rnode->SetTarget(this);
    tnode = new TransformationNode();
    tnode->AddNode(rnode);
}

Target::~Target() {

}

void Target::Handle(ProcessEventArg arg) {
    target = heightMap.HeightAt(tnode->GetPosition());
    tnode->SetPosition(target);
    target[1] = max(target[1]+1.0f,1.0f);
}

TransformationNode* Target::GetTargetNode() {
    return tnode;
}

void Target::SetActive(bool active) {
    this->active = active;
}

Vector<3,float> Target::getTarget(){
    return target;
}

void Target::setTarget(float x, float y, float z){
    setTarget(Vector<3,float>(x,y,z));
    tnode->SetPosition(Vector<3,float>(x,y,z));
}

void Target::setTarget( Vector<3,float> v ){
    target = v;
    tnode->SetPosition(v);
}

void Target::printTarget() {
  logger.info << target << logger.end;
}

void Target::toggleRenderState() {
    rnode->render = !rnode->render;
}

Target::TargetRenderNode::TargetRenderNode() {
    render = true;
}
void Target::TargetRenderNode::SetTarget(Target* target) {
    this->target = target;
}
void Target::TargetRenderNode::Apply(RenderingEventArg arg, ISceneNodeVisitor& v) {
    if (render) {
        // Draw target
        if (target->active) {
            glColor3f( 0.8, 0.0, 0.0 );
            OpenGLUtil::GLSolidCube( 0.5 );
        }
        else {
            glColor3f( 0.0, 0.8, 0.0 );
            OpenGLUtil::GLSolidCube( 0.4 );
        }
    }
}
