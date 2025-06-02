#pragma once

#include <yoghurtgl.h>
#include <ecs.h>
#include <asset_manager.h>
#include <entities.h>
#include <mesh.h>
#include <renderer.h>
#include <glm/gtx/norm.hpp>
#include <random>
#include "buffer.h"
#include "material.h"
#include "shader.h"

struct alignas(16) Particle : ygl::Serializable {
	static const char *name;
	glm::vec3		   position = glm::vec3(0);
	float			   mass		= 1;
	glm::vec3		   velocity = glm::vec3(0);

	void serialize(std::ostream &) { assert(0); }
	void deserialize(std::istream &) { assert(0); }
};

// TODO!!!
class ParticleMesh : public ygl::MultiBufferMesh {
	struct alignas(16) ParticleData {
		glm::vec3 position;
		glm::vec3 velocity;
		float	  mass;
	};

   public:
	ygl::MutableBuffer &vertexBuffer;	  // particles

	ParticleMesh(std::size_t numParticles, ygl::MutableBuffer &buff) : vertexBuffer(buff) {
		this->verticesCount = numParticles;
		this->createVAO();
		glBindVertexArray(this->getVAO());

		this->addVBO(0, 4, vertexBuffer.getID(), GL_FLOAT, 0, sizeof(ParticleData), (const void *)0);
		this->addVBO(1, 4, vertexBuffer.getID(), GL_FLOAT, 0, sizeof(ParticleData), (const void *)(4 * sizeof(float)));

		cullFace = false;

		glBindVertexArray(0);
	}
};

class NBody : public ygl::ISystem {
   public:
	static const char			 *name;
	std::size_t					  numParticles;
	ygl::AssetManager			 *asman;
	ygl::Renderer				 *renderer;
	float						  deltaTime = 1. / 144 * .05;
	const float					  EPS		= .1;
	ygl::MutableBuffer			  particleData;
	std::unique_ptr<ParticleMesh> mesh;
	int							  particleShaderIndex = -1;
	uint						  material_index	  = -1;

	std::vector<Particle> particles;

	enum DrawMode {
		WHITE,
		VELOCITY,
		MASS,

		DRAWMODE_MAX
	};

	int drawMode = 0;

	std::unique_ptr<ygl::ComputeShader> velocityUpdate = std::make_unique<ygl::ComputeShader>("./shaders/gravity.comp");
	std::unique_ptr<ygl::ComputeShader> positionUpdate = std::make_unique<ygl::ComputeShader>("./shaders/update.comp");

	NBody(ygl::Scene *scene, std::size_t numParticles)
		: ISystem(scene),
		  numParticles(numParticles),
		  // particleData(GL_ARRAY_BUFFER, numParticles * sizeof(Particle), GL_DYNAMIC_STORAGE_BIT) {}
		  particleData(GL_ARRAY_BUFFER, numParticles * sizeof(Particle), GL_DYNAMIC_DRAW) {}

	void init() override {
		this->scene->registerComponentIfCan<Particle>();
		this->scene->setSystemSignature<NBody, Particle>();
		this->asman	   = scene->getSystem<ygl::AssetManager>();
		this->renderer = scene->getSystem<ygl::Renderer>();

		createParticles(numParticles);

		particleData.resize(particles.size() * sizeof(Particle));

		ygl::VFShader *shader = new ygl::VFShader("./shaders/particle.vs", "./shaders/particle.fs");
		particleShaderIndex	  = asman->addShader(shader, "particleShader", false);

		material_index	   = renderer->addMaterial(ygl::Material());
		ygl::Material &mat = renderer->getMaterial(material_index);
		mat.albedo		   = glm::vec3(1, 1, 1);

		dbLog(ygl::LOG_DEBUG, "count: ", particles.size());
		dbLog(ygl::LOG_DEBUG, "size: ", sizeof(Particle));
		setParticles();

		mesh.reset(new ParticleMesh(particles.size(), particleData));

		renderer->addDrawFunction([&]() {
			ygl::VFShader *shader = (ygl::VFShader *)asman->getShader(particleShaderIndex);
			shader->bind();

			if (shader->hasUniform("worldMatrix")) shader->setUniform("worldMatrix", glm::mat4(1));
			if (shader->hasUniform("renderMode")) shader->setUniform("renderMode", 0u);
			if (shader->hasUniform("material_index")) shader->setUniform("material_index", material_index);
			if (shader->hasUniform("drawMode")) shader->setUniform("drawMode", drawMode);
			if (shader->hasUniform("numParticles")) shader->setUniform("numParticles", (int)numParticles);

			mesh->bind();
			glEnable(GL_BLEND);
			glDisable(GL_DEPTH_TEST);
			glBlendEquation(GL_FUNC_ADD);
			glBlendFunc(GL_SRC_ALPHA, GL_ONE);

			glDrawArrays(GL_POINTS, 0, particles.size());

			glDisable(GL_BLEND);
			glEnable(GL_DEPTH_TEST);
			mesh->unbind();
			shader->unbind();
		});
	}

	void setParticles () {
		particleData.set(particles.data(), particles.size() * sizeof(Particle));
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

		setParticles();
	}

	void computeGPU() {
		velocityUpdate->bind();
		velocityUpdate->setUniform("deltaTime", deltaTime);
		velocityUpdate->setUniform("resolution", glm::ivec2(particles.size(), 1));
		ygl::Shader::setSSBO(particleData.getID(), 1);
		ygl::Renderer::compute(velocityUpdate.get(), particles.size(), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);

		positionUpdate->bind();
		positionUpdate->setUniform("deltaTime", deltaTime);
		positionUpdate->setUniform("resolution", glm::ivec2(particles.size(), 1));
		ygl::Shader::setSSBO(particleData.getID(), 1);
		ygl::Renderer::compute(positionUpdate.get(), particles.size(), 1, 1);
		glMemoryBarrier(GL_SHADER_STORAGE_BARRIER_BIT);
	}

	void doWork() override { computeGPU(); }

	void createParticles(std::size_t numParticles) {
		std::random_device				rd;
		std::mt19937					gen(rd());
		std::normal_distribution<float> d(0, 30);

		float mass = 10000 + .0 * numParticles;

		for (std::size_t i = 0; i < numParticles; ++i) {
			float x = d(gen);
			float y = d(gen);

			Particle p;
			p.position.x  = x;
			p.position.y  = y;
			glm::vec3 dir = glm::vec3(x, y, 0);
			dir			  = glm::normalize(dir);
			p.position += dir * 1.f;

			p.velocity.x = -y;
			p.velocity.y = x;
			p.velocity	 = glm::normalize(p.velocity);
			p.velocity *= glm::sqrt(mass / glm::length(p.position));

			p.mass = (rand() % 100 == 1) ? 3 : 1;
			particles.push_back(p);
		}

		Particle p;
		p.mass = 10000;
		particles.push_back(p);
	}

	void gui() {
		ImGui::Begin("NBody");
		ImGui::Text("Particles: %zu", numParticles);
		float dt = deltaTime * 100;
		ImGui::InputFloat("Delta Time", &dt, 0, 1);
		deltaTime = dt / 100;
		ImGui::SliderInt("Draw Mode", &drawMode, 0, DRAWMODE_MAX - 1);
		if (ImGui::Button("Reset")) {
			particles.clear();
			createParticles(numParticles);
			setParticles();
		}
		ImGui::End();
	}

	void write(std::ostream &) override { assert(0); }
	void read(std::istream &) override { assert(0); }
};
