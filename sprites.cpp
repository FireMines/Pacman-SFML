#define TINYOBJLOADER_IMPLEMENTATION //This needs to be defined exactly once so that tinyOBJ will work

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <iomanip>
#include <random>
#include <math.h>

#include "tinyobjloader/tiny_obj_loader.h"
#include "headers/map.h"
#include "headers/sprites.h"
#include "glm/glm/gtc/type_ptr.hpp"


const double	PI = 2.0 * acos(0.0);


/**
 *	Constructor
 */
Sprites::Sprites(Map* map) {
	this->map = map;
}

/**
 *	Destructor
 */
Sprites::~Sprites() {

}

/**
 * Checks if game is done by either collecting all pellets or dying to ghost(s)
 */
bool Sprites::checkIfGameIsDone(bool ghostCollision) {
	if (Sprites::getMap()->getp_count() < 0 || ghostCollision) {
		speed = 0;
		return true;
	}
	else
		return false;
}

/**
 *	Checks collision with walls
 */
bool Sprites::checkWallCollision(float posX, float posY) {
    std::vector<std::vector<int>> mapArr = Sprites::getMap()->getMapArray();

    float spriteRadius = Sprites::getMap()->getTileSize() / 2.f;
    std::pair<int, int> nextTile1;
    std::pair<int, int> nextTile2;


	// Checks with each direction
    switch (direction)
    {
    case 'U':
        nextTile1 = coordsToTile(posX + (spriteRadius - 0.1f), posY + spriteRadius);
        nextTile2 = coordsToTile(posX - (spriteRadius - 0.1f), posY + spriteRadius);
        break;
    case 'D':
        nextTile1 = coordsToTile(posX + (spriteRadius - 0.1f), posY - spriteRadius);
        nextTile2 = coordsToTile(posX - (spriteRadius - 0.1f), posY - spriteRadius);
        break;
    case 'L':
        nextTile1 = coordsToTile(posX - spriteRadius, posY + (spriteRadius - 0.1f));
        nextTile2 = coordsToTile(posX - spriteRadius, posY - (spriteRadius - 0.1f));
        break;
    case 'R':
        nextTile1 = coordsToTile(posX + spriteRadius, posY + (spriteRadius - 0.1f));
        nextTile2 = coordsToTile(posX + spriteRadius, posY - (spriteRadius - 0.1f));
        break;
    default:
        nextTile1 = coordsToTile(posX, posY);
        nextTile2 = coordsToTile(posX, posY);
        break;
    }

	if (mapArr[nextTile1.second][nextTile1.first] == 1 || mapArr[nextTile2.second][nextTile2.first] == 1) {
		return true;
	}
	else if (posX < spriteRadius || posX > (Sprites::getMap()->getWidth() - spriteRadius - 0.1f)){
		return true; 
	}
    else {
        return false;
    }
}

/**
 *	Changes the x and y coords to tiles
 */
std::pair<int, int> Sprites::coordsToTile(float x, float y) {
	return std::pair<int, int> {x, Sprites::getMap()->getHeight() - y - 1};
}

/**
 *	Moves everything in the shader with offsetX and offsetY
 */
void Sprites::moveAllToShader(float offsetX, float offsetY, GLuint shaderprogram) {
	glUseProgram(shaderprogram);

	// Gets the variable in the shader which 'transforms' stuff (moves, scales, etc)
	GLuint projmat = glGetUniformLocation(shaderprogram, "u_TransformationMat");

	// Creates transformasjonsmatrix, translation
	glm::mat4 transformation = glm::translate(glm::mat4(1), glm::vec3(offsetX, offsetY, 0.f));

	// Updates transform-variable with our translation
	glUniformMatrix4fv(projmat, 1, false, glm::value_ptr(transformation));
}


//////////////////////////////////////////////////////////////////

/**
 *	Constructor
 */
Ghosts::Ghosts(Map* map, GLuint shader) : Sprites(map) {
	this->ghost_Shader = shader;
}

/**
 *	Destructor
 */
Ghosts::~Ghosts() {
	delete ghost_points;

	Sprites::getMap()->CleanVAO(potVAO);
}

/**
*	
*/
bool Ghosts::checkIfGameIsDone(bool ghostCollision) {
	return Sprites::checkIfGameIsDone(ghostCollision);
}

