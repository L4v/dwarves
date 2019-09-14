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
#include <cstdio>
#include <cstring>
// TODO(l4v): Implement own functions
#include <math.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>


#define internal static
#define global_variable static
#define local_persist static

/*
  NOTE(l4v): 
  SLOW_BUILD:
  0 - No slow code allowed
  1 - Slow code allowed

  INTERNAL:
  0 - For public release
  1 - For developers
 */

#if SLOW_BUILD
#define Assert(Expression)			\
  if(!(Expression)) {*(int*)0 = 0;}
#else
#define Assert(Expression)
#endif

#define MAX_CONTROLLERS 4
#define Pi32 3.14159265359f
#define SDL_GAMEPAD_LEFT_THUMB_DEADZONE 7849
#define SDL_GAMEPAD_RIGHT_THUMB_DEADZONE 8689

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
#include "sdl_dwarves.h"

#include "SDL2/SDL.h"
#include <GL/glew.h>
#include <x86intrin.h>
#include <sys/mman.h>

SDL_GameController* ControllerHandles[MAX_CONTROLLERS];
SDL_Joystick* JoystickHandles[MAX_CONTROLLERS];
SDL_Haptic* RumbleHandles[MAX_CONTROLLERS];


global_variable sdl_audio_ring_buffer AudioRingBuffer;
global_variable sdl_offscreen_buffer GlobalBackBuffer;
global_variable uint64 GlobalPerfCountFrequency;

internal const char*
LoadShader(const char* path)
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

internal debug_read_file_result
DEBUGPlatformReadEntireFile(char* Filename)
{
  debug_read_file_result Result = {};
  // NOTE(l4v): Use O_CREATE flag to create a non existant
  // file
  int32 FileHandle = open(Filename, O_RDONLY);
  if(FileHandle == -1)
    {
      printf("DEBUGPlatformReadEntireFile failed to open!\n");
      return Result;
    }
  struct stat FileStatus;
  if(stat(Filename, &FileStatus) == -1)
    {
      printf("DEBUGPlatformReadEntireFile failed to get status!\n");
      close(FileHandle);
      return Result;
    }

  // NOTE(l4v): File sizes limited to 4GB
  Result.ContentsSize = SafeTruncateUInt64(FileStatus.st_size);
  
  Result.Contents = malloc(Result.ContentsSize);
  if(!Result.Contents)
    {
      printf("DEBUGPlatformReadEntireFile failed to allocate!\n");
      free(Result.Contents);
      Result.Contents = 0;
      Result.ContentsSize = 0;
      close(FileHandle);
      return Result;
    }

  ssize_t BytesToRead = Result.ContentsSize;
  uint8* NextByteLocation = (uint8*)Result.Contents;
  while(BytesToRead)
    {
      ssize_t BytesRead = read(FileHandle, NextByteLocation, BytesToRead);
      if(BytesRead == -1)
	{
	  printf("DEBUGPlatformReadEntireFile failed to read!\n");
	  free(Result.Contents);
	  Result.Contents = 0;
	  Result.ContentsSize = 0;
	  close(FileHandle);
	  return Result;
	}
      BytesToRead -= BytesRead;
      NextByteLocation += BytesRead;
    }
  
  close(FileHandle);
  return Result;
}
internal void
DEBUGPlatformFreeFileMemory(void* Memory)
{
  if(Memory)
    {
      free(Memory);
    }
}

internal bool32
DEBUGPlatformWriteEntireFile(char* Filename,
			     uint32 MemorySize,
			     void* Memory)
{
  int32 FileHandle = open(Filename, O_WRONLY | O_CREAT, S_IRUSR |
			  S_IWUSR | S_IRGRP | S_IROTH);

  if(FileHandle == -1)
    {
      printf("DEBUGPlatformWriteEntireFile failed to open file!\n");
      return 0;
    }
  uint32 BytesToWrite = MemorySize;
  uint8* NextByteLocation = (uint8*)Memory;
  while(BytesToWrite)
    {
      ssize_t BytesWritten = write(FileHandle, NextByteLocation, BytesToWrite);
      if(BytesWritten == -1)
	{
	  printf("DEBUGPlatformWriteEntireFile failed to write!\n");
	  close(FileHandle);
	  return 0;
	}
      BytesToWrite -= BytesWritten;
      NextByteLocation += BytesWritten;
    }

  close(FileHandle);

  return true;
}

