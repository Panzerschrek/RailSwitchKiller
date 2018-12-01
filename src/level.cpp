#include <iostream>
#include <PanzerJson/parser.hpp>
#include "level.hpp"

Level::Path ParsePath( const PanzerJson::Value& path_json )
{
	Level::Path result;

	for( const PanzerJson::Value victim_json : path_json["victims"].array_elements() )
	{
		const auto victim_name= victim_json.AsString();

		if( PanzerJson::StringCompare( victim_name, "civilian" ) == 0 )
			result.path_victims.push_back( Victim::Civilian );
		else if( PanzerJson::StringCompare( victim_name, "liar" ) == 0 )
			result.path_victims.push_back( Victim::Liar );
		else if( PanzerJson::StringCompare( victim_name, "murder" ) == 0 )
			result.path_victims.push_back( Victim::Murder );
		else
			std::cout << "unsupported victim: " << victim_name << std::endl;
	}

	if( path_json.IsMember("fork") )
	{
		result.fork.reset( new Level::Fork );
		result.fork->lower_path= ParsePath( path_json["fork"]["lower_path"] );
		result.fork->upper_path= ParsePath( path_json["fork"]["upper_path"] );
	}

	for( const PanzerJson::Value rail_segment_json : path_json["rails"] )
	{
		Level::RailSegment::Direction direction;
		const auto direction_name= rail_segment_json["direction"].AsString();
		if( PanzerJson::StringCompare( direction_name, "x" ) == 0 )
			direction= Level::RailSegment::Direction::X;
		else if( PanzerJson::StringCompare( direction_name, "y" ) == 0 )
			direction= Level::RailSegment::Direction::Y;
		else if( PanzerJson::StringCompare( direction_name, "x_to_up" ) == 0 )
			direction= Level::RailSegment::Direction::XToUp;
		else if( PanzerJson::StringCompare( direction_name, "x_to_down" ) == 0 )
			direction= Level::RailSegment::Direction::XToDown;
		else if( PanzerJson::StringCompare( direction_name, "up_to_x" ) == 0 )
			direction= Level::RailSegment::Direction::UpToX;
		else if( PanzerJson::StringCompare( direction_name, "down_to_x" ) == 0 )
			direction= Level::RailSegment::Direction::DownToX;
		else if( PanzerJson::StringCompare( direction_name, "fork" ) == 0 )
			direction= Level::RailSegment::Direction::Fork;
		else
		{
			std::cout << "unsupported rails direction: " << direction_name << std::endl;
			continue;
		}

		result.rails.emplace_back();
		result.rails.back().x= rail_segment_json["x"].AsInt();
		result.rails.back().y= rail_segment_json["y"].AsInt();
		result.rails.back().direction= direction;
	}

	return result;
}

Level ParseLevel( const PanzerJson::Value& level_json )
{
	Level result;

	result.name= level_json["name"].AsString();
	result.description= level_json["description"].AsString();
	result.think_time_sec= level_json["think_time"].AsInt();

	result.root_path= ParsePath( level_json["root_path"] );

	return result;
}
