#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <stb_image.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <learnopengl/filesystem.h>
#include <learnopengl/shader.h>
#include <learnopengl/camera.h>
#include <learnopengl/model.h>

#include <iostream>
#include "Sphere.h"
#define PI 3.14159265359		// define pi

// functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);

// settings
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;

bool sphere_moves = true;
GLfloat scale = 0.1f;
GLfloat speed = 1.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

int main()
{
	// glfw: initialize and configure
	// ------------------------------
	glfwInit();
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

#ifdef __APPLE__
	glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
#endif

	// glfw window creation
	// --------------------
	GLFWwindow* window = glfwCreateWindow(SCR_WIDTH, SCR_HEIGHT, "LightingFace", NULL, NULL);
	if (window == NULL)
	{
		std::cout << "Failed to create GLFW window" << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	// tell GLFW to capture our mouse
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);

	// glad: load all OpenGL function pointers
	// ---------------------------------------
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cout << "Failed to initialize GLAD" << std::endl;
		return -1;
	}

	// configure global opengl state
	// -----------------------------
	glEnable(GL_DEPTH_TEST);

	// build and compile shaders
	// -------------------------
	Shader sphereShader("light_shader.vs", "light_shader.fs");	//shader for sphere/lamp
	Shader faceShader("face_shader.vs", "face_shader.fs");		//shader for face
	
	// load model for face and shpere
	// -----------
	Model Cece(FileSystem::getPath("resources/objects/head_obj/woman1.obj"));
	Sphere sphere(15, 15);

	float radius = 1.5f;
	float offset = 2.5f;

	//faceShader.use();
	//faceShader.setInt("material.diffuse", 0);
	//faceShader.setInt("material.specular", 1);

	// variables used in render loop
	GLuint cnt = 0;
	GLfloat x, z;

	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
		
		if (sphere_moves == true)
			cnt++;
		
		// input
		// -----
		processInput(window);

		// render
		// ------
		glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);


		// configure transformation matrices
		glm::mat4 projection = glm::perspective(glm::radians(camera.Zoom), (float)SCR_WIDTH / (float)SCR_HEIGHT, 0.1f, 100.0f);
		glm::mat4 view = camera.GetViewMatrix();
		
		// sphere
		// ------
		sphereShader.use();

		sphereShader.setMat4("projection", projection);
		sphereShader.setMat4("view", view);

		glm::mat4 model = glm::mat4(1.0f);

		// sphere rotation around face:
		if (sphere_moves == true) {
			// calculate current angle
			float angle = 0.0012f * cnt * speed;
			radius = 7.0f * 2 * scale;
			x = radius * sin(PI* angle / 360);
			z = radius * cos(PI* angle / 360);
		}

		model = glm::translate(model, glm::vec3(x, 0.0, z));	// translate sphere for rotation using the x and y calculated for this frame
		model = glm::scale(model, glm::vec3(1.0f * scale));		// scale sphere so that it fit the window
		sphereShader.setMat4("model", model);

		sphere.Draw(); //draw model

		// face
		// ----
		faceShader.use();

		faceShader.setVec3("light.position", glm::vec3(x, 0.0, z)); // give current light position to face fragment shader to calculate lighting 
		faceShader.setVec3("viewPos", camera.Position);
		faceShader.setMat4("projection", projection);
		faceShader.setMat4("view", view);

		// light properties
		faceShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		faceShader.setVec3("light.diffuse", 0.6f, 0.6f, 0.6f);
		faceShader.setVec3("light.specular", 0.8f, 0.8f, 0.8f);
		faceShader.setFloat("light.constant", 1.0f);
		faceShader.setFloat("light.linear", 0.05f);
		faceShader.setFloat("light.quadratic", 0.012f);

		// material properties
		faceShader.setFloat("material.shininess", 6.0f);

		glm::mat4 model2 = glm::mat4(1.0f);
		model2 = glm::scale(model2, glm::vec3(0.05f * scale));
		faceShader.setMat4("model", model2);

		Cece.Draw(faceShader);
		
		// glfw: swap buffers and poll IO events (keys pressed/released, mouse moved etc.)
		// -------------------------------------------------------------------------------
		glfwSwapBuffers(window);
		glfwPollEvents();
	}


	glfwTerminate();
	return 0;
}

// process all input: query GLFW whether relevant keys are pressed/released this frame and react accordingly
// ---------------------------------------------------------------------------------------------------------
void processInput(GLFWwindow* window)
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)		// close window with ESC
		glfwSetWindowShouldClose(window, true);
	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)			// move forward with W
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)			// move backward with S
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)			// move left with A
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)			// move right with D
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)			// pause the sphere with P
		sphere_moves = false;
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)			// unpause with U
		sphere_moves = true;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)			// increase speed of sphere with J
		speed += 0.05;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)			// decrease speed of sphere with H
	{
		speed -= 0.05;
		if (speed < 0.0) speed = 0.0;
	}

}

// glfw: whenever the window size changed (by OS or user resize) this callback function executes
// ---------------------------------------------------------------------------------------------
void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	// make sure the viewport matches the new window dimensions; note that width and 
	// height will be significantly larger than specified on retina displays.
	glViewport(0, 0, width, height);
}

// glfw: whenever the mouse moves, this callback is called
// -------------------------------------------------------
void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos; // reversed since y-coordinates go from bottom to top

	lastX = xpos;
	lastY = ypos;

	camera.ProcessMouseMovement(xoffset, yoffset);
}

// glfw: whenever the mouse scroll wheel scrolls, this callback is called
// ----------------------------------------------------------------------
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(yoffset);
}



