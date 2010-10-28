// Fire ball effect
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "OEFireBall.h"

#include <Math/Vector.h>
#include <ParticleSystem/ParticleSystem.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>
#include <ParticleSystem/LinearValueModifier.h>

using OpenEngine::Math::Vector;
using OpenEngine::Resources::ResourceManager;
using OpenEngine::Resources::ITexture2D;
using OpenEngine::ParticleSystem::LinearValueModifier;

OEFireBall::OEFireBall(OpenEngine::ParticleSystem::ParticleSystem& system,
                       HeightMap& heightMap,
                       BoidsSystem& boidsSystem): 
    emitter(new SimpleEmitter(system,
                              100,     //numParticles
                              0.01,   //emitRate
                              0.50,   //life
                              0.1,    //lifeVar
                              2*PI,   //angle
                              0.0,    //spin
                              0.0,    //spinVar
                              9.0,    //speed
                              2.0,    //speedVar
                              0.0,    //size
                              0.0     //sizeVar
                              )),                 
    exp(Explosion(system, boidsSystem)),
    // transMod(emitter, 50.0, heightMap, exp),
    charging(false),
    firing(false),
    charge(0.0), chargeStep(0.01), initLife(emitter->GetLife()), initSize(3),
    initSpeed(30.0),
    heightMap(heightMap)
{
    // receive processing time
    system.ProcessEvent().Attach(*this);

    // load textures
    ITexture2DPtr tex1 = 
        // ResourceManager<ITexture2D>::Create("Smoke/smoke01.tga");
        ResourceManager<ITexture2D>::Create("star.jpg");
    emitter->SetTexture(tex1);
     
    LinearValueModifier<SimpleEmitter::TYPE,Vector<4,float> >& colormod = emitter->GetColorModifier();
    LinearValueModifier<SimpleEmitter::TYPE,float>&  sizem = emitter->GetSizeModifier();

    // // color modifier
    colormod.AddValue( 1.0, Vector<4,float>(0.1, 0.01, .01, .1)); // blackish
    colormod.AddValue( .7, Vector<4,float>(0.1, 0.01, .01, .4)); // blackish
    //colormod.AddValue( .7, Vector<4,float>( .7,  0.3,  .1, .6)); // redish
    colormod.AddValue( .2, Vector<4,float>( .9, 0.75,  .2, .7)); // orangeish
    colormod.AddValue( .0, Vector<4,float>(0.2,  0.2,  .3, .1)); // blueish

    // // size variations 
    sizem.AddValue(1.0, 8.0); 
    sizem.AddValue(.8, 15.0);
    sizem.AddValue( .2, 8.0);    
    sizem.AddValue( .0, 2.0);    
   
    // attach explosion rendernode 
    t = new TransformationNode();
    tmp = new TransformationNode();
    // transMod.SetTransformationNode(t);
    t->AddNode(emitter);
    t->AddNode(exp.GetSceneNode());
    emitter->SetActive(false);
}

OEFireBall::~OEFireBall() {
    //system.ProcessEvent().Detach(*this);
} 

void OEFireBall::Charge() {
    if (charging || firing)
        return;
    emitter->SetLife(initLife);
    charge = 0.0;
    charging = true;
    emitter->SetActive(true);
}

void OEFireBall::Fire() {
    if (!charging) return;
    charging = false;
    if (charge < 0.1) {
        emitter->SetActive(false);
        return;
    }
    exp.SetCharge(charge);
    speed = initSpeed + 2 * initSpeed * charge;
    time = 0.0;
    Quaternion<float> q;
    Vector<3,float> pos;
    t->GetAccumulatedTransformations(&pos, &q);
    deltapos = q.RotateVector(Vector<3,float>(0.0,-1.0,0.0)) * speed;
    tmp->SetPosition(pos);
    tmp->SetRotation(q);
    firing = true;
    // transMod.SetSpeed(initSpeed + 2 * initSpeed * charge);
    // transMod.SetActive(true);
}

void OEFireBall::Handle(ParticleEventArg arg) {
    if (charging) {
        charge += chargeStep;
        if (charge > 1.0) {
            charge = 1.0;
        }
        emitter->SetLife(initLife +  0.5 * charge);
        emitter->Handle(arg);
        return;
    }
    if (firing) {
        float dt = arg.dt;
        time += dt;
        tmp->SetPosition(tmp->GetPosition()+deltapos*dt);
        deltapos += Vector<3,float>(0.0,-1.0,0.0)*dt; // add gravity
        Vector<3,float> h = heightMap.HeightAt(tmp->GetPosition());
        if (tmp->GetPosition()[1] < h[1]-0.1 || time > 3.0) {
            emitter->SetActive(false);
            exp.Fire(tmp->GetPosition());
            firing = false;
        }
        t->RemoveNode(emitter);
        tmp->AddNode(emitter);
        emitter->Handle(arg);        
        tmp->RemoveNode(emitter);
        t->AddNode(emitter);
        return;
    }
    emitter->Handle(arg);
    // transMod.Process(e.dt);        
}

void OEFireBall::Reset() {
    emitter->Reset();
    firing = charging = false;
};


ISceneNode* OEFireBall::GetSceneNode() {
    return t;
}

