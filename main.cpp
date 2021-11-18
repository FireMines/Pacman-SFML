/**
 *	This is group 17th source code for the game 'Pacman'
 *
 * @file	main.cpp
 * @authors	Lars Blütecher Holter, Lillian Alice Wangerud, Matthias David Greeven
 */

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <random>
#include <set>
#include <vector>
#include <glm/glm.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "headers/map.h"
#include "headers/sprites.h"
#include <stb_image.h>

#include "shaders/spriteShader.h"

int windowWidth, windowHeight, sizePerSquare = 20.f;
int ghost_amount = 0;

std::string filePath = "../../../../levels/level0"; //CHANGE THIS IF YOU WANT TO LOAD A DIFFERENT MAP

void Camera					(const GLuint shaderprogram);
void setWindowSize			(std::string filePath);
void error_callback			(int error, const char* description);
void mouse_callback			(GLFWwindow* window, double xpos, double ypos);


static void key_callback	(GLFWwindow* window, int key, int scancode, int action, int mods);

GLuint CompileShader		(const std::string& vertexShader, const std::string& fragmentShader, const std::string& geometryShader = "");
GLuint load_opengl_texture	(const std::string& filepath, GLuint slot);
std::vector<Pacman*> gPacman;		//vector onlyincludeing pacman


/**
 *	Main program
 */
int main() {
	//loader map size
	setWindowSize(filePath); //made this a function to allow for other levels to be loaded

	// Initialises openGL
	if (!glfwInit()) {
		std::cout << "OpenGL failed to initialize" << std::endl;
		return -1;
	}
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4); // 4
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3); // 3 => 4.3
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE); // Demands support for new functions
	glfwWindowHint(GLFW_SAMPLES, 4); // Anti-aliasing


	// Makes a window and give message if something is wrong
	GLFWwindow* window = glfwCreateWindow(windowWidth, windowHeight, "Pacman project autumn 2021", NULL, NULL);
	if (window == NULL) { // If window doesnt open or it detects a fault
		std::cout << "OpenGL-window could not be created." << std::endl;
		glfwTerminate();
		return -1;
	}
	glfwSetWindowAspectRatio(window, windowWidth, windowHeight);



	// Sets the 'context' to be our window
	// Tells it to draw stuff on 'window'
	glfwMakeContextCurrent(window);
	gladLoadGL();

	// Tells openGL which callback functions we use
	glfwSetKeyCallback(window, key_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);


	// Combines shaders to a single shader program
	GLuint shader_program = CompileShader(mapVertexShaderSrc,
		mapFragmentShaderSrc);

	GLuint pellet_shaderprogram = CompileShader(pelletVertexShaderSrc,
		pelletFragmentShaderSrc, pelletGeometryShaderSrc);

	GLuint sprite_shaderprogram = CompileShader(spriteVertexShaderSrc,
		spriteFragmentShaderSrc);

	double currentTime = 0.0;	// Sets a current time variable
	glfwSetTime(0.0);			// Resets time


	// Creates new objects
	Map map(filePath);
	//Sprites pacman(&map, sprite_shaderprogram, ghost_shaderprogram);
	Pacman pacman(&map, sprite_shaderprogram);
	gPacman.push_back(&pacman);
	

	auto spriteSheet = load_opengl_texture("assets/pacman.png", 0);
	auto wallTexture = load_opengl_texture("assets/walls.png", 0);

	// Create a texture coordinate as an aditional attribute for the square vertices
	auto pacmanVAO = pacman.initPacman();
	
	
	std::vector<GLuint> ghost_shaderprograms;
	std::vector<Ghosts*> ghosts;
	std::vector<GLuint> ghostVAOs; 
	for (int i = 0; i < ghost_amount; i++) {
		ghost_shaderprograms.push_back(CompileShader(spriteVertexShaderSrc,
			spriteFragmentShaderSrc));
		ghosts.push_back(new Ghosts(&map, ghost_shaderprograms[i]));
		ghostVAOs.push_back(ghosts[i]->initGhost(time(nullptr) + i));
	}

	bool gameDone = false;		// true if game is done 
	bool fullscreen = false;
	// 'Gameloopen' 
	while (!glfwWindowShouldClose(window)) {
		
		double pastTime = currentTime;
		currentTime = glfwGetTime();		// Time management
		double dt = currentTime - pastTime;

		glfwGetFramebufferSize(window, &windowWidth, &windowHeight);
		glViewport(0, 0, windowWidth, windowHeight);

		if (glfwGetKey(window, GLFW_KEY_F) == GLFW_PRESS) {
			if (!fullscreen) {
				fullscreen = true;
				//glfwSetWindowMonitor(window, glfwGetPrimaryMonitor(), NULL, windowWidth, windowWidth, windowHeight, 60);
				std::cout << glfwGetPrimaryMonitor() << std::endl;
			}
			else {
				fullscreen = false;
				glfwSetWindowMonitor(window, NULL, NULL, windowWidth, windowWidth, windowHeight, 60);
			}
		}

		// Clear screen with white
		glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
		
		glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
		glEnable(GL_DEPTH_TEST);

		// Draws items on the screen
		glUseProgram(shader_program);		// Tells our code which shader program we use

		// Enables textures for the walls
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, wallTexture); 
		map.drawMap();

		glUseProgram(pellet_shaderprogram);
		map.drawPellets();

		// auto samplerSlotLocation0 = glGetUniformLocation(sprite_shaderprogram, "uTextureA");
		glUseProgram(sprite_shaderprogram);
		glBindVertexArray(pacmanVAO);
		glActiveTexture(GL_TEXTURE0);
		glBindTexture(GL_TEXTURE_2D, spriteSheet);

		pacman.drawPacman();
		gameDone = pacman.movement(window, dt, ghosts, gameDone);
		
		for (int i = 0; i < ghost_amount; i++) {
			auto samplerSlotLocation1 = glGetUniformLocation(ghost_shaderprograms[i], "uTextureA");
			glUseProgram(ghost_shaderprograms[i]);
			glBindVertexArray(ghostVAOs[i]);
			glActiveTexture(GL_TEXTURE0);
			glBindTexture(GL_TEXTURE_2D, spriteSheet);

			ghosts[i]->drawGhosts();
			ghosts[i]->movement(window, dt, gameDone, time(nullptr) + i);
			Camera(ghost_shaderprograms[i]);
		}

		Camera(shader_program);
		Camera(pellet_shaderprogram);
		Camera(sprite_shaderprogram);

		// Updates
		glfwPollEvents();

		// Display
		glfwSwapBuffers(window);
	}

	// Lag en funksjon som sletter shaderprograms

	glDeleteTextures(1, &spriteSheet);
	// Terminate
	glfwDestroyWindow(window);
	glfwTerminate();
	return 0;
}


