#include "stdafx.h"
#include "ParticleSystem.h"

#include "RenderCam.h"
#include "GraphicsManager.h"
#include "MotionProperties.h"

#define CHECKERROR { \
GLenum err = glGetError(); \
if (err != GL_NO_ERROR) { \
	SIK_ERROR("OpenGL error:\"{}\": \n", reinterpret_cast<const char*>(glewGetErrorString(err))); \
	SIK_ASSERT(false, "");} \
 }


////////////////////////////////////////////////////////////////////////////////
// Particle Emitter
////////////////////////////////////////////////////////////////////////////////

ParticleEmitter::ParticleEmitter(Uint32 mesh_vbo) {
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Create buffers
	glGenBuffers(1, &positions_sizes_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, positions_sizes_vbo);
	glBufferData(GL_ARRAY_BUFFER, ParticleEmitter::MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

	glGenBuffers(1, &colors_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferData(GL_ARRAY_BUFFER, ParticleEmitter::MAX_PARTICLES * 4 * sizeof(GLfloat), NULL, GL_DYNAMIC_DRAW);

	// Configure attributes
	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, mesh_vbo);
	glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (void*)(3 * sizeof(GLfloat)));
	
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, positions_sizes_vbo);
	glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glEnableVertexAttribArray(3);
	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, 0, (void*)0);

	glVertexAttribDivisor(0, 0); // vertices. always reuse the same mesh.
	glVertexAttribDivisor(1, 0); // uv. always reuse the same UVs.
	glVertexAttribDivisor(2, 1); // positions + sizes. 1 per quad.
	glVertexAttribDivisor(3, 1); // color. 1 per quad.

	glBindVertexArray(0);

	CHECKERROR
}
ParticleEmitter::~ParticleEmitter() noexcept {
	glDeleteBuffers(1, &positions_sizes_vbo);
	glDeleteBuffers(1, &colors_vbo);
	glDeleteVertexArrays(1, &vao);
}

Particle ParticleEmitter::RandParticle() const {
	Float32 final_theta = rand_float->GenInRange(theta.min, theta.max) - 3.142f;
	Float32 final_phi = rand_float->GenInRange(phi.min, phi.max) + 3.142f / 2.0f;
	Float32 sin_theta = sinf(final_theta);
	Float32 sin_phi = sinf(final_phi);
	Float32 cos_theta = cosf(final_theta);
	Float32 cos_phi = cosf(final_phi);

	return Particle{
				.secs_remaining = rand_float->GenInRange(lifetime_secs_gen_bnds.min, lifetime_secs_gen_bnds.max),
				.position = (glm::toMat3(orientation) * RandVec3(pos_gen_bnds.min,   pos_gen_bnds.max, *rand_float))
							+ ParticleEmitter::position,
				.uniform_scale = rand_float->GenInRange(uniform_scale_gen_bnds.min, uniform_scale_gen_bnds.max),
				.color = RandVec4(color_gen_bnds.min, color_gen_bnds.max, *rand_float),
				.velocity = glm::toMat3(orientation) * Vec3(sin_theta * sin_phi, cos_phi, cos_theta * sin_phi)
				            * rand_float->GenInRange(speed_gen_bnds.min, speed_gen_bnds.max)
	};
}

void ParticleEmitter::EmitParticles(Uint32 particles_to_emit) {
	Uint32 remaining_space = MAX_PARTICLES - static_cast<Uint32>(particles.size());
	Uint32 overflow_count = (particles_to_emit > remaining_space) ? particles_to_emit - remaining_space : 0;
	particles_to_emit -= overflow_count;

	// If we need to overwrite the whole array, don't bother adding to the end.
	if (overflow_count >= MAX_PARTICLES) {
		overflow_count = MAX_PARTICLES;
		particles_to_emit = 0;
	}

	// Add new particles to end of the array
	for (Uint32 i = 0; i < particles_to_emit; ++i) {
		particles.push_back(RandParticle());
	}
	// Add new particles to the beginning, overwriting particles which are far
	// far from the camera (because the array is partially sorted)
	for (Uint32 i = 0; i < overflow_count; ++i) {
		particles[i] = RandParticle();
	}
}