/**
 *	Draws the sprites
 */
void Ghosts::drawGhosts() {
	glBindVertexArray(potVAO);	// Tell the code which VAO to use
	glDrawArrays(GL_TRIANGLES, 6, getSize());
}

// -----------------------------------------------------------------------------
// Code handling the camera
// -----------------------------------------------------------------------------
GLuint Ghosts::LoadModel(const std::string path)
{

	//We create a vector of Vertex structs. OpenGL can understand these, and so will accept them as input.
	std::vector<Vertex> vertices;

	//Some variables that we are going to use to store data from tinyObj
	tinyobj::attrib_t attrib;
	std::vector<tinyobj::shape_t> shapes;
	std::vector<tinyobj::material_t> materials; //This one goes unused for now, seeing as we don't need materials for this model.

	//Some variables incase there is something wrong with our obj file
	std::string warn;
	std::string err;

	//We use tinobj to load our models. Feel free to find other .obj files and see if you can load them.
	tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, (path + "/snowman.obj").c_str(), path.c_str());

	if (!warn.empty()) {
		std::cout << warn << std::endl;
	}

	if (!err.empty()) {
		std::cerr << err << std::endl;
	}

	//For each shape defined in the obj file
	for (auto shape : shapes)
	{
		//We find each mesh
		for (auto meshIndex : shape.mesh.indices)
		{
			//And store the data for each vertice, including normals
			glm::vec3 vertice = {
				attrib.vertices[(meshIndex.vertex_index * 3) + 2],
				attrib.vertices[meshIndex.vertex_index * 3],
				attrib.vertices[(meshIndex.vertex_index * 3) + 1]
			};
			glm::vec3 normal = {
				attrib.normals[(meshIndex.normal_index * 3) + 2],
				attrib.normals[meshIndex.normal_index * 3],
				attrib.normals[(meshIndex.normal_index * 3) + 1]
			};
			glm::vec2 textureCoordinate = {                         //These go unnused, but if you want textures, you will need them.
				attrib.texcoords[meshIndex.texcoord_index * 2],
				attrib.texcoords[(meshIndex.texcoord_index * 2) + 1]
			};

			vertices.push_back({ vertice, normal, textureCoordinate }); //We add our new vertice struct to our vector

		}
	}

	GLuint VAO;
	glGenVertexArrays(1, &VAO);
	glBindVertexArray(VAO);

	GLuint VBO;
	glGenBuffers(1, &VBO);
	glBindBuffer(GL_ARRAY_BUFFER, VBO);

	//As you can see, OpenGL will accept a vector of structs as a valid input here
	glBufferData(GL_ARRAY_BUFFER, sizeof(Vertex) * vertices.size(), &vertices[0], GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, nullptr);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

	//This will be needed later to specify how much we need to draw. Look at the main loop to find this variable again.
	setSize(vertices.size());

	return VAO;
}

// -----------------------------------------------------------------------------
// Code handling Transformations
// -----------------------------------------------------------------------------
void Ghosts::Transform(
	const GLuint shaderprogram,
	const glm::vec3& translation,
	const float& radians,
	const glm::vec3& rotation_axis,
	const glm::vec3& scale
)
{

	//Presentation below purely for ease of viewing individual components of calculation, and not at all necessary.

	//Translation moves our object.        base matrix      Vector for movement along each axis
	glm::mat4 translate = glm::translate(glm::mat4(1), translation);

	//Rotate the object            base matrix      degrees to rotate   axis to rotate around
	glm::mat4 rotate = glm::rotate(glm::mat4(1), radians, rotation_axis);

	//Scale the object             base matrix      vector containing how much to scale along each axis (here the same for all axis)
	glm::mat4 scaling = glm::scale(glm::mat4(1), scale);

	//Create transformation matrix      These must be multiplied in this order, or the results will be incorrect
	glm::mat4 transformation = translate * rotate * scaling;


	//Get uniform to place transformation matrix in
	//Must be called after calling glUseProgram     shader program in use   Name of Uniform
	GLuint transformationmat = glGetUniformLocation(shaderprogram, "u_TransformationMat");

	//Send data from matrices to uniform
	//We also add a check to make sure that we found the location of the matrix before trying to write to it
	if (transformationmat != -1)
		//                     Location of uniform  How many matrices we are sending    value_ptr to our transformation matrix
		glUniformMatrix4fv(transformationmat, 1, false, glm::value_ptr(transformation));
}


