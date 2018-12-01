#include <iostream>
#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "game_logic.hpp"
#include "level.hpp"


// View
const unsigned int c_window_width= 1024;
const unsigned int c_window_height= 768;

const int c_graphics_scale = 3;

SDL_Window* window_= nullptr;
SDL_Surface* surface_= nullptr;
TTF_Font* font_= nullptr;

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

SDL_Surface* tram= nullptr;

SDL_Surface* blood[4]= { nullptr, nullptr, nullptr, nullptr };

}

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

	Images::tram= IMG_Load( "res/tram.bmp" );

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
	SDL_SetColorKey( Images::tram, 1, transparent_color_key );

	for( int i= 0; i < 4; ++i )
	{
		Images::blood[i]= IMG_Load( ("res/blood" + std::to_string(i) + ".bmp").c_str() );
		SDL_SetColorKey( Images::blood[i], 1, transparent_color_key );
	}
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
	//test_text_= TTF_RenderUTF8_Blended_Wrapped( font_, "Тестовый текст\nна несколько строк\nwith latin\nand utf ööüç characters!", color, c_window_width );
}

void DeInitFont()
{
	TTF_CloseFont( font_ );
	TTF_Quit();
}

void DrawPath( const LevelState& level_state, const Level::Path& path )
{
	using Direction = Level::RailSegment::Direction;

	size_t rail_index= 0;
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

		if( rail_index >= path.rails.size() - path.path_victims.size() )
		{
			const Victim& victim= path.path_victims[ rail_index - (path.rails.size() - path.path_victims.size() ) ];

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

			const auto it= level_state.victims_state.find( &victim );
			if( it != level_state.victims_state.end() && it->second == LevelState::VictimState::Dead )
			{
				int blood_index= ( int(rail_index) + int(reinterpret_cast<uintptr_t>(&path) / 4u ) ) & 3;

				SDL_Surface* s= Images::blood[blood_index];

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
		}

		++rail_index;
	}

	if( path.fork != nullptr )
	{
		{
			Level::RailSegment segment= path.rails.back();
			++segment.x;

			SDL_Surface* s= nullptr;
			const auto it= level_state.forks_state.find( path.fork.get() );
			if( it != level_state.forks_state.end() && it->second == LevelState::ForkState::Up )
				s= Images::swith_on;
			else
				s= Images::swith_off;

			SDL_Rect src_rect{ 0, 0, s->w, s->h };
			SDL_Rect dst_rect{
				segment.x * s->w * c_graphics_scale,
				segment.y * s->h * c_graphics_scale,
				s->w * c_graphics_scale,
				s->h * c_graphics_scale };

			SDL_UpperBlitScaled( s, &src_rect, surface_, &dst_rect );
		}

		DrawPath( level_state, path.fork->lower_path );
		DrawPath( level_state, path.fork->upper_path );
	}
}