void ParticleEmitter::Update(Float32 dt) {

	SIK_ASSERT(particles.size() <= MAX_PARTICLES, "Particle limit exceeded!");

	// Compute number of particles to push to the back of particles array, and
	// the number that will need to be added by overwriting the front of the
	// particles array
	if (is_active) {
		emission_accumulator += dt * particles_per_sec;
		Uint32 particles_to_emit = static_cast<Uint32>(emission_accumulator);
		if (particles_to_emit > 0) {
			emission_accumulator -= particles_to_emit;

			EmitParticles(particles_to_emit);
		}
	}

	// Camera position will be needed to update our sorting attribute, sqr_dist_from_camera
	Vec3 const camera_pos = (p_graphics_manager->GetPActiveCam())->GetPosition();
	
	// Update emitter
	sqr_dist_from_camera = glm::length2(position - camera_pos);

	// Update particles
	for (auto&& p : particles) {
		UpdateParticle(p);

		p.velocity += MotionProperties::gravity * gravity_scale * dt;
		p.position += p.velocity * dt;

		p.sqr_dist_from_camera = glm::length2(p.position - camera_pos);
		
		p.secs_remaining -= dt;
	}

	// Remove dead particles
	std::erase_if(particles, [](Particle const& p) { return p.secs_remaining < 0.0f; });

	// Sort particles based on distance from camera
	std::sort(particles.begin(), particles.end());

	// Grow GPU data arrays if needed
	if (colors.size() < particles.size()) {
		positions_sizes.resize(particles.size());
		colors.resize(particles.size());
	}

	// Load particle data into buffers for transmission to the GPU
	SizeT idx = 0;
	for (auto&& p : particles) {
		positions_sizes[idx] = Vec4(p.position, p.uniform_scale);
		colors[idx] = p.color;
		idx++;
	}

	// Send buffers to GPU
	glBindVertexArray(vao);
	glBindBuffer(GL_ARRAY_BUFFER, positions_sizes_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * 4 * sizeof(GLfloat), positions_sizes.data());

	glBindBuffer(GL_ARRAY_BUFFER, colors_vbo);
	glBufferSubData(GL_ARRAY_BUFFER, 0, particles.size() * 4 * sizeof(GLfloat), colors.data());
	glBindVertexArray(0);
}

void ParticleEmitter::Draw(Uint32 shader_id) {
	glBindVertexArray(vao);

	// Bind texture, if there is one
	if (texture == nullptr) {
		p_graphics_manager->SetUniform(shader_id, 0, "TextureEnabled");
	}
	else {
		p_graphics_manager->SetUniform(shader_id, 1, "TextureEnabled");
		texture->Bind(0, shader_id, "Texture");
	}

	// Draw!
	glDrawArraysInstanced(GL_TRIANGLE_STRIP, 0, 4, (GLsizei)(particles.size()));

	if (texture != nullptr) {
		texture->Unbind();
	}
	
	glBindVertexArray(0);
}

////////////////////////////////////////////////////////////////////////////////
// Particle System
////////////////////////////////////////////////////////////////////////////////

ParticleSystem::~ParticleSystem() noexcept {
	glDeleteBuffers(1, &quad_vbo);
}


ParticleEmitter* ParticleSystem::NewEmitter() {
	if (emitters.size() < MAX_EMITTERS) {
		ParticleEmitter* e = emitters.emplace(quad_vbo);
		sorted_emitters.push_back(e);

		return e;
	}

	return nullptr;
}

void ParticleSystem::EraseEmitter(ParticleEmitter* emitter) {
	sorted_emitters.erase(
		std::find(sorted_emitters.begin(), sorted_emitters.end(), emitter)
	);

	emitters.erase(emitter);
}

void ParticleSystem::Init() {
	glGenBuffers(1, &quad_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, quad_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof(quad_verts), quad_verts, GL_STATIC_DRAW);
}

void ParticleSystem::Update(Float32 dt) {

	// Update all the emitters
	for (auto r = emitters.all(); not r.is_empty(); r.pop_front()) {
		r.front().Update(dt);
	}

	// We want to sort the emitters so that MOST of their particles are also sorted
	// when they are loaded to the GPU in one big buffer. This operates under the
	// assumption that emitter position takes precedence over individual particle
	// positions... meaning if we placed two emitters near each other such that
	// their particles overlapped, we would see the particles of the emitter
	// closest to the camera drawn "in front" regardless of their actual position
	std::sort(sorted_emitters.begin(), sorted_emitters.end(), 
		[](ParticleEmitter* const& pe0, ParticleEmitter* const& pe1) { 
			return *pe0 < *pe1; 
		}
	);
}


void ParticleSystem::Draw(Uint32 shader_id, Mat4 const& proj_view, Vec3 const& cam_right, Vec3 const& cam_up) const {
	
	// Use shader
	glUseProgram(shader_id);

	// Pass uniforms common to all emitters
	p_graphics_manager->SetUniform(shader_id, proj_view, "ProjView");
	p_graphics_manager->SetUniform(shader_id, cam_right, "CamRight");
	p_graphics_manager->SetUniform(shader_id, cam_up, "CamUp");

	for (auto&& e : sorted_emitters) {
		e->Draw(shader_id);
	}
	
	// Reset shader program
	glUseProgram(0);
}