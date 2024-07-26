# OpenGLRenderingProject

This scene uses custom GLSL shader code for texturing, lighting and point sprites. I have adjusted the GLSL code slighty to incoporate basic texture scaling that can be determined by the programmer and to allow a pass through for mouse coordinates for the lighting of the scene. I am using Sine wave animation to control the movement of the slimes, changing of the particle colour/opacity and handling the particle movement. Lighting is controlled via the mouse position, this can also be changed to get different variants of lighting mode, one where Blue in RGB is locked to 0(1), Locked to the mouse X(2) and locked to the Mouse Y (3) 

User Instructions
These are the following controls for the scene.
Key Action
Left Arrow Key Moves Slime -0.01 in the X
Right Arrow Key Moves Slime +0.01 in the X
Num 1 Changes Lighting mode to 1 (Blue = 0)
Num 2 Changes Lighting mode to 2 (Blue = Mouse X)
Num 3 Changes Lighting mode to 3 (Blue = Mouse Y)
Key ’Q’ Acts as a toggle for static and non static lighting