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

next up, some degree of chunk management
also texturing blocks and stuff