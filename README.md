# Voxel Game

- FPS camera
- context refactor? it might be ok atm
- texturing modes: everything is minimum or ones where its just one
   - and other shit like crops

- different blocks modes like walk thru or not, transparency
- chunk paging, loading visible etc.

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


food for thought how about an enormous scale voxel game where like 1 block is a house
might want to marching cubes that one



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




back to the structure, maybe FP's can still access globals and shit and its not a closure per se
that could make them way more useful. Just have ptrs to the things in main or whatever scope they are defined in


---------------
Thu 25 Jun

Todos:
- Offsets are fixed, still got to work out the "cant place in certain orientations" bug
   fixed. directions < 1, cast to int, then not > 0. lol.
   automatic coercion so bad.


- time to make some textures


- placing distance
- ghost blocks for placing
- chunk slots and  all air, all dirt optimizations

greedy meshing + AO would be another big one

fizix

console