// header file
#include "Dragon.h"

// from module
#include "Tube.h"

// from project
#include "../../Common/Follower.h"

#include <Meta/OpenGL.h>

#include "../../Common/FaceListUtil.h"
#include "../../Common/VectorExt.h"
#include "../Island/HeightMap.h"
#include "../Target/Target.h"
#include "../Boid/BoidsSystem.h"
#include "../Particle/BreathWeapon.h"
#include "../Particle/OEFireBall.h"

#include <Resources/IModelResource.h>
#include <Resources/ResourceManager.h>
#include <Renderers/TextureLoader.h>
#include <Renderers/IRenderingView.h>
#include <Scene/TransformationNode.h>
#include <Scene/RenderStateNode.h>
#include <Math/Math.h>
#include <Geometry/FaceSet.h>
#include <Geometry/Face.h>
#include <Math/Quaternion.h>

#include <Geometry/Line.h>

#include <ParticleSystem/ParticleSystem.h>

using namespace OpenEngine::Resources;
using namespace OpenEngine::Geometry;
//using namespace OpenEngine::Renderers::OpenGL;
using OpenEngine::Math::PI;
using OpenEngine::Math::Quaternion;

Dragon::Dragon(HeightMap* heightMap, BoidsSystem& boidssystem, Target* target, 
               TextureLoader& textureLoader, 
               OpenEngine::ParticleSystem::ParticleSystem& oeparticlesys, ISceneNode* particleRoot)

    : heightMap(heightMap), target(target),  
      oeparticlesys(oeparticlesys), 
      breathweapon(new BreathWeapon(oeparticlesys, *heightMap, boidssystem)),
      fireball(new OEFireBall(oeparticlesys, *heightMap, boidssystem)),
      particleRoot(particleRoot) {

    jawPos = 0.0f;
    enableTexture = enabled = true;
    numberOfRenderStates = 4;
    renderState = numberOfRenderStates-1;
    enableDebug = usingBreathWeapon = chargingFireball = false;
    breathweapon->SetActive(false);

    //@todo does this make a difference?
    /*
    glRotatef(-5,1,0,0);

    glScalef(scale,scale,scale);
    glTranslatef(0.0,0.0,0.8);
    */

    folder = "Dragon/";

    
    headNode = new TransformationNode();    
    rsn = new RenderStateNode();
    this->AddNode(rsn);
    rsn->AddNode(headNode);

    TransformationNode* craniumNode = new TransformationNode();
    float scale = 0.2; //cpvc 5.0f;
    craniumNode->Scale(scale,scale,scale);
    craniumNode->SetPosition(Vector<3,float>(0.0,0.0,0.8));
    Quaternion<float> q( PI*(5/360),
			 Vector<3,float>(1.0,0.0,0.0) );
    craniumNode->SetRotation(q);
    headNode->AddNode(craniumNode);

    // offset the jaws angle node
    jawAngleNode = new TransformationNode();
    Vector<3,float> offset(0.0,-0.15,-15.7); //cpvc 0.0, -0.15, -0.7
    jawAngleNode->SetPosition(offset);
    craniumNode->AddNode(jawAngleNode);

    TransformationNode* jawNode = new TransformationNode();
    jawNode->SetPosition(-offset);
    jawAngleNode->AddNode(jawNode);

    IModelResourcePtr craniumRes = ResourceManager<IModelResource>::
      Create(folder + "DragonHead.obj");
    craniumRes->Load();
    //FaceListUtil::Unitize(gNode->GetFaceSet());
    //FaceListUtil::OverrideSoftNormsWithHardNorm(gNode->GetFaceSet());
    craniumNode->AddNode(craniumRes->GetSceneNode());
	
    IModelResourcePtr jawRes = ResourceManager<IModelResource>::
      Create(folder + "DragonJaw.obj");
    jawRes->Load();
    //FaceListUtil::Unitize(gNode->GetFaceSet());
    //FaceListUtil::OverrideSoftNormsWithHardNorm(gNode->GetFaceSet());
    jawNode->AddNode(jawRes->GetSceneNode());


    neckTexture = ResourceManager<ITexture2D>
        ::Create(folder + "DragonNeckColor128x512.tga");
    //neckTexture->Load();
    textureLoader.Load(neckTexture);

    // Adjust the breath weapon emit point and direction relative
    // to the headNode transformation.
    TransformationNode* breathTrans = new TransformationNode; // leak on dragon destroy
    breathTrans->Rotate(-0.5*PI,0.0,0.0);
    headNode->AddNode(breathTrans);
    breathTrans->AddNode(breathweapon->GetSceneNode());
    
    TransformationNode* ballTrans = new TransformationNode;    // leak on dragon destroy
    ballTrans->Move(0.0,-1,3.0);
    ballTrans->Rotate(-0.5*PI,0.0,0.0);
    headNode->AddNode(ballTrans);
    ballTrans->AddNode(fireball->GetSceneNode());
}

