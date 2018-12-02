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
const unsigned char c_level_tile_color_color[]= { 96, 32, 32 };
const SDL_Color c_font_color{ 240, 240, 240, 0 };

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

SDL_Surface* victims[ int(Victim::Count) ] = { 0 };

SDL_Surface* tram= nullptr;
SDL_Surface* tram_up= nullptr;
SDL_Surface* tram_down= nullptr;

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

	Images::victims[ int(Victim::Civilian) ]= IMG_Load( "res/victim_civilian.bmp" );
	Images::victims[ int(Victim::CivilianChild) ]= IMG_Load( "res/victim_child.bmp" );
	Images::victims[ int(Victim::CivilianOldster) ]= IMG_Load( "res/victim_oldster.bmp" );
	Images::victims[ int(Victim::Liar) ]= IMG_Load( "res/victim_liar.bmp" );
	Images::victims[ int(Victim::Thief) ]= IMG_Load( "res/victim_thief.bmp" );
	Images::victims[ int(Victim::Murderer) ]= IMG_Load( "res/victim_murderer.bmp" );
	Images::victims[ int(Victim::Rapist) ]= IMG_Load( "res/victim_rapist.bmp" );
	Images::victims[ int(Victim::Maniac) ]= IMG_Load( "res/victim_maniac.bmp" );
	Images::victims[ int(Victim::Capitalist) ]= IMG_Load( "res/victim_capitalist.bmp" );

	Images::tram= IMG_Load( "res/tram.bmp" );
	Images::tram_up= IMG_Load( "res/tram_up.bmp" );
	Images::tram_down= IMG_Load( "res/tram_down.bmp" );

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
	SDL_SetColorKey( Images::tram, 1, transparent_color_key );
	SDL_SetColorKey( Images::tram_up, 1, transparent_color_key );
	SDL_SetColorKey( Images::tram_down, 1, transparent_color_key );

	for( int i= 0; i < 4; ++i )
	{
		Images::blood[i]= IMG_Load( ("res/blood" + std::to_string(i) + ".bmp").c_str() );
		SDL_SetColorKey( Images::blood[i], 1, transparent_color_key );
	}

	for( int i= 0; i < int(Victim::Count); ++i )
	{
		if( Images::victims[i] != nullptr )
			SDL_SetColorKey( Images::victims[i], 1, transparent_color_key );
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
				SDL_Surface* s= Images::victims[ int(victim) ];
				if( s == nullptr )
					s= Images::victims[ int(Victim::Civilian) ];

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

	if( level_state.level_stage == LevelState::LevelStage::Intro )
	{
		SDL_Surface* const description_surface=
			TTF_RenderUTF8_Blended_Wrapped( font_, level_state.level_data->description.c_str(), c_font_color, c_window_width );

		SDL_Rect src_rect{ 0, 0, description_surface->w, description_surface->h };

		SDL_Rect dst_rect{
			( surface_->w - description_surface->w ) / 2,
			( surface_->h - description_surface->h ) / 2,
			description_surface->w,
			description_surface->h };

		SDL_UpperBlit( description_surface, &src_rect, surface_, &dst_rect );
		SDL_FreeSurface(description_surface);
	}
	else if( level_state.level_stage == LevelState::LevelStage::Finish )
	{
		std::string text= level_state.finish_state.map_failed ? u8"Уровень провален.\n" : u8"Уровень пройден.\n";
		text+= u8"Убито:\n";
		for( int i= 0; i < int(Victim::Count); ++i )
		{
			if( level_state.finish_state.intial_count[i] == 0 )
				continue;

			std::string name;
			switch( Victim(i) )
			{
			case Victim::Civilian: name= u8"Гражданский"; break;
			case Victim::CivilianChild: name= u8"Ребёнок"; break;
			case Victim::CivilianOldster: name= u8"Старик"; break;
			case Victim::Liar: name= u8"Лжец"; break;
			case Victim::Thief: name= u8"Вор"; break;
			case Victim::Murderer: name= u8"Убийца"; break;
			case Victim::Rapist: name= u8"Насильник"; break;
			case Victim::Maniac: name= u8"Маньяк"; break;
			case Victim::Capitalist: name= u8"Капиталист"; break;
			case Victim::Count: break;
			};

			text+= "  " + name + " - " + std::to_string(level_state.finish_state.killed[i]) + "/" + std::to_string(level_state.finish_state.intial_count[i]) + "\n";
		}
		text+= u8"Счёт: ";
		for( int i= 0; i < 3; ++i )
			text+= i < level_state.finish_state.stars ? u8"★" : u8"☆";

		if( level_state.finish_state.stars == 3 )
			text+= "\n\n" + level_state.level_data->success_description;

		SDL_Surface* const finish_text_surface=
			TTF_RenderUTF8_Blended_Wrapped( font_, text.c_str(), c_font_color, c_window_width / 3 );

		SDL_Rect src_rect{ 0, 0, finish_text_surface->w, finish_text_surface->h };

		SDL_Rect dst_rect{
			( surface_->w - finish_text_surface->w ) / 2,
			( surface_->h - finish_text_surface->h ) / 2,
			finish_text_surface->w,
			finish_text_surface->h };

		SDL_UpperBlit( finish_text_surface, &src_rect, surface_, &dst_rect );
		SDL_FreeSurface(finish_text_surface);
	}
	else
	{
		{ // draw stage info
			std::string text;
			if( level_state.level_stage == LevelState::LevelStage::Countdown )
				text= std::to_string(level_state.countdown_time_left_s) + "...";
			else if( level_state.level_stage == LevelState::LevelStage::Action )
				text= "Action!";

			SDL_Surface* const stage_text_surface= TTF_RenderUTF8_Blended_Wrapped( font_, text.c_str(), c_font_color, c_window_width );

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
			const Level::Path& tram_path= *level_state.tram_state.current_path;
			const size_t rail_segment_index= std::min( size_t(level_state.tram_state.path_progress), tram_path.rails.size() - 1u);
			const float part= level_state.tram_state.path_progress - float(rail_segment_index);
			const float part_minus_one= 1.0f - part;

			float x, y;
			SDL_Surface* s= nullptr;
			if( rail_segment_index == tram_path.rails.size() - 1u )
			{
				if( tram_path.fork != nullptr )
				{
					const auto it= level_state.forks_state.find( tram_path.fork.get() );
					const Level::Path* next_path= nullptr;
					if( it != level_state.forks_state.end() && it->second == LevelState::ForkState::Down )
						next_path= &tram_path.fork->lower_path;
					else
						next_path= &tram_path.fork->upper_path;

					x= tram_path.rails[rail_segment_index].x * part_minus_one + part * next_path->rails[0u].x;
					y= tram_path.rails[rail_segment_index].y * part_minus_one + part * next_path->rails[0u].y;

					if( tram_path.rails[rail_segment_index].x < next_path->rails[0u].x )
						s= Images::tram;
					else if( tram_path.rails[rail_segment_index].y > next_path->rails[0u].y )
						s= Images::tram_up;
					else if( tram_path.rails[rail_segment_index].y < next_path->rails[0u].y )
						s= Images::tram_down;
					else
						s= Images::tram;
				}
				else
				{
					x= tram_path.rails[rail_segment_index].x;
					y= tram_path.rails[rail_segment_index].y;
					s= Images::tram;
				}
			}
			else
			{
				x= tram_path.rails[rail_segment_index].x * part_minus_one + part * tram_path.rails[rail_segment_index+1u].x;
				y= tram_path.rails[rail_segment_index].y * part_minus_one + part * tram_path.rails[rail_segment_index+1u].y;

				if( tram_path.rails[rail_segment_index].x < tram_path.rails[rail_segment_index+1u].x )
					s= Images::tram;
				else if( tram_path.rails[rail_segment_index].y > tram_path.rails[rail_segment_index+1u].y )
					s= Images::tram_up;
				else if( tram_path.rails[rail_segment_index].y < tram_path.rails[rail_segment_index+1u].y )
					s= Images::tram_down;
				else
					s= Images::tram;
			}

			const int sprite_x_offset= ( Images::rails_x->w - s->w ) / 2;
			const int sprite_y_offset= ( Images::rails_x->h - s->h ) / 2;

			SDL_Rect src_rect{ 0, 0, s->w, s->h };
			SDL_Rect dst_rect{
				int(x * Images::rails_x->w) * c_graphics_scale + sprite_x_offset * c_graphics_scale,
				int(y * Images::rails_x->h) * c_graphics_scale + sprite_y_offset * c_graphics_scale,
				s->w * c_graphics_scale,
				s->h * c_graphics_scale };

			SDL_UpperBlitScaled( s, &src_rect, surface_, &dst_rect );
		}
	}
}

