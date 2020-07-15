
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