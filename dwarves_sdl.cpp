#include <iostream>

#include "SDL2/SDL.h"
#include <GL/gl.h>

#define internal static
#define global_variable static
#define local_persist static

#define Kibibytes(Value) ((Value)*1024LL)
#define Mebibytes(Value) (Kibibytes(Value)*1024LL)
#define Gibibytes(Value) (Mebibytes(Value)*1024LL)

#define MAX_CONTROLLERS 4

typedef int8_t int8;
typedef int16_t int16;
typedef int32_t int32;
typedef int64_t int64;
typedef int32 bool32;

typedef uint8_t uint8;
typedef uint16_t uint16;
typedef uint32_t uint32;
typedef uint64_t uint64;

typedef size_t memory_index;

typedef float real32;
typedef double real64;

SDL_GameController* ControllerHandles[MAX_CONTROLLERS];
SDL_Haptic* RumbleHandles[MAX_CONTROLLERS];

struct sdl_audio_ring_buffer
{
  int32 Size;
  int32 WriteCursor;
  int32 PlayCursor;
  void* Data;
};

global_variable sdl_audio_ring_buffer AudioRingBuffer;

internal bool
HandleEvent(SDL_Event* Event)
{
  bool ShouldQuit = false;

  switch(Event->type)
    {
      // NOTE(l4v): In case of quitting the window
    case SDL_QUIT:
      {
	ShouldQuit = true; 
      }break;

      // NOTE(l4v): Window events, resize and such
    case SDL_WINDOWEVENT:
      {
	switch(Event->window.event)
	  {
	    // NOTE(l4v): In case the window is resized
	  case SDL_WINDOWEVENT_RESIZED:
	    {
	    }break;
	  case SDL_WINDOWEVENT_EXPOSED:
	    {
	      // TODO(l4v): Maybe update window when exposed only?
	    }break;
	  }
      }break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
      {
	SDL_Keycode Keycode = Event->key.keysym.sym;
	bool IsDown = (Event->key.state == SDL_PRESSED);
	bool WasDown = false;

	if(Event->key.state == SDL_RELEASED)
	  {
	    WasDown = true;
	  }
	else if(Event->key.repeat)
	  {
	    WasDown = true;
	  }

	if(!(Event->key.repeat))
	  {
	    if(Keycode == SDLK_w)
	      {
	      }
	    else if(Keycode == SDLK_a)
	      {
	      }
	    else if(Keycode == SDLK_s)
	      {
	      }
	    else if(Keycode == SDLK_d)
	      {
	      }
	    else if(Keycode == SDLK_q)
	      {
	      }
	    else if(Keycode == SDLK_e)
	      {
	      }
	    else if(Keycode == SDLK_UP)
	      {
	      }
	    else if(Keycode == SDLK_LEFT)
	      {
	      }
	    else if(Keycode == SDLK_DOWN)
	      {
	      }
	    else if(Keycode == SDLK_RIGHT)
	      {
	      }
	    else if(Keycode == SDLK_ESCAPE)
	      {
		
	      }
	    else if(Keycode == SDLK_SPACE)
	      {
		
	      }
	  }
	
      }break;
    }
  
  return ShouldQuit;
}

internal void
SDLOpenGameControllers()
{
  for(int32 i = 0; i < MAX_CONTROLLERS; ++i){
    ControllerHandles[i] = 0;
    RumbleHandles[i] = 0;
  }
  
  int32 MaxJoysticks = SDL_NumJoysticks();
  int32 ControllerIndex = 0;

  for(int JoystickIndex = 0; JoystickIndex < MaxJoysticks; ++JoystickIndex)
    {
      if(!SDL_IsGameController(JoystickIndex))
	{
	  continue;
	}
      if(ControllerIndex >= MAX_CONTROLLERS)
	{
	  break;
	}
      ControllerHandles[ControllerIndex] = SDL_GameControllerOpen(JoystickIndex);
      RumbleHandles[ControllerIndex] = SDL_HapticOpen(JoystickIndex);
      ControllerIndex++;
    }  
}