// -----------------------------------------------------------------------------
// Code handling the Lighting
// -----------------------------------------------------------------------------
void Ghosts::Light(
	const GLuint shaderprogram,
	const glm::vec3 pos,
	const glm::vec3 color,
	const glm::mat4 light_Projection,
	const glm::vec3 look_at,
	const glm::vec3 up_vec,
	const float spec
)
{

	//Get uniforms for our Light-variables.
	GLuint lightPos = glGetUniformLocation(shaderprogram, "u_LightPosition");
	GLuint lightColor = glGetUniformLocation(shaderprogram, "u_LightColor");
	GLuint lightDir = glGetUniformLocation(shaderprogram, "u_LightDirection");
	GLuint specularity = glGetUniformLocation(shaderprogram, "u_Specularity");
	GLuint lightSpace = glGetUniformLocation(shaderprogram, "u_LightSpaceMat");

	//Make some computations that would be cumbersome to inline
	//Here we figure out the combination of the projection and viewmatrixes for the lightsource
	glm::mat4 lightLookat = glm::lookAt(pos, look_at, glm::vec3(0.0f, 1.f, 0.f));
	glm::mat4 lightspacematrix = light_Projection * lightLookat;

	//Send Variables to our shader
	if (lightPos != -1)
		glUniform3f(lightPos, pos.x, pos.y, pos.z);             //Position of a point in space. For Point lights.
	if (lightDir != -1)
		glUniform3f(lightDir, 0 - pos.x, 0 - pos.y, 0 - pos.z); //Direction vector. For Directional Lights.
	if (lightColor != -1)
		glUniform3f(lightColor, color.r, color.g, color.b);     //RGB values
	if (specularity != -1)
		glUniform1f(specularity, spec);                         //How much specular reflection we have for our object

	//Values for Shadow computation
	if (lightSpace != -1)
		glUniformMatrix4fv(lightSpace, 1, false, glm::value_ptr(lightspacematrix));
}


/**
 *	Initialises ghosts with all its values
 */
GLuint Ghosts::initGhost(time_t seed) {

	potVAO = LoadModel("../../../../assets/model");

	// Set random positions for the ghosts
	std::vector<std::vector<int>> checkArray = Sprites::getMap()->getMapArray();
	srand(seed);
	int ghostSpawnX, ghostSpawnY;
	do { // Gets random positions for the ghosts
		ghostSpawnX = rand() % Sprites::getMap()->getWidth();
		ghostSpawnY = rand() % Sprites::getMap()->getHeight();
	} while (checkArray[ghostSpawnY][ghostSpawnX] != 0);

	sprite_positions.push_back(getMap()->getScreenCoords(ghostSpawnX + 0.5f, ghostSpawnY - 0.5f));

	sprite_velX.push_back(0.f);
	sprite_velY.push_back(0.f);

	/**

	std::vector<std::vector<int>> checkArray = Sprites::getMap()->getMapArray();
	srand(seed);

	//for (int i = 0; i < ghost_amount; i++) {

		int ghostSpawnX, ghostSpawnY;
		do { // Gets random positions for the ghosts
			ghostSpawnX = rand() % Sprites::getMap()->getWidth();
			ghostSpawnY = rand() % Sprites::getMap()->getHeight();
		} while (checkArray[ghostSpawnY][ghostSpawnX] != 0);

		sprite_positions.push_back(getMap()->getScreenCoords(ghostSpawnX + 0.5f, ghostSpawnY - 0.5f));

		sprite_velX.push_back(0.f);
		sprite_velY.push_back(0.f);

		// Bottom Left
		ghost_points->push_back(-0.5f);
		ghost_points->push_back(-0.5f);
		ghost_points->push_back(1.f); ghost_points->push_back(1.f); ghost_points->push_back(0.f);
		ghost_points->push_back(4.f/6.f);
		ghost_points->push_back(1.f); // 4.f/4.f

		// Bottom Right
		ghost_points->push_back(-0.5f + Sprites::getMap()->getTileSize());
		ghost_points->push_back(-0.5f);
		ghost_points->push_back(1.f); ghost_points->push_back(1.f); ghost_points->push_back(0.f);
		ghost_points->push_back(5.f/6.f);
		ghost_points->push_back(1.f);

		// Top Right
		ghost_points->push_back(-0.5f + Sprites::getMap()->getTileSize());
		ghost_points->push_back(-0.5f + Sprites::getMap()->getTileSize());
		ghost_points->push_back(1.f); ghost_points->push_back(1.f); ghost_points->push_back(0.f);
		ghost_points->push_back(5.f / 6.f);
		ghost_points->push_back(3.f / 4.f);

		// Top Left
		ghost_points->push_back(-0.5f);
		ghost_points->push_back(-0.5f + Sprites::getMap()->getTileSize());
		ghost_points->push_back(1.f); ghost_points->push_back(1.f); ghost_points->push_back(0.f);
		ghost_points->push_back(4.f / 6.f);
		ghost_points->push_back(3.f/ 4.f);

	//}


	glEnableVertexAttribArray(0); //Enable Location = 0 (Vertices)
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 0));

	glEnableVertexAttribArray(1); //Enable Location = 1 (Normals)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3));

	glEnableVertexAttribArray(2); //Enable Location = 2 (UVs)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6));

	*/

	/**
	ghost_indices = new std::vector<unsigned int>;
	for (int i = 0; i < ghost_points->size(); i+= 5) {
		ghost_indices->push_back(0);
		ghost_indices->push_back(1);
		ghost_indices->push_back(3);

		ghost_indices->push_back(1);
		ghost_indices->push_back(2);
		ghost_indices->push_back(3);
	}

	glGenBuffers(1, &ghost_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ghost_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*ghost_indices), &(*ghost_indices)[0], GL_DYNAMIC_DRAW);
	*/
	glBindVertexArray(potVAO);
	glDrawArrays(GL_TRIANGLES, 6, getSize());
	return potVAO;
}

