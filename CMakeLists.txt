# Project name. Should be the same as the project directory name and
# will become the name of the binary target.
SET( PROJECT_NAME "DragonPanic")

# Project source code list
SET( PROJECT_SOURCES
  # Add all the cpp source files here
  main.cpp
  KeyHandler.cpp
  LightFader.cpp
  GameState.cpp

  Common/utilities.cpp
  Common/OpenGLUtil.cpp
  Common/FaceListUtil.cpp
  Common/Follower.cpp

  Modules/Island/Island.cpp
  Modules/Island/HeightMap.cpp
  Modules/Island/Tree.cpp

  Modules/Target/Target.cpp
  Modules/OscSurface/OscSurface.cpp

  Modules/Boid/Boid.cpp
  Modules/Boid/BoidsSystem.cpp

#  Modules/Particle/ParticleSystem.cpp
#  Modules/Particle/Particle.cpp
#  Modules/Particle/FireBall.cpp
  Modules/Particle/BreathWeapon.cpp
  Modules/Particle/BoidFire.cpp
  Modules/Particle/OEFireBall.cpp
  Modules/Particle/Explosion.cpp

  Modules/Dragon/Dragon.cpp
  Modules/Dragon/Tube.cpp

  HUD/DragonHUD.cpp
)

# Include needed to use SDL under Mac OS X
IF(APPLE)
  SET(PROJECT_SOURCES ${PROJECT_SOURCES}  ${SDL_MAIN_FOR_MAC})
ENDIF(APPLE)

# Project executable
ADD_EXECUTABLE(${PROJECT_NAME}
  ${PROJECT_SOURCES}
)

# Project dependencies
TARGET_LINK_LIBRARIES(${PROJECT_NAME}
  # Core library dependencies
  OpenEngine_Core
  OpenEngine_Math
  OpenEngine_Logging
  OpenEngine_Display
  OpenEngine_Resources
  OpenEngine_Devices
  OpenEngine_Scene
  OpenEngine_Utils
  OpenEngine_Geometry
  # Extension dependencies
  Extensions_GenericHandlers
  Extensions_HUD
  Extensions_OBJResource
  Extensions_RAWResource
  Extensions_SDLImage
  Extensions_OpenGLRenderer
  Extensions_SDL
  Extensions_CairoResource
  Extensions_VorbisResource
  Extensions_OpenALSoundSystem
  Extensions_MusicPlayer
  Extensions_OEParticleSystem
)
