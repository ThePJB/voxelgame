# Voxel Game
its a voxel game
currently in babby's first openGL stage

just got fly camera working.

Next up: 
 * textures 
 * abstract camera to fly camera
 * make fps camera
 * basically can start making not minecraft after that

facing ratio light
easy


warn forgotten return type strikes again need to do something about that.
if we are talking maintenance probably a makefile, folder structure, compiler warnings

lets go chunks

oo managing the meshes will be interesting. maybe we just generate them from scratch, push to GPU and throw them away

like working out the delta wouldnt be impossible

maybe we use a VLA
or just a big buffer and keep recycling it

-----------
Wed 17/6/20

this context thing has gotten really out of hand. might need some revision. Passing around pointer to context is ok I think. Globals can get a bit dirty making them known where they're needed in C.

fixed trump its now texture time

Chunks not working, have done like no testing though like print out how many triangles it meshes etc.

debug info like fps, cam pos etc would be nice

OK lets get started on non cooked meshing
first figure out what the faces correspond to

drawing coordinate system would be good
or just facing angle at least

-----
Fri 19 Jun
- Ok we gotta get freetype going for some debug info
- do chunk things with chunks - freeing the vao to remesh etc
- texture atlas and texturing voxels individually
- basic culling

--------
Sat 20 Jun

Right now calling draw_text is causing it to not draw my chunk
So using glUseProgram(text_shader) causes it

also im worried about the orthographic projection

------------
Sun 21 Jun

Ok so the chunk was 29724 triangles
now it works down to 6000. easy. was just indexing array wrong

next up, some degree of chunk management -- check

also texturing blocks and stuff -- check

so
 - switch to chunk shader
 -  >> do modulo in frag shader      --- mostly this
    oh mod is only needed for greedy meshing
 - bind atlas when drawing chunks

then world gen. bust out the noise
also macro world gen -- think about it and have a big scale map visualizer thing


------------
Tue 23 Jun

world gen time. bust out simplex and get it going
then go macro

some more textures
probably want modes so theres some that look below (eg grass) and some that look just at the one (eg dirt)



proc gen palm tree

how about that domain warping
what about if u did 2d noise and warped dimension 3 by some amount
gradient info would be nice
volcano lines or leylines or ridgelines theres pointy noise. i forget whats it called.


hey the vertex shader could probably generate texture coordinates itself...


ok theres some really random polys, not sure if they are a bad block ID or what.



how much shit can we load



terrace biome would be cool

theres some weird 2x2 and even 3x3 artifacts

food for thought how about an enormous scale voxel game where like 1 block is a house
might want to marching cubes that one

debug info


quick optimizations
- shortcut way below (chunk = full dirt)
- shortcut way above (chunk = full air)
- load a long wide rect
- dont load occluded chunks
- chunk manager time

debug info
- looking at block
- ram and vram usage
   jank but done
   - ram on windows: GetProcessMemoryInfo(GetCurrentProcess()).WorkingSetSize
   - on linux: oof maybe getrusage

   vram usage nvidia-smi

game features
- picking / placing blocks
- entities, procedural trees im so keen!

world features
- vegetation (crossy and squarey)
- noise experimentation, domain warping etc. read up on types of noise

graphical features
- AO
- light level

assets
- flowers, cattails, wood etc

janitorial
- todos fix cam speed, and cam in general
- todos fix fov
- todos fix the size of vertex data
- todos new seed as well. maybe have a button and regen one
- the whole context thing. if its always a ptr then stuff doesnt shit itself because it doesnt need to know
just have a function that allocates it

enum orientations? or just vec2i yeah

------------
So RAM and VRAM usage is sitting pretty at 3.1GB
with 10*10*10 of chunks so 1000 * 16^3 
4 million blocks
812 bytes per block? thats munted
I wonder if im leaking some shit. lets see what happens if i remake the chunk

generating time would be a good one as well



---- 
ok circular dependencies with context struct are no good
probably need to break it up

one root problem is the way inputs getting handled. maybe its command pattern time.

can I even separate input and graphics?

input depends on game state
input depends on graphics
graphics doesnt depend on game state
does graphics depend on input? it kind of needs the callbacks

      window
      ^     ^
   input   graphics

input is a bit vague there

so when u make the graphics u could pass in the callbacks and any other info

the callbacks need access I think to the graphics to update it yeesh.

maybe start by separatign out stuff that is strictly rendering and doesnt depend on anything

Im being tested by this event queue shit. I feel like theres a simpler way to do things. Maybe. I want things to be synchronous so not even really a queue, more of an observer situation. Runtime dependencies instead of compile time dependencies.

That could be OK, it would basically be like a ptr to a function that takes a void ptr. Things register for the callback say when key press happens, or not.

Basically its just a way of pythoning it up. That might be a smell. The compiler is my friend. I think.

What if instead I can tidy things up and its not a problem. should probably do that regardless.

or just use globals. but then it needs to know the types and thats a pain...



--------

so how are we gonna do picking / placing

cast ray up to view distance and test cubes for opaqueness

OK block picking is weird. why does it end up with a z coordinate of 18447635409834095834

get and set block not quite right. I sometimes get - blocks.
I think it rounds differently when negative from memory.

sometimes end up in block 16 instead of block 15, wat

get block set block seem correct except for climbing ram usage lol


in future maybe the concept of a "chunk slot", vbo and vao could belong to that