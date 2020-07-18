
-----------------
Thu 2 Jul Signoff


Up next:
 * how to turn off vsync?
 * world treadmill: replace chunks as u move through the world
 * 3d perlin worldgen
 * opts backface culling
 * greedy meshing + ao
 * physics, fps cam (lets do arena shooter speed)
 * console




Light todos:
------------
 - get the infinity other cases working
  - remove light
  - place/remove block
  - skylight
 - gamma it so theres a nice long and smoooth falloff like IRL
 - its really a pest that chunks are interdependent

 then do skylights maybe

 then do removing lights

 then do AO and greedy meshes



 hey in t he witching hour can we like divide all light levels by 4 or something
 could probably do that in shader with a uniform tbh

Wed 15 Jul
----------
Block lighting seems to entirely work, up next is sky lighting

then AO and greedy meshing

or maybe world map screen if i want a break from this tricky stuff




OK im making it so that the generate chunk function also initializes light.

make lighting touch the chunk below and so forth

int64 tick when lighting last changed?

i could just remove empty chunks, the ram usage is really not a problem any more
and its a real pest of a case

block removal sort of works, as for block placement? not sure



when I replace / CHUNK_RADIX with >> 4 I wonder how much of a speedup that will be


world gen ideas. make a pass at geology. have proc gen rock layers and have minerals tend to occur in one of them. oo and angley boys as well maybe


block placement reduces it by 1 for some reason

sky, day and night will be nice

gamma correction of course
time of day light colour would be nice
shader is going to need both light thingies passed in
make all player light warmer as a bit of a cheeky hack
smooth lighting

its also pretty unstable

not sure how to sort out lighting when chunk loads either. maybe copy the code but make it stop at the chunk boundary? hmm idk. maybe just issuing the update is fine because it will stop eventually

bit burnt out on lighting hey, maybe take a break and work on something cool

some other good things are happening at the same time

probably a good idea to have a data structure that stores the highest opaque block in the world at each pos
and use that for the lighting

refacting debug overlay
then refactoring getblock setblock gety sety getlight setlight etc
should have some kind of global settings thing as well
so there isnt magic numbers like "pick distance" etc


ok very confused about the xy surface setting thing.
it sets just fine in my test thing
ingame it says its got the right values but when it set it its just 12

so there has to be something else different right?

so I think I know what the problem is. it samples a chunk and gets 255 for its sky light value and propagates that down lol
so go be more careful with those maybes hey. and can just do if its higher than highest opaque set its brightness to SKY_BRIGHTNESS


yo world gen: multiply noise for hillier areas and flatter areas
and likewise with caves maybe rather than do the threshold thing im doing

multiplying vs changing the cutoff interesting

and obivously domain warping


with lighting, do I just want one function that updates lights?
and be very sure of how it works.

cases: this is > 1 higher: spread to that neighbour
this is > 1 lower: delete this and propagate from neighbour

if you delete a light source you better add it to propagation queue

for fixing these light holes we need to propagate down and sideways

and ultimately probably unit test it