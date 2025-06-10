#include <iostream>

#include <yoghurtgl.h>
#include <window.h>
#include <NBody.hpp>
#include "GLFW/glfw3.h"
#include "asset_manager.h"
#include "camera.h"
#include "input.h"
#include <renderer.h>
#include <bench.hpp>

int main() {
	if (ygl::init()) { dbLog(ygl::LOG_ERROR, "Failed to initialize YoghurtGL"); }

	ygl::Window window = ygl::Window(800, 600, "YoghurtGL Window", false);
	srand(time(0));

	ygl::Scene scene;
	scene.registerSystem<ygl::Renderer>(&window);
	scene.registerSystem<NBodyGPU>(10000);

	ygl::OrthographicCamera camera(20, window, .1, 10);
	camera.transform.position = glm::vec3(0.0f, 0.0f, 5.0f);

	ygl::Renderer	  *renderer = scene.getSystem<ygl::Renderer>();
	ygl::AssetManager *asman	= scene.getSystem<ygl::AssetManager>();
	auto			  *nbody	= scene.getSystem<NBodyGPU>();
	renderer->setMainCamera(&camera);
	renderer->loadData();

	ygl::VFShader *sh =
		new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/unlit.vs", YGL_RELATIVE_PATH "./shaders/unlit.fs");
	renderer->setDefaultShader(asman->addShader(sh, "defaultShader"));

	ygl::Keyboard::addKeyCallback([&](GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) { asman->reloadShaders(); }
	});

	window.beginFrame();
	BENCH({ 
		nbody->doWork();
		glFinish();
	}, 100, std::chrono::milliseconds);
	window.swapBuffers();

	ygl::terminate();
	return 0;
}
