#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <string>
#include <vector>
#include <iomanip>
#include <random>
#include <math.h>

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
	delete ghost_indices;

	Sprites::getMap()->CleanVAO(ghost_vao);
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
	glBindVertexArray(ghost_vao);	// Tell the code which VAO to use
	glDrawElements(GL_TRIANGLES, ghost_indices->size(), GL_UNSIGNED_INT, 0);
}

/**
 *	Initialises ghosts with all its values
 */
GLuint Ghosts::initGhost(time_t seed) {
	glGenVertexArrays(1, &ghost_vao);
	glBindVertexArray(ghost_vao);

	ghost_points = new std::vector<float>;

	std::vector<std::vector<int>> checkArray = Sprites::getMap()->getMapArray();
	std::pair<float, float> ghostPos;
	srand(seed);

	//for (int i = 0; i < ghost_amount; i++) {
		do {		// Gets random positions for the ghosts
			ghostPos = Sprites::getMap()->getScreenCoords((rand() % Sprites::getMap()->getWidth())+0.5f, (rand() % Sprites::getMap()->getHeight())-0.5f);
		} while (checkArray[floor(ghostPos.second)][floor(ghostPos.first)] != 0);
		sprite_positions.push_back(ghostPos);
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

	glGenBuffers(1, &ghost_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, ghost_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof_v(*ghost_points), &(*ghost_points)[0], GL_DYNAMIC_DRAW);

	glEnableVertexAttribArray(0); //Enable Location = 0 (Point position)
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 0));

	glEnableVertexAttribArray(1); //Enable Location = 1 (Color)
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 2));

	glEnableVertexAttribArray(2); //Enable Location = 2 (Tex position)
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 7, (void*)(sizeof(float) * 5));

	ghost_indices = new std::vector<unsigned int>;
	//for (int i = 0; i < ghost_amount; i++) {
		ghost_indices->push_back(0);
		ghost_indices->push_back(1);
		ghost_indices->push_back(3);

		ghost_indices->push_back(1);
		ghost_indices->push_back(2);
		ghost_indices->push_back(3);
	//}

	glGenBuffers(1, &ghost_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ghost_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*ghost_indices), &(*ghost_indices)[0], GL_DYNAMIC_DRAW);

	
	return ghost_vao;
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

	glGenBuffers(1, &ghost_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ghost_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*ghost_indices), &(*ghost_indices)[0], GL_DYNAMIC_DRAW);
	glBindVertexArray(0);

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
	if (!checkIfGameIsDone(gameDone)) {
		if (glfwGetKey(window, GLFW_KEY_UP) == GLFW_PRESS) {
			previousDir = Sprites::getDirection();
			Sprites::setDirection('U');
			if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f + Sprites::getSpeed() * dt)) {
				velX = 0;
				velY = Sprites::getSpeed();
			}
			else
				Sprites::setDirection(previousDir);
		}
		if (glfwGetKey(window, GLFW_KEY_DOWN) == GLFW_PRESS) {
			previousDir = Sprites::getDirection();
			Sprites::setDirection('D');
			if (!checkWallCollision(pacPos2.first + 0 * dt, pacPos2.second - 1.f - Sprites::getSpeed() * dt)) {
				velX = 0;
				velY = -Sprites::getSpeed();
			}
			else
				Sprites::setDirection(previousDir);
		}
		if (glfwGetKey(window, GLFW_KEY_RIGHT) == GLFW_PRESS) {
			previousDir = Sprites::getDirection();
			Sprites::setDirection('R');
			if (!checkWallCollision(pacPos2.first + Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
				velX = Sprites::getSpeed();
				velY = 0;
			}
			else
				Sprites::setDirection(previousDir);
		}
		if (glfwGetKey(window, GLFW_KEY_LEFT) == GLFW_PRESS) {
			previousDir = Sprites::getDirection();
			Sprites::setDirection('L');
			if (!checkWallCollision(pacPos2.first - Sprites::getSpeed() * dt, pacPos2.second - 1.f + 0 * dt)) {
				velX = -Sprites::getSpeed();
				velY = 0;
			}
			else
				Sprites::setDirection(previousDir);
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

