#pragma once

#include <random>

#include <yoghurtgl.h>
#include <ecs.h>
#include <asset_manager.h>
#include <entities.h>
#include <mesh.h>
#include <renderer.h>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/norm.hpp>
#include "imgui.h"

#include <buffer.h>
#include <material.h>
#include <shader.h>

struct alignas(16) Particle : ygl::Serializable {
	static const char *name;
	glm::vec3		   position = glm::vec3(0);
	float			   mass		= 1;
	glm::vec3		   velocity = glm::vec3(0);

	void serialize(std::ostream &) { assert(0); }
	void deserialize(std::istream &) { assert(0); }
};

struct alignas(16) ParticleData {
	glm::vec3 position;
	glm::vec3 velocity;
	float	  mass;
};

// TODO!!!
class ParticleMesh : public ygl::MultiBufferMesh {
   public:
	ygl::MutableBuffer &vertexBuffer;		// particles
	ygl::MutableBuffer	triangleBuffer;		// triangle for particles

	ParticleMesh(std::size_t numParticles, ygl::MutableBuffer &buff)
		: vertexBuffer(buff), triangleBuffer(GL_ARRAY_BUFFER, 12 * sizeof(float), GL_STATIC_DRAW) {
		this->verticesCount = numParticles;
		this->createVAO();
		glBindVertexArray(this->getVAO());

		this->addVBO(0, 4, vertexBuffer.getID(), GL_FLOAT, 1, sizeof(ParticleData), (const void *)0);
		this->addVBO(1, 4, vertexBuffer.getID(), GL_FLOAT, 1, sizeof(ParticleData), (const void *)(4 * sizeof(float)));

		float triangle[] = {
			0.0f,	  2.0f,	 0.f, 1.f,	   // bottom left
			1.7321f,  -1.0f, 0.f, 1.f,	   // bottom right
			-1.7321f, -1.0f, 0.f, 1.f,	   // top
		};
		this->triangleBuffer.set((void *)triangle, 12 * sizeof(float));

		this->addVBO(2, 4, triangleBuffer.getID(), GL_FLOAT, 0, sizeof(float) * 4, (const void *)0);

		cullFace = false;

		glBindVertexArray(0);
	}
};

class NBodyInstanced : public ygl::ISystem {
   public:
	std::size_t		   N;
	std::size_t		   numParticles;
	ygl::AssetManager *asman;
	ygl::Renderer	  *renderer;
	float			   deltaTime = 1. / 144 * .05;
	const float		   EPS		 = .1;

	ygl::MutableBuffer particleData;
	int				   particleMeshIndex   = -1;
	int				   particleShaderIndex = -1;
	uint			   material_index	   = -1;
	int				   particleSize		   = 2;
	float			   transparency		   = .3;
	float			   sharpness		   = 1.0f;
	bool			   running			   = true;

	bool draw  = true;
	int	 frame = 0;

	enum DrawMode : int {
		WHITE,
		VELOCITY,
		MASS,

		DRAWMODE_MAX
	};
	DrawMode drawMode = WHITE;

	NBodyInstanced(ygl::Scene *scene, std::size_t N, bool draw = true)
		: ISystem(scene), N(N), particleData(GL_ARRAY_BUFFER, N * sizeof(Particle), GL_DYNAMIC_DRAW), draw(draw) {}

	void init() override;

	static void createParticles(std::size_t N, const std::function<void(const Particle &)> &initFunc);
	void		setParticles(const std::vector<Particle> &particles) {
		   particleData.resize(particles.size() * sizeof(Particle));
		   particleData.set((void *)particles.data(), particles.size() * sizeof(Particle));
		   numParticles = particles.size();
	}

	virtual void reset() = 0;

	void gui() {
		ImGui::Begin("NBody");
		ImGui::Text("Particles: %zu", numParticles);
		ImGui::Text("Frame: %d", frame);
		float dt = deltaTime * 100;
		ImGui::InputFloat("Delta Time", &dt, 0, 1);
		deltaTime = dt / 100;
		ImGui::SliderInt("Draw Mode", (int *)&drawMode, 0, DRAWMODE_MAX - 1);
		if (ImGui::Button("Reset")) { reset(); }

		ImGui::SliderFloat("transparency", &transparency, 0.f, 1.f);
		ImGui::SliderInt("particle size", &particleSize, 1, 20);
		ImGui::SliderFloat("sharpness", &sharpness, 0.f, 10.f);

		if (ImGui::Button("Start/Stop")) running = !running;

		ImGui::End();
	}

