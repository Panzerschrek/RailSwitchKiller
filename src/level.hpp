#pragma once
#include <memory>
#include <string>
#include <vector>

enum class Victim
{
	Civilian,
	CivilianChild,
	CivilianOldster,
	Liar,
	Thief,
	Murder,
	Rapist,
	Maniac, // rapist + murder
	Capitalist,
	// TODO - add more victims

	Count,
};

struct Level
{
public:
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

	struct Fork;
	struct Path
	{
		std::vector<Victim> path_victims;
		std::unique_ptr<Fork> fork; // Optional. End paths have no forks.

		std::vector<RailSegment> rails; // if fork exists, last segment must be fork.
	};

	struct Fork
	{
		Path upper_path;
		Path lower_path;
	};

public:
	std::string name;
	std::string description;
	int think_time_sec= 5;

	Path root_path;
};

Level LoadLevel( int number );
