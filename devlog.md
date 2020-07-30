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

--------------------

alright im doing the decoration pass and trees and shit

first just put it in generate

then move to decorate and make it its own queue and gate it behind neighbours loaded

ok yeah so worst case for the light is insane
and cumulitively its a lot as well
and lots of time wasted remeshing even though it doesnt actually take very long on its own

i think part of light slowness is flood filling everywhere
maybe the slowness of all the lookups takes the back seat

so theres the idea of computing sky light first using the surface height map
and then you have to figure out which neighbours to push
maybe you could see if neighbours have a thing over themand if they do, push their chunk for updating/ thats clever

maybe want to do one pass for direct sky light and then a pass to clean it up

skip lighting if the whole chunk was air

ok so I think I found a way way better way of doing the sunlight queue thing
still improvements like extra pass and skip air chunks but this is a nice start
anyway I think sunlight propagation is broken because it just doesnt work
ok actually its fine

old
cum gen time: 1.358404
cum light time: 7.285655
cum mesh time: 2.974892
max gen time: 0.031502
max light time: 0.149932
max mesh time: 0.005387

new
cum gen time: 5.691829
cum light time: 1.900093
cum mesh time: 7.718285
max gen time: 0.053209
max light time: 0.029724
max mesh time: 0.007667

incredible

still a couple of problems like chunks not updating their light when they load in next to a lit chunk (they need to look, or not load til neighbours maybe... probably former is better)

and one time i found a random dark patch on the ground, not sure whats up with that but anyway

but yeah its 1000x better now and 5x faster. so good


todo next time make trees not terrible: pink noise & chunk crossing & not loading and meshing chunks for no reason please and thank you
maybe pine trees on snow :)