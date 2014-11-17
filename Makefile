CFLAGS=-g #-DNOMUSIC
CPPFLAGS=-g #-DNOMUSIC
LDFLAGS=-g -lSDL
SRCS=sp_main.c sp_graph.c sp_audio.c sp_data.c c_game.c c_draw.c c_trace.c id_in.c id_sd.c id_us_1.c id_us_2.c id_vw.c opl.cpp
SRCS_C3=sp_data_c3.c c3_act1.c c3_draw.c c3_game.c c3_main.c c3_play.c c3_state.c c3_wiz.c
SRCS_C4=sp_data_c4.c c4_act1.c c4_draw.c c4_game.c c4_main.c c4_play.c c4_state.c c4_wiz.c gelib.c c4_win.c
OBJECTS=$(subst .c,.o,$(subst .cpp,.o,$(SRCS)))
OBJECTS_C3=$(subst .c,.o,$(subst .cpp,.o,$(SRCS_C3)))
OBJECTS_C4=$(subst .c,.o,$(subst .cpp,.o,$(SRCS_C4)))

test : cata3d cata4


testc4 : cata4
	cd Abyss && ../cata4

debugc4 : cata4
	cd Abyss && gdb -ex run --args ../cata4 --nograb

cata4 : $(OBJECTS) $(OBJECTS_C4)
	gcc $(OBJECTS) $(OBJECTS_C4) -lstdc++ -o cata4 $(LDFLAGS)


executables : cata3d cata4 

testc3 : cata3d
	cd Cat3D && ../cata3d


debugc3 : cata3d
	cd Cat3D && gdb -ex run --args ../cata3d --nograb

cata3d : $(OBJECTS) $(OBJECTS_C3)
	gcc $(OBJECTS) $(OBJECTS_C3) -lstdc++ -o cata3d $(LDFLAGS)



clean :
	rm -f cata3d *.o *~


.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

include .depend