	void saveFrame(std::string folder) {
		std::string filename = folder + std::format("/nbody_frame_{:05d}.jpg", frame);
		renderer->screenShot(filename);
		++frame;
	}
};

class NBodyGPU : public NBodyInstanced {
   public:
	static const char *name;

	std::unique_ptr<ygl::ComputeShader> velocityUpdate = std::make_unique<ygl::ComputeShader>("./shaders/gravity.comp");
	std::unique_ptr<ygl::ComputeShader> positionUpdate = std::make_unique<ygl::ComputeShader>("./shaders/update.comp");

	NBodyGPU(ygl::Scene *scene, std::size_t numParticles, bool draw) : NBodyInstanced(scene, numParticles, draw) {
		dbLog(ygl::LOG_DEBUG, "NBodyGPU created with ", numParticles, " particles");
		dbLog(ygl::LOG_DEBUG, "Shader Group Size: ", velocityUpdate->groupSize.x, " ", velocityUpdate->groupSize.y, " ",
			  velocityUpdate->groupSize.z);
	}

	void computeGPU() {
		int y = (numParticles + velocityUpdate->groupSize.x - 1) / velocityUpdate->groupSize.x;
		int x = velocityUpdate->groupSize.x;

		velocityUpdate->bind();
		velocityUpdate->setUniform("deltaTime", deltaTime);
		velocityUpdate->setUniform("resolution", glm::ivec2(x, y));
		ygl::Shader::setSSBO(particleData.getID(), 1);
		ygl::Renderer::compute(velocityUpdate.get(), x, y, 1);	   // TODO: use better group size
		// glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		positionUpdate->bind();
		positionUpdate->setUniform("deltaTime", deltaTime);
		positionUpdate->setUniform("resolution", glm::ivec2(x, y));
		ygl::Shader::setSSBO(particleData.getID(), 1);
		ygl::Renderer::compute(positionUpdate.get(), x, y, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void reset() override {
		std::vector<Particle> particles;
		this->createParticles(N, [&particles](const Particle &p) { particles.push_back(p); });

		dbLog(ygl::LOG_DEBUG, "count: ", particles.size());
		dbLog(ygl::LOG_DEBUG, "size: ", sizeof(Particle));
		setParticles(particles);
		frame = 0;
	}

	void doWork() override {
		if (!running) return;
		computeGPU();
	}

	void write(std::ostream &) override { assert(0); }
	void read(std::istream &) override { assert(0); }
};

class NBodyCPU : public NBodyInstanced {
   public:
	static const char	 *name;
	std::vector<Particle> particles;

	NBodyCPU(ygl::Scene *scene, std::size_t N, bool draw = true) : NBodyInstanced(scene, N, draw) {}

	void reset() override {
		particles.clear();
		this->createParticles(N, [&](const Particle &p) { particles.push_back(p); });
		setParticles(particles);
	}

	void computeCPU() {
		for (std::size_t i = 0; i < particles.size(); ++i) {
			Particle &p1 = particles[i];

			for (std::size_t j = 0; j < particles.size(); ++j) {
				if (j >= i) break;
				Particle &p2	  = particles[j];
				glm::vec3 diff	  = p2.position - p1.position;
				float	  dist_sq = glm::length2(diff);
				float	  dist	  = sqrt(dist_sq);
				float	  div	  = 1.f / (glm::max(dist_sq, EPS) * dist);
				p1.velocity += diff * p2.mass * div * deltaTime;
				p2.velocity -= diff * p1.mass * div * deltaTime;
			}
		}
		for (Particle &p1 : particles) {
			p1.position += p1.velocity * deltaTime;
		}

		// this->setParticles(particles);
	}

	void doWork() override {
		if (!running) return;
		computeCPU();
	}

	void write(std::ostream &) override { assert(0); }
	void read(std::istream &) override { assert(0); }
};
