#include <fstream>
#include <iostream>

#include <yoghurtgl.h>
#include <window.h>
#include <NBody.hpp>
#include <GLFW/glfw3.h>
#include <renderer.h>
#include <bench.hpp>
#include <ratio>

#define BENCH_GPU(x, n, f)                                                                                           \
	{                                                                                                                \
		GLuint query = 0;                                                                                            \
		glGenQueries(1, &query);                                                                                     \
		glBeginQuery(GL_TIME_ELAPSED, query);                                                                        \
		for (int i = 0; i < n; ++i) {                                                                                \
			x;                                                                                                       \
		}                                                                                                            \
		GLuint64 timeElapsed = 0;                                                                                    \
		glFinish();                                                                                                  \
		glEndQuery(GL_TIME_ELAPSED);                                                                                 \
		glGetQueryObjectui64v(query, GL_QUERY_RESULT, &timeElapsed);                                                 \
		glDeleteQueries(1, &query);                                                                                  \
		auto duration = std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::nanoseconds(timeElapsed)); \
		duration /= n;                                                                                               \
		f(duration);                                                                                                 \
	}



std::ofstream logFile_cpu("test_cpu.log", std::ios::out | std::ios::trunc);
std::ofstream logFile_gpu("test_gpu.log", std::ios::out | std::ios::trunc);

void testGPU(int N, ygl::Window &window, int M = 100) {
	ygl::Scene scene;
	scene.registerSystem<ygl::Renderer>(&window);
	scene.registerSystem<NBodyGPU>(N, false);
	auto *nbody = scene.getSystem<NBodyGPU>();

	std::cout << "Running test " << nbody->N << std::endl;

	BENCH_GPU(
		{
			glfwPollEvents();
			nbody->doWork();
			glfwSwapBuffers(window.getHandle());
		},
		M,
		[&](auto duration) {
			logFile_gpu << N << ", " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << std::endl;
		});
	glFinish();
}

void testCPU(int N, ygl::Window &window, int M = 100) {
	ygl::Scene scene;
	scene.registerSystem<ygl::Renderer>(&window);
	scene.registerSystem<NBodyCPU>(N, false);
	auto *nbody = scene.getSystem<NBodyCPU>();

	std::cout << "Running CPU test " << nbody->N << std::endl;

	BENCH(
		{
			glfwPollEvents();
			nbody->doWork();
			glfwSwapBuffers(window.getHandle());
		},
		M,
		[&](auto duration) {
			logFile_cpu << N << ", " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count() << std::endl;
		});
}

int main() {
	if (ygl::init()) { dbLog(ygl::LOG_ERROR, "Failed to initialize YoghurtGL"); }

	ygl::Window window = ygl::Window(800, 600, "YoghurtGL Window", false);
	srand(time(0));
	window.beginFrame();
	window.swapBuffers();
	
	logFile_cpu << "N, Time" << std::endl;
	logFile_gpu << "N, Time" << std::endl;

	testGPU(10, window, 100);
	testGPU(33, window, 100);
	testGPU(100, window, 100);
	testGPU(333, window, 100);
	testGPU(1000, window, 100);
	testGPU(3333, window, 100);
	testGPU(10000, window, 100);
	testGPU(33333, window, 20);
	testGPU(50000, window, 10);
	testGPU(75000, window, 10);
	testGPU(100000, window, 10);

	//testCPU(10, window, 100);
	//testCPU(33, window, 100);
	//testCPU(100, window, 100);
	//testCPU(333, window, 100);
	//testCPU(1000, window, 100);
	//testCPU(3333, window, 100);
	//testCPU(10000, window, 100);
	//testCPU(33333, window, 20);
	//testCPU(50000, window, 10);
	//testCPU(75000, window, 5);
	//testCPU(100000, window, 5);

	ygl::terminate();
	return 0;
}
