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

SDL_Surface* rails_x= nullptr;
SDL_Surface* rails_y= nullptr;
SDL_Surface* rails_x_to_up= nullptr;
SDL_Surface* rails_x_to_down= nullptr;
SDL_Surface* rails_up_to_x= nullptr;
SDL_Surface* rails_down_to_x= nullptr;
SDL_Surface* rails_fork= nullptr;

SDL_Surface* swith_off= nullptr;
SDL_Surface* swith_on= nullptr;

}

struct RailSegment
{
	enum class Direction
	{
		X,
		Y,
		XToUp,
		XToDown,
		UpToX,
		DownToX,
		Fork,
	};

	int x;
	int y;
	Direction direction;

};

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
	Images::rails_x= IMG_Load( "res/rails_x.bmp" );
	Images::rails_y= IMG_Load( "res/rails_y.bmp" );
	Images::rails_x_to_up= IMG_Load( "res/rails_turn_up.bmp" );
	Images::rails_x_to_down= IMG_Load( "res/rails_turn_down.bmp" );
	Images::rails_up_to_x= IMG_Load( "res/rails_turn_up_to_x.bmp" );
	Images::rails_down_to_x= IMG_Load( "res/rails_turn_down_to_x.bmp" );
	Images::rails_fork= IMG_Load( "res/rails_fork.bmp" );

	Images::swith_off= IMG_Load( "res/swith_off.bmp" );
	Images::swith_on= IMG_Load( "res/swith_on.bmp" );
}

void FreeImages()
{
	// TODO - free it
}

void Draw()
{
	const SDL_Rect bg_rect{ 0, 0, surface_->w, surface_->h };
	SDL_FillRect( surface_, &bg_rect, SDL_MapRGB( surface_->format, c_background_color[0], c_background_color[1], c_background_color[2] ) );

	static const RailSegment way[]
	{
		RailSegment{ 0, 5, RailSegment::Direction::X, },
		RailSegment{ 1, 5, RailSegment::Direction::X, },
		RailSegment{ 2, 5, RailSegment::Direction::X, },
		RailSegment{ 3, 5, RailSegment::Direction::Fork, },

		RailSegment{ 3, 4, RailSegment::Direction::Y, },
		RailSegment{ 3, 3, RailSegment::Direction::Y, },
		RailSegment{ 3, 2, RailSegment::Direction::UpToX, },
		RailSegment{ 4, 2, RailSegment::Direction::X, },
		RailSegment{ 5, 2, RailSegment::Direction::XToDown, },
		RailSegment{ 5, 3, RailSegment::Direction::Y, },

		RailSegment{ 3, 6, RailSegment::Direction::Y, },
		RailSegment{ 3, 7, RailSegment::Direction::Y, },
		RailSegment{ 3, 8, RailSegment::Direction::DownToX, },
		RailSegment{ 4, 8, RailSegment::Direction::X, },
		RailSegment{ 5, 8, RailSegment::Direction::XToUp, },
		RailSegment{ 5, 7, RailSegment::Direction::Y, },
	};
	{
		for( const RailSegment& segment : way )
		{
			SDL_Surface* s= nullptr;
			switch(segment.direction)
			{
			case RailSegment::Direction::X: s= Images::rails_x; break;
			case RailSegment::Direction::Y:  s= Images::rails_y; break;
			case RailSegment::Direction::XToUp: s= Images::rails_x_to_up; break;
			case RailSegment::Direction::XToDown: s= Images::rails_x_to_down; break;
			case RailSegment::Direction::UpToX: s= Images::rails_up_to_x; break;
			case RailSegment::Direction::DownToX:  s= Images::rails_down_to_x; break;
			case RailSegment::Direction::Fork:  s= Images::rails_fork; break;
			};

			SDL_Rect src_rect{ 0, 0, s->w, s->h };
			SDL_Rect dst_rect{
				segment.x * s->w * c_graphics_scale,
				segment.y * s->h * c_graphics_scale,
				s->w * c_graphics_scale,
				s->h * c_graphics_scale };

			SDL_UpperBlitScaled( s, &src_rect, surface_, &dst_rect );
		}

		auto time= SDL_GetTicks();

		RailSegment segment;
		segment.x= 4;
		segment.y= 5;
		SDL_Surface* s= ( (time / 1000)&1) ? Images::swith_off : Images::swith_on;

		SDL_Rect src_rect{ 0, 0, s->w, s->h };
		SDL_Rect dst_rect{
			segment.x * s->w * c_graphics_scale,
			segment.y * s->h * c_graphics_scale,
			s->w * c_graphics_scale,
			s->h * c_graphics_scale };

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
		SDL_Delay(1);
		//SDL_WaitEvent( &event ); // Wait for events. If there are no events - we can nothing to do.
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

	FreeImages();
	DeInitWindow();

	return 0;
}