// -----------------------------------------------------------------------------
// Code handling the camera
// -----------------------------------------------------------------------------
void Camera(const GLuint shaderprogram) {

	//Mouse implimentation
// camera
	glm::vec3 cameraPos = glm::vec3(gPacman[0]->getPacPos().first, gPacman[0]->getPacPos().second, 1.f);
	glm::vec3 cameraFront = gPacman[0]->getCameraFront();
	glm::vec3 cameraUp = glm::vec3(0.f, 0.f, 1.f);

	glUseProgram(shaderprogram);

	glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)windowWidth / (float)windowHeight, 0.1f, 100.0f);

	//Matrix which defines where in the scene our camera is
	//                           Position of camera     Direction camera is looking     Vector pointing upwards
	glm::mat4 view = glm::lookAt(cameraPos, (cameraPos + cameraFront), cameraUp);

	//Get unforms to place our matrices into
	GLuint projmat = glGetUniformLocation(shaderprogram, "u_ProjectionMat");
	GLuint viewmat = glGetUniformLocation(shaderprogram, "u_ViewMat");

	//Send data from matrices to uniform
	glUniformMatrix4fv(projmat, 1, false, glm::value_ptr(projection));
	glUniformMatrix4fv(viewmat, 1, false, glm::value_ptr(view));
}


void mouse_callback(GLFWwindow* window, double xpos, double ypos){
	
	gPacman[0]->updateCamera(xpos, ypos);
}

