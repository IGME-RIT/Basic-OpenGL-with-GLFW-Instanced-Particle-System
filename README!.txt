Documentation Author: Niko Procopi 2019

This tutorial was designed for Visual Studio 2017 / 2019
If the solution does not compile, retarget the solution
to a different version of the Windows SDK. If you do not
have any version of the Windows SDK, it can be installed
from the Visual Studio Installer Tool

Welcome to the Instanced Render Particles Tutorial!
Prerequesites: 
	2D Particles, 
	Instanced Rendering,
	Bufferless Rendering

Just like the previous tutorial, particleSystem.cpp
updates each particle indiivdually on the CPU: 
updating its position, velocity, age, and rotation, etc.
This tutorial uses instanced rendering to draw all particles
with one draw command. Instanced Rendering only requires
a Vertex Shader and a Fragment Shader.

Every particle will be a quad; rather than making a vertex
buffer full of quads, we can procedurally generate quads
in the Vertex Shader, just like previous tutorials.

We need all of the particles to be accessible by the Vertex
Shader, one of the easiest ways to do this is with a Uniform
Buffer. We've used uniforms before to pass individual 
variables to the GPU, but this time we will use a Uniform
to pass an entire buffer to the GPU, which holds all of 
our particles.

First, we use glGenBuffers to initialize the uniform buffer.
	(constructor) glGenBuffers(1, &m_UBO);
	
Next, we need to find the uniform index of the buffer, just like
any other uniform. We do this by using glGetUniformBlockIndex.
If you look inside the Vertex shader, you'll see our buffer is
called "Particles" so let's search for it
	glGetUniformBlockIndex(program, "Particles");
	
We calculate the size of the buffer (supports a maximum of 65kb)
	int size = MAX_PARTICLES * sizeof(Particle);

Next, we bind our buffer to activate the ability to write to it:
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	
We bind particle data to the buffer, and we use GL_DYNAMIC_DRAW
to allow the shader to be altered multiple times without issues:
	glBufferData(GL_UNIFORM_BUFFER, size, m_particles, GL_DYNAMIC_DRAW);
	
Lastly we unbind the buffer, because we don't need to write to it
until the next frame comes:
	glBindBuffer(GL_UNIFORM_BUFFER, 0);
	
After we have the binding index (which tells us where in the shader
the uniform buffer is), and after we have data in the uniform buffer, 
we need to assign a buffer to the index, we do this with glBindBufferRange.
	glBindBufferRange(GL_UNIFORM_BUFFER, uniform_block_index, m_UBO, 0, size);

Finally, we call a draw command for instanced rendering
Each object will have 4 vertices, and there will be MAX_PARTICLES amount
of objects in the scene
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, MAX_PARTICLES);	
	
Vertex Shader:
We make a structure for Particle that is identical to the one
that we have in the C++ code
struct Particle
{
	vec4 in_position;
	vec4 in_velocity;
	vec4 in_color;
	float in_rotation;
	float in_angular;
	float in_age;
	float junkPadding;
};

We make a uniform buffer that holds an array of Particle,
we will call this array "parts", and we will call the buffer "Particles"
	layout (std140) uniform Particles {
		Particle parts[1024];
	};
	
Vertex Shader lines 53-59, we use gl_InstanceID
to determine which particle we are drawing,
then we pull position, rotation, color, and age
out of the structure to be used in rendering.

We clamp color between 0 and 1,
We use in_rotation, scale, and our viewport
to make a transformation matrix "T" for each particle.

We use gl_VertexID to generate 4 UV coordinates and 
position coordinates for each quad (4 per quad, one quad per particle).
UVs are 0 to 1 range, for the texture to be applied to the quad
position is -0.5 to 0.5, so that (0,0) is the center of the quad,
to get position we just subtract 0.5 from UV: "uv - vec2(0.5)"
We multiply T by position to get the offset.

To get the position of the quad on the screen, we multiply 
our view matrix by the particle's position. Keep in mind that
in_position is the CENTER of the quad, in_position is not the 
vertex position.
	cameraView * in_position
	
We then take the position of the particle, and add an offset,
which is for each corner of the particle (each vertex)
	gl_Position = (cameraView * in_position) + vec4(offset, 0, 0);

Fragment Shader:
Couldn't be easeir, just draw the texture and color
	gl_FragColor = texture(tex, uv) * color; 

The next tutorial is identical to this tutorial, except
the for the use of Instanced Rendering vs Geometry Shaders.
Instanced Rendering is much faster than Geometry Shaders,
but do not allow as many particles to be rendered. This
tutorial will draw 1024 particles, the next one can draw 8192.

If you want to have more than 1024 particles with instanced
rendering, make more uniform buffers and more draw calls.
Most particle effects do not require more than 1024 particles,
unless you are simulating an ocean, or a volcano eruption.

How to improve:
Use Geometry Shaders to handle more particles.
Then use Compute Shaders to handle all particle processing on the GPU
