// Key handler.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#include "KeyHandler.h"

#include "HUD/DragonHUD.h"
#include "Modules/Boid/BoidsSystem.h"
#include "Modules/Dragon/Dragon.h"
#include "Modules/Island/Island.h"
#include "Modules/OscSurface/OscSurface.h"
#include "Modules/Target/Target.h"

#include <Logging/Logger.h>
#include <Devices/IMouse.h>
#include <Math/Vector.h>
#include <Sound/MusicPlayer.h>
#include <Scene/RenderStateNode.h>

#include <Display/IFrame.h>

using OpenEngine::Math::Vector;
using OpenEngine::Scene::TransformationNode;
using OpenEngine::Scene::RenderStateNode;

KeyHandler::KeyHandler(FollowCamera& camera,
                       TransformationNode& targetTNode,
                       Target& targetBox,
                       HeightMap& hmap,
                       IMouse& mouse,
                       Island* island,
                       Dragon* dragon,
                       BoidsSystem* boidssystem,
                       TimeModifier& timeModifier,
                       GameState& gamestate,
                       MusicPlayer& musicplayer,
                       OscSurface& oscs,
                       DragonHUD& hud,
                       IFrame& frame,
                       RenderStateNode* rn)
    : camera(camera)
    , targetTNode(targetTNode)
    , targetBox(targetBox)
    , hmap(hmap)
    , mouse(mouse)
    , timeModifier(timeModifier) 
    , gamestate(gamestate)
    , oscs(oscs)
    , hud(hud)
    , frame(frame)
    , rn(rn)
    , up(0),down(0),left(0),right(0)
    , cam_in(0),cam_out(0),cam_up(0),cam_down(0),cam_left(0),cam_right(0)
    , musicplayer(musicplayer)
{

  this->island = island;
  this->dragon = dragon;
  this->boidssystem = boidssystem;

  camera.Follow(&targetTNode);

  timeFactor = 1.0;
  gainStep = 0.1;
  done = pause = false;
  rotChunkMouse = 0.05;
  moveChunkMouse = 0.0015;
  rotChunkKeyboard = 0.2; //rotChunkMouse*200;
  moveChunkKeyboard = 2; //moveChunkMouse*400;
  warping = false;

  ResetCamera();
  ResetTarget();
}

void KeyHandler::ResetTarget() {
    targetTNode.SetPosition(Vector<3,float>(0,0,0));
}
void KeyHandler::ResetCamera() {
    camera.SetPosition(Vector<3,float>(0,50,100));
    camera.LookAt(0,targetTNode.GetPosition()[1],0);
}
void KeyHandler::ResetGame() {
    ResetTarget();
    ResetCamera();
    boidssystem->ResetBoids();
    timeFactor = 1.0;
    timeModifier.SetFactor(timeFactor);
    done = pause = false;
    gamestate.Reset();
}
void KeyHandler::TogglePauseGame() {
    if (pause) {
        timeModifier.SetFactor(timeFactor);
        gamestate.Unpause();
    } else {
        timeModifier.SetFactor(0.0);
        gamestate.Pause();
    }
    pause = !pause;
}

KeyHandler::~KeyHandler() {}

// set state of keys on up/down events
void KeyHandler::Handle(KeyboardEventArg arg) {
    if (arg.type == EVENT_PRESS)
        HandleDown(arg.sym);
    else
        HandleUp(arg.sym);
}

void KeyHandler::Handle(OpenEngine::Core::InitializeEventArg arg) {
    mouse.HideCursor();
}

void KeyHandler::Handle(OpenEngine::Core::ProcessEventArg arg) {
    if (gamestate.GetTimeLeft() <= 0) {
        done = true;
        timeModifier.SetFactor(0.0);
    }
    
    float move_factor = 1.0;
	MoveForward(up*move_factor);
	MoveBack(down*move_factor);
	MoveLeft(left*move_factor);
	MoveRight(right*move_factor);

    float zoom_factor = 1.0;
	ZoomIn(cam_in*zoom_factor);
	ZoomOut(cam_out*zoom_factor);

    float rot_factor_vertical = 0.1;
    RotateLeft(cam_left*rot_factor_vertical);
	RotateRight(cam_right*rot_factor_vertical);

    float rot_factor_horizontal = 1.0;
	RotateUp(cam_up*rot_factor_horizontal);
	RotateDown(cam_down*rot_factor_horizontal);

    CheckCameraCollision();
}

