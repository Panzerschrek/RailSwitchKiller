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
endif()

#
# HexLib
#

set( SOURCES
	src/main.cpp
	)

set( HEADERS
	 )
	
add_executable( Game ${SOURCES} ${HEADERS} )

if( WIN32 )
	target_link_libraries( Game opengl32 )
	target_link_libraries( Game ${SDL_LIBS_DIR_ABSOLUTE}/SDL2main.lib )
	target_link_libraries( Game ${SDL_LIBS_DIR_ABSOLUTE}/SDL2.lib )
else()
	target_link_libraries( Game pthread )
	target_link_libraries( Game GL )
	target_link_libraries( Game SDL2 )
endif()