Dragon::~Dragon() {
    delete headFocus;
    delete headPos;

//@todo: how to handle deallocation of particle renderer?
//     particleRoot->RemoveNode(breathweapon->GetSceneNode());
//     particleRoot->RemoveNode(fireball->GetSceneNode());
//     delete breathweapon;
//     delete fireball;
}

void Dragon::Handle(InitializeEventArg arg) {
    usingBreathWeapon = false;
    breathweapon->SetActive(false);
    
    chargingFireball = false;
    fireballCharge = 0.0;
    prevTime = 0.0;
    neckLength = 45-5;
    neck = new Tube( 2.5, 1.2, 1.0, 0.5, neckLength );
    headFocus = new Follower(target->getTarget());
    headPos = new Follower(target->getTarget());
    moveNeckBack = 5.0;
}

void Dragon::Apply(RenderingEventArg arg, ISceneNodeVisitor& v) {
    if (enabled) {
        if (enableDebug) {
            // debug lines
            list<Line*>::iterator i;
            for(i=bluelines.begin(); i != bluelines.end(); ++i)
                arg.renderer.DrawLine(*(*i), Vector<3,float>(0.0,0.0,1.0) , 1.5);
            bluelines.clear();
            
            for(i=redlines.begin(); i != redlines.end(); ++i)
                arg.renderer.DrawLine(*(*i), Vector<3,float>(1.0,0.0,0.0) , 1.5);
            redlines.clear();
            
            for(i=greenlines.begin(); i != greenlines.end(); ++i)
                arg.renderer.DrawLine(*(*i), Vector<3,float>(0.0,1.0,0.0) , 1.5);
            greenlines.clear();
        } else {
            GLboolean t = glIsEnabled(GL_TEXTURE_2D);
            CHECK_FOR_GL_ERROR();
            if (enableTexture) {
                glEnable( GL_TEXTURE_2D );
                glBindTexture(GL_TEXTURE_2D, neckTexture->GetID());
            } else
                glDisable( GL_TEXTURE_2D );
            neck->draw();
            if (enableTexture && !t)
                glDisable( GL_TEXTURE_2D );

            // render head green if on textures
            if (enableTexture) {
                //glColor3f(1.0, 1.0, 1.0);
                rsn->EnableOption(RenderStateNode::TEXTURE);
            } else {
                //glColor3f(0.2, 0.8, 0.2);
                rsn->DisableOption(RenderStateNode::TEXTURE);
            }
            VisitSubNodes(v);
        }
    }
}

