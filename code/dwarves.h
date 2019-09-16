#ifndef DWARVES_H

/*
  NOTE(l4v): 
  SLOW_BUILD:
  0 - No slow code allowed
  1 - Slow code allowed

  INTERNAL:
  0 - For public release
  1 - For developers
*/

#include <stdint.h>
#include <cstddef>
#include <cstdio>
#include <cstring>
// TODO(l4v): Implement own functions
#include <math.h>
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

#define Kibibytes(Value) ((Value) * 1024LL)
#define Mebibytes(Value) (Kibibytes(Value) * 1024LL)
#define Gibibytes(Value) (Mebibytes(Value) * 1024LL)


#define internal static
#define global_variable static
#define local_persist static

#if SLOW_BUILD
#define Assert(Expression)			\
  if(!(Expression)) {*(int*)0 = 0;}
#else
#define Assert(Expression)
#endif

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(l4v): swap, min, max.... MACROS???
internal inline uint32
SafeTruncateUInt64(uint64 Value)
{
  Assert(Value <= 0xFFFFFFFF);
  uint32 Result = (uint32)Value;
  return Result;
}
/*
  NOTE(l4v): Services that the platform layer provides to the game
 */
#if INTERNAL_BUILD
/* IMPORTANT(l4v):
   
   These are not for doing anything in the shipping game - 
   they are blocking and the write isn't protected against
   data loss!
  
 */
struct debug_read_file_result
{
  uint32 ContentsSize;
  void* Contents;
};

#define DEBUG_PLATFORM_READ_ENTIRE_FILE(name) debug_read_file_result name(char* Filename)
typedef DEBUG_PLATFORM_READ_ENTIRE_FILE(debug_platform_read_entire_file);


void DEBUGPlatformFreeFileMemory(void* Memory);

bool32 DEBUGPlatformWriteEntireFile(char* Filename,
				    uint32 MemorySize,
				    void* Memory);
#endif

/*
  NOTE(l4v): Services that the game provides to the platform layer
  (may expand in the future - sound on seperate thread, etc.)
 */
// Timing, input, sound buffer to use

struct game_sound_output_buffer
{
  int32 SamplesPerSec;
  int32 SampleCount;
  int16* Samples;
};

// TODO(l4v): Rendering specifically will become a
// three - tiered abstraction!!!
struct game_offscreen_buffer
{
  void* Memory;
  int32 Width;
  int32 Height;
  int32 Pitch;
  int32 BytesPerPixel;
};

struct game_button_state
{
  int32 HalfTransitionCount;
  bool32 EndedDown;
};

struct game_controller_input
{
  bool32 IsConnected;
  bool32 IsAnalog;
  real32 StickAverageX;
  real32 StickAverageY;
  union
  {
    game_button_state Buttons[12];
    struct{
      game_button_state MoveUp;
      game_button_state MoveDown;
      game_button_state MoveLeft;
      game_button_state MoveRight;
      game_button_state ActionUp;
      game_button_state ActionDown;
      game_button_state ActionLeft;
      game_button_state ActionRight;
      
      game_button_state LeftShoulder;
      game_button_state RightShoulder;

      game_button_state Back;
      game_button_state Start;

      // TODO(l4v): Maybe not use an anonymous array and avoid having
      // to use the Terminator?
      // NOTE(l4v): This should always be the last button,
      // used as a temporary check for the size of the array.
      game_button_state Terminator;
    };
  };
};

struct game_input
{
  // TODO(l4v): Insert clock value here
  game_controller_input Controllers[5];
};

inline internal game_controller_input*
GetController(game_input* Input, uint32 ControllerIndex)
{
  Assert(ControllerIndex < ArrayCount(Input->Controllers));
  game_controller_input* Result = &Input->Controllers[ControllerIndex];
  return Result;
}

struct game_memory
{
  bool32 IsInitialized;
  uint64 PermanentStorageSize;
  // NOTE(l4v): REQUIRED to be cleared to 0
  void* PermanentStorage;
  uint64 TransientStorageSize;
  // NOTE(l4v): REQUIRED to be cleared to 0
  void* TransientStorage;
};

#define GAME_UPDATE_AND_RENDER(name) void name(game_memory* Memory, game_input* Input, game_offscreen_buffer* Buffer)
typedef GAME_UPDATE_AND_RENDER(game_update_and_render);
GAME_UPDATE_AND_RENDER(GameUpdateAndRenderStub)
{
}
// NOTE(l4v): At the moment this has to be a very fast
// function, cannot take more than a millisecond or so
// TODO(l4v): Reduce the pressure on the function's performance
// by measuring it?
void GameGetSoundSamples(game_memory* Memory,
				  game_sound_output_buffer* SoundBuffer);
#define GAME_GET_SOUND_SAMPLES(name) void name(game_memory* Memory, game_sound_output_buffer* SoundBuffer)
typedef GAME_GET_SOUND_SAMPLES(game_get_sound_samples);
GAME_GET_SOUND_SAMPLES(GameGetSoundSamplesStub)
{
}

//
//
//

struct game_state
{
  int32 ToneHz;
  int32 BlueOffset;
  int32 GreenOffset;
};

#define DWARVES_H
#endif
