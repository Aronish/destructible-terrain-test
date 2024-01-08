# Scalar Density Field Polygonizer Tests

This repository contains testing of various testing of polygonizing a density field.

## Marching cubes
I started with marching cubes and got destructible 2D terrain working. This was eventually done through compute shaders to speed up the process.

![World](/doc/world.png)
Experimenting with different density functions using noise layers. (Fog to hide chunk edges, triplanar mapping for texturing).
![Sculpting](/doc/sculpt.png)
Some carved caves and sculpted overhangs.

## Dual Contouring
The problem with dual contouring is that it does not preserve sharp features. Dual contouring is supposed to fix that, but I never got it to work properly before moving on from this endeavour.

## PhysX

At some point, I implemented PhysX for mesh colliders and player controllers. This worked beautifully, but was never merged with the destructible meshes. Another thing I noticed was that my own raycasting compute shader for sculpting was something like 10x faster than the one built into PhysX :D.

---
This was only a brief overview, a lot of neat stuff can be dug out from old commits.