/**
 *	Initializes movement of a sprite
 */
void Ghosts::movement(GLFWwindow* window, double dt, bool gameStatus, time_t seed) {
	if (!checkIfGameIsDone(gameStatus)) {
		srand(seed);

		for (int i = 0; i < sprite_positions.size(); i++) {
			int random = rand() % 4;
			switch (random) {
			case 0:
				Sprites::setDirection('U');
				sprite_velX.at(i) = 0;
				sprite_velY.at(i) = Sprites::getSpeed();
				break;
			case 1:
				Sprites::setDirection('D');
				sprite_velX.at(i) = 0;
				sprite_velY.at(i) = -Sprites::getSpeed();
				break;
			case 2:
				Sprites::setDirection('R');
				sprite_velX.at(i) = Sprites::getSpeed();
				sprite_velY.at(i) = 0;
				break;
			case 3:
				Sprites::setDirection('L');
				sprite_velX.at(i) = -Sprites::getSpeed();
				sprite_velY.at(i) = 0;
				break;
			}

			if (!checkWallCollision(sprite_positions.at(i).first + sprite_velX.at(i) * dt, sprite_positions.at(i).second - 1.f + sprite_velY.at(i) * dt)) {
				sprite_positions.at(i).first += sprite_velX.at(i) * dt;
				sprite_positions.at(i).second += sprite_velY.at(i) * dt;
			}

			ghostAnimate();
		
			moveAllToShader(sprite_positions.at(i).first, sprite_positions.at(i).second, ghost_Shader);
		}
	}
}

/*
	Animate ghosts
*/
void Ghosts::ghostAnimate() {
	
	/* Finn en bedre måte å cleane på >:(
	Sprites::getMap()->CleanVAO(ghost_vao);
	
	// Change the texture coordinates based on the direction
	switch (Sprites::getDirection()) {
	case 'U': heightY = (2.f / 4.f); break;		// UP
	case 'D': heightY = (1.f / 4.f); break;		// DOWN
	case 'L': heightY = (3.f / 4.f); break;		// LEFT
	case 'R': heightY = (1); break;				// RIGHT
	}

	//Bottom left
	ghost_points->at(5) = widthX; ghost_points->at(6) = heightY;

	//Bottom Right
	ghost_points->at(12) = widthX + (1.f / 6.f); ghost_points->at(13) = heightY;

	//Top Right
	ghost_points->at(19) = widthX + (1.f / 6.f); ghost_points->at(20) = heightY - (1.f / 4.f);

	//Top Left
	ghost_points->at(26) = widthX; ghost_points->at(27) = heightY - (1.f / 4.f);


	glGenVertexArrays(1, &ghost_vao);
	glGenBuffers(1, &ghost_vbo);
	glBindVertexArray(this->ghost_vao);
	glBindBuffer(GL_ARRAY_BUFFER, ghost_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof_v(*ghost_points), &(*ghost_points)[0], GL_DYNAMIC_DRAW);

	// Set position into the Shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 2));

	// Texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 5));
	/**
	glGenBuffers(1, &ghost_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ghost_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*ghost_indices), &(*ghost_indices)[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(0);
	*/
}