void KeyHandler::HandleDown(Key key) {
    switch (key) {
    case KEY_0:
        hud.toggleRenderState();
        break;
    case KEY_1:
        boidssystem->toggleRenderState();
        break;
    case KEY_2:
        dragon->toggleRenderState();
        break;
    case KEY_3:
        island->toggleRenderState();
        break;
    case KEY_4:
        oscs.toggleRenderState();
        break;
    case KEY_5:
        targetBox.toggleRenderState();;
        break;
    case KEY_6:
        //ParticleSystem::getInstance()->enableDisable();
        break;
    case KEY_8:
        boidssystem->DecNumberOfShownBoids();
        break;
    case KEY_9:
        boidssystem->IncNumberOfShownBoids();
        break;
    case KEY_a:
        left = moveChunkKeyboard;
        break;
    case KEY_d:
        right = moveChunkKeyboard;
        break;
    case KEY_w:
        up = moveChunkKeyboard;
        break;
    case KEY_s:
        down = moveChunkKeyboard;
        break;
    case KEY_z:
        rn->ToggleOption(RenderStateNode::LIGHTING);
        rn->ToggleOption(RenderStateNode::WIREFRAME);
        break;
    case KEY_x:
        rn->ToggleOption(RenderStateNode::TEXTURE);
        break;
    case KEY_p:
        //inputgrabber->printLocation();
        break;
    case KEY_f:
        // @todo: only working on linux
        //frame.ToggleOption(FRAME_FULLSCREEN);
        break;
    case KEY_SPACE:
        TogglePauseGame();
        break;
    case KEY_r:
        ResetGame();
        break;
    case KEY_b:
        boidssystem->ResetBoids();
        break;
    case KEY_t:
        boidssystem->DecAlignment();
        break;
    case KEY_y:
        boidssystem->IncAlignment();
        break;
    case KEY_e:
        dragon->useBreathWeapon( true );
        break;
    case KEY_q:
        dragon->ChargeFireball();
        break;
    case KEY_g:
        musicplayer.Next();
    case KEY_COMMA:
        musicplayer.SetGain(musicplayer.GetGain()-gainStep);
        break;
    case KEY_PERIOD:
        musicplayer.SetGain(musicplayer.GetGain()+gainStep);
        break;

//     case KEY_F1:
//         //inputgrabber->rotateViewAbsolute( 30, 5, 50, 1.0 );
//         targetTNode->setTarget(0, 0, 0);
//         break;
//     case KEY_F2: // ved siden af hovedet
//         //inputgrabber->rotateViewAbsolute( 25.8872, 257.27, 13.8784, 1.0 );
//         targetTNode->setTarget( -10.5859, 5.4856, -10.0325 );
//         break;
//     case KEY_F3:
//         //inputgrabber->rotateViewAbsolute( 16.1391, 335, 500, 1.0 );
//         targetTNode->setTarget(-10.5859, 5.4856, -10.0325);
//         break;
//     case KEY_F4: // inde i munden
//         //inputgrabber->rotateViewAbsolute( 54.4593, 266.533, 7.75, 1.0 );
//         targetTNode->setTarget( -2.77879, 3.63685, 0.168352);
//         break;
//     case KEY_F5: //Mellem bjergene
//         //inputgrabber->rotateViewAbsolute( 340, 245.064, 68.548, 1.0 );
//         targetTNode->setTarget( 39.9721, 6.39716, -27.3046 );
//         break;

//     case KEY_F9:
//         //inputgrabber->scaleGlobal( -0.05 );
//         break;
//     case KEY_F10:
//         //inputgrabber->scaleGlobal( 0.05 );
//         break;
    case KEY_n: //KEY_F11
         if (done) return;
         timeFactor -= 0.1;
	 if (timeFactor < 0.0) timeFactor = 0.0;
         timeModifier.SetFactor(timeFactor);
	 logger.info << "time factor: " << timeFactor << logger.end;
         break;
     case KEY_m: //KEY_F12
         if (done) return;
         timeFactor += 0.1;
	 if (timeFactor > 100.0) timeFactor = 100.0;
         timeModifier.SetFactor(timeFactor);
	 logger.info << "time factor: " << timeFactor << logger.end;
         break;

    case KEY_PAGEUP:
        cam_in = moveChunkKeyboard;
        break;
    case KEY_PAGEDOWN:
        cam_out = moveChunkKeyboard;
        break;
    case KEY_UP:
        cam_up = moveChunkKeyboard;
        break;
    case KEY_DOWN:
        cam_down = moveChunkKeyboard;
        break;
    case KEY_LEFT:
        cam_left = rotChunkKeyboard;
        break;
    case KEY_RIGHT:
        cam_right = rotChunkKeyboard;
        break;
    default:
        break;
    }
}

void KeyHandler::MoveForward(float d) {
    targetTNode.Move(0, 0, -d);
}
void KeyHandler::MoveBack(float d) {
    targetTNode.Move(0, 0, d);
}
void KeyHandler::MoveLeft(float d) {
    targetTNode.Move(-d, 0, 0);
}
void KeyHandler::MoveRight(float d) {
    targetTNode.Move(d, 0, 0);
}
void KeyHandler::RotateUp(float d) {
    // check that the camera does not roll over the top
    if (0.95f < camera.GetDirection().RotateVector(Vector<3,float>(0,0,1))[1]) return;
    camera.Move(0, 0, d);
    camera.LookAt(0,targetTNode.GetPosition()[1],0);    
}
void KeyHandler::RotateDown(float d) {
    Vector<3,float> p = camera.GetPosition();
    float h = hmap.HeightAt(p)[1];
    if (h + 10 > p[1]) return;
    camera.Move(0, 0, -d);
    camera.LookAt(0,targetTNode.GetPosition()[1],0);
}
void KeyHandler::RotateRight(float d) {
    targetTNode.Rotate(0, d, 0);
}
void KeyHandler::RotateLeft(float d) {
    targetTNode.Rotate(0, -d, 0);
}
void KeyHandler::ZoomIn(float d) {
    Vector<3,float> v(targetTNode.GetPosition() - camera.GetPosition());
    if (v*v < 1000) return;
    camera.Move(d,0,0);
}
void KeyHandler::ZoomOut(float d) {
    Vector<3,float> v(targetTNode.GetPosition() - camera.GetPosition());
    if (v*v > 10000) return;
    camera.Move(-d,0,0);
}

