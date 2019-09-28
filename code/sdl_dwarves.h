#ifndef SDL_DWARVES_H

struct sdl_audio_ring_buffer
{
  int32 Size;
  int32 WriteCursor;
  int32 PlayCursor;
  void* Data;
};

struct sdl_offscreen_buffer
{
  void* Memory;
  int32 Width;
  int32 Height;
  int32 Pitch;
  int32 BytesPerPixel;
};

struct sdl_sound_output
{
  int32 SamplesPerSec;
  uint32 RunningSampleIndex;
  int32 BytesPerSample;
  int32 SecondaryBufferSize;
  int32 SafetyBytes;
  real32 tSine;
  int32 LatencySampleCount;
};

struct sdl_debug_time_marker
{
  int32 OutputPlayCursor;
  int32 OutputWriteCursor;
  int32 OutputLocation;
  int32 OutputByteCount;

  int32 ExpectedFlipPlayCursor;
  int32 FlipPlayCursor;
  int32 FlipWriteCursor;
};

struct sdl_game_code
{
  void* GameCodeDynLib;
  time_t DynLibLastWriteTime;
  game_update_and_render* UpdateAndRender;
  game_get_sound_samples* GetSoundSamples;

  bool32 IsValid;
};

struct sdl_state
{
  uint64 TotalSize;
  void* GameMemoryBlock;
  
  int32 RecordingHandle;
  int32 InputRecordingIndex;

  int32 PlaybackHandle;
  int32 InputPlayingIndex;
};

#define SDL_DWARVES_H
#endif
