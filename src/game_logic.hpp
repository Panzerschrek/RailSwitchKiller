#include <functional>
#include <unordered_map>
#include "level.hpp"

// State of level for game logic/painting.
struct LevelState
{
	enum class LevelStage
	{
		Intro,
		Countdown,
		Action,
		Finish,
	};

	enum class ForkState
	{
		Up,
		Down,
	};

	enum class VictimState
	{
		Alive,
		Dead,
	};

	LevelStage level_stage= LevelStage::Intro;
	std::unique_ptr<Level> level_data;
	int countdown_time_left_s= 0;

	std::unordered_map<const Level::Fork*, ForkState> forks_state;
	std::unordered_map<const Victim*, VictimState> victims_state;

	struct
	{
		const Level::Path* current_path= nullptr;
		float path_progress= 0.0f; // В единицах рельсов
	} tram_state;

	struct
	{
		int intial_count[int(Victim::Count)];
		int killed[int(Victim::Count)];
		int stars= 0; // 0 - 3
		bool map_failed= false;
		bool aborted= false; // Player whants to quit.

	} finish_state;
};

struct IntermissionState
{
	struct LevelState
	{
		int stars= 0; // 0 - 3
		bool completed= false;
	};

	static constexpr int c_level_count= 20;
	LevelState levels_state[20];
	int first_incomplete_level= 0;

	static constexpr int c_columns= 6;
	static constexpr int c_tile_size= 40;
	static constexpr int c_tile_border= 8;
};

struct InputEvent
{
	enum class Kind
	{
		Mouse,
		Key,
		Quit,
	};
	Kind kind;
	int x, y;
};

using DrawLevelFunc= std::function< void(const LevelState&) >;
using DrawIntermissionMenuFunc= std::function< void(const IntermissionState&) >;
using MainLoopFunc= std::function< std::vector<InputEvent>() >;

LevelState RunLevel( std::unique_ptr<Level> level, MainLoopFunc main_loop_func, DrawLevelFunc draw_level_func );
int RunIntermissionMenu(
	const IntermissionState& intermission_state,
	MainLoopFunc main_loop_func,
	DrawIntermissionMenuFunc draw_intermission_menu_func );
