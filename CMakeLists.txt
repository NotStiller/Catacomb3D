cmake_minimum_required (VERSION 2.6)


include(FindPkgConfig)


project(CataEngine)

find_package(SDL REQUIRED)
include_directories(${SDL_INCLUDE_DIR})

add_library(CataEngine 
id_us_1.c
id_us_2.c
id_vw.c
opl.cpp
sp_audio.c
sp_data.c
sp_graph.c
sp_input.c
sp_main.c
c_draw.c
c_game.c
c_trace.c
c_win.c
)

project (cata3)

add_executable(cata3
c3_act1.c
c3_draw.c
c3_game.c
c3_data.c
c3_main.c
c3_play.c
c3_state.c
c3_wiz.c
)
target_link_libraries(cata3 ${SDL_LIBRARY} CataEngine)

project (cata4)

add_executable(cata4 
c4_act1.c
c4_draw.c
c4_game.c
c4_main.c
c4_play.c
c4_state.c
c4_wiz.c
c4_data.c
gelib.c)
target_link_libraries(cata4 ${SDL_LIBRARY} CataEngine)


project (objextract)

add_executable(objextract objextract.c)



