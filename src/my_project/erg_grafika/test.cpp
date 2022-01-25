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

//functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow* window);
unsigned int loadTexture(const char* path);

// settings
const unsigned int SCR_WIDTH = 900;
const unsigned int SCR_HEIGHT = 600;

// camera
Camera camera(glm::vec3(0.0f, 0.0f, 3.0f));
float lastX = (float)SCR_WIDTH / 2.0f;
float lastY = (float)SCR_HEIGHT / 2.0f;
bool firstMouse = true;
//TODO remove these 3
bool move_of_planet = true;
GLfloat scale = 0.1f;
GLfloat speed = 1.0f;

// timing
float deltaTime = 0.0f;
float lastFrame = 0.0f;

//lighting
glm::vec3 lightingPossistion(0.0f, 0.0f, 0.0f);

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
	Shader sphereShader("light_shader.vs", "light_shader.fs");		//shader for planet
	Shader faceShader("face_shader.vs", "face_shader.fs");				//shader for boxes
	// load model for planet
	// -----------

	Model Cece(FileSystem::getPath("resources/objects/head_obj/woman1.obj"));
	Sphere sphere(15, 15);

	float radius = 1.5f;
	float offset = 2.5f;

	faceShader.use();
	faceShader.setInt("material.diffuse", 0);
	faceShader.setInt("material.specular", 1);

	//variables used in render loop
	GLuint cnt = 0;
	GLfloat x, z, y2, z2;

	y2 = 0;
	z2 = 0;
	float time;
	// render loop
	// -----------
	while (!glfwWindowShouldClose(window))
	{
		// per-frame time logic
		// --------------------
		float currentFrame = glfwGetTime();
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;
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
		
		sphereShader.use();

		sphereShader.setMat4("projection", projection);
		sphereShader.setMat4("view", view);

		// draw sphere
		glm::mat4 model = glm::mat4(1.0f);
		//if (move_of_planet == true) {							//planet moves
		//	time = glfwGetTime() * speed;
		//	x = sin(time) * radius;
		//	z = cos(time) * radius;
		//}
		if (move_of_planet == true) {							//planet moves
			float angle = 0.006f * cnt * speed / 2;					//calculate angle
			radius = 5.0f * 2 * scale;
			x = radius * sin(PI * 2 * angle / 360);
			z = radius * cos(PI * 2 * angle / 360);
		}

		model = glm::translate(model, glm::vec3(x, 0.0, z));		//translate sphere so that it rotates on our window
		model = glm::scale(model, glm::vec3(1.0f * scale));		//scale sphere so that it fit the window
		sphereShader.setMat4("model", model);
		sphere.Draw(); //draw model

		// draw boxes
		faceShader.use();
		faceShader.setVec3("light.position", glm::vec3(x, 0.0, z));
		faceShader.setVec3("viewPos", camera.Position);

		// light properties so that our boxes are lighted up in a nice way

		faceShader.setVec3("light.ambient", 0.2f, 0.2f, 0.2f);
		faceShader.setVec3("light.diffuse", 0.5f, 0.5f, 0.5f);
		faceShader.setVec3("light.specular", 1.0f, 1.0f, 1.0f);
		faceShader.setFloat("light.constant", 1.0f);
		faceShader.setFloat("light.linear", 0.001f);
		faceShader.setFloat("light.quadratic", 0.000012f);
		// material properties
		faceShader.setFloat("material.shininess", 64.0f);

		faceShader.setMat4("projection", projection);
		faceShader.setMat4("view", view);
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
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)				//close window with ESCAPE key
		glfwSetWindowShouldClose(window, true);

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)					//move forward with W key
		camera.ProcessKeyboard(FORWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)					//move backward with S key
		camera.ProcessKeyboard(BACKWARD, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)					//move left with A key
		camera.ProcessKeyboard(LEFT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)					//move right with D key
		camera.ProcessKeyboard(RIGHT, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS)					//move upwards with UP key
		camera.ProcessKeyboard(UPWARDS, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS)				//move downwards with DOWN key
		camera.ProcessKeyboard(DOWNWARDS, deltaTime);
	if (glfwGetKey(window, GLFW_KEY_P) == GLFW_PRESS)					//pause the movement of planet and boxes with P key
		move_of_planet = false;
	if (glfwGetKey(window, GLFW_KEY_U) == GLFW_PRESS)					//unpause the movement of planet and boxes with U key
		move_of_planet = true;
	if (glfwGetKey(window, GLFW_KEY_J) == GLFW_PRESS)					//make planet and boxes move faster with Q key
		speed += 0.01;
	if (glfwGetKey(window, GLFW_KEY_H) == GLFW_PRESS)					//make planet and boxes move slower with E key
	{
		speed -= 0.01;
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

// utility function for loading a 2D texture from file
// ---------------------------------------------------
unsigned int loadTexture(char const* path)
{
	unsigned int textureID;
	glGenTextures(1, &textureID);

	int width, height, nrComponents;
	unsigned char* data = stbi_load(path, &width, &height, &nrComponents, 0);
	if (data)
	{
		GLenum format;
		if (nrComponents == 1)
			format = GL_RED;
		else if (nrComponents == 3)
			format = GL_RGB;
		else if (nrComponents == 4)
			format = GL_RGBA;

		glBindTexture(GL_TEXTURE_2D, textureID);
		glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
		glGenerateMipmap(GL_TEXTURE_2D);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		stbi_image_free(data);
	}
	else
	{
		std::cout << "Texture failed to load at path: " << path << std::endl;
		stbi_image_free(data);
	}

	return textureID;
}



