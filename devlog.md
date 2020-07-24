
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

----

why does sunlight propagate down but not sideways?


---------

how lighting works:
    - update: propagate light outward from the source, 4con, with intensity of source -1

    - update that might reduce light: delete all light that could have been from this thing, then update from the edges and any light sources within


opaque blocks always have light 0.
placing a block is equivalent to deleting a light source.
deleting a block is a bit like deleting a light source. you set to 0 in case the block emitted.

i could also make 4 run "fix light" on the chunk


----
ok current problem, you delete something, it doesnt update propagation queue

ok thats all G.

can there just be a general light update that does a deletey update? its not much to check really
actually i think light add is a bit different to light delete. light delete could be light update.

i think there might be a sticky extra case with deleting as there is with adding.

deleting you want to remember the brightness to delete. if its not the first one you also want to re add though
ok that didnt have the effect i thought


OK thats nice, I improved the block deleting case and simplified code a bit.

Skylight propagates correctly as you delete blocks btw. and says it is propagating sideways in doing chunk deleteys.

maybe its getting zeroed in the fix thing

fix that before moving onto the larger artifacts in the world.

and then some relaxing terrain generation.


ok fix nice

its really gonna need some unit tests though its pretty unreliable

---------------------------
current state of lighting:
    - inconsistent
    - chunk load order dependent I think
    - with lighting we can use a heuristic to see involving the xy height
    - code could be cleaned up a bit too

    noise abstractions dont really work, they needa go


    so with lighting we are deleting properly im pretty sure
    i broke world gen lol


    trying toget surface of unloaded chunk a lot

    open simplex noise is a bit gay to use. why is everything 0


    for some reason it was the vec thing

    im gonna make a superflat world gen option now, world gen func fn ptr, and look at light. its also really sluggish. its doing a lot of unnecessary work with the light updates

    maybe i need a "distance to unloaded" heuristic or something to sort the chunk lighting


I think world gen stuff worked so far, the frequency coefficient
it needs an abstraction and also running like shit



sooo sunlight propagation must be too aggressive
or maybe deletion doesnt do it properly when push comes to shove



ok so not deleting the right stuff with sunlight. its a bit weird
so if the overhang is placed presumably the 16 below is deleted.  and that should then delete the 15 next to it right
instead it doesnt, it only deletes down

lol condition was wrong

ok then i think its mostly fixed except cooked loading order stuff



----------
Tue 21 Jul
 
 So lighting is kind of getting there
 I need to improve performance and fix edgy bugs
 
 Information: time world generation vs. lighting calculations vs. meshing

 One option would be to have a priority queue of jobs

 Performance:
    - Sampling noise is slow / gets done heaps.
        soln: lower res ones that are done once per chunk (caveyness, temp, etc) and maybe lerp. maybe. it means noise octaves are pricey as well.

    - Lots of time spent accessing hash maps. so probably if there was fast path for the light that would be nice


im gonna try profiling flat world to see why the lighting is so bad

maybe i could memoize the chunk idx or something

todo profile, test flags for main

so loosely speaking chunk generation takes 5ms per chunk, lighting 1.5ms per chunk, meshing 1.4ms per chunk in the benchmarks. 


oh I just realised that in these benchmarks im still neighbour handshaking. gonna have to do something about that. that could be the root of the slowness, maybe generating is fast


also quite curious to see if bigger chunks is better

ok without the handshake its generation 3-4ms lighting 2ms

with bigger chunks its 25 ms per chunk
and 36 ms per chunk
and 5ms per chunk

/8 = 3ms, 4.5ms 0.somethingms

yeah so im happy saying 16^3 chunks rn cause 32 isnt faster

maybe we need all light then all mesh


OK making gen, light, mesh all be on a queue AND making mesh dependent on light FIRST fixed some of the artifacts.

But the light not spreading thing is still happening a bit and thats the LIGHT LOGIC's fault

theres an additional problem which is that there are artifacts where the chunks bordered once nothing. could maybe remesh the edge chunks when a new one is loaded in, or just better guess at the ligh level (guess full sky bright)


maybe unwrapper with a default if not ok?
uint8_t x = maybe_fallback(light_get_blah, SKY_FULL);
#define maybe_fallback(X, Y) X.ok ? X.value, Y
it requires computing X twice like with maybe_panic not sure about getting around it



could smarten up the no adjacent chunk meshing heuristic (NACMH) to check the surface height map (SHM) so that i dont now get lighting artifacts down in caves.


ok yeah so now the performance is a lot better, theres a loady part at the start
artifacts are reduced
can look at world gen a bit now



so the same piece of tech for the world map screen could also lod up very distant terrain. fuck that would be awesome.

have, say, a "lodchunk" where say the 2d heightmap is sampled at each corner only and used to make polys

would need a way to guess colour too, say blocks have a colour

i think therell have to be some abstracting of all the 2d height map stuff and also a bit of cheeky undersampling and interpolating where possible


do i even want to think about hiding occluded chunks? sweet christ not really
i guess it wouldnt be that hard

------------------

do lodchunks and see how that looks
also can be used for world map

could speed worldgen up a lot if I used the same X and Y values for height (have to do something different for cliffs tho)

is there something it would be cool to have at 0,0?

maybe im meant to be timesing by some kind of amplitude coefficient

ok performance is improved a bit


need to get the scale just right. its definitely too big at small scales
and now ive got cave openings being blocked too


maybe 1/2 ratio is wasteful and should try 1/3 or 1/4
ah freq was just way too low

got to do something about light issue remesh during generation
it just spills into neighbouring chunks and its a waste of time. maybe have a flag for light calculated yet?

but it shouldnt be that much of a problem if its pre lit by the down light calculation

what if we had a separate pass for downward skylight and sideways skylight
did downward skylight first
dont even use a queue just do it recursively, but add along the way if anyone is blocked, to the queue thing. (if its blocked = query the highestmap)

that could be more correct and faster

-------

so up next
- lod chunks(int nsquares_per_side) -> mesh. 1 (1), 2(4), 3(9)
- or better lighting
- also fix up world gen some more. try and get me smooooth on

-------------------

lodmesh todo list:
    - unloading
    - o i also keep vertex data in ram for some reason lol
    - how not to draw over terrain
    - beautification (like colour flattening out over distance like mountains do turning blue)



so up next i could put you on a really big round island and have that hopefully be visible from the world
try and get the scale right kinda, and see how far draw distance can be pushed
and maybe the chunks for the lod need to be bigger

uniform pos and fade to blue
trying to do that and one corner is quite blue, i wonder if im putting the wrong value in the uniform or if somethings wrong in the shader


ok lod update is always loading every single chunk lol so 

never forget that the key has to be the first thing

the problem with the blue is funny its around 0,0,0 of course. im sus that the uniform isnt getting uploaded properly.
anyway thats enough for today.

todo tomoz, fix the blue thing, tweak terrain into a better shape
(i think atm the amplitude scaler needs to be more of a low pass filter i.e ignore some low frequencies because it just pulls them down to 0)

world gen performance continues to be a bit disappointing. we'll see. maybe fast path light. gating 3d noise fn behind 2d noise is a good idea also

im pretty close to be able to do world map screen and continent generation, however thats done. like idea before, make it an island at sea
water, animated!

janitor stuff:

    rename chunk_manager to world

    there was something else as well
    chunk folder to world folder maybe
    no xyz convention

    yeah gotta put these functions and shit into sensible folders

    generation

    