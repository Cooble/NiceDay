#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <iostream>
#include <chrono>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 800;
const unsigned int SCR_HEIGHT = 600;


//this forces nvidia driver to use high-performance gpu instead of integrated
/*extern "C" {
	_declspec(dllexport) DWORD NvOptimusEnablement = 1;
}*/

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "OPENGL test", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	int interval = 0;
	std::cout << "Enter SwapInterval:" << std::endl;
	std::cin >> interval;
	std::cout << "SwapInterval set to: "<<interval << std::endl;
	
	glfwMakeContextCurrent(window);
	glfwSwapInterval(interval);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	//print info
	const GLubyte* vendor = glad_glGetString(GL_VENDOR);
	const GLubyte* renderer = glad_glGetString(GL_RENDERER);
	const GLubyte* gl = glad_glGetString(GL_VERSION); 
	const GLubyte* glsl = glad_glGetString(GL_SHADING_LANGUAGE_VERSION);

	std::cout << "Graphics card info:"<< std::endl;
	std::cout << (char*)vendor << std::endl;
	std::cout << (char*)renderer << std::endl;
	std::cout << "GL version: " << gl << std::endl;
	std::cout << "GLSL version: " << glsl << std::endl << std::endl;
	std::cout << "Type anything to start" << std::endl;
	std::cin.get();
	std::cin.get();
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.2f, 0.3f, 0.3f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT);
		

		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		auto  start =std::chrono::high_resolution_clock::now();
		
		glfwSwapBuffers(window);
		
		long long micros = std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count();
		std::cout << "took " << (micros/1000.f) <<"ms"<<std::endl;
		
		glfwPollEvents();
	}

	// glfw: terminate, clearing all previously allocated GLFW resources.
	// ------------------------------------------------------------------
	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
		glfwSetWindowShouldClose(window, true);
}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}