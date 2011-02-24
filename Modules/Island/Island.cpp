// class header
#include "Island.h"

// module classes
#include "HeightMap.h"
#include "Tree.h"

//#include "../../Common/utilities.h"

#include <Math/Math.h>
#include <iostream>

#include <Logging/Logger.h>
#include <Renderers/IRenderingView.h>
#include <Resources/DirectoryManager.h>
#include <Resources/ITexture2D.h>
#include <Resources/ResourceManager.h>
#include <Scene/TransformationNode.h>
#include <Scene/RenderStateNode.h>

using OpenEngine::Resources::DirectoryManager;
using OpenEngine::Resources::ITexture2D;
using OpenEngine::Resources::ITexture2DPtr;
using OpenEngine::Resources::ResourceManager;
using OpenEngine::Math::PI;

Island::Island(HeightMap* heightMap) : heightMap(heightMap) {
    bRender = enabled = renderTrees = enableTexture = true;
    numberOfRenderStates = 4;
    renderState = numberOfRenderStates-1;

    rsn = new RenderStateNode();
    this->AddNode(rsn);
    rsn->AddNode(heightMap);

    //initialize trees position
    trees = new TransformationNode();
    numberOfTrees = 0;
    for (int xx=-150; xx<150; xx+=10)
        for (int zz=-150; zz<  0; zz+=10) {
            Vector<3,float> pos(xx+fmod(sin((double)(xx+1000*zz))*100000,10),
                                0,
                                zz+fmod(cos((double)(xx+1000*zz))*100000,10));
            pos = heightMap->HeightAt(pos);

            // only place tree in lanscape where slope is less than maxY
            //float maxY = 0.8; // max value of normal y coordinate
            float maxAngle = PI/7.5f; // max angle for the normals
            if (
                // heightMap->NormalAt(xx,    zz   )[1]>maxY &&
                // heightMap->NormalAt(xx+10, zz   )[1]>maxY &&
                // heightMap->NormalAt(xx,    zz+10)[1]>maxY &&
                // heightMap->NormalAt(xx+10, zz+10)[1]>maxY &&
                heightMap->SlopeAt(xx,    zz   )<maxAngle &&
                heightMap->SlopeAt(xx+10, zz   )<maxAngle &&
                heightMap->SlopeAt(xx,    zz+10)<maxAngle &&
                heightMap->SlopeAt(xx+10, zz+10)<maxAngle &&
                pos[1]>2 && // no trees below waterline (lava)
				(xx*xx+zz*zz)>50*50) { //no trees closer than 50 from center
	      trees->AddNode(new Tree(pos));
	      numberOfTrees++;
            }
        }
    logger.info << "Number of trees generated: " << numberOfTrees << logger.end;
}

Island::~Island() {
    // No need to delete height-map and trees as they are sub-node
    // and will be deleted when the scene graph is destroyed.
}

void Island::Apply(RenderingEventArg arg, ISceneNodeVisitor& v) {
    // Draw trees
    if (renderTrees)
        trees->Accept(v);
    if (enabled) {
        VisitSubNodes(v);
    }
}

void Island::toggleRenderState(){
    renderState++;
    renderState %= numberOfRenderStates;

    enabled = true;
    switch (renderState) {
    case 0: //disable texture
        rsn->DisableOption(RenderStateNode::TEXTURE);
        break;
    case 1: // wireframe
        rsn->InheritOption(RenderStateNode::TEXTURE);
        rsn->EnableOption(RenderStateNode::WIREFRAME);
        break;
    case 2: //all off
        enabled = false;
        break;
    default: //render all, reset variables
        rsn->InheritOption(RenderStateNode::WIREFRAME);
        enabled = true;
        rsn->InheritOption(RenderStateNode::TEXTURE);
        break;
    }
}

void Island::toggleRenderStateOnTrees(){
    renderTrees = ! renderTrees;
}
