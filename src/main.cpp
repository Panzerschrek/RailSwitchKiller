#include <iostream>
#include <SDL.h>
#include <SDL_image.h>


// View
const unsigned int c_window_width= 1024;
const unsigned int c_window_height= 768;

const int c_graphics_scale = 4;

SDL_Window* window_= nullptr;
SDL_Surface* surface_= nullptr;

const unsigned char c_background_color[]= { 32, 32, 32 };

namespace Images
{

SDL_Surface* rails= nullptr;

}

void InitWindow()
{
	window_= SDL_CreateWindow(
		"Game",
		SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
		c_window_width, c_window_height,
		0 );

	if( window_ == nullptr )
	{
		std::cout << "Can not create window" << std::endl;
		std::exit(-1);
	}

	surface_= SDL_GetWindowSurface( window_ );

	if( surface_->format == nullptr || surface_->format->BytesPerPixel != 4 )
	{
		std::cout << "Unexpected window pixel depth. Expected 32 bit" << std::endl;
		std::exit(-1);
	}
}

void DeInitWindow()
{
	SDL_DestroyWindow( window_ );
}

void LoadImages()
{
	Images::rails= IMG_Load( "res/rails.bmp" );
}

void Draw()
{
	const SDL_Rect bg_rect{ 0, 0, surface_->w, surface_->h };
	SDL_FillRect( surface_, &bg_rect, SDL_MapRGB( surface_->format, c_background_color[0], c_background_color[1], c_background_color[2] ) );

	{
		SDL_Surface* const s= Images::rails;

		SDL_Rect src_rect{ 0, 0, s->w, s->h };
		SDL_Rect dst_rect{ 0, 0, s->w * c_graphics_scale, s->h * c_graphics_scale };
		SDL_UpperBlitScaled( s, &src_rect, surface_, &dst_rect );
	}
}

void MainLoop()
{
	while(true)
	{
		// Draw
		if( SDL_MUSTLOCK( surface_ ) )
			SDL_LockSurface( surface_ );

		Draw();

		if( SDL_MUSTLOCK( surface_ ) )
			SDL_UnlockSurface( surface_ );

		SDL_UpdateWindowSurface( window_ );

		// Process events
		SDL_Event event;
		SDL_WaitEvent( &event ); // Wait for events. If there are no events - we can nothing to do.
		do
		{
			switch(event.type)
			{
			case SDL_WINDOWEVENT:
				if( event.window.event == SDL_WINDOWEVENT_CLOSE )
					return;
				break;

			case SDL_QUIT:
				return;
			}
		} while( SDL_PollEvent(&event) );
	}
}

int main()
{
	std::cout<< "Test" << std::endl;

	InitWindow();
	LoadImages();

	MainLoop();

	DeInitWindow();

	return 0;
}
