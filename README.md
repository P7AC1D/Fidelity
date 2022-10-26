# Fidelity Engine
Fidelity is a real-time 3D rendering engine built in OpenGL 4.1 and written for Windows and OSX. It will probably build for Linux but I have not attempted this yet.
The goal of this project is to implement an easy to use rendering engine to learn the latest graphics rendering techniques and optimization.
It is still under heavy development so expect bugs and performance issues.

## Features
- Physically Based Deferred Rendering Pipeline
- Cascaded Shadow Maps for Directional Lights
- Soft Shadows using Poisson disc sampling and PCF
- Screen space ambient occlusion
- HDR with Physically Based Bloom
- Scenegraph with Frustrum Culling
- Editor UI for Debugging

## Screenshots
![](./Resources/Screenshots/sponza_pbr_1.png)
![](./Resources/Screenshots/sponza_pbr_2.png)

## Future Work
- [x] Cascaded Shadow Maps for Directional Lights
- [x] Soft Shadows using Poisson disc sampling and PCF
- [x] Screen space ambient occlusion
- [x] HDR Tone Mapping
- [x] Physically based rendering
- [x] Bloom
- [ ] Image Based Lighting
- [ ] Point Light Shadow Maps
- [ ] Screen space reflection
- [ ] Motion Blur
- [ ] Depth of Field
- [ ] FXAA
- [ ] Clustered Shading 


## References
#### General
* https://learnopengl.com/
* https://blogs.igalia.com/itoral/2018/04/17/frame-analysis-of-a-rendering-of-the-sponza-model/
* https://github.com/lettier/3d-game-shaders-for-beginners
#### Frustrum Culling
* http://www.lighthouse3d.com/tutorials/view-frustum-culling/
#### Shadow Mapping
* https://alextardif.com/shadowmapping.html
* https://therealmjp.github.io/posts/shadow-maps/
* https://learn.microsoft.com/en-us/windows/win32/dxtecharts/common-techniques-to-improve-shadow-depth-maps
#### Screen Space Ambient Occlusion
* http://john-chapman-graphics.blogspot.com/2013/01/ssao-tutorial.html
#### Physically Based Shading
* https://blog.selfshadow.com/publications/s2013-shading-course/karis/s2013_pbs_epic_notes_v2.pdf
* https://blog.selfshadow.com/publications/s2013-shading-course/hoffman/s2013_pbs_physics_math_notes.pdf
* https://web.archive.org/web/20060818101028/http://harkal.sylphis3d.com/2006/05/20/how-to-do-good-bloom-for-hdr-rendering
#### Clustered Shading
* http://www.aortiz.me/2018/12/21/CG.html
