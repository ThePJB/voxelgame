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
- todos fix cam speed
- todos fix fov
- todos fix the size of vertex data
- todos new seed as well. maybe have a button and regen one

enum orientations? or just vec2i yeah