void DrawLevel(const LevelState& level_state )
{
	const SDL_Rect bg_rect{ 0, 0, surface_->w, surface_->h };
	SDL_FillRect( surface_, &bg_rect, SDL_MapRGB( surface_->format, c_background_color[0], c_background_color[1], c_background_color[2] ) );

	const SDL_Color font_color{ 240, 240, 240, 0 };

	if( level_state.level_stage == LevelState::LevelStage::Intro )
	{
		SDL_Surface* const description_surface=
			TTF_RenderUTF8_Blended_Wrapped( font_, level_state.level_data->description.c_str(), font_color, c_window_width );

		SDL_Rect src_rect{ 0, 0, description_surface->w, description_surface->h };

		SDL_Rect dst_rect{
			( surface_->w - description_surface->w ) / 2,
			( surface_->h - description_surface->h ) / 2,
			description_surface->w,
			description_surface->h };

		SDL_UpperBlit( description_surface, &src_rect, surface_, &dst_rect );
		SDL_FreeSurface(description_surface);
	}
	else
	{
		{ // draw stage info
			std::string text;
			if( level_state.level_stage == LevelState::LevelStage::Countdown )
				text= std::to_string(level_state.countdown_time_left_s) + "...";
			else if( level_state.level_stage == LevelState::LevelStage::Action )
				text= "Action!";
			else if( level_state.level_stage == LevelState::LevelStage::Finish )
				text= "Finish!";

			SDL_Surface* const stage_text_surface= TTF_RenderUTF8_Blended_Wrapped( font_, text.c_str(), font_color, c_window_width );

			SDL_Rect src_rect{ 0, 0, stage_text_surface->w, stage_text_surface->h };

			SDL_Rect dst_rect{
				( surface_->w - stage_text_surface->w ) / 2,
				stage_text_surface->h / 2,
				stage_text_surface->w,
				stage_text_surface->h };

			SDL_UpperBlit( stage_text_surface, &src_rect, surface_, &dst_rect );
			SDL_FreeSurface(stage_text_surface);
		}

		DrawPath( level_state, level_state.level_data->root_path );

		{

			const int sprite_x_offset= ( Images::rails_x->w - Images::tram->w ) / 2;
			const int sprite_y_offset= ( Images::rails_x->h - Images::tram->h ) / 2;

			const Level::Path& tram_path= *level_state.tram_state.current_path;
			const size_t rail_segment_index= std::min( size_t(level_state.tram_state.path_progress), tram_path.rails.size() - 1u);
			const float part= level_state.tram_state.path_progress - float(rail_segment_index);
			const float part_minus_one= 1.0f - part;

			float x, y;
			if( rail_segment_index == tram_path.rails.size() - 1u )
			{
				x= tram_path.rails[rail_segment_index].x;
				y= tram_path.rails[rail_segment_index].y;
			}
			else
			{
				x= tram_path.rails[rail_segment_index].x * part_minus_one + part * tram_path.rails[rail_segment_index+1u].x;
				y= tram_path.rails[rail_segment_index].y * part_minus_one + part * tram_path.rails[rail_segment_index+1u].y;
			}

			SDL_Rect src_rect{ 0, 0, Images::tram->w, Images::tram->h };
			SDL_Rect dst_rect{
				int(x * Images::rails_x->w) * c_graphics_scale + sprite_x_offset * c_graphics_scale,
				int(y * Images::rails_x->h) * c_graphics_scale + sprite_y_offset * c_graphics_scale,
				Images::tram->w * c_graphics_scale,
				Images::tram->h * c_graphics_scale };

			SDL_UpperBlitScaled( Images::tram, &src_rect, surface_, &dst_rect );
		}
	}
}

std::vector<InputEvent> MainLoop()
{
	std::vector<InputEvent> input_events;

	SDL_Event event;
	SDL_Delay(10);
	do
	{
		switch(event.type)
		{
		case SDL_WINDOWEVENT:
			if( event.window.event == SDL_WINDOWEVENT_CLOSE )
			{
				input_events.emplace_back();
				input_events.back().kind= InputEvent::Kind::Quit;
			}
		break;

		case SDL_QUIT:
			{
				input_events.emplace_back();
				input_events.back().kind= InputEvent::Kind::Quit;
			}
			break;

		case SDL_KEYDOWN:
			{
				input_events.emplace_back();
				input_events.back().kind= InputEvent::Kind::Key;
			}
			break;
		}
	} while( SDL_PollEvent(&event) );

	return input_events;
}

int main()
{
	SDL_Init( SDL_INIT_AUDIO | SDL_INIT_VIDEO );

	InitWindow();
	LoadImages();
	InitFont();

	RunLevel(
		std::unique_ptr<Level>( new Level(LoadLevel(0)) ),
		MainLoop,
		[]( const LevelState& level_state )
		{
			if( SDL_MUSTLOCK( surface_ ) )
				SDL_LockSurface( surface_ );

			DrawLevel(level_state);

			if( SDL_MUSTLOCK( surface_ ) )
				SDL_UnlockSurface( surface_ );

			SDL_UpdateWindowSurface( window_ );
		});


	DeInitFont();
	FreeImages();
	DeInitWindow();

	return 0;
}
