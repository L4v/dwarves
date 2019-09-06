#ifndef DWARVES_H

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

internal void GameUpdateAndRender(game_offscreen_buffer* Buffer,
				  int32 XOffset, int32 YOffset,
				  game_sound_output_buffer* SoundBuffer);


#define DWARVES_H
#endif
