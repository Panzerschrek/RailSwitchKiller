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


Level LoadLevel( int number )
{
	Level result;

	const std::string file_name= "res/levels/" + std::to_string(number) + ".json";

	std::FILE* const f= std::fopen( file_name.c_str() , "rb" );
	if( f == nullptr )
	{
		std::cout << "Can not open file " << file_name << std::endl;
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

	return ParseLevel( parse_result->root );
}
