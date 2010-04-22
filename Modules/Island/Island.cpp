// class header
#include "Island.h"

// module classes
#include "HeightMap.h"
#include "Tree.h"

#include "../../Common/utilities.h"

#include <Math/Math.h>
#include <iostream>

#include <Logging/Logger.h>
#include <Renderers/IRenderingView.h>
#include <Resources/DirectoryManager.h>
#include <Resources/ITexture2D.h>
#include <Resources/ResourceManager.h>
#include <Scene/TransformationNode.h>

using OpenEngine::Resources::DirectoryManager;
using OpenEngine::Resources::ITexture2D;
using OpenEngine::Resources::ITexture2DPtr;
using OpenEngine::Resources::ResourceManager;

Island::Island(HeightMap* heightMap) : heightMap(heightMap) {
    bRender = enabled = renderTrees = enableTexture = true;
    numberOfRenderStates = 5;
    renderState = numberOfRenderStates-1;

    this->AddNode(heightMap);

    trees = new TransformationNode();

    //initialize trees position
    numberOfTrees = 0;
    for (int xx=-150; xx<150; xx+=10)
        for (int zz=-150; zz<  0; zz+=10) {
            Vector<3,float> pos(xx+fmod(sin((double)(xx+1000*zz))*100000,10),
				0,
				zz+fmod(cos((double)(xx+1000*zz))*100000,10));
            pos = heightMap->HeightAt(pos);

	    // only place tree in lanscape where slope is less than maxY
	    float maxY = 0.8; // max value of normal y coordinate
            if (heightMap->
		NormalAt(Vector<3,float>(xx,0,zz))[1]>maxY &&
                
		heightMap->
		NormalAt(Vector<3,float>(xx+10,0,zz))[1]>maxY &&

                heightMap->
		NormalAt(Vector<3,float>(xx,0,zz+10))[1]>maxY &&
                
		heightMap->
		NormalAt(Vector<3,float>(xx+10,0,zz+10))[1]>maxY &&

                pos[1]>2 && // no trees below waterline (lava)
		
		(xx*xx+zz*zz)>50*50) { //no trees closer than 50 from center
	      trees->AddNode(new Tree(pos));
	      numberOfTrees++;
            }
        }
    logger.info << "Number of trees generated: " << numberOfTrees << logger.end;
}

Island::~Island() {
    // No need to delete height-map and trees as they are sub-nodes.
}

void Island::Apply(RenderingEventArg arg, ISceneNodeVisitor& v) {
    // Draw trees
    if (renderTrees)
        trees->Accept(v);
    VisitSubNodes(v);
}

void Island::toggleRenderState(){
    enabled = true;
    switch ( renderState ){
    case 0: //no trees
        renderTrees = false;
        break;
    case 1: //disable texture
        enableTexture = false;
        break;
    case 2: //grid layout
        cout << "setting GL_LINE render" << endl;
        bRender = false;
        break;
    case 3: //all off
        enabled = false;
        break;
    default: //render all, reset variables
        bRender = true;
        enabled = true;
        renderTrees = true;
        enableTexture = true;
        break;
    }
    renderState++;
    renderState %= numberOfRenderStates;
}
