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

Textures not working but texture coords are fine, so not sure whats up there. where is le trump :(

Chunks not working, have done like no testing though like print out how many triangles it meshes etc.