// -----------------------------------------------------------------------------
// COMPILE SHADER
// -----------------------------------------------------------------------------
GLuint CompileShader(const std::string& vertexShaderSrc,
	const std::string& fragmentShaderSrc, const std::string& geometryShaderSrc) {

	auto vertexSrc = vertexShaderSrc.c_str();
	auto fragmentSrc = fragmentShaderSrc.c_str();
	auto geometrySrc = geometryShaderSrc.c_str();

	auto shaderProgram = glCreateProgram();

	int  success;
	char infoLog[512];

	//create and attach vertex shader
	auto vertexShader = glCreateShader(GL_VERTEX_SHADER);
	glShaderSource(vertexShader, 1, &vertexSrc, nullptr);
	glCompileShader(vertexShader);
	glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::VERTEX::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	glAttachShader(shaderProgram, vertexShader);
	glDeleteShader(vertexShader);

	//create and attach geometry shader if not a empty string
	if (geometryShaderSrc != "") {
		auto geometryShader = glCreateShader(GL_GEOMETRY_SHADER);
		glShaderSource(geometryShader, 1, &geometrySrc, nullptr);
		glCompileShader(geometryShader);

		glGetShaderiv(geometryShader, GL_COMPILE_STATUS, &success);
		if (!success) {
			glGetShaderInfoLog(geometryShader, 512, NULL, infoLog);
			std::cout << "ERROR::SHADER::GEOMETRY::COMPILATION_FAILED\n" << infoLog << std::endl;
		}
		glAttachShader(shaderProgram, geometryShader);
		glDeleteShader(geometryShader);
	}

	//create and attach fragemnt shader
	auto fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
	glShaderSource(fragmentShader, 1, &fragmentSrc, nullptr);
	glCompileShader(fragmentShader);
	glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
	if (!success) {
		glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
		std::cout << "ERROR::SHADER::FRAGMENT::COMPILATION_FAILED\n" << infoLog << std::endl;
	}
	glAttachShader(shaderProgram, fragmentShader);
	glDeleteShader(fragmentShader);
	
	glLinkProgram(shaderProgram);
	glUseProgram(shaderProgram);

	return shaderProgram;
}


/**
 *	Error callback
 */
void error_callback(int error, const char* description) {
	fprintf(stderr, "Error: %s\n", description);
}

/**
 *	Key callback
 */
static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
		glfwSetWindowShouldClose(window, GLFW_TRUE);
}


GLuint load_opengl_texture(const std::string& filepath, GLuint slot) {
	/**
	 *  - Use the STB Image library to load a texture in here
	 *  - Initialize the texture into an OpenGL texture
	 *    - This means creating a texture with glGenTextures or glCreateTextures (4.5)
	 *    - And transferring the loaded texture data into this texture
	 *    - And setting the texture format
	 *  - Finally return the valid texture
	 */

	 /** Image width, height, bit depth */
	int w, h, bpp;
	auto pixels = stbi_load(filepath.c_str(), &w, &h, &bpp, STBI_rgb_alpha);

	/*Generate a texture object and upload the loaded image to it.*/
	GLuint tex;
	glGenTextures(1, &tex);
	glActiveTexture(GL_TEXTURE0 + slot);//Texture Unit
	glBindTexture(GL_TEXTURE_2D, tex);
	glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0, GL_RGBA, GL_UNSIGNED_BYTE, pixels);

	/** Set parameters for the texture */
	//Wrapping
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	//Filtering 
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	/** Very important to free the memory returned by STBI, otherwise we leak */
	if (pixels)
		stbi_image_free(pixels);

	return tex;
}

// -----------------------------------------------------------------------------
// Code handling the map size
// -----------------------------------------------------------------------------
void setWindowSize(std::string filePath) {
	std::ifstream in(filePath);
	if (in) {
		in >> windowWidth; in.ignore(1); in >> windowHeight;	//Read the first string (amount of tiles in X and Y axis)
	}															//nothing else
	windowWidth = windowWidth * sizePerSquare;					//multiplies the amount of tiles with the size for each tile
	windowHeight = windowHeight * sizePerSquare;
}

