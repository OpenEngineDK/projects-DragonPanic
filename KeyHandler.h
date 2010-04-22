// Key handler.
// -------------------------------------------------------------------
// Copyright (C) 2007 OpenEngine.dk (See AUTHORS) 
// 
// This program is free software; It is covered by the GNU General 
// Public License version 2 or any later version. 
// See the GNU General Public License for more details (see LICENSE). 
//--------------------------------------------------------------------

#ifndef _KEY_HANDLER_H_
#define _KEY_HANDLER_H_

#include "TimeModifier.h"

// inherits from
#include <Core/IListener.h>
#include <Core/EngineEvents.h>

#include <Display/FollowCamera.h>
/*
#include <Devices/IMouse.h>
#include <Scene/TransformationNode.h>
*/
#include <Devices/IKeyboard.h>
#include <Devices/IJoystick.h>

#include <list>

#include "Modules/Island/HeightMap.h"
#include "GameState.h"

class BoidsSystem;
class Dragon;
class InputGrabber;
class Intro;
class Island;
class Target;
namespace OpenEngine {
    namespace Devices {
        class IKeyboard;
        class IMouse;
    }
    namespace Display {
        class Camera;
        class IFrame;
    }
    namespace Scene {
        class TransformationNode;
    }
    namespace Sound {
        class MusicPlayer;
    }
}

using namespace OpenEngine::Core;
using namespace OpenEngine::Display;
using namespace OpenEngine::Scene;
using namespace OpenEngine::Devices;
using namespace OpenEngine::Sound;

using std::list;

class KeyHandler : public IListener<ProcessEventArg>,
		   public IListener<KeyboardEventArg>,
		   public IListener<JoystickButtonEventArg>,
		   public IListener<JoystickAxisEventArg> {
private:
    Intro* intro;
    Island* island;
    Dragon* dragon;
    BoidsSystem* boidssystem;
    FollowCamera& camera;
    TransformationNode& target;
    HeightMap& hmap;
    IMouse& mouse;
    TimeModifier& timeModifier;
    GameState& gamestate;
    IFrame& frame;
    RenderStateNode *rn;

    bool done, pause;

    list<Key> keysPressed;
    float timeFactor;
    float gainStep;

    float moveChunkMouse, rotChunkMouse, moveChunkKeyboard, rotChunkKeyboard;
    int mousex_prev, mousey_prev, mousex_orig, mousey_orig;
    bool warping;

    float up,down,left,right;
    float cam_up,cam_down,cam_left,cam_right;

    MusicPlayer& musicplayer;

    void CheckCameraCollision();

public:
    KeyHandler(FollowCamera& camera,
               TransformationNode& target,
               HeightMap& hmap,
               IMouse& mouse,
               Island* island,
               Dragon* dragon,
               BoidsSystem* boidssystem,
               TimeModifier& timeModifer,
               GameState& gamestate,
               MusicPlayer& musicplayer,
               IFrame& frame,
               RenderStateNode* rn);
    ~KeyHandler();

    void Handle(KeyboardEventArg arg);
    void HandleUp(Key key);
    void HandleDown(Key key);

    void Handle(ProcessEventArg arg);
    void Handle(JoystickAxisEventArg arg);
    void Handle(JoystickButtonEventArg arg);

    void ResetTarget();
    void ResetCamera();
    void ResetGame();
    void TogglePauseGame();

    void MoveForward(float d);
    void MoveBack(float d);
    void MoveRight(float d);
    void MoveLeft(float d);
    void RotateUp(float d);
    void RotateDown(float d);
    void RotateRight(float d);
    void RotateLeft(float d);
    void ZoomIn(float d);
    void ZoomOut(float d);

};

#endif // _KEY_HANDLER_H_
