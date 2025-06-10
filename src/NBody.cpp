#include <NBody.hpp>

const char *Particle::name = "Particle";

const char *NBodyGPU::name = "NBodyGPU";
const char *NBodyCPU::name = "NBodyCPU";

void NBodyInstanced::createParticles(std::size_t numParticles, const std::function<void(const Particle &)> &initFunc) {
	std::random_device				rd;
	std::mt19937					gen(rd());
	std::normal_distribution<float> d(0, 30);

	float mass = 10000 + .7 * numParticles;

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
		initFunc(p);
	}

	Particle p;
	p.mass = 10000;
	initFunc(p);
}

void NBodyInstanced::init() {
	this->asman	   = scene->getSystem<ygl::AssetManager>();
	this->renderer = scene->getSystem<ygl::Renderer>();

	reset();

	ygl::VFShader *shader = new ygl::VFShader("./shaders/particle.vs", "./shaders/particle.fs");
	particleShaderIndex	  = asman->addShader(shader, "particleShader", false);

	material_index	   = renderer->addMaterial(ygl::Material());
	ygl::Material &mat = renderer->getMaterial(material_index);
	mat.albedo		   = glm::vec3(1, 1, 1);

	particleMeshIndex =
		asman->addMesh((ygl::Mesh *)new ParticleMesh(numParticles, particleData), "particleMesh", false);

	renderer->addDrawFunction([&]() {
		ygl::VFShader *shader = (ygl::VFShader *)asman->getShader(particleShaderIndex);
		shader->bind();

		if (shader->hasUniform("worldMatrix")) shader->setUniform("worldMatrix", glm::mat4(1));
		if (shader->hasUniform("renderMode")) shader->setUniform("renderMode", 0u);
		if (shader->hasUniform("material_index")) shader->setUniform("material_index", material_index);
		if (shader->hasUniform("drawMode")) shader->setUniform("drawMode", drawMode);
		if (shader->hasUniform("numParticles")) shader->setUniform("numParticles", (int)numParticles);
		if (shader->hasUniform("particleSize")) shader->setUniform("particleSize", (int)particleSize);
		if (shader->hasUniform("transparency")) shader->setUniform("transparency", transparency);

		ParticleMesh *mesh = (ParticleMesh *)asman->getMesh(particleMeshIndex);
		mesh->bind();
		glEnable(GL_BLEND);
		glDisable(GL_DEPTH_TEST);
		glBlendEquation(GL_FUNC_ADD);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE);

		glDrawArrays(GL_POINTS, 0, numParticles);

		glDisable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		mesh->unbind();
		shader->unbind();
	});
}
