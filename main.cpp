#include <iostream>

#include <yoghurtgl.h>
#include <window.h>
#include <NBody.hpp>
#include "GLFW/glfw3.h"
#include "asset_manager.h"
#include "camera.h"
#include "input.h"
#include <renderer.h>

int main(int argc, char *argv[]) {
	int N = 10000;
	if (argc > 1) {
		try {
			N = std::stoi(argv[1]);
		} catch (const std::exception &e) {
			std::cerr << "Invalid number of particles: " << argv[1] << ". Using default: " << N << std::endl;
		}
	}

	if (ygl::init()) { dbLog(ygl::LOG_ERROR, "Failed to initialize YoghurtGL"); }

	ygl::Window window = ygl::Window(1080, 1080, "YoghurtGL Window", false, false);
	srand(time(0));

	ygl::Scene scene;
	scene.registerSystem<ygl::Renderer>(&window);
	scene.registerSystem<NBodyGPU>(N, true);

	ygl::OrthographicCamera camera(200, window, .1, 10);
	camera.transform.position = glm::vec3(0.0f, 0.0f, 5.0f);

	ygl::Renderer	  *renderer = scene.getSystem<ygl::Renderer>();
	ygl::AssetManager *asman	= scene.getSystem<ygl::AssetManager>();
	auto			  *nbody	= scene.getSystem<NBodyGPU>();
	renderer->setMainCamera(&camera);
	renderer->loadData();

	bool gui	= true;
	bool record = false;
	int frame = 0;

	ygl::VFShader *sh =
		new ygl::VFShader(YGL_RELATIVE_PATH "./shaders/unlit.vs", YGL_RELATIVE_PATH "./shaders/unlit.fs");
	renderer->setDefaultShader(asman->addShader(sh, "defaultShader"));

	ygl::Keyboard::addKeyCallback([&](GLFWwindow *window, int key, int scancode, int action, int mods) {
		if (key == GLFW_KEY_R && action == GLFW_PRESS && mods == GLFW_MOD_CONTROL) { asman->reloadShaders(); }
		if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) { nbody->running = !nbody->running; }
		if (key == GLFW_KEY_Q && mods == GLFW_MOD_CONTROL && action == GLFW_RELEASE) {
			renderer->screenShot("nbody_screenshot.png");
		}
		if (key == GLFW_KEY_G && action == GLFW_RELEASE) { gui = !gui; }

		if (key == GLFW_KEY_R && action == GLFW_RELEASE) {
			record = !record;
			nbody->reset();
			if(!nbody->running) {
				nbody->running = true;
			}
			if (record) {
				std::cout << "Recording frames..." << std::endl;
				nbody->frame = 0;
			} else {
				std::cout << "Stopped recording." << std::endl;
			}
		}
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

		if (gui) { nbody->gui(); }

		camera.update();
		scene.doWork();

		if (record) {
			if(frame % 10 == 0) {
				nbody->saveFrame("animation");
			}
			++frame;
		}

		window.swapBuffers();
	}

	ygl::terminate();
	return 0;
}
