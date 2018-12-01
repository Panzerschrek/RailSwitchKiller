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
	std::unordered_map<const Victim*, ForkState> victims_state;

	struct
	{
		const Level::Path* current_path= nullptr;
		float path_progress= 0.0f; // В единицах рельсов
	} tram_state;
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
using MainLoopFunc= std::function< std::vector<InputEvent>() >;

void RunLevel( std::unique_ptr<Level> level, MainLoopFunc main_loop_func, DrawLevelFunc draw_level_func );
