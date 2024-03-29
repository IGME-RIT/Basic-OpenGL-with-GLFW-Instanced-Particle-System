﻿/*
Title: Improved Particle System
File Name: particleSystem.cpp
Copyright ? 2016, 2019
Author: David Erbelding, Niko Procopi
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

#include "particleSystem.h"

ParticleSystem::ParticleSystem(Texture* texture)
{
    // Setup shaders and shader program.
    ShaderProgram* program = new ShaderProgram();
    program->AttachShader(new Shader("../Assets/vertex.glsl", GL_VERTEX_SHADER));
    program->AttachShader(new Shader("../Assets/fragment.glsl", GL_FRAGMENT_SHADER));
    m_particleMat = new Material(program);
    m_particleMat->Bind();
    m_particleMat->SetTexture((char*)"tex", texture);

    // Make a buffer for our particle data.
    glGenBuffers(1, &m_UBO);
}

ParticleSystem::~ParticleSystem()
{
    glDeleteBuffers(1, &m_UBO);
}

Material * ParticleSystem::GetMaterial()
{
    return m_particleMat;
}

void ParticleSystem::Update(float dt)
{
    // This timer is used to keep track of how often to spawn new particles.
    m_internalTimer += dt;

    for (int i = 0; i < MAX_PARTICLES; i++)
    {
        // Get a reference to that particle, not a copy.
        Particle& p = m_particles[i];

        // Particle is alive, update its position and rotation etc.
        if (p.m_age > 0)
        {
            p.m_age -= dt;
            p.m_position += p.m_velocity * dt;
            p.m_velocity += glm::vec4(m_acceleration, 0) * dt;
            p.m_rotation += p.m_angularVelocity * dt;
        }
        // Particle is dead, and we can spawn a new one if one hasn't already been spawned recently.
        else if (m_internalTimer > m_spawnFrequency)
        {
            // Decrement this timer by the frequency, so that this test will eventually pass if the spawner has caught up with delta time.
            m_internalTimer -= m_spawnFrequency;

            // Reset the particle position.
            p.m_position = glm::vec4(m_position, 1);


            // Calculate a random direction and speed for an initial particle velocity.
            glm::vec3 initialVelocity = glm::vec3(rand(), rand(), rand());
            initialVelocity -= glm::vec3(RAND_MAX * .5f);
            initialVelocity = glm::normalize(initialVelocity);
            float speed = rand() / (float)RAND_MAX * m_startVelocityRange + m_startVelocityMin;
            initialVelocity = initialVelocity * speed;

            // Random rotation and angular velocity.
            float initialRotation = (rand() / (float)RAND_MAX) * m_rotationRange.y + m_rotationRange.x;
            float initialAngular = (rand() / (float)RAND_MAX) * m_angularVelocityRange.y + m_angularVelocityRange.x;

            // Set the particle up.
            p.m_velocity = glm::vec4(initialVelocity, 0);
            p.m_rotation = initialRotation;
            p.m_angularVelocity = initialAngular;
            p.m_color = m_color;

            // Set the particle timer back up so that it will update.
            p.m_age = m_lifeTime;
        }
    }
}

void ParticleSystem::Draw()
{
    // Enable blending when rendering particles
    glEnable(GL_BLEND);
    // SRC_ALPHA, and ONE give additive blending
    // Try SRC_ALPHA with DST_ALPHA for a cool looking effect.
    glBlendFunc(GL_SRC_ALPHA, GL_ONE);
    // NOTE: particles will draw behind other particles and be blended.
    // This means that the fragment shader has to run once for every particle on each pixel.
    // If the particles take up too much of the screen, they can become extremely slow.

	// Bind material and draw
	m_particleMat->Bind();

	GLuint program = m_particleMat->m_shaderProgram->GetGLShaderProgram();

	// get the index of the buffer in the shader program
	GLint uniform_block_index = glGetUniformBlockIndex(program, "Particles");

	// calculate size
	int size = MAX_PARTICLES * sizeof(Particle);

	// bind buffer, send data, and unbind
	glBindBuffer(GL_UNIFORM_BUFFER, m_UBO);
	glBufferData(GL_UNIFORM_BUFFER, size, m_particles, GL_DYNAMIC_DRAW);
	glBindBuffer(GL_UNIFORM_BUFFER, 0);

	// bind buffer to material
	glBindBufferRange(GL_UNIFORM_BUFFER, uniform_block_index, m_UBO, 0, size);

    // The particle size is used in the geometry shader to create quads.
    m_particleMat->SetVec2((char*)"particleSize", m_particleSize);

    // Draw one quad for every particle
    glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, MAX_PARTICLES);

    // reset everything:
    m_particleMat->Unbind();

	// disable blending
    glDisable(GL_BLEND);
}
