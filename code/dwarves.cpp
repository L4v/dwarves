#include "dwarves.h"

internal void
GameOutputSound(game_state* GameState, game_sound_output_buffer* SoundBuffer)
{
#if 0
  int16 ToneVolume = 3000;
  int32 WavePeriod = SoundBuffer->SamplesPerSec / GameState->ToneHz;
  
  int16 *Samples = SoundBuffer->Samples;
  for(int32 SampleIndex = 0;
      SampleIndex < SoundBuffer->SampleCount;
      ++SampleIndex)
    {
      real32 SineValue = sinf(GameState->tSine);
      int16 SampleValue = (int16)(SineValue * ToneVolume);
      *Samples++ = SampleValue;
      *Samples++ = SampleValue;
      GameState->tSine += 2.0f * Pi32 * 1.0f / (real32)WavePeriod;
      if(GameState->tSine >= 2.0f * Pi32)
	{
	  GameState->tSine -= 2.0f * Pi32;
	}
    }
#endif 
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
RenderPlayer(game_offscreen_buffer* Buffer, int32 PlayerX, int32 PlayerY)
{
  uint8* EndOfBuffer = (uint8*)Buffer->Memory + Buffer->BytesPerPixel *
    Buffer->Width + Buffer->Pitch * Buffer->Height;
  uint32 Color = 0xFFFFFFFF;
  int32 Top = PlayerY;
  int32 Bottom = PlayerY + 10;
  for(int32 X = PlayerX;
	X < PlayerX + 10;
      ++X)
    {
      uint8* Pixel = ((uint8*)Buffer->Memory +
		      X * Buffer->BytesPerPixel +
		      Top * Buffer->Pitch);

      for(int32 Y = Top;
	  Y < Bottom;
	  ++Y)
	{
	  if((Pixel >= Buffer->Memory) && (Pixel < EndOfBuffer))
	    {
	      *(uint32*)Pixel = Color;
	      Pixel += Buffer->Pitch;
	    }
	}
    }
}

extern "C" GAME_UPDATE_AND_RENDER(GameUpdateAndRender)
{
  // NOTE(l4v): Pointer arithmetic to check whether the array is of
  // the correct size
  Assert((&Input->Controllers[0].Terminator - &Input->Controllers[0].Buttons[0])
	 == (ArrayCount(Input->Controllers[0].Buttons)));
  Assert(sizeof(game_state) <= Memory->PermanentStorageSize);
  
  game_state* GameState = (game_state*)Memory->PermanentStorage;
  if(!Memory->IsInitialized)
    {
      GameState->ToneHz = 512;
      GameState->BlueOffset = 0;
      GameState->GreenOffset = 0;
      GameState->tSine = 0.0f;

      GameState->PlayerX = 100;
      GameState->PlayerY = 100;
#if 0
      char Filename[32] = "../code/";
      strcat(Filename, __FILE__);

      debug_read_file_result BitmapMemory = Memory->DEBUGPlatformReadEntireFile(Filename);
      if(BitmapMemory.Contents)
	{
	  Memory->DEBUGPlatformWriteEntireFile("../data/test.out", BitmapMemory.ContentsSize, BitmapMemory.Contents);
	  Memory->DEBUGPlatformFreeFileMemory(BitmapMemory.Contents);
	}
#endif
      
      // TODO(l4v): Maybe more appropriate for the platform layer
      Memory->IsInitialized = true;
    }
  
  for(uint32 ControllerIndex = 0;
      ControllerIndex < ArrayCount(Input->Controllers);
      ++ControllerIndex)
    {
    game_controller_input* Controller = GetController(Input, ControllerIndex);
    if(Controller->IsAnalog)
      {
	// NOTE(l4v): Use analog movement
	// TODO(l4v): IsAnalog is not zeroed, so this can crash the game
	GameState->ToneHz = 256 + (int32)(128.0f * (Controller->StickAverageX));
	GameState->GreenOffset += (int32)(4.0f * (Controller->StickAverageY));
	GameState->BlueOffset += (int32)(4.0f * (Controller->StickAverageX));
      }
    else
      {
	// NOTE(l4v): Use digital movement
	if(Controller->MoveLeft.EndedDown)
	  {
	    GameState->BlueOffset--;
	  }
	if(Controller->MoveRight.EndedDown)
	  {
	    GameState->BlueOffset++;
	  }
	if(Controller->MoveUp.EndedDown)
	  {
	    GameState->GreenOffset++;
	  }
	if(Controller->MoveDown.EndedDown)
	  {
	    GameState->GreenOffset--;
	  }
      }
    GameState->PlayerX += (int32)(4.0f * (Controller->StickAverageX));
    GameState->PlayerY -= (int32)(4.0f * (Controller->StickAverageY ));

    if(GameState->tJump > 0)
      {
	GameState->PlayerY += (int32)(10.0f * sinf(GameState->tJump));
      }
    
    if(Controller->ActionDown.EndedDown)
      {
	GameState->tJump = 1.0f;
      }
    GameState->tJump -= 0.033f;
    }
  
  // NOTE(l4v): So as not to cause a crash
  if(GameState->ToneHz == 0)
    {
      GameState->ToneHz = 1;
    }
  RenderWeirdGradient(Buffer, GameState->BlueOffset,
		      GameState->GreenOffset);
  RenderPlayer(Buffer, GameState->PlayerX, GameState->PlayerY);
}

extern "C" GAME_GET_SOUND_SAMPLES(GameGetSoundSamples)
{
  game_state* GameState = (game_state*)Memory->PermanentStorage;
  GameOutputSound(GameState, SoundBuffer);
}