void Dragon::Handle(ProcessEventArg arg) {
    unsigned int dt = arg.approx;
    float deltaTime = ((float)dt)/1000.0;
    float timeStep = deltaTime / 1000.0;

    double time = prevTime + timeStep;

    float unitsFromTarget = 12;
    //    float moveNeckBack = 5.0;

    greenlines.clear();
    bluelines.clear();
    redlines.clear();

    // hack to make the neck move back when charging fireball
    if (chargingFireball) {
        moveNeckBack += timeStep*5;
        moveNeckBack = min(moveNeckBack, 10.0f);
    } else {
        moveNeckBack -= timeStep*10;
        moveNeckBack = max(moveNeckBack, 5.0f);
    }
    
    float dragonLength = neckLength+moveNeckBack;


    Vector<3,float> startP = Vector<3,float>(30,-5,0);
    startP[1] = 0.0;

    Vector<3,float> tempDir = target->getTarget()-(startP);

    Vector<3,float> attackDir = (
                      tempDir*((tempDir.GetLength()/(dragonLength)))
                      +Vector<3,float>(0,-40,0)
                      -tempDir.GetNormalize()*pow(20.0/tempDir.GetLength(),2)
                      ).GetNormalize();

    Vector<3,float> headTarget = target->
      getTarget()-attackDir*(unitsFromTarget+sin(time*PI*0.5));

    headFocus->update(target->getTarget(),12,12,timeStep);
    headPos->update(headTarget,5,5,timeStep);

    Vector<3,float> fireSourcePrev = fireSource;
    fireSource = headPos->getPos();

    if ((fireSource-startP).GetLength()>dragonLength*0.8) {
        fireSource = startP+
	  ((fireSource-startP).GetNormalize()*dragonLength*0.8);
    }

    Vector<3,float> fireSourceHeight    = heightMap->HeightAt(fireSource);

    Vector<3,float> fireSourceMidHeight = heightMap->
      HeightAt(fireSource*0.7+startP*0.3);

    if (fireSource[1]-fireSourceHeight[1]<5) {
        fireSource = fireSourceHeight+Vector<3,float>(0,5,0);
    }

    if (fireSource[1]-fireSourceMidHeight[1]<5) {
        fireSource = Vector<3,float>(fireSource[0],
				     fireSourceMidHeight[1]+5,
				     fireSource[2]);
    }

    Vector<3,float> tmpDir(headFocus->getPos()-fireSource);
    if(!tmpDir.IsZero())
      fireDir = tmpDir.GetNormalize();

    fireSource = fireSource-fireDir*moveNeckBack;
    if (timeStep > 0.000001)
      fireSourceVel = (fireSource-fireSourcePrev)/timeStep;

//     neck->update(startP,Vector<3,float>(0.1,1,0),Vector<3,float>(1,0,0),
// 		 fireSource,fireDir,fireSource-startP,true);


    // update the headNode Transformation node according to the new neck
    // position and rotation
    float* m = neck->getLinkMatrix(Tube::links-1);
    headNode->SetRotation(Quaternion<float>(Matrix<3,3,float>(m[0],m[1],m[2],
                                                              m[4],m[5],m[6],
                                                              m[8],m[9],m[10])));
    headNode->SetPosition(Vector<3,float>(m[12],m[13],m[14]));


    if (chargingFireball) jawPos = min(jawPos+timeStep*21,21.0f);
    else if (usingBreathWeapon) jawPos = min(jawPos+timeStep*120,21.0f);
    else jawPos -= timeStep*60;
    jawPos = max(0.0f,jawPos);

    Vector<3,float> jawRotateAxis = neck->getLinkX(Tube::links-1);

    Vector<3,float> newHeadDir = VectorExt::
      RotateAround(fireDir,jawRotateAxis,-jawPos/2/180*PI);

    neck->update(startP,
		 Vector<3,float>(0.1,1,0),Vector<3,float>(1,0,0),
		 fireSource,newHeadDir,fireSource-startP,false);
    
	//bluelines.push_back( new Line( fireSource,( fireSource + newHeadDir )*2.0 ) );
	//bluelines.push_back( new Line( fireSource,( fireSource + (fireSource-startP) )*2.0 ) );
	
    for(int i=0; i< Tube::links; i++) {
      redlines.push_back( new Line( neck->getLinkPosition(i),( neck->getLinkPosition(i)+neck->getLinkX(i)*5 ) ) );
      greenlines.push_back( new Line( neck->getLinkPosition(i),( neck->getLinkPosition(i)+neck->getLinkY(i)*5 ) ) );
      bluelines.push_back( new Line( neck->getLinkPosition(i), (neck->getLinkPosition(i)+neck->getLinkZ(i)*5 ) ) );
    }
    /*
    bluelines.push_back( new Line(startP,startP+Vector<3,float>(1,0,0)) );
    redlines.push_back( new Line(fireSource,fireSource+fireSource-startP)); //up vector for head
    */

    // Create smoke from mouth while charging
//    if (chargingFireball) {
//        fireballCharge = min(fireballCharge+timeStep,1.0f);
//         particlesystem->CreateParticles(
//             time, prevTime, 25*fireballCharge,
//             fireSource+fireDir*4.0+jawRotateAxis*0.75, 
// 	    jawRotateAxis*+5+fireDir*1+fireSourceVel/2, 3.0,
//             1.0+1.0*fireballCharge, 1.0
//             );
//         particlesystem->CreateParticles(
//             time, prevTime, 25*fireballCharge,
//             fireSource+fireDir*4.0-jawRotateAxis*0.75, 
// 	    jawRotateAxis*-5+fireDir*1+fireSourceVel/2, 3.0,
//             1.0+1.0*fireballCharge, 1.0
//             );
//    }
    // Create fireball
//    else if (fireballCharge>0.1) {
//         particlesystem->
// 	  CreateFireball(fireSource+fireDir*5,
// 			 (fireDir*75.0)+(fireSourceVel*0.5),
// 			 2.0+fireballCharge*5.0);

        //fireball->Fire();
//         fireballCharge = 0.0;
//     }
    
    // Create breath weapon particles
    //    if (isUsingBreathWeapon()) {
    //       Vector<3,float> sourceVec = fireSource+fireDir*3;
    //       Vector<3,float> destinationVec = (fireDir*60.0)+(fireSourceVel*0.5);
    //       particlesystem->
    // 	CreateParticles(time, prevTime, 100,
    // 			sourceVec , destinationVec,
    // 			7.0, 2.0, 2.0);
    //    }

    prevTime = time;

    // update jaw angle
    Quaternion<float> q( PI*(jawPos/180.0),
			 Vector<3,float>(1.0, 0.0, 0.0));
    jawAngleNode->SetRotation(q);
}

void Dragon::toggleRenderState(){
    enabled = true;
    renderState++;
    renderState %= numberOfRenderStates;

    switch ( renderState ){
    case 0: //disable texture
        enableTexture = false;
        break;
    case 1: //debug geometry
        enableDebug = true;
        break;
    case 2: //disable dragon
        enabled = false;
        break;
    default: //render all, reset variables
        enabled = true;
        enableTexture = true;
        enableDebug = false;
        break;
    }
}

bool Dragon::isUsingBreathWeapon(){
   return usingBreathWeapon;
}

void Dragon::useBreathWeapon( bool input ) {
    breathweapon->SetActive(input);
    target->SetActive(input);
    usingBreathWeapon = input;
}

void Dragon::ChargeFireball() {
    fireball->Charge();
    chargingFireball = true;
}

void Dragon::ShootFireball() {
    fireball->Fire();
    chargingFireball = false;

}
