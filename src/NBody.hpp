#pragma once

#include <yoghurtgl.h>
#include <ecs.h>
#include "GLFW/glfw3.h"
#include "asset_manager.h"
#include "entities.h"
#include "mesh.h"
#include "renderer.h"

struct Particle : ygl::Serializable {
	static const char *name;
	glm::vec3		   velocity = glm::vec3(0);
	float			   mass		= 1;

	void serialize(std::ostream &) { assert(0); }
	void deserialize(std::istream &) { assert(0); }
};

class NBody : public ygl::ISystem {
   public:
	static const char *name;
	ygl::Entity		   e;
	std::size_t		   numParticles;
	ygl::AssetManager *asman;
	ygl::Renderer	  *renderer;

	NBody(ygl::Scene *scene, std::size_t numParticles) : ISystem(scene), numParticles(numParticles) {}

	void init() override {
		this->scene->registerComponentIfCan<Particle>();
		this->scene->setSystemSignature<NBody, ygl::Transformation, Particle>();
		this->asman	   = scene->getSystem<ygl::AssetManager>();
		this->renderer = scene->getSystem<ygl::Renderer>();

		ygl::Mesh *mesh = new ygl::QuadMesh(1);
		mesh->setCullFace(false);
		int			  meshIndex = asman->addMesh(mesh, "Particle");
		ygl::Material mat		= ygl::Material();
		mat.albedo				= glm::vec3(1, 1, 1);
		int materialIndex		= renderer->addMaterial(mat);

		for (std::size_t i = 0; i < numParticles; ++i) {
			float x = rand() % 100 / 10. - 5;
			float y = rand() % 100 / 10. - 5;
		
			Particle p;
			//p.velocity = glm::vec3(rand() % 3 -1, rand() % 3 - 1, 0);
			p.mass = rand() % 100 / 10. + 1;
			e = scene->createEntity();
			scene->addComponent<Particle>(e, Particle());
			scene->addComponent<ygl::Transformation>(
				e, ygl::Transformation(glm::vec3(x, y, 0), glm::vec3(0, 0, 0), glm::vec3(1)));
			scene->addComponent<ygl::RendererComponent>(e, ygl::RendererComponent(-1, meshIndex, materialIndex));
		}
	}

	void doWork() override {
		for (ygl::Entity e : entities) {
			Particle			&p1 = scene->getComponent<Particle>(e);
			ygl::Transformation &t = scene->getComponent<ygl::Transformation>(e);

			glm::vec3 force = glm::vec3(0, 0, 0);
			for (ygl::Entity other : entities) {
				if (other == e) continue;
				ygl::Transformation &t2	  = scene->getComponent<ygl::Transformation>(other);
				Particle			&p2 = scene->getComponent<Particle>(other);
				glm::vec3			 diff = t2.position - t.position;
				float				 dist = glm::length(diff);
				diff					  = glm::normalize(diff);
				float add = p2.mass / (dist * dist);
				glm::clamp(add, -0.1f, 0.1f);
				force += diff * add;
			}

			p1.velocity += force * float(renderer->getWindow()->deltaTime);
			t.position += p1.velocity * float(renderer->getWindow()->deltaTime);
			t.updateWorldMatrix();
		}
	}

	void write(std::ostream &out) override { assert(0); }
	void read(std::istream &in) override { assert(0); }
};
