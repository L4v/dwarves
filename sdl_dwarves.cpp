/*
  TODO(l4v): NOT THE FINAL PLATFORM LAYER!

  - Saved game locations
  - Getting a handle to our own exec file
  - Asset loading path
  - Threading (launch a thread)
  - Raw input (support multiple keyboards)
  - Sleep / timeBeginPeriod
  - ClipCursor() (for multimonitor support)
  - Fullscreen support
  - Control cursor visibility
  - QueryCancelAutoplay
  - Active app
  - GetKeyboardLayout (for french, international WASD)
  ...
 */
#include <stdint.h>
#include <cstddef>

#define internal static
#define global_variable static
#define local_persist static

#define Kibibytes(Value) ((Value)*1024LL)
#define Mebibytes(Value) (Kibibytes(Value)*1024LL)
#define Gibibytes(Value) (Mebibytes(Value)*1024LL)

#define MAX_CONTROLLERS 4
#define Pi32 3.14159265359f

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

#include "dwarves.h"
#include "dwarves.cpp"

// TODO(l4v): Implement own functions
#include <math.h>
#include "SDL2/SDL.h"
#include <GL/glew.h>
#include <x86intrin.h>
#include <sys/mman.h>

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
global_variable void* BitmapMemory;
global_variable int32 BitmapWidth;
global_variable int32 BitmapHeight;


struct sdl_sound_output
{
  int32 SamplesPerSec;
  int32 ToneHz;
  int16 ToneVolume;
  uint32 RunningSampleIndex;
  int32 WavePeriod;
  int32 BytesPerSample;
  int32 SecondaryBufferSize;
};

internal const char* LoadShader(const char* path)
{
  char* shaderText = 0;
  int64 length;

  FILE* file = fopen(path, "rb");
  
  if(file)
    {
      fseek(file, 0, SEEK_END);
      length = ftell(file);
      fseek(file, 0, SEEK_SET);
      shaderText = (char*)malloc(length); // TODO(l4v): Use memory arenas
      if(shaderText)
	{
	  fread(shaderText, 1, length, file);
	}
      fclose(file);
    }
  return shaderText;
}

internal void
SDLFillSoundBuffer(sdl_sound_output* SoundOutput, int32 ByteToLock, int32 BytesToWrite)
{
  void* Region1 = (uint8*) AudioRingBuffer.Data + ByteToLock;
  int32 Region1Size = BytesToWrite;
  if(Region1Size + ByteToLock > SoundOutput->SecondaryBufferSize)
    {
      Region1Size = SoundOutput->SecondaryBufferSize - ByteToLock;
    }
  void* Region2 = (uint8*) AudioRingBuffer.Data;
  int32 Region2Size = BytesToWrite - Region1Size;
  int32 Region1SampleCount = Region1Size / SoundOutput->BytesPerSample;
  int16* SampleOut = (int16*) Region1;

  // NOTE(l4v): Audio test
  for(int32 SampleIndex = 0;
      SampleIndex < Region1SampleCount;
      ++SampleIndex)
    {
      real32 t = 2.0f * Pi32 *(real32)SoundOutput->RunningSampleIndex++ / (real32)SoundOutput->WavePeriod;
      real32 SineValue = sinf(t);
      int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;
    }
  // TODO(l4v): Collapse the for loops
      
  int32 Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
  SampleOut = (int16*) Region2;
  for(int32 SampleIndex = 0;
      SampleIndex < Region2SampleCount;
      ++SampleIndex)
    {
      real32 t = 2.0f * Pi32 *(real32)SoundOutput->RunningSampleIndex++ / (real32)SoundOutput->WavePeriod;
      real32 SineValue = sinf(t);
      int16 SampleValue = (int16)(SineValue * SoundOutput->ToneVolume);
      *SampleOut++ = SampleValue;
      *SampleOut++ = SampleValue;
    }
}

