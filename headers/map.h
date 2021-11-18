#ifndef MAP_H // include guard
#define MAP_H
#include <fstream>
#include <vector>
#include <map>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>


class Map {
public:
	Map						(std::string filePath);
	~Map					();

	void CleanVAO			(GLuint& vao);
	void deletePellet		(std::pair<int, int> position);
	void drawPellets		();
	void drawMap			();
	void fromFile			(std::ifstream& in);
	void initPellets		();							// Initialiserer pellets
	void initVerts			();

	int	 getStartX			()							{ return startX;	}
	int  getStartY			()							{ return startY;	}
	int	 getWidth			()							{ return width;		}
	int  getHeight			()							{ return height;	}
	int  getp_count			()							{ return p_count;	}

	float getTileSize		()							{ return tileSize;	}

	bool getPellet			(int x, int y)				{ return p_active[y][x]; }

	std::pair<float,float> getScreenCoords(float tileX, float tileY);

	std::vector<std::vector<int>> getMapArray()			{ return mapArr; }

	template <typename T>
	int sizeof_v (std::vector <T> vec) { 
		return sizeof(std::vector<T>) + sizeof(T)*vec.size(); 
	}

private:
	int									height,
										width,
										startX,
										startY;
	float								mapStartX	= 0.f,
										mapStartY	= 0.f,
										tileSize	= 1.f;
	std::vector<std::vector<int>>		mapArr;

	GLuint								vbo,
										vao,
										ebo;
	std::vector<float>*					points;
	std::vector<unsigned int>*			indices;

	int									p_slices	= 10,		// Amount of 'slices' that a pellet's circle contains
										p_count		= 0;		// Counts total pellets created
	float								p_radius	= 0.25f;	// The radius of a pellet
	GLuint								p_vbo,
										p_vao,
										p_ebo;
	std::vector<float>*					p_points;
	std::vector<unsigned int>*			p_indices;
	std::vector<std::vector<bool>>		p_active;
	std::map<std::pair<int, int>, int>	p_positions;
};

#endif /* MAP_H */