////////////////////////////////////////////////////////////////////////

/**
 *	Constructor
 */
Pacman::Pacman(Map* map, GLuint shader) : Sprites(map) {
	this->pacman_Shader = shader;
}

/**
 *	Destructor
 */
Pacman::~Pacman() {
	delete pac_points;
	delete pac_indices;

	Sprites::getMap()->CleanVAO(pac_vao);
}

/**
 *	Checks collision with ghosts
 */
bool Pacman::checkGhostCollision(std::vector<Ghosts*> ghosts, float posX, float posY) {

	
	std::pair<int, int> pacmanTile = coordsToTile(posX, posY);

	for (int i = 0; i < ghosts.size(); i++) {
		std::pair<int, int> ghostTile = Sprites::coordsToTile(ghosts[i]->getGhostPos(0).first, ghosts[i]->getGhostPos(0).second);
		if (ghostTile == pacmanTile) {
			return true;
		}
	}
	return false;
}

/**
*
*/
bool Pacman::checkIfGameIsDone(bool ghostCollision) {
	return Sprites::checkIfGameIsDone(ghostCollision);
}

/**
 *	Checks collision with pellets
 */
bool Pacman::checkPelletCollision(std::pair<int, int> currentTile) {
	if (Sprites::getMap()->getPellet(currentTile.first, currentTile.second))
		return true;
	else
		return false;
}

/**
 *	Draws the sprites
 */
void Pacman::drawPacman() {
	glBindVertexArray(pac_vao);			// Tell the code which VAO to use 
	glDrawElements(GL_TRIANGLES, pac_indices->size(), GL_UNSIGNED_INT, 0);
}

/**
 *	Initialises pacman with all its values
 */