void DrawIntermissionMenu(const IntermissionState& intermission_state)
{
	const SDL_Rect bg_rect{ 0, 0, surface_->w, surface_->h };
	SDL_FillRect( surface_, &bg_rect, SDL_MapRGB( surface_->format, c_background_color[0], c_background_color[1], c_background_color[2] ) );

	for( int i= 0; i < IntermissionState::c_level_count; ++i )
	{
		int x= i % IntermissionState::c_columns;
		int y= i / IntermissionState::c_columns;

		const SDL_Rect fill_rect{
			( x * IntermissionState::c_tile_size + IntermissionState::c_tile_border / 2 ) * c_graphics_scale,
			( y * IntermissionState::c_tile_size + IntermissionState::c_tile_border / 2 ) * c_graphics_scale,
			( IntermissionState::c_tile_size - IntermissionState::c_tile_border ) * c_graphics_scale,
			( IntermissionState::c_tile_size - IntermissionState::c_tile_border ) * c_graphics_scale };

		// Rect.
		SDL_FillRect( surface_, &fill_rect, SDL_MapRGB( surface_->format, c_level_tile_color_color[0], c_level_tile_color_color[1], c_level_tile_color_color[2] ) );

		// Level caption.
		std::string text= "level " + std::to_string(i) + "\n";
		if( intermission_state.levels_state[i].completed )
		{
			for( int j= 0; j < 3; ++j )
				text+= j < intermission_state.levels_state[i].stars ? u8"★" : u8"☆";
		}
		else
			text+= "-";

		auto color= c_font_color;
		if( !intermission_state.levels_state[i].completed && i != intermission_state.first_incomplete_level )
		{
			color.r/= 2; color.g/= 2; color.b/= 2;
		}

		SDL_Surface* const caption_text_surface= TTF_RenderUTF8_Blended_Wrapped( font_, text.c_str(), color, c_window_width );

		SDL_Rect caption_src_rect{ 0, 0, caption_text_surface->w, caption_text_surface->h };

		SDL_Rect caption_dst_rect= fill_rect;

		SDL_UpperBlit( caption_text_surface, &caption_src_rect, surface_, &caption_dst_rect );
		SDL_FreeSurface(caption_text_surface);
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

		case SDL_MOUSEBUTTONDOWN:
			input_events.emplace_back();
			input_events.back().kind= InputEvent::Kind::Mouse;
			input_events.back().x= event.button.x / c_graphics_scale;
			input_events.back().y= event.button.y / c_graphics_scale;
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

	IntermissionState intermission_state= LoadIntermissionState();

	while(true)
	{
		const int level_number=
			RunIntermissionMenu(
				intermission_state,
				MainLoop,
				[]( const IntermissionState& intermission_state )
				{
					if( SDL_MUSTLOCK( surface_ ) )
						SDL_LockSurface( surface_ );

					DrawIntermissionMenu(intermission_state);

					if( SDL_MUSTLOCK( surface_ ) )
						SDL_UnlockSurface( surface_ );

					SDL_UpdateWindowSurface( window_ );
				} );

		if( level_number < 0 )
			break;

		const LevelState level_state=
			RunLevel(
				std::unique_ptr<Level>( new Level(LoadLevel(level_number) ) ),
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

		if( level_state.finish_state.aborted )
			break;
		if( !level_state.finish_state.map_failed )
		{
			intermission_state.levels_state[level_number].completed= true;
			intermission_state.levels_state[level_number].stars= level_state.finish_state.stars;
			intermission_state.first_incomplete_level= std::max( intermission_state.first_incomplete_level, level_number + 1 );
		}

		SaveIntermissionState(intermission_state);
	}

	SaveIntermissionState(intermission_state);

	DeInitFont();
	FreeImages();
	DeInitWindow();

	return 0;
}
