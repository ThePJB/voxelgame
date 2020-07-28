Video improvements:
 * how to turn off vsync?
 * opts backface culling
 * greedy meshing + ao
 * physics, fps cam (lets do arena shooter speed)
 * console

Light:
 * redo once again, per chunk
 * gamma correction
 * day/night
 * witching hour for lights
 * time of day light colour would be nice
 * make block light warmer nice hack

World gen:
 * world map screen
 * continents etc
 * tweak it, i want plateaus and stuff too and plains, beaches, maybe the minecraft way is the only way with discrete biomes
 * make a pass at geology. have proc gen rock layers and have minerals tend to occur in one of them. oo and angley boys as well maybe
 * maybe halving amplitude and doubling frequency isnt that good idk. whats the equivalent of 1/f?
 * domain warping

World:
 * sky box
 * water (& for lod)

Optimization:
 * im thinking maybe memoized hashmap index could have merit
 * last i checked 16 was optimal chunk size

OK making gen, light, mesh all be on a queue AND making mesh dependent on light FIRST fixed some of the artifacts.
But the light not spreading thing is still happening a bit and thats the LIGHT LOGIC's fault

lighting heuristics based on chunk height. thats my new favourite thing

got to do something about 0,0 generation. is there something it would be cool to have at 0,0?



what if we had a separate pass for downward skylight and sideways skylight
did downward skylight first
dont even use a queue just do it recursively, but add along the way if anyone is blocked, to the queue thing. (if its blocked = query the highestmap)

that could be more correct and faster

so up next i could put you on a really big round island and have that hopefully be visible from the world
also primitive water please

never forget that the key has to be the first thing

Code cleanup:
 * rename chunk_manager to world
 * there was something else as well
 * chunk folder to world folder maybe
 * no xyz convention
 * defactor where its stupid. drawing should just be one big long thing.
 * some kind of unified settings and constants storage could be good
can probably seriously optimize world gen by having a priority queue and, using the height heuristic, downrank chunks that arent near surface level by a lot

-----------------------------------------------------

also why does reticle still squish stupidly? it could relate to the smelly way that window and stuff is glued together

you know if block defs were in block.h that would be sensible
maybe can static it

also texture atlas is still handled pretty retardedly

idk if graphics context is that necessary

maybe for tweaking worldgen the amplitudes are easier than the frequencies


else if its 0..180 its day - clear blue sky
else if its 180+theta_s .. 360 - theta_s its night - dark sky
else if its below 270 its morning twilight
else its evening twilight

anyway morning and evening twilight are the same

so for base sky colour maybe just lerp
but also make some gradient sunlight thing


might chuck in the sun
so the uniform to the shader will be solar angle
intensity is like lerp night sky to day sky


sky: procedural sun,moon,stars,sky,sunset just for a bit of a laff?


cool this light stuff is basically working. I should also change skylight effect on blocks. shader needs to take block light and sky light and then uniform dayness
and level is max(block_light, dayness * skylight)


oh have planetary motion and eclipses and shit that would be fucking awesome!