GLuint Pacman::initPacman() {
	glGenVertexArrays(1, &pac_vao);
	glBindVertexArray(pac_vao);

	pac_points = new std::vector<float>;

	//std::pair<float, float> pacPos = Sprites::getMap()->getScreenCoords(Sprites::getMap()->getStartX() + 0.5f, Sprites::getMap()->getStartY() - 0.5f);
	pacPos2 = Sprites::getMap()->getScreenCoords(Sprites::getMap()->getStartX() + 0.5f, Sprites::getMap()->getStartY() - 0.5f);

	// Bottom Left
	pac_points->push_back(-0.5f);
	pac_points->push_back(-0.5f);
	pac_points->push_back(0.f); pac_points->push_back(0.f); pac_points->push_back(0.f); // Color
	pac_points->push_back(0.f);					//nr 5 Texture coords X
	pac_points->push_back(1.f);					//nr 6 Y

	// Bottom Right
	pac_points->push_back(-0.5f + Sprites::getMap()->getTileSize());
	pac_points->push_back(-0.5f);
	pac_points->push_back(0.f); pac_points->push_back(0.f); pac_points->push_back(0.f); // Color
	pac_points->push_back(1.f / 6.f);			//nr 12 X
	pac_points->push_back(1.f);					//nr 13 Y

	// Top Right
	pac_points->push_back(-0.5f + Sprites::getMap()->getTileSize());
	pac_points->push_back(-0.5f + Sprites::getMap()->getTileSize());
	pac_points->push_back(0.f); pac_points->push_back(0.f); pac_points->push_back(0.f); // Color
	pac_points->push_back(1.f / 6.f);			//nr 19X
	pac_points->push_back(3.f / 4.f);			//nr 20Y

	// Top Left
	pac_points->push_back(-0.5f);
	pac_points->push_back(-0.5f + Sprites::getMap()->getTileSize());
	pac_points->push_back(0.f); pac_points->push_back(0.f); pac_points->push_back(0.f); // Color
	pac_points->push_back(0.f);					//nr 26 X
	pac_points->push_back(3.f / 4.f);			//nr 27 Y


	glGenVertexArrays(1, &pac_vao);
	glGenBuffers(1, &pac_vbo);
	glBindVertexArray(this->pac_vao);
	glBindBuffer(GL_ARRAY_BUFFER, pac_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof_v(*pac_points), &(*pac_points)[0], GL_DYNAMIC_DRAW);

	//Set position into the Shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 2));

	//Texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 5));

	pac_indices = new std::vector<unsigned int>;
	pac_indices->push_back(0);
	pac_indices->push_back(1);
	pac_indices->push_back(3);

	pac_indices->push_back(1);
	pac_indices->push_back(2);
	pac_indices->push_back(3);

	glGenBuffers(1, &pac_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pac_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*pac_indices), &(*pac_indices)[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(0);

	return (pac_vao);
}

/**
 *	Initializes movement of a sprite
 */
bool Pacman::movement(GLFWwindow* window, double dt, std::vector<Ghosts*> ghosts, bool gameStatus) {
	bool gameDone = gameStatus;
	char previousDir = ' ';
	findCameraDirection();
	//std::cout << Sprites::getDirection() << std::endl;
	if (!checkIfGameIsDone(gameDone)) {
		switch (Sprites::getViewDir()){			//Sets movement based on the direction the player is facing
		case 'U':								// U = UP (North), R = RIGHT (East), D = DOWN (South), L = LEFT (West)
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('D');
				if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f - Sprites::getSpeed() * dt)) {
					velX = 0;
					velY = -Sprites::getSpeed();
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('U');
				if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f + Sprites::getSpeed() * dt)) {
					velX = 0;
					velY = Sprites::getSpeed();
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('L');
				if (!checkWallCollision(pacPos2.first - Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
					velX = -Sprites::getSpeed();
					velY = 0;
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('R');
				if (!checkWallCollision(pacPos2.first + Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
					velX = Sprites::getSpeed();
					velY = 0;
				}
				else
					Sprites::setDirection(previousDir);
			}
			break;
		case 'D':
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('U');
				if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f + Sprites::getSpeed() * dt)) {
					velX = 0;
					velY = Sprites::getSpeed();
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('D');
				if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f - Sprites::getSpeed() * dt)) {
					velX = 0;
					velY = -Sprites::getSpeed();
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('R');
				if (!checkWallCollision(pacPos2.first + Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
					velX = Sprites::getSpeed();
					velY = 0;
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('L');
				if (!checkWallCollision(pacPos2.first - Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
					velX = -Sprites::getSpeed();
					velY = 0;
				}
				else
					Sprites::setDirection(previousDir);
			}
			break;
		case 'R':	
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('R');
				if (!checkWallCollision(pacPos2.first + Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
					velX = Sprites::getSpeed();
					velY = 0;
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('L');
				if (!checkWallCollision(pacPos2.first - Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
					velX = -Sprites::getSpeed();
					velY = 0;
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('D');
				if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f - Sprites::getSpeed() * dt)) {
					velX = 0;
					velY = -Sprites::getSpeed();
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('U');
				if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f + Sprites::getSpeed() * dt)) {
					velX = 0;
					velY = Sprites::getSpeed();
				}
				else
					Sprites::setDirection(previousDir);
			}
			break;
		case 'L':	
			if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('L');
				if (!checkWallCollision(pacPos2.first - Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
					velX = -Sprites::getSpeed();
					velY = 0;
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('R');
				if (!checkWallCollision(pacPos2.first + Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
					velX = Sprites::getSpeed();
					velY = 0;
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('U');
				if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f + Sprites::getSpeed() * dt)) {
					velX = 0;
					velY = Sprites::getSpeed();
				}
				else
					Sprites::setDirection(previousDir);
			}
			if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS) {
				previousDir = Sprites::getDirection();
				Sprites::setDirection('D');
				if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f - Sprites::getSpeed() * dt)) {
					velX = 0;
					velY = -Sprites::getSpeed();
				}
				else
					Sprites::setDirection(previousDir);
			}
			break;
		default:	break;
		}

		if (!checkWallCollision(pacPos2.first + velX * dt, pacPos2.second - 1.f + velY * dt)) {
			pacPos2.first += velX * dt;
			pacPos2.second += velY * dt;
		}
	
		std::pair<int, int> currentTile = coordsToTile(pacPos2.first, pacPos2.second - 1);
		if (checkPelletCollision(currentTile))
			Sprites::getMap()->deletePellet(currentTile);
		
		
		if (checkGhostCollision(ghosts, pacPos2.first, pacPos2.second))
					gameDone = true;
					

		gameDone = false;


		if (Sprites::getMovAni() == 30) {
			pacAnimate(); Sprites::setMovAni(0);
		}

		Sprites::setMovAni(Sprites::getMovAni() + 1);

		moveAllToShader(pacPos2.first, pacPos2.second, pacman_Shader);
	}

	return gameDone;
}