internal void
SDLFillSoundBuffer(sdl_sound_output* SoundOutput, int32 ByteToLock,
		   int32 BytesToWrite,
		   game_sound_output_buffer* SourceBuffer)
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
  int16* DestSample = (int16*) Region1;
  int16* SourceSample = (int16*) SourceBuffer->Samples;

  // NOTE(l4v): Audio test
  for(int32 SampleIndex = 0;
      SampleIndex < Region1SampleCount;
      ++SampleIndex)
    {
      *DestSample++ = *SourceSample++;
      *DestSample++ = *SourceSample++;
      ++SoundOutput->RunningSampleIndex;
    }
  // TODO(l4v): Collapse the for loops
      
  int32 Region2SampleCount = Region2Size / SoundOutput->BytesPerSample;
  DestSample = (int16*) Region2;
  for(int32 SampleIndex = 0;
      SampleIndex < Region2SampleCount;
      ++SampleIndex)
    {
      *DestSample++ = *SourceSample++;
      *DestSample++ = *SourceSample++;
      ++SoundOutput->RunningSampleIndex;
    }
}

internal void
SDLProcessGameControllerButton(game_button_state* OldState,
			       game_button_state* NewState,
			       bool32 Value)
{
  NewState->EndedDown = Value;
  NewState->HalfTransitionCount =
    (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}
internal void
SDLProcessGameJoystickButton(game_button_state* OldState,
			       game_button_state* NewState,
			       SDL_Joystick* ControllerHandle,
			       int32 Button)
{
  NewState->EndedDown = SDL_JoystickGetButton(ControllerHandle, Button);
  NewState->HalfTransitionCount =
    (OldState->EndedDown != NewState->EndedDown) ? 1 : 0;
}

internal void
SDLProcessGameKeyboardButton(game_button_state* NewState,
			     bool32 IsDown)
{
  Assert(NewState->EndedDown != IsDown);
  NewState->EndedDown = IsDown;
  ++NewState->HalfTransitionCount;
}

internal real32
SDLProcessGameControllerAxisValue(int16 Value, int16 DeadZoneThreshold)
{
  real32 Result = 0;
  if(Value < -DeadZoneThreshold)
    {
      Result = (real32)((Value + DeadZoneThreshold) /
			(32768.0f - DeadZoneThreshold));
    }
      else if(Value > DeadZoneThreshold)
    {
      Result = (real32)((Value - DeadZoneThreshold) /
			(32767.0f - DeadZoneThreshold));
    }
  
  return Result;
}

internal void
SDLWindowResize(sdl_offscreen_buffer* Buffer, int32 Width, int32 Height)
{
  glViewport(0, 0, Width, Height);
}

internal void
SDLOpenGameControllers()
{
  for(int32 i = 0; i < MAX_CONTROLLERS; ++i){
    ControllerHandles[i] = 0;
    RumbleHandles[i] = 0;
    JoystickHandles[i] = 0;
  }
  
  int32 MaxJoysticks = SDL_NumJoysticks();
  int32 ControllerIndex = 0;

  for(int JoystickIndex = 0; JoystickIndex < MaxJoysticks; ++JoystickIndex)
    {
      if(!SDL_IsGameController(JoystickIndex))
	{
	  JoystickHandles[JoystickIndex] = SDL_JoystickOpen(JoystickIndex);
	  printf("Is joystick!\n");
	}
      if(ControllerIndex >= MAX_CONTROLLERS)
	{
	  break;
	}
      ControllerHandles[ControllerIndex] = SDL_GameControllerOpen(JoystickIndex);
      RumbleHandles[ControllerIndex] = SDL_HapticOpen(JoystickIndex);
      ControllerIndex++;
      printf("Is game controller!\n");
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
  RingBuffer->WriteCursor = (RingBuffer->PlayCursor + Length) % RingBuffer->Size;
}

internal void
SDLInitAudio(int32 SamplesPerSec, int32 BufferSize)
{
  SDL_AudioSpec AudioSettings = {0};

  AudioSettings.freq = SamplesPerSec;
  AudioSettings.format = AUDIO_S16LSB;
  AudioSettings.channels = 2;
  AudioSettings.samples = 512;
  AudioSettings.callback = &SDLAudioCallback;
  AudioSettings.userdata = &AudioRingBuffer;
  
  AudioRingBuffer.Size = BufferSize;
  AudioRingBuffer.Data = calloc(BufferSize, 1);
  AudioRingBuffer.PlayCursor = AudioRingBuffer.WriteCursor = 0;
  
  SDL_OpenAudio(&AudioSettings, 0);

  if(AudioSettings.format != AUDIO_S16LSB)
    {

      SDL_CloseAudio();
    }
}
internal bool32
SDLHandleEvent(SDL_Event* Event, game_controller_input* NewKeyboardController)
{
  bool32 ShouldQuit = false;

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
	      SDLWindowResize(&GlobalBackBuffer, Event->window.data1, Event->window.data2);
	    }break;
	  case SDL_WINDOWEVENT_EXPOSED:
	    {
	      // TODO(l4v): Maybe update window when exposed only?
	    }break;
	  }
      }break;

    case SDL_JOYAXISMOTION:
      {
	if(Event->jaxis.which == 0)
	  {
	    if(Event->jaxis.axis == 0)
	      {
		if(Event->jaxis.value < 0)
		  {
		    printf("Left\n");
		  }
	      }
	  }
      }break;

    case SDL_KEYDOWN:
    case SDL_KEYUP:
      {
	SDL_Keycode Keycode = Event->key.keysym.sym;
	bool32 IsDown = (Event->key.state == SDL_PRESSED);
	//bool32 WasDown;

	if(Event->key.state == SDL_RELEASED)
	  {
	    //  WasDown = true;
	  }
	else if(Event->key.repeat)
	  {
	    //WasDown = true;
	  }

	if(!(Event->key.repeat))
	  {
	    if(Keycode == SDLK_w)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->MoveUp,
					     IsDown);
	      }
	    else if(Keycode == SDLK_a)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->MoveLeft,
					     IsDown);
	      }
	    else if(Keycode == SDLK_s)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->MoveDown,
					     IsDown);
	      }
	    else if(Keycode == SDLK_d)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->MoveRight,
					     IsDown);
	      }
	    else if(Keycode == SDLK_q)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->LeftShoulder,
					     IsDown);
	      }
	    else if(Keycode == SDLK_e)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->RightShoulder,
					     IsDown);
	      }
	    else if(Keycode == SDLK_UP)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->ActionUp,
					     IsDown);
	      }
	    else if(Keycode == SDLK_LEFT)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->ActionLeft,
					     IsDown);
	      }
	    else if(Keycode == SDLK_DOWN)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->ActionDown,
					     IsDown);
	      }
	    else if(Keycode == SDLK_RIGHT)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->ActionRight,
					     IsDown);
	      }
	    else if(Keycode == SDLK_ESCAPE)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->Back,
					     IsDown);
		ShouldQuit = true;
	      }
	    else if(Keycode == SDLK_SPACE)
	      {
		SDLProcessGameKeyboardButton(&NewKeyboardController->Start,
					     IsDown);
	      }
	  }
	
      }break;
    }
  
  return ShouldQuit;
}

