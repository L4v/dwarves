#ifndef DWARVES_H

#define Kibibytes(Value) ((Value) * 1024LL)
#define Mebibytes(Value) (Kibibytes(Value) * 1024LL)
#define Gibibytes(Value) (Mebibytes(Value) * 1024LL)

#define ArrayCount(Array) (sizeof(Array) / sizeof((Array)[0]))
// TODO(l4v): swap, min, max.... MACROS???

/*
  TODO(l4v): Services that the platform layer provides to the game
 */

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
  bool32 IsAnalog;
  
  real32 StartX;
  real32 StartY;
  
  real32 MinX;
  real32 MinY;

  real32 MaxX;
  real32 MaxY;

  real32 EndX;
  real32 EndY;
  union
  {
    game_button_state Buttons[6];
    struct{
      game_button_state Up;
      game_button_state Down;
      game_button_state Left;
      game_button_state Right;
      game_button_state LeftShoulder;
      game_button_state RightShoulder;
    };
  };
};

struct game_input
{
  game_controller_input Controllers[4];
};

struct game_memory
{
  bool32 IsInitialized;
  uint64 PermanentStorageSize;
  void* PermanentStorage;
};

internal void GameUpdateAndRender(game_memory* Memory,
				  game_input* Input,
				  game_offscreen_buffer* Buffer,
				  game_sound_output_buffer* SoundBuffer);

struct game_state
{
  int32 ToneHz;
  int32 BlueOffset;
  int32 GreenOffset;
};

#define DWARVES_H
#endif