/**
 *	Animates pacman sprite
 */
void Pacman::pacAnimate() {

	Sprites::getMap()->CleanVAO(pac_vao);


	if (increaseStep == true) { Step++; }
	else if (increaseStep == false) { Step--; }

	widthX = (Step / 6.f);

	// Change the texture coordinates based on the direction
	switch (Sprites::getDirection()) {
	case 'U': heightY = (2.f / 4.f); break;		// UP
	case 'D': heightY = (1.f / 4.f); break;		// DOWN
	case 'L': heightY = (3.f / 4.f); break;		// LEFT
	case 'R': heightY = (1); break;				// RIGHT
	}
	if (Step == 3) { increaseStep = false; }
	else if (Step == 0) { increaseStep = true; }


	// Edit the vertices 5 & 6, 12 & 13, 19 & 20, 26 & 27
	// They are our texture coordinates in pac_points

	//Bottom left
	pac_points->at(5) = widthX; pac_points->at(6) = heightY;

	//Bottom Right
	pac_points->at(12) = widthX + (1.f / 6.f); pac_points->at(13) = heightY;

	//Top Right
	pac_points->at(19) = widthX + (1.f / 6.f); pac_points->at(20) = heightY - (1.f / 4.f);

	//Top Left
	pac_points->at(26) = widthX; pac_points->at(27) = heightY - (1.f / 4.f);


	glGenVertexArrays(1, &pac_vao);
	glGenBuffers(1, &pac_vbo);
	glBindVertexArray(this->pac_vao);
	glBindBuffer(GL_ARRAY_BUFFER, pac_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof_v(*pac_points), &(*pac_points)[0], GL_DYNAMIC_DRAW);

	// Set position into the Shader
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 0));

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 2));

	// Texture coordinates
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 5));

	glGenBuffers(1, &pac_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, pac_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*pac_indices), &(*pac_indices)[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(0);

}

/**
*	find and sets direction of camera
*/
void Pacman::findCameraDirection() {
	if		(cameraFront.x > -0.5f && cameraFront.x < 0.5f && cameraFront.y < 0) Sprites::setViewDirection('U');
	else if (cameraFront.x > -0.5f && cameraFront.x < 0.5f && cameraFront.y > 0) Sprites::setViewDirection('D');
	else if (cameraFront.y > -0.5f && cameraFront.y < 0.5f && cameraFront.x > 0) Sprites::setViewDirection('R');
	else if (cameraFront.y > -0.5f && cameraFront.y < 0.5f && cameraFront.x < 0) Sprites::setViewDirection('L');
}

/**
 *	Updates the firstperson camera on pacman
 */
void Pacman::updateCamera(double xpos, double ypos) {
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

	float sensitivity = 0.1f; // change this value to your liking
	xoffset *= sensitivity;
	yoffset *= sensitivity;

	yaw += xoffset;
	pitch += yoffset;

	// make sure that when pitch is out of bounds, screen doesn't get flipped
	if (pitch > 89.0f)
		pitch = 89.0f;
	if (pitch < -89.0f)
		pitch = -89.0f;

	glm::vec3 direction;
	direction.z = sin(glm::radians(pitch));
	direction.x = -cos(glm::radians(yaw)) * cos(glm::radians(pitch));
	direction.y = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
	cameraFront = glm::normalize(direction);

}