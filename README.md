# Voxel Game
## Dependencies
 * freetype
   * `apt install libfreetype6-dev`
 * glfw
   * `apt install libglfw3-dev`
 * cglm
   * `git clone https://github.com/recp/cglm.git`
   * `make`
   * `make install`
 * opengl

## Controls
 * Change block: Q,E
 * place block: left click
 * mine block: right click
 * wireframe: 1
 * debug info: 2
 * regen world: 3
 * reload chunk: 4
 * move camera: wasd
 * sprint: left control

# Planned features (out of date disregard)
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
