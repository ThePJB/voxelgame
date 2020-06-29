# Chunk Manager
Top level thing

# Chunk Slot
This represents a slot for a chunk. It corresponds to long lived things like VBOs
Chunk manager has a fixed number of these (basically depending on ur render distance)
Im thinking just an array b/c there isnt that many of them

chunk manager needs to dynamically load and unload chunks
chunks should be able to be unloaded to ram (basically using ram but not vram) synchronously
and from there asynchronously paged to disk

would you need like another 3d array to spatially represent for each chunk if its loaded or unloaded. Would that mean checking for each chunk with each thing in range to see if its loaded? I guess its not impossible to do it that way

3d array though as well. maybe a hashmap of coordinates to status isnt such a bad idea

***maybe client could just issue a request directly when they cross a chunk boundary and notify that others can be unloaded. you know thats simpler***

# Chunk
Maybe we can restrict this to basically the level information therein, so they dont worry about loading and unloading essentially




# Generation Implementation
- find the right set of abstractions


e.g. for biomes:
top block
next 4 blocks
rest

------
Fri 26 Jun

Lets try working out when the player has crossed a chunk boundary
ok that was easy enough. now we can load and unload chunks
for now dont worry about the disk just delete them

if we keep track of slot indexes we can just delete that one and make the new one like a 3d treadmill

Sun 28 Jun
----------

World treadmill time

walk: unload back and load front

vs.

update: figger out and load

update necessary for teleportation, load save etc


could maybe store chunks in some kind of contiguous thing instead of having to do a linear search of chunks. but linear search of chunks might be FA idk
does locality matter much at chunk scales? 4kb?

could make chunks wider too as a way to bias it toward loading wide


Mon 29 Jun
----------
with empty air chunks
vram 1809
ram  1168

without empty air chunks
maybe like 500mb of extra ram

a lot of it was the vertex buffer

picking broke again oof
probably should set up some kind of automated test with like full on inputs and stuff

ok this is pretty weird. So there are sometimes holes in the mesh (but not always from the start) at chunk pos 15
so it like either thinks its occluded or thinks the block there is empty. maybe empty.
And also picking there doesnt seem to work but maybe it does

picking messages could be better
and could refactor a bit probably and things would straighten out.