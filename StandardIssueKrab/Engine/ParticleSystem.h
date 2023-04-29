#pragma once

#include "Bounds.h"
#include "RandomGenerator.h"
#include "Texture.h"
#include "FixedObjectPool.h"

// Fwd decls
class Mesh;
class Material;
struct Particle;
struct ParticleEmitter;
class ParticleSystem;


struct Particle {
	Float32 sqr_dist_from_camera = 0.0f;
	Float32 secs_remaining = 0.0f;

	Vec3 position{};
	Float32 uniform_scale = 1.0f;
	Vec4 color{};
	
	Vec3 velocity{};
};
// Want to sort particles such that the ones farthest from the camera
// are drawn first
constexpr bool operator<(Particle const& l, Particle const& r) {
	return l.sqr_dist_from_camera > r.sqr_dist_from_camera;
}


// Null function used to initialize ParticleEmitter::UpdateParticle
inline void NullParticleUpdate(Particle&) {}

struct ParticleEmitter {
	friend class ParticleSystem;
	
	static constexpr SizeT MAX_PARTICLES = 1024;
	using ParticleUpdateFn = std::function<void(Particle&)>;

	Float32 sqr_dist_from_camera = 0.0f;
	Vector<Particle> particles;

	// data used to update and draw particles
	Bool			 is_active			   = false;
	Vec3             position{};
	Quat             orientation{};
	Float32          gravity_scale         = 1.0f;
	Float32           particles_per_sec     = 0;
	Texture*         texture               = nullptr;
	ParticleUpdateFn UpdateParticle        = NullParticleUpdate;
	Float32			 emission_accumulator  = 0.0f;

	// data used to generate particles
	UniquePtr<RandomGenerator<Float32>> rand_float = std::make_unique<UniformRandFloat32>();
	Bounds<Vec3>	pos_gen_bnds		   = { .min = Vec3(-1), .max = Vec3(1) };
	Bounds<Vec4>    color_gen_bnds		   = { .min = Vec4(0,0,0,1),  .max = Vec4(1) };
	// angle with x-axis on x-z plane
	Bounds<Float32> theta				   = { .min = -3.142f,  .max = 3.142f };
	// angle with x-axis on x-y plane
	Bounds<Float32> phi					   = { .min = -3.142f / 2.0f, .max = 3.142f / 2.0f };
	Bounds<Float32> speed_gen_bnds         = { .min = 0.5f,     .max = 1.0f };
	Bounds<Float32> lifetime_secs_gen_bnds = { .min = 0.5f,     .max = 1.0f };
	Bounds<Float32> uniform_scale_gen_bnds = { .min = 0.5f,     .max = 1.0f };

	// Data for all particles which will be passed to OpenGL
	Vector<Vec4> positions_sizes{};
	Vector<Vec4> colors{};

	// OpenGL buffers
	Uint32 vao = 0;
	Uint32 positions_sizes_vbo = 0;
	Uint32 colors_vbo = 0;

	// Note that these can only be called with an active OpenGL context,
	// and should not be called directly. They are here only so that
	// ParticleEmitter objects can be stored in containers. Instead,
	// create a ParticleEmitter by calling ParticleSystem::NewEmitter.
	ParticleEmitter(Uint32 mesh_vbo);
	~ParticleEmitter() noexcept;

	void EmitParticles(Uint32 particles_to_emit);

private:
	// Called by ParticleSystem. 
	void Update(Float32 dt);
	void Draw(Uint32 shader_id);

	// Called internally.
	// Helper to generate a randomized particle
	Particle RandParticle() const;
};
// We'll sort these in the same way as we sort particles themselves
constexpr bool operator<(ParticleEmitter const& l, ParticleEmitter const& r) {
	return l.sqr_dist_from_camera > r.sqr_dist_from_camera;
}


class ParticleSystem {
public:
	static constexpr SizeT MAX_EMITTERS = 2048;

private:
	FixedObjectPool<ParticleEmitter, MAX_EMITTERS> emitters{};
	Vector<ParticleEmitter*> sorted_emitters{};

	Uint32 quad_vbo = 0;

	static constexpr GLfloat quad_verts[] = {
		// vertices			    uv
	 -0.5f, -0.5f, 0.0f,     0.0f, 1.0f,
	  0.5f, -0.5f, 0.0f,     1.0f, 1.0f,
	 -0.5f,  0.5f, 0.0f,	 0.0f, 0.0f,
	  0.5f,  0.5f, 0.0f,	 1.0f, 0.0f
	};
public:
	~ParticleSystem() noexcept;

	ParticleEmitter* NewEmitter();
	void EraseEmitter(ParticleEmitter* emitter);

	// Initializes quad vbo in OpenGL
	void Init();

	// Update all particles and emitters, then prepare data for rendering
	void Update(Float32 dt);

	// Render particles
	void Draw(Uint32 shader_id, Mat4 const& proj_view, Vec3 const& cam_right, Vec3 const& cam_up) const;

	inline void Clear() {
		sorted_emitters.clear();
		emitters.clear();
	}
};

extern ParticleSystem* p_particle_system;