void KeyHandler::CheckCameraCollision() {
    // if the camera is under the height map move it up
    Vector<3,float> t, c, p, r;
    t = targetTNode.GetPosition();
    p = camera.GetPosition();
    float h = hmap.HeightAt(p)[1] + 10;
    if (h > p[1]) {
        camera.Move(0, 0, h - p[1]);
        camera.LookAt(0,t[1],0);
    }
}

void KeyHandler::HandleUp(Key key) {
    switch (key) {
    case KEY_e:
        dragon->useBreathWeapon( false );
        break;
    case KEY_q:
        dragon->ShootFireball();
        break;
    case KEY_a:
        left = 0.0f;
        break;
    case KEY_d:
        right = 0.0f;
        break;
    case KEY_w:
        up = 0.0f;
        break;
    case KEY_s:
        down = 0.0f;
        break;
    case KEY_PAGEUP:
        cam_in = 0.0;
        break;
    case KEY_PAGEDOWN:
        cam_out = 0.0;
        break;
    case KEY_UP:
        cam_up = 0.0;
        break;
    case KEY_DOWN:
        cam_down = 0.0;
        break;
    case KEY_LEFT:
        cam_left = 0.0;
        break;
    case KEY_RIGHT:
        cam_right = 0.0;
        break;
    default:
        break;
    }
}

void KeyHandler::Handle(JoystickButtonEventArg arg) {
    switch (arg.button) {
    case JBUTTON_TWO:
	dragon->useBreathWeapon( arg.type == JoystickButtonEventArg::PRESS);
	break;
    case JBUTTON_THREE:
        if ( arg.type == JoystickButtonEventArg::PRESS )
            dragon->ChargeFireball();
        else dragon->ShootFireball();
	break;
    case JBUTTON_NINE:
        if(arg.type == JoystickButtonEventArg::PRESS)
            ResetGame();
        break;
    case JBUTTON_TEN:
        if(arg.type == JoystickButtonEventArg::PRESS)
            TogglePauseGame();
        break;
    case JBUTTON_FIVE:
        boidssystem->DecAlignment();
        break;
    case JBUTTON_SIX:
        boidssystem->IncAlignment();
        break;
    case JBUTTON_SEVEN:
	if (arg.type == JoystickButtonEventArg::PRESS) {
        if (done) return;
	    timeFactor -= 0.1;
	    if (timeFactor < 0.0) timeFactor = 0.0;
	    timeModifier.SetFactor(timeFactor);
	    logger.info << "time factor: " << timeFactor << logger.end;
	    break;
	}
    case JBUTTON_EIGHT:
	if (arg.type == JoystickButtonEventArg::PRESS) {
        if (done) return;
	    timeFactor += 0.1;
	    if (timeFactor > 100.0) timeFactor = 100.0;
	    timeModifier.SetFactor(timeFactor);
	    logger.info << "time factor: " << timeFactor << logger.end;
	    break;
	}

    default:
	break;
    }
}

void KeyHandler::Handle(JoystickAxisEventArg arg) {

    float max = 1 << 15;
    float thres1 = 0.1;
    float thres2 = 0.15;

    up = (-arg.state.axisState[1])/max;
    if (up < thres1) up = 0.0;
    down = (arg.state.axisState[1])/max;
    if (down < thres1) down = 0.0;

    left = (-arg.state.axisState[0])/max;
    if (left < thres1) left = 0.0;
    right = (arg.state.axisState[0])/max;
    if (right < thres1) right = 0.0;

    
    cam_in = (-arg.state.axisState[3])/max;
    if (cam_in < thres2) cam_in = 0.0;
    cam_out = (arg.state.axisState[3])/max;
    if (cam_out < thres2) cam_out = 0.0;
    
    cam_left = (-arg.state.axisState[2])/max;
    if (cam_left < thres2) cam_left = 0.0;
    cam_right = (arg.state.axisState[2])/max;
    if (cam_right < thres2) cam_right = 0.0;

    float move_factor = 2.5;
    left *= move_factor;
    right *= move_factor;
    up *= move_factor;
    down *= move_factor;
    float zoom_factor = 3.0;
    cam_in *= zoom_factor;
    cam_out *= zoom_factor;
    float rot_factor = 1.0;
    cam_left *= rot_factor;
    cam_right *= rot_factor;
}
