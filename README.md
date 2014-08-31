Catacomb 3-D: The Descent SDL port
----------------------------------

License
-------
This repository contains the source code for the SDL port of Catacomb 3-D (also known as
Catacombs 3 or Catacomb 3-D: A New Dimension). 

It is released under the GNU GPLv2. Please see COPYING for license details.

This release does not affect the licensing for the game data files. You will
need to legally acquire the game data in order to use the exe built from this
source code.


Compilation and running
-------------------------

I compiled the game for a x86 system with gcc version 4.7.2 (Debian 4.7.2-5) and SDL 1.2.15. There is an exemplary makefile that should do the trick.

The source port SHOULD work for all platforms, but frankly, I haven't tested it with 64bit or any other compiler. Packing issues in other parts than the sound system might emerge. If it does, please contact me at @NotStiller.

If you want sound output, you have to download the following two files:
http://sourceforge.net/p/dosbox/code-0/3812/tree/dosbox/trunk/src/hardware/opl.cpp
http://sourceforge.net/p/dosbox/code-0/3812/tree/dosbox/trunk/src/hardware/opl.h

After you download them, get rid of opl.c and compile the new files with gcc -c opl.cpp -o opl.o.
I did not include them here, because I was unsure of how to combine LGPL and GPL licensed software. 

The game works great with the GOG release (http://www.gog.com/game/catacombs_pack).


Gameplay instructions
---------------------

Pressing 'm' will grab the mouse, pressing 'm' again will release it. While mouse grab is active strafing is always on and vertical mouse movement is ignored. The rest is well documented in the game/manual.

Overall the game is surprisingly fun !


General notes
-----------

There is a lot to be done. Provided that there is any demand for it, I will also convert the later Catacombs games. But for now, I just want to get this thing out.

I have not fixed the aspect ratio. That was never on my radar, before @fabynou did it with Wolfenstein.

If you have any comments or bugs to report, please do so via @NotStiller (twitter.com/NotStiller) or github.com/NotStiller. I'd love to hear from you !


Porting issues (aka the fun part)
---------------------------------

The most frequent difficulty lay in adjusting all the types to their 16bit counterparts. Any unsigned for example was supposed to read unsigned short (for 32bit targets, hopefully that works also for 64bit targets.) This yielded all kinds of arithmetic and packing bugs.

Closely related to this were some packing issues where a struct consisting of a long and a short was considered to be 8 bytes long by gcc, whereas in the data files it is packed as 6 bytes. I fixed this in the new sound code, but it might show up again anywhere. If your compiler supports packing and alignment options, try to make it pack everything tightly.

A further big problem was that the programmers made liberal use of direct memory access, sometimes with the segment address set implicitly in a different place. I had forgotten almost all about the DOS memory model, for example that A000:0010 and A001:0000 are the same adresses, so this gave me quite a few puzzling moments. Overall, it wasn't too much of a problem, because most of the time the memory accesses would happen in assembly code, which I replaced with newly written C code.

Related to the former issue was the question of how to handle the EGA output. I was very indecisive whether to cut it all out and replace it with SDL functions or to try to emulate the EGA functionality via macros and a huge buffer, which would hold all the planes. As you will see, I largely went with the former option, even though I have tried to change the original files as little as possible. From a perfomance point of view, this solution is crap, because I do the planar-to-linear-memory conversion each time I draw it. I could cache it as in the sound code, but for now I find this solution the most authentic.

I hardly touched the rendering code and the new code is basically a hack, so it might be a little bit off. But it looks and plays great.

The module that gave me the most trouble was the sound code. At first I tried to run the OPL emulator (from DOSBOX source, originally by Ken Silverman) on-the-fly in the sound callback. That would have allowed me to leave the sound code in peace and simply replace the alOut function in id_sd.c. This didn't work. At first I thought that was because the id soundcode works at 560 (8*70) Hz whereas the SDL sound callback is called about 172 times per second (at most!). I tried to get it work, but I couldn't and switched to the method I use now. Later I learned that a) loading of the sounds was incorrect, no wonder it sounded wrong and b) my PC is too slow to emulate the AdLib on-the-fly anyway. The way I do it now is simply to prerender the sounds into buffers and then to fill the buffer in the callback with the currently placing music and the currently playing sample (if any). I didn't increase the number of channels, because if you run into a wall, for instance, you would get tens or even hundreds of the same sample playing almost at the same time, blasting your ears off in a huge cacophony. I guess you could keep track which sounds are played on a per sample basis, but I didn't mind. This is more authentic.

A lot of the further modules were removed or severely crippled. This includes the memory manager which could handle lower, expanded and extended memory. The architecture of this thing was fun to study, but keeping it in order would have been more work than simply cutting it out. The same goes for most of the graphics backend, including the screen panning and region updating, that was such an important step forward at the time. It wasn't used in Catacombs 3D and hence implementing it would have been a distraction.

As for the general architecture, I decided to do all the SDL specific stuff in a new thread. The original code was based on using interrupts (a concept which is of course related to multithreading, but not the same) to magically set variables like last key pressed etc. so that whereever keyboard input is accessed, it does not call a function but merely accesses a variable. I could have touched that code and replaced the variables with functions, but I wanted to keep the original code. The multithreading approach works fine in that respect.
Also I didn't mind to carefully wrap everything with mutexes. So race conditions might be existant. Unfortunately, there is still sometimes a crash at exit, which might have to do with his.


Bugs
----

- Rendering bug in Skull and Bones. Just make the ball touch the right side.

- Crash at exit. I don't know why.

- Sometimes ray tracing goes out of map, I don't know if I introduced that or if it always was there. I put an assert there.


