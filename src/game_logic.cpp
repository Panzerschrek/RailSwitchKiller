#include <SDL.h>
#include "game_logic.hpp"

namespace Constants
{

const float tram_speed= 2.0f;
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

void RunLevel( std::unique_ptr<Level> level, MainLoopFunc main_loop_func, DrawLevelFunc draw_level_func )
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
				return;
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
}
