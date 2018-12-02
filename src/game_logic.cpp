#include <fstream>
#include <iostream>
#include <SDL.h>
#include <PanzerJson/parser.hpp>
#include <PanzerJson/streamed_serializer.hpp>
#include "game_logic.hpp"

namespace Constants
{

const float tram_speed= 3.0f;
const int tile_size= 16;

const char save_file_name[]= "save.json";

}

void InitPaths_r( LevelState& level_state, const Level::Path& path )
{
	if( path.fork != nullptr )
	{
		level_state.forks_state[ path.fork.get() ]= LevelState::ForkState::Down;
		InitPaths_r( level_state, path.fork->lower_path );
		InitPaths_r( level_state, path.fork->upper_path );
	}

	for( const Victim& victim : path.path_victims )
		level_state.victims_state[ &victim ]= LevelState::VictimState::Alive;
}

LevelState InitLevelState( std::unique_ptr<Level> level )
{
	LevelState level_state;
	level_state.level_data= std::move(level);

	level_state.tram_state.current_path= &level_state.level_data->root_path;
	level_state.tram_state.path_progress= 0.0f;

	InitPaths_r( level_state, level_state.level_data->root_path );

	return level_state;
}

void TryChangeForkState( const int click_tile_x, const int click_tile_y, LevelState& level_state, const Level::Path& path )
{
	if( path.fork != nullptr )
	{
		if( click_tile_x == path.rails.back().x + 1 &&
			click_tile_y == path.rails.back().y )
		{
			LevelState::ForkState& fork_state= level_state.forks_state[ path.fork.get() ];
			if( fork_state == LevelState::ForkState::Down )
				fork_state= LevelState::ForkState::Up;
			else
				fork_state= LevelState::ForkState::Down;
		}

		TryChangeForkState( click_tile_x, click_tile_y, level_state, path.fork->lower_path );
		TryChangeForkState( click_tile_x, click_tile_y, level_state, path.fork->upper_path );
	}
}

int ScoreForVictim( const Victim victim )
{
	// Здесь сосредоточены веса за убийства всех персонажей.
	// Чем меньше наберёт игрок штрафных баллов - тем лучше.

	switch(victim)
	{
	case Victim::Civilian: return 100;
	case Victim::CivilianChild: return 200;
	case Victim::CivilianOldster: return 50;
	case Victim::Liar: return 30;
	case Victim::Thief: return 10;
	case Victim::Murderer: return -100;
	case Victim::Rapist: return -50;
	case Victim::Maniac: return -200;
	case Victim::Capitalist: return -500;

	case Victim::Count:
		break;
	};

	return 0;
}

// Возвращает пару максимальный + минимальный штрафы за проезд участка пути.
std::pair<int, int> CalculateWorstAndBestScore( const Level::Path& path )
{
	int path_score= 0;
	for( const Victim& victim : path.path_victims )
		path_score+= ScoreForVictim(victim);

	std::pair<int, int> result;
	result.first= result.second= path_score;

	if( path.fork != nullptr )
	{
		std::pair<int, int> lower_result= CalculateWorstAndBestScore( path.fork->lower_path );
		std::pair<int, int> upper_result= CalculateWorstAndBestScore( path.fork->upper_path );

		result.first+= std::min( lower_result.first, upper_result.first );
		result.second+= std::max( lower_result.second, upper_result.second );
	}

	return result;
}

