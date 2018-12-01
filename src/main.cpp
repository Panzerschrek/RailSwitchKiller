#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "level.hpp"


// View
const unsigned int c_window_width= 1024;
const unsigned int c_window_height= 768;

const int c_graphics_scale = 4;

SDL_Window* window_= nullptr;
SDL_Surface* surface_= nullptr;
TTF_Font* font_= nullptr;

SDL_Surface* test_text_= nullptr;

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

SDL_Surface* victim_civilian= nullptr;

}

Level current_level_;

void InitWindow()
{
	window_= SDL_CreateWindow(
		u8"Стрелочный убийца",
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

	Images::victim_civilian= IMG_Load( "res/victim_civilian.bmp" );

	auto transparent_color_key= SDL_MapRGB( surface_->format, 0, 0, 0 );
	SDL_SetColorKey( Images::rails_x, 1, transparent_color_key );
	SDL_SetColorKey( Images::rails_y, 1, transparent_color_key );
	SDL_SetColorKey( Images::rails_x_to_up, 1, transparent_color_key );
	SDL_SetColorKey( Images::rails_x_to_down, 1, transparent_color_key );
	SDL_SetColorKey( Images::rails_up_to_x, 1, transparent_color_key );
	SDL_SetColorKey( Images::rails_down_to_x, 1, transparent_color_key );
	SDL_SetColorKey( Images::rails_fork, 1, transparent_color_key );
	SDL_SetColorKey( Images::swith_off, 1, transparent_color_key );
	SDL_SetColorKey( Images::swith_on, 1, transparent_color_key );
	SDL_SetColorKey( Images::victim_civilian, 1, transparent_color_key );
}

void FreeImages()
{
	// TODO - free it
}

void InitFont()
{
	TTF_Init();

	font_= TTF_OpenFont( "res/DejaVuSans.ttf", 20 );

	SDL_Color color{ 240, 240, 240, 0 };
	test_text_= TTF_RenderUTF8_Blended_Wrapped( font_, "Тестовый текст\nна несколько строк\nwith latin\nand utf ööüç characters!", color, c_window_width );
}

void DeInitFont()
{
	TTF_CloseFont( font_ );
	TTF_Quit();
}

void DrawPath( const Level::Path& path )
{
	using Direction = Level::RailSegment::Direction;

	size_t number= 0;
	for( const Level::RailSegment& segment : path.rails )
	{
		SDL_Surface* rail_surface= nullptr;
		switch(segment.direction)
		{
		case Direction::X: rail_surface= Images::rails_x; break;
		case Direction::Y: rail_surface= Images::rails_y; break;
		case Direction::XToUp: rail_surface= Images::rails_x_to_up; break;
		case Direction::XToDown: rail_surface= Images::rails_x_to_down; break;
		case Direction::UpToX: rail_surface= Images::rails_up_to_x; break;
		case Direction::DownToX:  rail_surface= Images::rails_down_to_x; break;
		case Direction::Fork:  rail_surface= Images::rails_fork; break;
		};

		SDL_Rect src_rect{ 0, 0, rail_surface->w, rail_surface->h };
		SDL_Rect dst_rect{
			segment.x * rail_surface->w * c_graphics_scale,
			segment.y * rail_surface->h * c_graphics_scale,
			rail_surface->w * c_graphics_scale,
			rail_surface->h * c_graphics_scale };

		SDL_UpperBlitScaled( rail_surface, &src_rect, surface_, &dst_rect );

		if( path.path_victims.size() > 0u && path.path_victims.size() >= path.rails.size() - number )
		{
			SDL_Surface* s= Images::victim_civilian;

			const int x_offset= ( rail_surface->w - s->w ) / 2;
			const int y_offset= ( rail_surface->h - s->h ) / 2;

			SDL_Rect src_rect{ 0, 0, s->w, s->h };
			SDL_Rect dst_rect{
				segment.x * rail_surface->w * c_graphics_scale + x_offset * c_graphics_scale,
				segment.y * rail_surface->h * c_graphics_scale + y_offset * c_graphics_scale,
				s->w * c_graphics_scale,
				s->h * c_graphics_scale };

			SDL_UpperBlitScaled( s, &src_rect, surface_, &dst_rect );
		}

		++number;
	}

	if( path.fork != nullptr )
	{
		{
			auto time= SDL_GetTicks();

			Level::RailSegment segment= path.rails.back();
			++segment.x;
			SDL_Surface* s= ( (time / 1000)&1) ? Images::swith_off : Images::swith_on;

			SDL_Rect src_rect{ 0, 0, s->w, s->h };
			SDL_Rect dst_rect{
				segment.x * s->w * c_graphics_scale,
				segment.y * s->h * c_graphics_scale,
				s->w * c_graphics_scale,
				s->h * c_graphics_scale };

			SDL_UpperBlitScaled( s, &src_rect, surface_, &dst_rect );
		}

		DrawPath( path.fork->lower_path );
		DrawPath( path.fork->upper_path );
	}
}

void Draw()
{
	const SDL_Rect bg_rect{ 0, 0, surface_->w, surface_->h };
	SDL_FillRect( surface_, &bg_rect, SDL_MapRGB( surface_->format, c_background_color[0], c_background_color[1], c_background_color[2] ) );

	DrawPath( current_level_.root_path );

	{
		SDL_Rect src_rect{ 0, 0, test_text_->w, test_text_->h };
		SDL_Rect dst_rect{
			100,
			200,
			test_text_->w,
			test_text_->h };

		SDL_UpperBlit( test_text_, &src_rect, surface_, &dst_rect );

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
		SDL_Delay(10);
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
	SDL_Init( SDL_INIT_AUDIO | SDL_INIT_VIDEO );

	InitWindow();
	LoadImages();
	InitFont();

	current_level_= LoadLevel(0);

	MainLoop();

	DeInitFont();
	FreeImages();
	DeInitWindow();

	return 0;
}
