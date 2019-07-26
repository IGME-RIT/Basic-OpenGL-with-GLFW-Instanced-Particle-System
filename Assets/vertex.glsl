/*
Title: Improved Particle System
File Name: vertex.glsl
Copyright ? 2019
Author: Niko Procopi
Written under the supervision of David I. Schwartz, Ph.D., and
supported by a professional development seed grant from the B. Thomas
Golisano College of Computing & Information Sciences
(https://www.rit.edu/gccis) at the Rochester Institute of Technology.

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or (at
your option) any later version.

This program is distributed in the hope that it will be useful, but
WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/


#version 400 core

// camera view projection matrix.
uniform mat4 cameraView;
uniform vec2 particleSize;
uniform vec2 viewport;

out vec2 uv;
out vec4 color;

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

layout (std140) uniform Particles {
  Particle parts[1024];
};

void main(void)
{
	int index = gl_InstanceID;
	
	vec4 in_position =	parts[index].in_position;
	vec4 in_color =		parts[index].in_color;
	float in_rotation = parts[index].in_rotation;
	float in_age =		parts[index].in_age;

	// Make the color fade with age (in 1 to 0 range)
	color = in_color * clamp(in_age, 0, 1);
	
	// Make a transform matrix that rotates and scales vertices.
	float c = cos(in_rotation);
	float s = sin(in_rotation);
	vec2 scale = particleSize / viewport;

	// move and shrink particle while it ages
	mat2 T = mat2(c * scale.x, s * scale.y, -s * scale.x, c * scale.y) * in_age;

	uv = vec2(gl_VertexID % 2, gl_VertexID/2);
	// 0 0
	// 1 0
	// 0 1
	// 1 1

	// uv - vec2(0.5)
	// turn each 0 into -0.5
	// turn each 1 into +0.5
	// makes the coordinates of a quad

	// T applies the particle's scale
	// and translation as it ages

	// T * position
	vec2 offset = T * (uv - vec2(0.5));

	gl_Position = (cameraView * in_position) + vec4(offset, 0, 0);
}