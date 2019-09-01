#include <iostream>

#include "SDL2/SDL.h"
#include <GL/gl.h>

#define internal static
#define global_variable static
#define local_persist static

#define Kibibytes(Value) ((Value)*1024LL)
#define Mebibytes(Value) (Kibibytes(Value)*1024LL)
#define Gibibytes(Value) (Mebibytes(Value)*1024LL)

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

bool HandleEvent(SDL_Event* Event)
{
  bool ShouldQuit = false;

  switch(Event->type)
    {
      // NOTE(l4v): In case of quitting the window
    case SDL_QUIT:
      {
	ShouldQuit = true;
      }break;

    case SDL_WINDOWEVENT:
      {
	switch(Event->window.event)
	  {
	    // NOTE(l4v): In case the window is resized
	  case SDL_WINDOWEVENT_RESIZED:
	    {
	    }break;
	  }
      }break;
    }
  
  return ShouldQuit;
}

int main(void)
{

  SDL_Window* Window = 0;
  SDL_GLContext GLContext;
  bool quit = false;

  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 0);
  SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

  if(SDL_Init(SDL_INIT_VIDEO) > 0)
    {
      std::cout << "ERROR::SDL:COULD_NOT_INIT_VIDEO" << std::endl;
      return 1;
    }
  
  Window = SDL_CreateWindow(
			    "Dwarves",
			    SDL_WINDOWPOS_UNDEFINED,
			    SDL_WINDOWPOS_UNDEFINED,
			    640,
			    480,
			    SDL_WINDOW_RESIZABLE | SDL_WINDOW_SHOWN | SDL_WINDOW_OPENGL
);

  if(!Window)
    {
      std::cout << "ERROR::SDL:COULD_NOT_CREATE_WINDOW" << std::endl;
      return 1;
    }

  GLContext = SDL_GL_CreateContext(Window);

  if(!GLContext)
    {
      std::cout << "ERROR::SDL:COULD_NOT_CREATE_GL_CONTEXT" << std::endl;
      return 1;
    }


  // NOTE(l4v): Setup the viewport
  SDL_DisplayMode DisplayMode;
  SDL_GetCurrentDisplayMode(0, &DisplayMode);
  int32 Width = DisplayMode.w;
  int32 Height = DisplayMode.h;
  glViewport(0, 0, Width, Height);

  // NOTE(l4v): For capturing the mouse and making it invisible
  // SDL_ShowCursor(SDL_DISABLE);
  // SDL_CaptureMouse(SDL_TRUE);
  // SDL_SetRelativeMouseMode(SDL_TRUE);

  // NOTE(l4v): Enable z-buffer
  glEnable(GL_DEPTH_TEST);

  // NOTE(l4v): Main loop
  while(1)
    {
      SDL_Event Event;
      SDL_WaitEvent(&Event);
      if(HandleEvent(&Event))
	break;

      glClearColor(0.8f, 0.0f, 0.8f, 1.0f);
      glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

      SDL_GL_SwapWindow(Window);
    }

  SDL_DestroyWindow(Window);
  SDL_Quit();
  return 0;
}
