// Fire ball effect
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "Explosion.h"

#include <ParticleSystem/ParticleSystem.h>
#include <Renderers/TextureLoader.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>


using OpenEngine::Resources::ResourceManager;
using OpenEngine::Resources::ITexture2D;

Explosion::Explosion(OpenEngine::ParticleSystem::ParticleSystem& system,
                     BoidsSystem& boidsSystem): 
    
    emitter(new SimpleEmitter(system,
                              100,     //numParticles
                              0.03,    //emitRate
                              0.2,     //life
                              0.1,     //lifeVar
                              2*PI,    //angle
                              230.0,     //spin
                              100.0,     //spinVar
                              15.0,      //speed
                              5.0,       //speedVar
                              0.0,      //size
                              0.0       //sizeVar
                              )),
    charge(1.0), initLife(emitter->GetLife()), initSize(3), initSpeed(emitter->GetSpeed()), 
    boidsMod(boidsSystem, 700), maxEmits(50), boidsSystem(boidsSystem)
{
    ITexture2DPtr tex1 = 
        ResourceManager<ITexture2D>::Create("Smoke/smoke01.tga");
    emitter->SetTexture(tex1);
    
    // // color modifier
    // colormod.AddValue( .9, Vector<4,float>(0.1, 0.01, .01, .4)); // blackish
    // colormod.AddValue( .7, Vector<4,float>( .7,  0.3,  .1, .6)); // redish
    // colormod.AddValue( .2, Vector<4,float>( .9, 0.75,  .2, .7)); // orangeish
    // colormod.AddValue( .0, Vector<4,float>(0.2,  0.2,  .3, .1)); // blueish

    // // size variations 
    // sizem.AddValue(1.0, 2); 
    // sizem.AddValue(.65, 9);
    // sizem.AddValue( .2, 7);    
    // sizem.AddValue( .0, 4);    

    // SetTransformationNode(&tnode);
    // tnode.AddNode(emitter);
    system.ProcessEvent().Attach(*this);
}

Explosion::~Explosion() {
    //system.ProcessEvent().Detach(*this);
} 

void Explosion::Fire(Vector<3,float> pos) {
    // GetTransformationNode()->SetPosition(pos);
    emitter->SetActive(true);
    boidsSystem.Explosion(pos, 200*charge);
}

void Explosion::Handle(ParticleEventArg e) {
    emitter->Handle(e);
    // if (GetTotalEmits() >= maxEmits) {
    //     SetActive(false);
    //     Reset();
    // }
    // for (particles->iterator.Reset(); 
    //      particles->iterator.HasNext(); 
    //      particles->iterator.Next()) {
    //     TYPE& particle = particles->iterator.Element();
    //     boidsMod.Process(e.dt, particle);
    // }
}

void Explosion::SetCharge(float p) {
    emitter->SetLife(initLife + initLife * charge);
    emitter->SetSpeed(initSpeed + initSpeed * charge);
    boidsMod.SetStrength(700*charge);
}

float Explosion::GetCharge() {
    return charge;
}

ISceneNode* Explosion::GetSceneNode() {
    return emitter;
}
