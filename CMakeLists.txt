cmake_minimum_required(VERSION 2.8)

project(ProjectName)

set( CMAKE_CXX_STANDARD 11 )

#if( ${CMAKE_BUILD_TYPE} EQUAL Debug )
#	add_definitions( -DDEBUG )
#else()
#endif()

# Set path to your SDL2 here.
if( WIN32 )
	set( SDL_BASE_DIR ../SDL2-2.0.3 )
	set( SDL_INCLUDES_DIR ${SDL_BASE_DIR}/include )
	set( SDL_LIBS_DIR ${SDL_BASE_DIR}/lib/x86 )
	get_filename_component( SDL_LIBS_DIR_ABSOLUTE ${SDL_LIBS_DIR} ABSOLUTE )
else()
	set( SDL_INCLUDES_DIR /usr/include/SDL2 )
	set( SDL_IMAGE_INCLUDES_DIR /usr/include/SDL2 )
	set( SDL_TTF_INCLUDES_DIR /usr/include/SDL2 )
endif()

#
# HexLib
#

set( SOURCES
	src/level.cpp
	src/main.cpp
	)

set( HEADERS
	src/level.hpp )

set( PANZER_JSON_HEADERS
	PanzerJson/include/PanzerJson/fwd.hpp
	PanzerJson/include/PanzerJson/value.hpp
	PanzerJson/include/PanzerJson/value.inl
	PanzerJson/include/PanzerJson/parser.hpp
	PanzerJson/include/PanzerJson/serializer.hpp
	PanzerJson/include/PanzerJson/serializer.inl
	PanzerJson/include/PanzerJson/streamed_serializer.hpp
	PanzerJson/include/PanzerJson/streamed_serializer.inl )

set( PANZER_JSON_SOURCES
	PanzerJson/src/panzer_json_assert.hpp
	PanzerJson/src/parser.cpp
	PanzerJson/src/serializer.cpp
	PanzerJson/src/serializers_common.cpp
	PanzerJson/src/serializers_common.hpp
	PanzerJson/src/serializers_common.inl
	PanzerJson/src/value.cpp )

add_executable( Game ${SOURCES} ${HEADERS} ${PANZER_JSON_SOURCES} ${PANZER_JSON_HEADERS} )

if( WIN32 )
	target_link_libraries( Game opengl32 )
	target_link_libraries( Game ${SDL_LIBS_DIR_ABSOLUTE}/SDL2main.lib )
	target_link_libraries( Game ${SDL_LIBS_DIR_ABSOLUTE}/SDL2.lib )
else()
	target_link_libraries( Game pthread )
	target_link_libraries( Game GL )
	target_link_libraries( Game SDL2 )
	target_link_libraries( Game SDL2_image )
	target_link_libraries( Game SDL2_ttf )
endif()

target_include_directories( Game PRIVATE ${SDL_INCLUDES_DIR} )
target_include_directories( Game PRIVATE ${SDL_IMAGE_INCLUDES_DIR} )
target_include_directories( Game PRIVATE ${SDL_TTF_INCLUDES_DIR} )
target_include_directories( Game PRIVATE PanzerJson/include )
