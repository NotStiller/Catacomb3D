#CFLAGS=-Wall -g
CFLAGS= -g
LDFLAGS=-g -lSDL
SRCS=sdl_main.c c3_act1.c c3_draw.c c3_game.c c3_main.c c3_play.c c3_scale.c c3_state.c c3_trace.c c3_wiz.c data.c id_mm.c id_ca.c id_in.c id_sd.c id_us_1.c id_us_2.c id_vw.c opl.c
OBJECTS=$(subst .c,.o,$(SRCS))

cata3d : $(OBJECTS)
	gcc $(OBJECTS) -lstdc++ -o cata3d $(LDFLAGS)

clean :
	rm -f cata3d *.o *~

.depend: $(SRCS)
	rm -f ./.depend
	$(CXX) $(CPPFLAGS) -MM $^>>./.depend;

include .depend