internal void
SDLAudioCallback(void* UserData, uint8* AudioData, int32 Length)
{
  sdl_audio_ring_buffer* RingBuffer = (sdl_audio_ring_buffer*)UserData;

  int32 Region1Size = Length;
  int32 Region2Size = 0;

  if(RingBuffer->PlayCursor + Length > RingBuffer->Size)
    {
      Region1Size = RingBuffer->Size - RingBuffer->PlayCursor;
      Region2Size = Length - Region1Size;
    }

  memcpy(AudioData, (uint8*)(RingBuffer->Data) + RingBuffer->PlayCursor, Region1Size);
  memcpy(&AudioData[Region1Size], (uint8*)(RingBuffer->Data), Region2Size);
  RingBuffer->PlayCursor = (RingBuffer->PlayCursor + Length) % RingBuffer->Size;
  // NOTE(l4v): 2048 is the size of the SDL buffer in bytes
  RingBuffer->WriteCursor = (RingBuffer->PlayCursor + 2048) % RingBuffer->Size;
}

internal void
SDLInitAudio(int32 SamplesPerSec, int32 BufferSize)
{
  SDL_AudioSpec AudioSettings = {0};

  AudioSettings.freq = SamplesPerSec;
  AudioSettings.format = AUDIO_S16LSB;
  AudioSettings.channels = 2;
  AudioSettings.samples = BufferSize / 2;
  AudioSettings.callback = &SDLAudioCallback;
  AudioSettings.userdata = &AudioRingBuffer;
  
  AudioRingBuffer.Size = BufferSize;
  AudioRingBuffer.Data = malloc(BufferSize);
  AudioRingBuffer.PlayCursor = AudioRingBuffer.WriteCursor = 0;
  

  SDL_OpenAudio(&AudioSettings, 0);

  if(AudioSettings.format != AUDIO_S16LSB)
    {
      std::cout<<"ERROR::AUDIO:DID_NOT_GET_AUDIO_S16LE_BUFFER" << std::endl;
      SDL_CloseAudio();
    }
}

