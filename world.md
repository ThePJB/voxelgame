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