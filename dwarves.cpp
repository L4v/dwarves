#include "dwarves.h"

internal void
GameOutputSound(game_sound_output_buffer* SoundBuffer)
{
  local_persist real32 tSine = 0.0f;
  int16 ToneVolume = 3000;
  int32 ToneHz = 256;
  int32 WavePeriod = SoundBuffer->SamplesPerSec / ToneHz;

  int16 *Samples = SoundBuffer->Samples;
  for(int32 SampleIndex = 0;
      SampleIndex < SoundBuffer->SampleCount;
      ++SampleIndex)
    {
      real32 SineValue = sinf(tSine);
      int16 SampleValue = (int16)(SineValue * ToneVolume);
      *Samples++ = SampleValue;
      *Samples++ = SampleValue;
      tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
    }
}

internal void
RenderWeirdGradient(game_offscreen_buffer* Buffer, int32 XOffset,
		    int32 YOffset)
{
  // TODO(l4v): Pass by value for now, checking what the optimizer
  // does
  uint8* Row = (uint8*)Buffer->Memory;
  for(int32 Y = 0;
      Y < Buffer->Height;
      ++Y)
    {
      uint32* Pixel = (uint32*)Row;
      for(int32 X = 0;
	  X < Buffer->Width;
	  ++X)
	{
	  uint8 Blue = (X + XOffset);
	  uint8 Green = (Y + YOffset);

	  // NOTE(l4v): The pixels are written as: RR GG BB AA
	  *Pixel++ = ((Green << 8) | (Blue << 16));
	}
      Row += Buffer->Pitch;
    }
}

internal void
GameUpdateAndRender(game_offscreen_buffer* Buffer, game_sound_output_buffer* SoundBuffer)
{
  local_persist int32 XOffset = 0;
  local_persist int32 YOffset = 0;
  local_persist int32 ToneHz = 256;

  if(Input.IsAnalog)
    {
      // NOTE(l4v): Use analog movement
    }
  else
    {
      // NOTE(l4v): Use digital movement
      
    }
  
  // TODO(l4v): Allow sample offsets here for more robust
  // platform options
  GameOutputSound(SoundBuffer);
  RenderWeirdGradient(Buffer, XOffset, YOffset);
}
