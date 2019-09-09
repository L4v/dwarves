#include "dwarves.h"

internal void
GameOutputSound(game_sound_output_buffer* SoundBuffer, int32 ToneHz)
{
  local_persist real32 tSine = 0.0f;
  int16 ToneVolume = 3000;
  int32 WavePeriod = SoundBuffer->SamplesPerSec / ToneHz;

  printf("Hz: %i\n", ToneHz);
  
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
RenderWeirdGradient(game_offscreen_buffer* Buffer, int32 BlueOffset,
		    int32 GreenOffset)
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
	  uint8 Blue = (X + BlueOffset);
	  uint8 Green = (Y + GreenOffset);

	  // NOTE(l4v): The pixels are written as: RR GG BB AA
	  *Pixel++ = ((Green << 8) | (Blue << 16));
	}
      Row += Buffer->Pitch;
    }
}

internal void
GameUpdateAndRender(game_memory* Memory,
		    game_input* Input,
		    game_offscreen_buffer* Buffer,
		    game_sound_output_buffer* SoundBuffer)
{
  Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
  game_state* GameState = (game_state*)Memory->PermanentStorage;
  if(!Memory->IsInitialized)
    {
      GameState->ToneHz = 256;
      GameState->BlueOffset = 0;
      GameState->GreenOffset = 0;

      char Filename[32] = "../code/";
      strcat(Filename, __FILE__);

      debug_read_file_result BitmapMemory = DEBUGPlatformReadEntireFile(Filename);
      if(BitmapMemory.Contents)
	{
	  DEBUGPlatformWriteEntireFile("../data/test.out", BitmapMemory.ContentsSize, BitmapMemory.Contents);
	  DEBUGPlatformFreeFileMemory(BitmapMemory.Contents);
	}
      
      // TODO(l4v): Maybe more appropriate for the platform layer
      Memory->IsInitialized = true;
    }
  if(!GameState)
  GameState->BlueOffset++;
  game_controller_input* Input0 = &Input->Controllers[0];

  if(Input0->IsAnalog)
    {
      // NOTE(l4v): Use analog movement
      // TODO(l4v): IsAnalog is not zeroed, so this can crash the game
      //GameState->ToneHz = 256 * (int32)(128.0f * (Input0->EndX));
      GameState->BlueOffset += (int32)(4.0f * (Input0->EndY));
      printf("AAA %d", GameState->BlueOffset);
    }
  else
    {
      // NOTE(l4v): Use digital movement
      
    }

  if(Input0->Down.EndedDown)
    {
      GameState->GreenOffset--;
      printf("%d\n", GameState->GreenOffset);
    }

  // NOTE(l4v): So as not to cause a crash
  if(GameState->ToneHz == 0)
    GameState->ToneHz = 1;
  // TODO(l4v): Allow sample offsets here for more robust
  // platform options
  GameOutputSound(SoundBuffer, GameState->ToneHz);
  RenderWeirdGradient(Buffer, GameState->BlueOffset,
		      GameState->GreenOffset);
}
