#ifndef SPRITES_H
#define SPRITES_H
#include <fstream>
#include <vector>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>



class Sprites {
private:
	float								speed = 5.f;
	Map* map;
	int									movementAnimation = 20;
	char								direction = 'U';
	char								directionView = 'U';
public:
	Sprites						(Map* map);
	~Sprites					();

	virtual bool checkIfGameIsDone		(bool ghostCollision);
	bool checkWallCollision		(float posX, float posY);

	Map*  getMap()					{ return map; }
	float getSpeed()				{ return speed; }
	int	  getMovAni()				{ return movementAnimation; }
	char  getDirection()			{ return direction; }
	char  getViewDir()				{ return directionView; }
	void  setDirection(char dir)	{ direction = dir; }
	void  setViewDirection(char dir){ directionView = dir; }
	void  setMovAni(int newMovAni)	{ movementAnimation = newMovAni; }
	

	void moveAllToShader(float offsetX, float offsetY, const float& radians, GLuint shaderprogram);

	std::pair<int, int> Sprites::coordsToTile(float x, float y);

	template <typename T>
	int sizeof_v(std::vector <T> vec) { 
		return sizeof(std::vector<T>) + sizeof(T) * vec.size(); 
	}

};


class Ghosts : public Sprites {
private:
	GLuint								ghost_Shader,
										ghost_vbo,
										potVAO;

	std::vector<float>*					ghost_points;

	std::vector<std::pair<float, float>>sprite_positions;
	std::vector<float>					sprite_velX;
	std::vector<float>					sprite_velY;

	bool								increaseStep = true;
	int									Step = 0;
	int									size = 0;
	

//------------------------------------------------------------------------------
// VERTEX STRUCT
//------------------------------------------------------------------------------
	struct Vertex
	{
		glm::vec3 location;
		glm::vec3 normals;
		glm::vec2 texCoords;
	};

public:
	Ghosts(Map* map, GLuint shader);
	~Ghosts();

	std::pair<float, float> getGhostPos(int nr) { return sprite_positions[nr]; }

	int			 getSize() { return size; }
	void		 setSize(int newSize) { size = newSize; }
	void		 drawGhosts();
	GLuint		 initGhost(time_t seed);
	GLuint		 LoadModel(const std::string path);
	GLuint		 setpotVAO(GLuint modelFunction) { potVAO = modelFunction; }
	virtual void movement(GLFWwindow* window, double dt, bool gameStatus, time_t seed);
	virtual bool checkIfGameIsDone(bool ghostCollision);

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

	int									Step			= 0;
	float								widthX			= (1.f / 6.f),	// Each sprite i divided into 6ths on the X axis
										heightY			= (1.f / 4.f),	// divided into 4ths on the Y axis
										velX			= 0.f,
										velY			= 0.f;
	bool								increaseStep	= true;
	bool								firstMouse		= true;

	float								yaw				= 180.0f;    // yaw is initialized to -90.0 degrees since a yaw of 0.0 results in a direction vector pointing to the right so we initially rotate a bit to the left.
	float								pitch			= 0.0f;
	float								lastX			= 0;
	float								lastY			= 0;
	float								fov				= 40.0f;
	glm::vec3							cameraFront		= glm::vec3(0.f, 0.f, 50.f);

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

	void updateCamera(double xpos, double ypos);
	void findCameraDirection();
	glm::vec3 getCameraFront() { return cameraFront; }
	std::pair<float, float> getPacPos() { return pacPos2; }

};
#endif // !sprites_h



