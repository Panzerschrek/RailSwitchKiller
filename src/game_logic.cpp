#include <SDL.h>
#include "game_logic.hpp"

namespace Constants
{

const float tram_speed= 3.0f;
const int tile_size= 16;

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
		result.second+= std::max( lower_result.second, upper_result.first );
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
				break;

			case InputEvent::Kind::Quit:
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
				const int victim_number=
					int(touched_rail) -
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
