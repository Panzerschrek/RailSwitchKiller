#include <SDL.h>
#include "game_logic.hpp"

void RunLevel( std::unique_ptr<Level> level, MainLoopFunc main_loop_func, DrawLevelFunc draw_level_func )
{
	LevelState level_state;
	level_state.level_data= std::move(level);

	Uint32 stage_start_time= SDL_GetTicks();

	while(true)
	{
		for( const InputEvent& input_event : main_loop_func() )
		{
			switch(input_event.kind)
			{
			case InputEvent::Kind::Mouse: break;
			case InputEvent::Kind::Key:
				if( level_state.level_stage == LevelState::LevelStage::Intro )
				{
					level_state.level_stage= LevelState::LevelStage::Countdown;
					stage_start_time= SDL_GetTicks();
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
			}
		}

		draw_level_func(level_state);
	}
}