internal void
SDLWindowResize(int32 Width, int32 Height)
{
  if(BitmapMemory)
    munmap(BitmapMemory, BitmapWidth * BitmapHeight * 4);
  
  BitmapWidth = Width;
  BitmapHeight = Height;
  int32 BitmapMemorySize = (Width * Height) * 4;
  BitmapMemory = mmap(0,
		      BitmapMemorySize,
		      PROT_READ | PROT_WRITE,
		      MAP_ANONYMOUS | MAP_PRIVATE,
		      -1,
		      0);
  glViewport(0, 0, Width, Height);

  int32 Pitch = Width * 4;
  uint8* Row = (uint8*)BitmapMemory;
  for(int32 Y = 0;
      Y < BitmapHeight;
      ++Y)
    {
      uint8* Pixel = (uint8*)Row;
      for(int32 X = 0;
	  X < BitmapWidth;
	  ++X)
	{
	  // Red
	  *Pixel = 0;
	  ++Pixel;

	  // Green
	  *Pixel = (uint8)Y;
	  ++Pixel;

	  // Blue
	  *Pixel = (uint8)X;
	  ++Pixel;

	  *Pixel = 255;
	  ++Pixel;
	}
      Row += Pitch;
    }
  
}

internal bool
SDLHandleEvent(SDL_Event* Event)
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
	      SDLWindowResize(Event->window.data1, Event->window.data2);
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
		ShouldQuit = true;
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

      return 1;
    }

  GLContext = SDL_GL_CreateContext(Window);

  if(!GLContext)
    {

      return 1;
    }

  // NOTE(l4v): Init SDL controllers
  SDLOpenGameControllers();

  // NOTE(l4v): Testing audio
  sdl_sound_output SoundOutput = {};
    
  SoundOutput.SamplesPerSec = 48000;
  SoundOutput.ToneHz = 256;
  SoundOutput.ToneVolume = 3000;
  SoundOutput.RunningSampleIndex = 0;
  SoundOutput.WavePeriod = SoundOutput.SamplesPerSec / SoundOutput.ToneHz;
  // int32 HalfWavePeriod = WavePeriod * 0.5f;
  SoundOutput.BytesPerSample = sizeof(int16) * 2;
  SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSec * SoundOutput.BytesPerSample;
  
  SDLInitAudio(SoundOutput.SamplesPerSec, SoundOutput.SecondaryBufferSize);
  SDLFillSoundBuffer(&SoundOutput, 0, SoundOutput.SecondaryBufferSize);  
  SDL_PauseAudio(0);
  
  // NOTE(l4v): Setup the viewport
  int32 Width, Height;
  SDL_GetWindowSize(Window, &Width, &Height);
  glewInit();
  glViewport(0, 0, Width, Height);
  BitmapWidth = Width;
  BitmapHeight = Height;

  SDLWindowResize(Width, Height);
  
  // NOTE(l4v): For capturing the mouse and making it invisible
  // SDL_ShowCursor(SDL_DISABLE);
  // SDL_CaptureMouse(SDL_TRUE);
  // SDL_SetRelativeMouseMode(SDL_TRUE);

  // NOTE(l4v): Enable z-buffer
  // glEnable(GL_DEPTH_TEST);

  uint64 LastCounter = SDL_GetPerformanceCounter();
  uint64 LastCycleCount = _rdtsc();
  uint64 PerfCountFrequency = SDL_GetPerformanceFrequency();

  const char* VertexShaderSource = LoadShader("triangle.vs");
  const char* FragmentShaderSource = LoadShader("triangle.fs");
  uint32 VertexShader;
  VertexShader = glCreateShader(GL_VERTEX_SHADER);
  glShaderSource(VertexShader, 1, &VertexShaderSource, 0);
  glCompileShader(VertexShader);

  int32 Success;
  char InfoLog[512];
  glGetShaderiv(VertexShader, GL_COMPILE_STATUS, &Success);
  if(!Success)
    {
      glGetShaderInfoLog(VertexShader, 512, 0, InfoLog);
      printf("ERROR::SHADER::VERTEX::COMPILATION_FAIL\n%s\n", InfoLog);
    }

  uint32 FragmentShader;
  FragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
  glShaderSource(FragmentShader, 1, &FragmentShaderSource, 0);
  glCompileShader(FragmentShader);
  glGetShaderiv(FragmentShader, GL_COMPILE_STATUS, &Success);
  if(!Success)
    {
      glGetShaderInfoLog(FragmentShader, 512, 0, InfoLog);
      printf("ERROR::SHADER::FRAGMENT::COMPILATION_FAIL\n%s\n", InfoLog);
    }

  uint32 ShaderProgram;
  ShaderProgram = glCreateProgram();

  glAttachShader(ShaderProgram, VertexShader);
  glAttachShader(ShaderProgram, FragmentShader);
  glLinkProgram(ShaderProgram);
      
  glGetProgramiv(ShaderProgram, GL_LINK_STATUS, &Success);
  if(!Success)
    {
      glGetProgramInfoLog(ShaderProgram, 512, 0, InfoLog);
      printf("ERROR::SHADER::PROGRAM::LINKING_FAIL\n%s\n", InfoLog);
    }

  glDeleteShader(VertexShader);
  glDeleteShader(FragmentShader);

  
  real32 Vertices[] = {
		       // positions          // texture coords
		       0.5f,  0.5f, 0.0f,    1.0f, 1.0f, // top right
		       0.5f, -0.5f, 0.0f,    1.0f, 0.0f, // bottom right
		       -0.5f, -0.5f, 0.0f,   0.0f, 0.0f, // bottom left
		       -0.5f,  0.5f, 0.0f,   0.0f, 1.0f  // top left 
  };

  uint32 Indices[] = {
		      0, 1, 3,
		      1, 2, 3
  };
  
  uint32 VBO, VAO, EBO;
  glGenBuffers(1, &VBO);
  glGenBuffers(1, &EBO);
  glGenVertexArrays(1, &VAO);

  glBindVertexArray(VAO);
  
  glBindBuffer(GL_ARRAY_BUFFER, VBO);
  glBufferData(GL_ARRAY_BUFFER, sizeof(Vertices), Vertices, GL_STATIC_DRAW);

  glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
  glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(Indices), Indices, GL_STATIC_DRAW);
  
  glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(real32),
			(void*)0);
  glEnableVertexAttribArray(0);

  glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(real32),
			(void*)(3 * sizeof(real32)));
  glEnableVertexAttribArray(1);

  uint32 texture;
  glGenTextures(1, &texture);
  glBindTexture(GL_TEXTURE_2D, texture);

  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, BitmapWidth, BitmapHeight,
	       0, GL_RGBA, GL_UNSIGNED_BYTE, BitmapMemory);
  glGenerateMipmap(GL_TEXTURE_2D);

  glUseProgram(ShaderProgram);
  glUniform1i(glGetUniformLocation(ShaderProgram, "texture1"), 0);

  // NOTE(l4v): Main loop
  while(1)
    {
      
      SDL_Event Event;
      SDL_WaitEvent(&Event);
      if(SDLHandleEvent(&Event))
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

      GameUpdateAndRender();
      
      SDL_LockAudio();
      int32 ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
      int32 BytesToWrite = 0;
      if(ByteToLock == AudioRingBuffer.PlayCursor)
	{
	  BytesToWrite = 0;
	}
      else if(ByteToLock  > AudioRingBuffer.PlayCursor)
	{
	  BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
	  BytesToWrite += AudioRingBuffer.PlayCursor;
	}
      else
	{
	  BytesToWrite = AudioRingBuffer.PlayCursor - ByteToLock;
	}
      SDL_UnlockAudio();
      SDLFillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite);

      // NOTE(l4v): Testing drawing
      // --------------------------
      glClearColor(0.8f, 0.0f, 0.8f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT);

      glBindTexture(GL_TEXTURE_2D, texture);
      
      glUseProgram(ShaderProgram);
      glBindVertexArray(VAO);
      glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      
      SDL_GL_SwapWindow(Window);

      uint64 EndCycleCount = _rdtsc();
      uint64 EndCounter = SDL_GetPerformanceCounter();
      uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
      uint64 CounterElapsed = EndCounter - LastCounter;
      // NOTE(l4v): Milliseconds per frame
      uint64 MSPerFrame = ((1000 * CounterElapsed) / PerfCountFrequency);
      uint32 FPS = PerfCountFrequency / CounterElapsed;
      // NOTE(l4v): Mega cycles per frame
      uint32 MCPF = (uint32)CyclesElapsed / 1000000;

      LastCounter = EndCounter;
      LastCycleCount = EndCycleCount;
    }

  // NOTE(l4v): On some Linux systems that don't use ALSA's dmix system
  // or PulseAudio, only one audio device can be open across the entire
  // system, thus to be safe, audio should be closed
  SDL_CloseAudio();
  if(BitmapMemory)
    munmap(BitmapMemory, Width * Height * 4);
  
  return 0;
}