void CalculateFinisScore( LevelState& level_state )
{
	for( auto& count : level_state.finish_state.intial_count )
		count= 0;
	for( auto& count : level_state.finish_state.killed )
		count= 0;

	const std::pair<int, int> worst_and_best_score= CalculateWorstAndBestScore( level_state.level_data->root_path );

	int score= 0;
	for( const auto& victim_pair : level_state.victims_state )
	{
		++level_state.finish_state.intial_count[ int(*victim_pair.first) ];

		// Получаем штрафы только за убитых.
		// Если у нас есть награда за убийство, то выгодно будет убивать некоторых, чем оставлять в живых.
		if( victim_pair.second == LevelState::VictimState::Dead )
		{
			score+= ScoreForVictim( *victim_pair.first );
			++level_state.finish_state.killed[ int(*victim_pair.first) ];
		}
	}

	if( score == worst_and_best_score.first )
	{
		// Набрали минимум - это победа
		level_state.finish_state.stars= 3;
		level_state.finish_state.map_failed= false;
	}
	else if( score == worst_and_best_score.second )
	{
		// Набрали максимум - это проигрыш
		level_state.finish_state.stars= 0;
		level_state.finish_state.map_failed= true;
	}
	else
	{
		level_state.finish_state.map_failed= false;
		level_state.finish_state.stars= 3 * ( score - worst_and_best_score.first ) / ( worst_and_best_score.second - worst_and_best_score.first );
	}
}

LevelState RunLevel( std::unique_ptr<Level> level, MainLoopFunc main_loop_func, DrawLevelFunc draw_level_func )
{
	LevelState level_state= InitLevelState(std::move(level));

	Uint32 stage_start_time= SDL_GetTicks();
	Uint32 prev_tick_time= stage_start_time;

	while(true)
	{
		const Uint32 tick_time= SDL_GetTicks();

		for( const InputEvent& input_event : main_loop_func() )
		{
			switch(input_event.kind)
			{
			case InputEvent::Kind::Mouse:
				if( level_state.level_stage == LevelState::LevelStage::Countdown )
					TryChangeForkState( input_event.x / Constants::tile_size, input_event.y / Constants::tile_size, level_state, level_state.level_data->root_path );
				break;

			case InputEvent::Kind::Key:
				if( level_state.level_stage == LevelState::LevelStage::Intro )
				{
					level_state.level_stage= LevelState::LevelStage::Countdown;
					stage_start_time= tick_time;
				}
				else if( level_state.level_stage == LevelState::LevelStage::Finish )
					return level_state;
				break;

			case InputEvent::Kind::Quit:
				level_state.finish_state.aborted= true;
				return level_state;
			};
		}

		if( level_state.level_stage == LevelState::LevelStage::Countdown )
		{
			const int time_since_countdown_start_s= ( SDL_GetTicks() - stage_start_time ) / 1000;
			level_state.countdown_time_left_s= level_state.level_data->think_time_sec - time_since_countdown_start_s;

			if( time_since_countdown_start_s >= level_state.level_data->think_time_sec )
			{
				level_state.level_stage= LevelState::LevelStage::Action;
				stage_start_time= tick_time;
			}
		}
		if( level_state.level_stage == LevelState::LevelStage::Action )
		{
			float dt_s= float(tick_time - prev_tick_time) / 1000.0f;
			dt_s= std::min(dt_s, 1.0f); // Ограничиваем шаг физики.
			float tram_pos_delta= Constants::tram_speed * dt_s;

			level_state.tram_state.path_progress+= tram_pos_delta;
			if( size_t(level_state.tram_state.path_progress) >= level_state.tram_state.current_path->rails.size() )
			{
				if( level_state.tram_state.current_path->fork != nullptr )
				{
					level_state.tram_state.path_progress-= float(level_state.tram_state.current_path->rails.size());
					level_state.tram_state.current_path=
						level_state.forks_state[ level_state.tram_state.current_path->fork.get() ] == LevelState::ForkState::Up
							? &level_state.tram_state.current_path->fork->upper_path
							: &level_state.tram_state.current_path->fork->lower_path;
				}
				else
				{
					// Приехали
					level_state.level_stage= LevelState::LevelStage::Finish;
					CalculateFinisScore( level_state );
				}
			}
			else
			{
				const size_t touched_rail= size_t(level_state.tram_state.path_progress + 0.5f);
				const size_t fork_offset= level_state.tram_state.current_path->fork == nullptr ? 0u : 1u;

				const int victim_number=
					int(touched_rail + fork_offset) -
					int(level_state.tram_state.current_path->rails.size() - level_state.tram_state.current_path->path_victims.size());
				if( victim_number >= 0 && victim_number < int(level_state.tram_state.current_path->path_victims.size()))
					level_state.victims_state[ &level_state.tram_state.current_path->path_victims[victim_number] ]= LevelState::VictimState::Dead;
			}
		}

		draw_level_func(level_state);

		prev_tick_time= tick_time;
	}

	return level_state;
}

