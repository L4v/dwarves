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
  real32 tSine;
  int32 LatencySampleCount;
};

#define SDL_DWARVES_H
#endif
