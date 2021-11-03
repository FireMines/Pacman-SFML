#ifndef SPRITES_H
#define SPRITES_H
#include <fstream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>


class Sprites {
private:
	float								speed = 5.f;
	Map* map;
	int									movementAnimation = 0;
	char								direction = ' ';
public:
	Sprites						(Map* map);
	~Sprites					();

	virtual bool checkIfGameIsDone		(bool ghostCollision);
	bool checkWallCollision		(float posX, float posY);

	Map*  getMap()					{ return map; }
	float getSpeed()				{ return speed; }
	int	  getMovAni()				{ return movementAnimation; }
	void  setDirection(char dir)	{ direction = dir; }
	void  setMovAni(int newMovAni)	{ movementAnimation = newMovAni; }
	char getDirection()				{ return direction; }

	void moveAllToShader(float offsetX, float offsetY, GLuint shaderprogram);

	std::pair<int, int> Sprites::coordsToTile(float x, float y);

	template <typename T>
	int sizeof_v(std::vector <T> vec) { 
		return sizeof(std::vector<T>) + sizeof(T) * vec.size(); 
	}

};


class Ghosts : public Sprites {
private:
	GLuint								ghost_amount = 1,
										ghost_Shader,
										ghost_vbo,
										ghost_vao,
										ghost_ebo;
	std::vector<float>*					ghost_points;
	std::vector<unsigned int>*			ghost_indices;

	std::vector<std::pair<float, float>>sprite_positions;
	std::vector<float>					sprite_velX;
	std::vector<float>					sprite_velY;

	bool								increaseStep = true;
	int									Step = 0;
	float								widthX = (4.f / 6.f),	// Each sprite i divided into 6ths on the X axis
										heightY = (1.f / 4.f);	// divided into 4ths on the Y axis

public:
	Ghosts(Map* map, GLuint shader);
	~Ghosts();

	int getGhostAmount()						{ return ghost_amount; }
	std::pair<float, float> getGhostPos(int nr) { return sprite_positions[nr]; }

	virtual bool checkIfGameIsDone(bool ghostCollision);
	void		 drawGhosts();
	GLuint		 initGhost(time_t seed);
	virtual void movement(GLFWwindow* window, double dt, bool gameStatus, time_t seed);
	void         ghostAnimate();
};

class Pacman : public Sprites {
private:
	GLuint								pac_vbo,
										pac_vao,
										pac_ebo,
										pacman_Shader;

	std::vector<float>*					pac_points;
	std::vector<unsigned int>*			pac_indices;
	std::pair<float, float>				pacPos2;

	int									Step = 0;
	float								widthX = (1.f / 6.f),	// Each sprite i divided into 6ths on the X axis
										heightY = (1.f / 4.f),	// divided into 4ths on the Y axis
										velX = 0.f,
										velY = 0.f;
	bool								increaseStep = true;

public:
	Pacman(Map* map, GLuint shader);
	~Pacman();

	void drawPacman();
	void pacAnimate();
	virtual bool movement(GLFWwindow* window, double dt, std::vector<Ghosts*> ghosts, bool gameStatus);

	bool checkGhostCollision(std::vector<Ghosts*> ghosts, float posX, float posY);
	virtual bool checkIfGameIsDone(bool ghostCollision);
	bool checkPelletCollision(std::pair<int, int> currentTile);

	GLuint initPacman();

};
#endif // !sprites_h