int GetLevelForCLick( int click_x, int click_y )
{
	for( int i= 0; i < IntermissionState::c_level_count; ++i )
	{
		int x= i % IntermissionState::c_columns;
		int y= i / IntermissionState::c_columns;

		int x_min= x * IntermissionState::c_tile_size + IntermissionState::c_tile_border / 2;
		int y_min= y * IntermissionState::c_tile_size + IntermissionState::c_tile_border / 2;
		int size= IntermissionState::c_tile_size - IntermissionState::c_tile_border;

		if( click_x >= x_min && click_y >= y_min && click_x < x_min + size && click_y < y_min + size )
			return i;
	}

	return -1;
}

int RunIntermissionMenu(
	const IntermissionState& intermission_state,
	MainLoopFunc main_loop_func,
	DrawIntermissionMenuFunc draw_intermission_menu_func )
{
	while(true)
	{
		for( const InputEvent& input_event : main_loop_func() )
		{
			switch(input_event.kind)
			{
			case InputEvent::Kind::Mouse:
			{
				int level= GetLevelForCLick( input_event.x, input_event.y );
				if( level >= 0 && level < IntermissionState::c_level_count &&
					level <= intermission_state.first_incomplete_level )
					return level;
			}
				break;

			case InputEvent::Kind::Key:
				break;

			case InputEvent::Kind::Quit:
				return -1;
			};
		}

		draw_intermission_menu_func(intermission_state);
	}

	return -1;
}

void SaveIntermissionState( const IntermissionState& intermission_state )
{
	std::ofstream stream( Constants::save_file_name );

	PanzerJson::StreamedSerializer<std::ofstream, PanzerJson::SerializationFormatting::TabIndents> serializer( stream );
	auto obj= serializer.AddObject();

	obj.AddNumber( "first_incomplete_level", intermission_state.first_incomplete_level );

	{
		auto arr= obj.AddArray( "levels_state" );
		for( int i= 0; i < IntermissionState::c_level_count; ++i )
		{
			auto level_obj= arr.AddObject();

			level_obj.AddBool( "completed", intermission_state.levels_state[i].completed );
			level_obj.AddNumber( "stars", intermission_state.levels_state[i].stars );
		}
	}
}

IntermissionState LoadIntermissionState()
{
	IntermissionState result;

	std::FILE* const f= std::fopen( Constants::save_file_name, "rb" );
	if( f == nullptr )
	{
		std::cout << "Can not open file " << Constants::save_file_name << std::endl;
		return result;
	}

	std::fseek( f, 0, SEEK_END );
	const size_t file_size= std::ftell( f );
	std::fseek( f, 0, SEEK_SET );

	std::vector<char> file_content( file_size );
	std::fread( file_content.data(), 1, file_size, f ); // TODO - check file errors
	std::fclose(f);

	const PanzerJson::Parser::ResultPtr parse_result=
		PanzerJson::Parser().Parse( file_content.data(), file_content.size() );
	if(  parse_result->error != PanzerJson::Parser::Result::Error::NoError )
	{
		std::cout << "Error, parsing json" << std::endl;
		return result;
	}

	const PanzerJson::Value& state_json= parse_result->root;
	result.first_incomplete_level= state_json["first_incomplete_level"].AsInt();

	for( int i= 0; i < IntermissionState::c_level_count; ++i )
	{
		const PanzerJson::Value& level_state_json= state_json["levels_state"][i];
		result.levels_state[i].completed= level_state_json["completed"].AsInt() != 0;
		result.levels_state[i].stars= level_state_json["stars"].AsInt();
	}

	return result;
}
