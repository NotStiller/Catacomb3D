CFLAGS=-g -DNOMUSIC
LDFLAGS=-g -lSDL
SRCS=sp_main.c sp_graph.c sp_audio.c sp_c3data.c c3_act1.c c3_draw.c c3_game.c c3_main.c c3_play.c c3_scale.c c3_state.c c3_trace.c c3_wiz.c id_ca.c id_in.c id_sd.c id_us_1.c id_us_2.c id_vw.c opl.cpp
OBJECTS=$(subst .c,.o,$(subst .cpp,.o,$(SRCS)))

test : cata3d
	cd Cat3D && ../cata3d

debug : cata3d
	cd Cat3D && gdb -ex run --args ../cata3d --nograb

cata3d : $(OBJECTS)
	gcc $(OBJECTS) -lstdc++ -o cata3d $(LDFLAGS)

clean :
	rm -f cata3d *.o *~

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

include .depend

