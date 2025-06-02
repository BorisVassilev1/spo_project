#include <iostream>

#include <yoghurtgl.h>
#include <window.h>
#include <NBody.hpp>
#include "GLFW/glfw3.h"
#include "asset_manager.h"
#include "camera.h"
#include "input.h"
#include <renderer.h>

int main() {
	if (ygl::init()) { dbLog(ygl::LOG_ERROR, "Failed to initialize YoghurtGL"); }

	ygl::Window window = ygl::Window(800, 600, "YoghurtGL Window", false);
	srand(time(0));

	ygl::Scene scene;
	scene.registerSystem<ygl::Renderer>(&window);
	scene.registerSystem<NBody>(10000);

	ygl::OrthographicCamera camera(20, window, .1, 10);
	camera.transform.position = glm::vec3(0.0f, 0.0f, 5.0f);

	ygl::Renderer	  *renderer = scene.getSystem<ygl::Renderer>();
	NBody			  *nbody	= scene.getSystem<NBody>();
	ygl::AssetManager *asman	= scene.getSystem<ygl::AssetManager>();
	renderer->setMainCamera(&camera);
	renderer->loadData();

	ygl::VFShader *sh =
		new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/unlit.vs", YGL_RELATIVE_PATH "./shaders/unlit.fs");
	renderer->setDefaultShader(asman->addShader(sh, "defaultShader"));

	ygl::Keyboard::addKeyCallback([&](GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) { asman->reloadShaders(); }
	});

	renderer->setClearColor(glm::vec4(0, 0, 0, 1));
	while (!window.shouldClose()) {
		window.beginFrame();

		if (ygl::Keyboard::getKey(GLFW_KEY_J) == GLFW_PRESS) {
			camera.width *= 1. - 1 * window.deltaTime;
			camera.updateProjectionMatrix();
		}
		if (ygl::Keyboard::getKey(GLFW_KEY_K) == GLFW_PRESS) {
			camera.width *= 1. + 1 * window.deltaTime;
			camera.updateProjectionMatrix();
		}
		float speed = camera.width * .5 * window.deltaTime;
		if (ygl::Keyboard::getKey(GLFW_KEY_W) == GLFW_PRESS) {
			camera.transform.position.y += speed;
			camera.getViewMatrix();
		}
		if (ygl::Keyboard::getKey(GLFW_KEY_A) == GLFW_PRESS) {
			camera.transform.position.x -= speed;
			camera.getViewMatrix();
		}
		if (ygl::Keyboard::getKey(GLFW_KEY_S) == GLFW_PRESS) {
			camera.transform.position.y -= speed;
			camera.getViewMatrix();
		}
		if (ygl::Keyboard::getKey(GLFW_KEY_D) == GLFW_PRESS) {
			camera.transform.position.x += speed;
			camera.getViewMatrix();
		}

		nbody->gui();
		camera.update();
		scene.doWork();

		window.swapBuffers();
	}

	ygl::terminate();
	return 0;
}