inline internal int64
SDLGetWallClock()
{
  int64 Result = SDL_GetPerformanceCounter();
  return Result;
}

inline internal real32
SDLGetSecondsElapsed(int64 Start, int64 End)
{
  real32 Result = (real32)(End - Start) / (real32)GlobalPerfCountFrequency;
  return Result;
}

int main(void)
{

  GlobalPerfCountFrequency = SDL_GetPerformanceFrequency();
  
  SDL_Window* Window = 0;
  SDL_GLContext GLContext;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  if(SDL_Init(SDL_INIT_VIDEO
	      | SDL_INIT_GAMECONTROLLER
	      | SDL_INIT_JOYSTICK
	      | SDL_INIT_HAPTIC
	      | SDL_INIT_AUDIO) > 0)
    {
      printf("Failed to initialize SDL!\n");
      return 1;
    }
  
  Window = SDL_CreateWindow(
			    "Dwarves",
			    SDL_WINDOWPOS_UNDEFINED,
			    SDL_WINDOWPOS_UNDEFINED,
			    1024,
			    768,
			    SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
);

  if(!Window)
    {
      printf("Failed to init SDL Window!\n");
      return 1;
    }

  GLContext = SDL_GL_CreateContext(Window);

  if(!GLContext)
    {
      printf("Failed to get GLContext!\n");
      return 1;
    }

  // NOTE(l4v): Init SDL controllers
  SDLOpenGameControllers();

  // NOTE(l4v): Testing audio
  sdl_sound_output SoundOutput = {};
    
  SoundOutput.SamplesPerSec = 48000;
  SoundOutput.RunningSampleIndex = 0;

  SoundOutput.BytesPerSample = sizeof(int16) * 2;
  SoundOutput.SecondaryBufferSize = SoundOutput.SamplesPerSec * SoundOutput.BytesPerSample;
  SoundOutput.tSine = 0.0f;
  SoundOutput.LatencySampleCount = SoundOutput.SamplesPerSec / 15;
  
  SDLInitAudio(SoundOutput.SamplesPerSec, SoundOutput.SecondaryBufferSize);
  int16* Samples = (int16*)calloc(SoundOutput.SamplesPerSec,
  				  SoundOutput.BytesPerSample);
  SDL_PauseAudio(0);

  // TODO(l4v): Check this with the system automatically
  int32 MonitorRefreshHz = 30;
  
  // NOTE(l4v): Setup the viewport
  int32 Width, Height;
  
  SDL_GetWindowSize(Window, &Width, &Height);
  glewInit();
  glViewport(0, 0, Width, Height);
  GlobalBackBuffer.Width = Width;
  GlobalBackBuffer.Height = Height;

  SDLWindowResize(&GlobalBackBuffer, Width, Height);
  
  // NOTE(l4v): For capturing the mouse and making it invisible
  // SDL_ShowCursor(SDL_DISABLE);
  // SDL_CaptureMouse(SDL_TRUE);
  // SDL_SetRelativeMouseMode(SDL_TRUE);

  // NOTE(l4v): Enable z-buffer
  // glEnable(GL_DEPTH_TEST);

  uint64 LastCounter = SDLGetWallClock();
  uint64 LastCycleCount = _rdtsc();

  const char* VertexShaderSource = LoadShader("../shaders/triangle.vs");
  const char* FragmentShaderSource = LoadShader("../shaders/triangle.fs");
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
		       1.0f,  1.0f, 0.0f,    1.0f, 1.0f, // top right
		       1.0f, -1.0f, 0.0f,    1.0f, 0.0f, // bottom right
		       -1.0f, -1.0f, 0.0f,   0.0f, 0.0f, // bottom left
		       -1.0f,  1.0f, 0.0f,   0.0f, 1.0f  // top left 
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

  glUseProgram(ShaderProgram);
  glUniform1i(glGetUniformLocation(ShaderProgram, "texture1"), 0);

  GlobalBackBuffer.BytesPerPixel = 4;
  GlobalBackBuffer.Width = Width;
  GlobalBackBuffer.Height = Height;
  int32 BitmapMemorySize = (GlobalBackBuffer.Width * GlobalBackBuffer.Height) * GlobalBackBuffer.BytesPerPixel;
  GlobalBackBuffer.Memory = mmap(0,
			BitmapMemorySize,
			PROT_READ | PROT_WRITE,
			MAP_ANONYMOUS | MAP_PRIVATE,
			-1,
			0);
  GlobalBackBuffer.Pitch = GlobalBackBuffer.Width * GlobalBackBuffer.BytesPerPixel;


  game_input Input[2];
  game_input* NewInput = &Input[0];
  game_input* OldInput = &Input[1];

  *NewInput = {};
  *OldInput = {};

  game_memory GameMemory = {};

#if INTERNAL
  void* BaseAddress = (void*) 0;
#else
  void* BaseAddress = (void*) Gibibytes(250);
#endif
  
  GameMemory.PermanentStorageSize = Mebibytes(64);
  GameMemory.TransientStorageSize = Gibibytes(4);
  uint64 TotalStorageSize = GameMemory.PermanentStorageSize
    + GameMemory.TransientStorageSize;
  
  GameMemory.PermanentStorage = mmap(BaseAddress,
				     TotalStorageSize,
				     PROT_READ | PROT_WRITE,
				     MAP_ANON | MAP_PRIVATE,
				     -1,
				     0);
  GameMemory.TransientStorage = ((uint8*)GameMemory.PermanentStorage
				 + GameMemory.PermanentStorageSize);
  
  if(Samples && GameMemory.PermanentStorage && GameMemory.TransientStorage)
    {
      bool32 Running = true;
      // NOTE(l4v): Main loop
      while(Running)
	{
	  // TODO(l4v): Zeroing macro
	  // TODO(l4v): Not poll for disconnected controllers
	  game_controller_input* OldKeyboardController = 
		GetController(OldInput, 0);
	  game_controller_input* NewKeyboardController = 
		GetController(NewInput, 0);
	  *NewKeyboardController = {};
	  NewKeyboardController->IsConnected = true;

	  for(uint32 ButtonIndex = 0;
	      ButtonIndex < ArrayCount(NewKeyboardController->Buttons);
	      ++ButtonIndex)
	    {
	      NewKeyboardController->Buttons[ButtonIndex].EndedDown =
		OldKeyboardController->Buttons[ButtonIndex].EndedDown;
	    }
      
	  SDL_Event Event;
	  while(SDL_PollEvent(&Event))
	    {
	      if(SDLHandleEvent(&Event, NewKeyboardController))
		Running = false;
	    }

	  uint32 MaxControllerCount = MAX_CONTROLLERS;
	  if(MaxControllerCount > (ArrayCount(NewInput->Controllers) - 1))
	    {
	      MaxControllerCount = (ArrayCount(NewInput->Controllers) + 1);
	    }
	  
	  // NOTE(l4v): We have a controller with index ControllerIndex.
	  for(uint32 ControllerIndex = 0;
	      ControllerIndex < MaxControllerCount;
	      ++ControllerIndex)
	    {
	      game_controller_input *OldController =
		GetController(OldInput, ControllerIndex);
	      game_controller_input *NewController =
		GetController(NewInput, ControllerIndex);
	      if(ControllerHandles[ControllerIndex] != 0
		 && SDL_GameControllerGetAttached(ControllerHandles[ControllerIndex]))
		{
		  // NOTE(l4v): Controller input
		  // ---------------------------
		  
		  NewController->IsConnected = true;
		  
		  // NOTE(l4v): Set controllers to be analog
        
		  int16 StickX = SDL_GameControllerGetAxis(ControllerHandles[ControllerIndex],
							   SDL_CONTROLLER_AXIS_LEFTX);
		  int16 StickY = SDL_GameControllerGetAxis(ControllerHandles[ControllerIndex],
							   SDL_CONTROLLER_AXIS_LEFTY);
		  // NOTE(l4v): DPad. Sets the IsAnalog to false so it would
		  // treat it as a DPad
		  
		  if(SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
						 SDL_CONTROLLER_BUTTON_DPAD_UP))
		    {
		      NewController->StickAverageY = 1.0f;
		      NewController->IsAnalog = false;
		    }
		  if(SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
						 SDL_CONTROLLER_BUTTON_DPAD_DOWN))
		    {
		      NewController->StickAverageY = -1.0f;
		      NewController->IsAnalog = false;
		    }
		  if(SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
						 SDL_CONTROLLER_BUTTON_DPAD_LEFT))
		    {
		      NewController->StickAverageX = -1.0f;
		      NewController->IsAnalog = false;
		    }
		  if(SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
						 SDL_CONTROLLER_BUTTON_DPAD_RIGHT))
		    {
		      NewController->StickAverageX = 1.0f;
		      NewController->IsAnalog = false;
		    }
		  
		  // NOTE(l4v): Normalizing the value and checking for deadzone
		  NewController->StickAverageX =
		    SDLProcessGameControllerAxisValue(StickX, SDL_GAMEPAD_LEFT_THUMB_DEADZONE);
		  NewController->StickAverageY =
		    SDLProcessGameControllerAxisValue(StickY, SDL_GAMEPAD_LEFT_THUMB_DEADZONE);
		  // NOTE(l4v): If the stick is used => the input is analog
		  if(NewController->StickAverageX != 0.0f
		     || NewController->StickAverageY != 0.0f)
		    {
		      NewController->IsAnalog = true;
		    }

		  // NOTE(l4v): In case of "dash"
		  real32 Threshold = 0.5f;
		  SDLProcessGameControllerButton(&NewController->MoveLeft,
						 &OldController->MoveLeft,
						 (NewController->StickAverageX < -Threshold) ? 1 : 0);
		  SDLProcessGameControllerButton(&NewController->MoveRight,
						 &OldController->MoveRight,
						 (NewController->StickAverageX > Threshold) ? 1 : 0);
		  SDLProcessGameControllerButton(&NewController->MoveUp,
						 &OldController->MoveUp,
						 (NewController->StickAverageY < -Threshold) ? 1 : 0);
		  SDLProcessGameControllerButton(&NewController->MoveDown,
						 &OldController->MoveDown,
						 (NewController->StickAverageY > Threshold) ? 1 : 0);

		  // TODO(l4v): Min / max macros
	      
		  SDLProcessGameControllerButton(&(OldController->ActionDown),
						 &(NewController->ActionDown),
						 SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
									     SDL_CONTROLLER_BUTTON_A));
		  SDLProcessGameControllerButton(&(OldController->ActionRight),
						 &(NewController->ActionRight),
						 SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
									     SDL_CONTROLLER_BUTTON_B));
		  SDLProcessGameControllerButton(&(OldController->ActionLeft),
						 &(NewController->ActionLeft),
						 SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
									     SDL_CONTROLLER_BUTTON_X));
		  SDLProcessGameControllerButton(&(OldController->ActionUp),
						 &(NewController->ActionUp),
						 SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
									     SDL_CONTROLLER_BUTTON_Y));
		  SDLProcessGameControllerButton(&(OldController->LeftShoulder),
						 &(NewController->LeftShoulder),
						 SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
									     SDL_CONTROLLER_BUTTON_LEFTSHOULDER));
		  SDLProcessGameControllerButton(&(OldController->RightShoulder),
						 &(NewController->RightShoulder),
						 SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
									     SDL_CONTROLLER_BUTTON_RIGHTSHOULDER));

		  
		  SDLProcessGameControllerButton(&(OldController->Start),
						 &(NewController->Start),
						 SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
									     SDL_CONTROLLER_BUTTON_START));
		  SDLProcessGameControllerButton(&(OldController->Back),
						 &(NewController->Back),
						 SDL_GameControllerGetButton(ControllerHandles[ControllerIndex],
									     SDL_CONTROLLER_BUTTON_BACK));
		}
	      else if(JoystickHandles[ControllerIndex] != 0
			&& SDL_JoystickGetAttached(JoystickHandles[ControllerIndex]))
		{
		  // NOTE(l4v): Joysticks
		  // -------------------
		  
		  NewController->IsConnected = true;

		  // NOTE(l4v): Set controllers to be analog
		  // TODO(l4v): For now?
		  NewController->IsAnalog = true;
		  // TODO(l4v): The buttons are manually mapped for now, should get
		  // mapping somehow???
		  int16 StickX = SDL_JoystickGetAxis(JoystickHandles[ControllerIndex],
							   0);
		  int16 StickY = SDL_JoystickGetAxis(JoystickHandles[ControllerIndex],
							   1);

		  // NOTE(l4v): Normalizing the value and checking for deadzone
		  NewController->StickAverageX =
		    SDLProcessGameControllerAxisValue(StickX, SDL_GAMEPAD_LEFT_THUMB_DEADZONE);
		  NewController->StickAverageY =
		    SDLProcessGameControllerAxisValue(StickY, SDL_GAMEPAD_LEFT_THUMB_DEADZONE);

		  // TODO(l4v): Min / max macros
	      
		  SDLProcessGameJoystickButton(&(OldController->ActionDown),
						 &(NewController->ActionDown),
						 JoystickHandles[ControllerIndex],
						 0);
		  SDLProcessGameJoystickButton(&(OldController->ActionRight),
						 &(NewController->ActionRight),
						 JoystickHandles[ControllerIndex],
						 1);
		  SDLProcessGameJoystickButton(&(OldController->ActionLeft),
						 &(NewController->ActionLeft),
						 JoystickHandles[ControllerIndex],
						 2);
		  SDLProcessGameJoystickButton(&(OldController->ActionUp),
						 &(NewController->ActionUp),
						 JoystickHandles[ControllerIndex],
						 3);
		  SDLProcessGameJoystickButton(&(OldController->LeftShoulder),
						 &(NewController->LeftShoulder),
						 JoystickHandles[ControllerIndex],
						 4);
		  SDLProcessGameJoystickButton(&(OldController->RightShoulder),
						 &(NewController->RightShoulder),
						 JoystickHandles[ControllerIndex],
						 5);
		}
	      else
		{
		  // TODO(l4v): This controller is not plugged in.
		  NewController->IsConnected = false;
		}
	    }
      
	  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, GlobalBackBuffer.Width, GlobalBackBuffer.Height,
		       0, GL_RGBA, GL_UNSIGNED_BYTE, GlobalBackBuffer.Memory);
	  glGenerateMipmap(GL_TEXTURE_2D);
      
	  SDL_LockAudio();
	  int32 ByteToLock = (SoundOutput.RunningSampleIndex * SoundOutput.BytesPerSample) % SoundOutput.SecondaryBufferSize;
	  int32 TargetCursor =
	    ((AudioRingBuffer.PlayCursor +
	      (SoundOutput.LatencySampleCount * SoundOutput.BytesPerSample))
	     % SoundOutput.SecondaryBufferSize);
	  int32 BytesToWrite = 0;
	  if(ByteToLock  > TargetCursor)
	    {
	      BytesToWrite = (SoundOutput.SecondaryBufferSize - ByteToLock);
	      BytesToWrite += TargetCursor;
	    }
	  else
	    {
	      BytesToWrite = TargetCursor - ByteToLock;
	    }
	  SDL_UnlockAudio();
      
	  game_sound_output_buffer SoundBuffer = {};
	  SoundBuffer.SamplesPerSec = SoundOutput.SamplesPerSec;
	  // NOTE(l4v): For 30fps
	  SoundBuffer.SampleCount = BytesToWrite / SoundOutput.BytesPerSample;
	  SoundBuffer.Samples = Samples;
      
	  game_offscreen_buffer Buffer = {};
	  Buffer.Memory = GlobalBackBuffer.Memory;
	  Buffer.Width = GlobalBackBuffer.Width;
	  Buffer.Height = GlobalBackBuffer.Height;
	  Buffer.Pitch = GlobalBackBuffer.Pitch;
	  Buffer.BytesPerPixel = GlobalBackBuffer.BytesPerPixel;
	  GameUpdateAndRender(&GameMemory, Input, &Buffer, &SoundBuffer);
      
	  SDLFillSoundBuffer(&SoundOutput, ByteToLock, BytesToWrite,
			     &SoundBuffer);

	  // NOTE(l4v): Timing
	  // -----------------
	  real32 TargetSecondsPerFrame = 1.0f / (real32)MonitorRefreshHz;

	  int64 WorkCounter = SDLGetWallClock();
	  real32 WorkSecondsElapsed = SDLGetSecondsElapsed(LastCounter,
							   WorkCounter);
	  real32 SecondsElapsedForFrame = WorkSecondsElapsed;
	  if(SecondsElapsedForFrame < TargetSecondsPerFrame)
	    {
	      while(SecondsElapsedForFrame < TargetSecondsPerFrame)
		{

		  // TODO(l4v): Look at nanosleep, poll, etc, because
		  // of sleep granularity
		  uint32 MSToSleepFor = (uint32)(1000.0f *
						 (TargetSecondsPerFrame -
						  SecondsElapsedForFrame));
		  SDL_Delay(MSToSleepFor);
		  SecondsElapsedForFrame =
		    SDLGetSecondsElapsed(LastCounter,
					 SDLGetWallClock());
		}
	    }
	  else
	    {
	      // TODO(l4v): Missed frame rate!!!
	      // TODO(l4v): Logging
	    }

	  // NOTE(l4v): Testing drawing
	  // --------------------------
	  glClearColor(0.8f, 0.0f, 0.8f, 1.0f);
	  glClear(GL_COLOR_BUFFER_BIT);

	  glBindTexture(GL_TEXTURE_2D, texture);
      
	  glUseProgram(ShaderProgram);
	  glBindVertexArray(VAO);
	  glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
      
	  SDL_GL_SwapWindow(Window);
	  
	  game_input* Temp = NewInput;
	  NewInput = OldInput;
	  OldInput = Temp;
	  // TODO(l4v): Should they be cleared?
	  
	  int64 EndCounter = SDLGetWallClock();
	  LastCounter = WorkCounter;

	  real32 MSPerFrame = 1000.0f * SDLGetSecondsElapsed(LastCounter,
							     EndCounter);
	  printf("%lf[ms/f]\n", MSPerFrame);
	  
	  uint64 EndCycleCount = _rdtsc();
	  uint64 CyclesElapsed = EndCycleCount - LastCycleCount;
	  LastCycleCount = EndCycleCount;
	}
    }
  else
    {
      // NOTE(l4v): Logging
      printf("Memory allocation failed!\n");
    }
  // NOTE(l4v): On some Linux systems that don't use ALSA's dmix system
  // or PulseAudio, only one audio device can be open across the entire
  // system, thus to be safe, audio should be closed
  SDL_CloseAudio();
  if(GlobalBackBuffer.Memory)
    munmap(GlobalBackBuffer.Memory, Width * Height * 4);
  
  return 0;
}
