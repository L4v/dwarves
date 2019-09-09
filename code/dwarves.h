#ifndef DWARVES_H

#define Kibibytes(Value) ((Value) * 1024LL)
#define Mebibytes(Value) (Kibibytes(Value) * 1024LL)
#define Gibibytes(Value) (Mebibytes(Value) * 1024LL)

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
internal debug_read_file_result DEBUGPlatformReadEntireFile(char* Filename);
internal void DEBUGPlatformFreeFileMemory(void* Memory);

internal bool32 DEBUGPlatformWriteEntireFile(char* Filename,
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
  bool32 IsAnalog;
  real32 StickAverageX;
  real32 StickAverageY;
  union
  {
    game_button_state Buttons[10];
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
    };
  };
};

struct game_input
{
  // TODO(l4v): Insert clock value here
  game_controller_input Controllers[5];
};

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
