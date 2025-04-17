#include <iostream>

#include <yoghurtgl.h>
#include <window.h>
#include <NBody.hpp>
#include "asset_manager.h"
#include "camera.h"
#include <renderer.h>

int main() {
	if (ygl::init()) { dbLog(ygl::LOG_ERROR, "Failed to initialize YoghurtGL"); }

	ygl::Window window = ygl::Window(800, 600, "YoghurtGL Window");

	ygl::Scene scene;
	scene.registerSystem<ygl::Renderer>(&window);
	scene.registerSystem<NBody>(10);

	ygl::OrthographicCamera camera(100, window, .1, 10);
	camera.transform.position = glm::vec3(0.0f, 0.0f, 5.0f);

	ygl::Renderer	  *renderer = scene.getSystem<ygl::Renderer>();
	ygl::AssetManager *asman	= scene.getSystem<ygl::AssetManager>();
	renderer->setMainCamera(&camera);
	renderer->loadData();

	ygl::VFShader *sh =
		new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/unlit.vs", YGL_RELATIVE_PATH "./shaders/unlit.fs");
	renderer->setDefaultShader(asman->addShader(sh, "defaultShader"));

	//glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	while (!window.shouldClose()) {
		window.beginFrame();

		camera.update();
		scene.doWork();

		window.swapBuffers();
	}

	ygl::terminate();
	return 0;
}
