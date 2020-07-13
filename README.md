# Voxel Game
## Dependencies
Just the openGL stuff and freetype I'm pretty sure

# todos
## Optimization
- Greedy meshing
   - to do texturing: need to do modulo of tex coords in frag shader
   - AO at the same time its a similar calculation

- How much of this data do i actually need to be sending to the GPU, for instance can normals be calced, texture coordinates, can it all just be done in a mesh shader from the raw blocks? nah gotta mesh
- all 1 type chunks

## Micro world gen
- how about cliffs and caves. maybe use a small amount of 3d?
- domain warping?
- ridge noise
- how to do trees? minecraft style or not. ultimately like to not and have some cool L system shit. L system coconut palm tree. oo climbing up it

### Features
- volcano
- terraces
- flat with mounds

## Macro world gen
- maybe diamond square? or some continenty thing or grammar thing... idk
   world map visualizer, make a mesh and have it spinning and stuff. like kart racer character select
- gradient info for erosion, fluid calcs

## Assets
- snow and dirt to spruce up world gen
- wood

## debug info
- looking at block



# Disregard this its just my scratchpad

-----------------
Thu 2 Jul Signoff

 - fixed reticle
   - in the process got testing infrastructure, debuggin infrastructure, 2d image drawing

 - fixed empty chunks not being upgraded to real chunks
   - should we be able to downgrade? I doubt it, who cares
   - though not sure if the optimization does anything as ram usage doesnt go up when we build into a new once empty chunk

 - fixed ram usage
   - now world can be way bigger. greedy meshing will still be a huge improvement

 - block highlighting
   - rn its wireframe, could be a bit prettier, can u make line thicker
   - yes but it still looks shit lol. maybe alpha and solid colour

 - refactoring
   - graphics - input split
   - chunk - world split

Up next:
 * how to turn off vsync?
 * world treadmill: replace chunks as u move through the world
 * 3d perlin worldgen
 * opts backface culling
 * greedy meshing + ao
 * physics, fps cam (lets do arena shooter speed)
 * console





 -----
 it mostly works but if u go really far it leaks memory for some reason. maybe if u free before the chunk is loaded? though I wouldn't think 

 o its double add

 todo: load list could be like a hashset so it wouldnt waste heaps of time adding a million chunks when u zoom around

 could resize the orthographic matrix for 2d



 ---

 how to do worldgen
 "carvers" for cliffs, caves, canyons
 carve certain octaves

 -------
 maybe for noise have vectors of freq and vectors of amplitude -> vectors of effect at scale
 then u can scoop them or whatever else for cliffs and such

 yooo caves are sick

 my idea of frequency might be reversed
 that could also mean im diminishing frequency instead of increasing it

 you could have the cave coefficient change with depth obviously to encourage more cave action


 Sat 11 Jul
 ----------
 Caves and cliffs are in. its cool
 shading so i can actually appreciate my beautiful creation
  first just arbitrary shadding, like add the vertex attribute and just set it arbitrarily
 block vartiance (dirt pockets etc.)
 domain warping?
 id better try and add some more block variance
 vegetation
 potential optimization, RLE chunks in ram? worthwhile? well then length would be quite variable. well chunks are individually allocated after all. but you would have to serial access them. maybe that wouldnt matter so much. most time spent doing On stuff like meshing
 multithreaded world loading would be nice

 O ao is gonna depend on the neighbouring chunks as well


 So heres what im gonna do.
 Add vertex attribs for shading amt and put some random values in
 add lighting map to chunks (maybe use a byte for it)


 load chunks proportional to urgency, and maybe preload at lower urgency?

 ok so the light belongs to the opaque block

 bfs uses a queue, maybe use ring buffer
 might have to roll one made from stb ds

block emission table
block opacity


janitor work:
  get rid of block struct
  get rid of chunk slot
  separate world, chunk manager

-----
thoughts on light

actually lets just do it at the world level of abstraction hey. for block light it seems straightforward ish.

queues could just be a macro tbh

-----
man my frame rate is not too good.

so light is kinda working. 
something wrong with queues where at 384? it has a length suddenly of -3713
and glitches at chunk borders which is fair enough, maybe the side that isnt loaded yet
but also when u place its dark

Light todos:
------------
 - get the infinity other cases working
  - remove light
  - place/remove block
  - skylight
 - gamma it so theres a nice long and smoooth falloff like IRL
 - its really a pest that chunks are interdependent

 only allow meshing after neighbours are loaded?
 it seems that telling the other guy that u were loaded doesnt apply

 alright idk why one of the chunks is trying to mesh but anyway
 maybe it was the last to load out of its group though that seems unlikely

 my 4con value is trash. i dont really know why. i think up next ill do a big refactor
 and put things in files like meshing, lighting, etc
 so i can find stuff easier, and pick some better names. itll save time in the long run.

 ----

 Signoff Sun 12 jul
 ------------------
 maybe have a read through the code to find gross stuff

 unify function signature style, cm_do_stuff(cm, x, y, z)
 improve naming

 up next get rid of chunk slot and maybe make key the chunk's position

 maybe get rid of block struct too if im feeling spiteful

and do something about the horrid if (each direction) pattern im using atm

 then debug 4con so i can debug meshing

 then do skylights maybe

 then do removing lights

 then do AO and 

 
 
 I hope hmputs works

 yesss finally fixed all the bugs i introduced so far. Tomorrow I proceed to implement the rest of the lighting stuff.

 ok its not actually perfect some chunk boundaries shit, i managed to segfault placing in an empty chunk. i think empty chunk border = white as well



 you know when this lighting is wrong its wrong on one face which kind of suggests its not the lighting algorithm itself. its got to be the mesh or something

 nice well i got the debug info coming out (handy) and my light calcs are fine, its the meshign thats fucked. who knew.