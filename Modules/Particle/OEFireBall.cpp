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
#include <Renderers/TextureLoader.h>
#include <Resources/ResourceManager.h>
#include <Resources/ITexture2D.h>

using OpenEngine::Math::Vector;
using OpenEngine::Resources::ResourceManager;
using OpenEngine::Resources::ITexture2D;

OEFireBall::OEFireBall(OpenEngine::ParticleSystem::ParticleSystem& system,
                       TextureLoader& textureLoader,
                       HeightMap& heightMap,
                       BoidsSystem& boidsSystem): 
    FireEffect(system,
               50,     //numParticles
               0.005,   //emitRate
               15.0,   //number 
               2.0,    //numberVar
               0.40,   //life
               0.1,    //lifeVar
               2*PI,   //angle
               230.0,    //spin
               100.0,    //spinVar
               9.0,    //speed
               2.0,    //speedVar
               Vector<3,float>(0.0,0.0,0.0),   //antigravity
               textureLoader),                 
    exp(Explosion(system, textureLoader, boidsSystem)),
    transMod(*this, 50.0, heightMap, exp),
    charging(false),
    firing(false),
    charge(0.0), chargeStep(0.01), initLife(life), initSize(3),
    initSpeed(50.0)
    
{
    // receive processing time
    system.ProcessEvent().Attach(*this);

    // load textures
    ITexture2DPtr tex1 = 
        ResourceManager<ITexture2D>::Create("Smoke/smoke01.tga");
    AddTexture(tex1);
 
    // color modifier
    colormod.AddValue( .9, Vector<4,float>(0.1, 0.01, .01, .4)); // blackish
    //colormod.AddValue( .7, Vector<4,float>( .7,  0.3,  .1, .6)); // redish
    colormod.AddValue( .2, Vector<4,float>( .9, 0.75,  .2, .7)); // orangeish
    colormod.AddValue( .0, Vector<4,float>(0.2,  0.2,  .3, .1)); // blueish

    // size variations 
    sizem.AddValue(1.0, 2); 
    sizem.AddValue(.65, 5);
    sizem.AddValue( .2, 4);    
    sizem.AddValue( .0, 2);    
   
    // attach explosion rendernode 
    GetSceneNode()->AddNode(exp.GetSceneNode());
    SetActive(false);
}

OEFireBall::~OEFireBall() {
    system.ProcessEvent().Detach(*this);
} 

void OEFireBall::Charge() {
    if (charging || firing)
        return;

    if (GetTransformationNode())
        transMod.SetTransformationNode(GetTransformationNode());

    life = initLife;
    charge = 0.0;
    charging = true;
    SetActive(true);
}

void OEFireBall::Fire() {
    if (!charging) return;
    charging = false;
    if (charge < 0.1) {
        SetActive(false);
        return;
    }
    firing = true;
    exp.SetCharge(charge);
    transMod.SetSpeed(initSpeed + 2 * initSpeed * charge);
    transMod.SetActive(true);
}

void OEFireBall::Handle(ParticleEventArg e) {
    if (charging) {
        charge += chargeStep;
        if (charge > 1.0) {
            charge = 1.0;
        }
        life = initLife +  0.5 * charge;
    }
    FireEffect::Handle(e);
//     for (particles->iterator.Reset(); 
//          particles->iterator.HasNext(); 
//          particles->iterator.Next()) {
//         TYPE& particle = particles->iterator.Element();
//     }
    transMod.Process(e.dt);
        
}

void OEFireBall::Reset() {
    FireEffect::Reset();
    firing = charging = false;
};

void OEFireBall::SetTransformationNode(TransformationNode* node) {
    FireEffect::SetTransformationNode(node);
}