int main(void)
{

  SDL_Window* Window = 0;
  SDL_GLContext GLContext;
  bool quit = false;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  if(SDL_Init(SDL_INIT_VIDEO
	      | SDL_INIT_GAMECONTROLLER
	      | SDL_INIT_HAPTIC
	      | SDL_INIT_AUDIO) > 0)
    {
      std::cout << "ERROR::SDL:COULD_NOT_INIT_SDL" << std::endl;
      return 1;
    }
  
  Window = SDL_CreateWindow(
			    "Dwarves",
			    SDL_WINDOWPOS_UNDEFINED,
			    SDL_WINDOWPOS_UNDEFINED,
			    640,
			    480,
			    SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
);

  if(!Window)
    {
      std::cout << "ERROR::SDL:COULD_NOT_CREATE_WINDOW" << std::endl;
      return 1;
    }

  GLContext = SDL_GL_CreateContext(Window);

  if(!GLContext)
    {
      std::cout << "ERROR::SDL:COULD_NOT_CREATE_GL_CONTEXT" << std::endl;
      return 1;
    }

  // NOTE(l4v): Init SDL controllers
  SDLOpenGameControllers();

  // NOTE(l4v): Testing audio
  int32 SamplesPerSec = 48000;
  int32 ToneHz = 256;
  int16 ToneVolume = 3000;
  uint32 RunningSampleIndex = 0;
  int32 SquareWavePeriod = SamplesPerSec / ToneHz;
  int32 HalfSquareWavePeriod = SquareWavePeriod * 0.5f;
  int32 BytesPerSample = sizeof(int16) * 2;
  int32 SecondaryBufferSize = SamplesPerSec * BytesPerSample;
  
  SDLInitAudio(SamplesPerSec, SecondaryBufferSize);
  bool IsSoundPlaying = false;
  
  // NOTE(l4v): Setup the viewport
  SDL_DisplayMode DisplayMode;
  SDL_GetCurrentDisplayMode(0, &DisplayMode);
  int32 Width = DisplayMode.w;
  int32 Height = DisplayMode.h;
  glViewport(0, 0, Width, Height);

  // NOTE(l4v): For capturing the mouse and making it invisible
  // SDL_ShowCursor(SDL_DISABLE);
  // SDL_CaptureMouse(SDL_TRUE);
  // SDL_SetRelativeMouseMode(SDL_TRUE);

  // NOTE(l4v): Enable z-buffer
  glEnable(GL_DEPTH_TEST);

  // NOTE(l4v): Main loop
  while(1)
    {
      SDL_Event Event;
      SDL_WaitEvent(&Event);
      if(HandleEvent(&Event))
	break;

      // NOTE(l4v): Controller input
      for(int32 ControllerIndex = 0;
	  ControllerIndex < MAX_CONTROLLERS;
	  ++ControllerIndex)
	{
	  if(ControllerHandles[ControllerIndex] != 0
	     && SDL_GameControllerGetAttached(ControllerHandles[ControllerIndex]))
	    {
	      // NOTE: We have a controller with index ControllerIndex.
	      bool Up = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_UP);
	      bool Down = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_DOWN);
	      bool Left = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_LEFT);
	      bool Right = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_DPAD_RIGHT);
	      bool Start = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_START);
	      bool Back = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_BACK);
	      bool LeftShoulder = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_LEFTSHOULDER);
	      bool RightShoulder = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_RIGHTSHOULDER);
	      bool AButton = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_A);
	      bool BButton = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_B);
	      bool XButton = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_X);
	      bool YButton = SDL_GameControllerGetButton(ControllerHandles[ControllerIndex], SDL_CONTROLLER_BUTTON_Y);

	      int16 StickX = SDL_GameControllerGetAxis(ControllerHandles[ControllerIndex], SDL_CONTROLLER_AXIS_LEFTX);
	      int16 StickY = SDL_GameControllerGetAxis(ControllerHandles[ControllerIndex], SDL_CONTROLLER_AXIS_LEFTY);
	    }
	  else
	    {
	      // TODO(l4v): This controller is note plugged in.
	    }
	}

      SDL_LockAudio();
      int32 BytesToLock = RunningSampleIndex * BytesPerSample % SecondaryBufferSize;
      int32 BytesToWrite;
      if(BytesToLock == AudioRingBuffer.PlayCursor)
	{
	  BytesToWrite = SecondaryBufferSize;
	}
      else if(BytesToLock  > AudioRingBuffer.PlayCursor)
	{
	  BytesToWrite = (SecondaryBufferSize - BytesToLock);
	  BytesToWrite += AudioRingBuffer.PlayCursor;
	}
      else
	{
	  BytesToWrite = AudioRingBuffer.PlayCursor - BytesToLock;
	}

      void* Region1 = (uint8*) AudioRingBuffer.Data + BytesToLock;
      int32 Region1Size = BytesToWrite;
      if(Region1Size + BytesToLock > SecondaryBufferSize)
	{
	  Region1Size = SecondaryBufferSize - BytesToLock;
	}
      void* Region2 = (uint8*) AudioRingBuffer.Data;
      int32 Region2Size = BytesToWrite - Region1Size;
      SDL_UnlockAudio();
      int32 Region1SampleCount = Region1Size / BytesPerSample;
      int16* SampleOut = (int16*) Region1;

      // NOTE(l4v): Audio test
      for(int32 SampleIndex = 0;
	  SampleIndex < Region1SampleCount;
	  ++SampleIndex)
	{
	  int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod)% 2)
	    ? ToneVolume : -ToneVolume;
	  *SampleOut++ = SampleValue;
	  *SampleOut++ = SampleValue;
	}  
      
      if(!IsSoundPlaying)
	{
	  SDL_PauseAudio(0);
	  IsSoundPlaying = true;
	}

      int32 Region2SampleCount = Region2Size / BytesPerSample;
      SampleOut = (int16*) Region2;
      for(int32 SampleIndex = 0;
	  SampleIndex < Region2SampleCount;
	  ++SampleIndex)
	{
	  int16 SampleValue = ((RunningSampleIndex++ / HalfSquareWavePeriod)% 2)
	    ? ToneVolume : -ToneVolume;
	  *SampleOut++ = SampleValue;
	  *SampleOut++ = SampleValue;
	}
      
      glClearColor(0.8f, 0.0f, 0.8f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      SDL_GL_SwapWindow(Window);
    }

  // NOTE(l4v): On some Linux systems that don't use ALSA's dmix system
  // or PulseAudio, only one audio device can be open across the entire
  // system, thus to be safe, audio should be closed
  SDL_CloseAudio();
  
  return 0;
}
