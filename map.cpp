#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <algorithm>
#include <iomanip>
#include <string>
#include <set>
#include <vector>

#include "headers/map.h"

const double	PI = 2.0*acos(0.0);


/**
 *	Constructor
 */
Map::Map(std::string filePath) {
	std::ifstream in(filePath);
	fromFile(in);
	initVerts();
	initPellets();
}

/**
 *	Destructor
 */
Map::~Map() {
	delete points;
	delete indices;

	delete p_points;
	delete p_indices;
	// Rydd opp i vao ebo etc...
	CleanVAO(vao);
	CleanVAO(p_vao);
}

// -----------------------------------------------------------------------------
// Clean VAO
// -----------------------------------------------------------------------------
void Map::CleanVAO(GLuint& vao)
{
	GLint nAttr = 0;
	std::set<GLuint> vbos;

	GLint eboId;
	glGetVertexArrayiv(vao, GL_ELEMENT_ARRAY_BUFFER_BINDING, &eboId);
	glDeleteBuffers(1, (GLuint*)&eboId);

	glGetIntegerv(GL_MAX_VERTEX_ATTRIBS, &nAttr);
	glBindVertexArray(vao);

	for (int iAttr = 0; iAttr < nAttr; ++iAttr)
	{
		GLint vboId = 0;
		glGetVertexAttribiv(iAttr, GL_VERTEX_ATTRIB_ARRAY_BUFFER_BINDING, &vboId);
		if (vboId > 0)
		{
			vbos.insert(vboId);
		}

		glDisableVertexAttribArray(iAttr);
	}

	for (auto vbo : vbos)
	{
		glDeleteBuffers(1, &vbo);
	}

	glDeleteVertexArrays(1, &vao);
}

/**
 *	Draws the map
 */
void Map::drawMap() { 
	glBindVertexArray(vao);				// Tell the code which VAO to use 
	glDrawElements(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, 0);

	glBindVertexArray(p_vao);			// Tell the code which VAO to use 
	glDrawElements(GL_TRIANGLES, 8 +p_indices->size(), GL_UNSIGNED_INT, 0);
}

/**
 *
 */
void Map::deletePellet(std::pair<int, int> position) {
	p_active[position.second][position.first] = false;
	p_count--;

	CleanVAO(p_vao);

	int range = p_slices * 15;
	int index = p_positions[position];

	for (int i = index; i < index + range; i++)
	{
		p_points->at(i) = 0.f;
	}

	glGenVertexArrays(1, &p_vao);
	glBindVertexArray(p_vao);

	glGenBuffers(1, &p_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, p_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof_v(*p_points), &(*p_points)[0], GL_STATIC_DRAW);

	// location=0 -> position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 0));

	// location=1 -> Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 2));

	// Element buffer object
	glGenBuffers(1, &p_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*p_indices), &(*p_indices)[0], GL_STATIC_DRAW);

}

/**
 *	Reads datainput from file
 */
void Map::fromFile(std::ifstream& in) {
	if (in) {
		in >> width; in.ignore(1); in >> height;

		GLuint temp = 0;

		for (int y = 0; y < height; y++) {
			std::vector<int> arr;
			std::vector<bool> p_activeArr;
			for (int x = 0; x < width; x++) {
				in >> temp;
				if (temp == 2) { startX = x; startY = y; }
				p_activeArr.push_back(true);
				arr.push_back(temp);
			}
			p_active.push_back(p_activeArr);
			mapArr.push_back(arr);
		}
	}
	else
		std::cout << "Couldnt find/ read from the file containing the map!" << std::endl;
}

/**
 *	Gets the screen-coordinates of a given tile
 *	@param tileX - The tile's X (ex. 0->28)
 *	@param tileY - The tile's Y (ex. 0->36)
 *	@return A pair containing the screen position's X and Y
 */
std::pair<float, float> Map::getScreenCoords(float tileX, float tileY) {
	return std::pair<float, float> { mapStartX + tileX,
									 mapStartY + (height - 1 - tileY) };
}

/**
 *	Initializes pellets
 *	(The map needs to be initialized first)
 *	@see Map::fromFile()
 *	@see Map::InitVerts()
 */
void Map::initPellets() {
	// Makes Vertex Array Object

	glGenVertexArrays(1, &p_vao);
	glBindVertexArray(p_vao);

	// Makes Vertex Buffer Object
	p_indices = new std::vector<unsigned int>;
	p_points = new std::vector<float>;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (mapArr[y][x] != 0) continue;

			p_count++;
			p_positions.insert(std::pair<std::pair<int, int>, int>({ x, y }, p_points->size()));

			std::pair<float, float> origo = getScreenCoords(x + (tileSize / 2.f), y - (tileSize / 2.f));

			float	degw = 2.f * PI / (float)p_slices;

			for (int i = 0; i < p_slices; i++) {
				float	deg0 = degw * (float)i,
					deg = degw * (float)(i + 1);

				p_points->push_back(origo.first);
				p_points->push_back(origo.second);
				p_points->push_back(1.f); p_points->push_back(1.f); p_points->push_back(0.f);

				p_points->push_back(origo.first + cos(deg0) * p_radius);
				p_points->push_back(origo.second + sin(deg0) * p_radius);
				p_points->push_back(1.f); p_points->push_back(1.f); p_points->push_back(0.f);

				p_points->push_back(origo.first + cos(deg) * p_radius);
				p_points->push_back(origo.second + sin(deg) * p_radius);
				p_points->push_back(1.f); p_points->push_back(1.f); p_points->push_back(0.f);

				p_indices->push_back(p_indices->size());
				p_indices->push_back(p_indices->size());
				p_indices->push_back(p_indices->size());
			}
		}
	}

	glGenBuffers(1, &p_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, p_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof_v(*p_points), &(*p_points)[0], GL_STATIC_DRAW);

	// location=0 -> position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 0));

	// location=1 -> Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float) * 2));

	// Element buffer object
	glGenBuffers(1, &p_ebo);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, p_ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*p_indices), &(*p_indices)[0], GL_STATIC_DRAW);
}

/**
 *	Initializes the walls in the game
 */
void Map::initVerts() {
	// Makes the Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);

	// Makes the Vertex Buffer Objecy
	points = new std::vector<float>;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			std::pair<float, float> botLeft = getScreenCoords(x, y);

			// Checks if index is a wall
			float isBlue = 0.f;
			if (mapArr[y][x] == 1) isBlue = 1.f;		

			/* Tile wall */
			// Bottom left
			points->push_back(botLeft.first);
			points->push_back(botLeft.second);
			points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);

			// Bottom right
			points->push_back(botLeft.first + tileSize);
			points->push_back(botLeft.second);
			points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);

			// Top right
			points->push_back(botLeft.first + tileSize);
			points->push_back(botLeft.second + tileSize);
			points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);

			// Top left
			points->push_back(botLeft.first);
			points->push_back(botLeft.second + tileSize);
			points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
		}
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof_v(*points), &(*points)[0], GL_STATIC_DRAW);

	// location=0 -> position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*0) );

	// location=1 -> Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 5, (void*)(sizeof(float)*2) );

	// Connects indices to points to make two triangles
	indices = new std::vector<unsigned int>;
	for (int i = 0; i < width * height; i++) {
		indices->push_back(i*4 + 0);	// i=1 => 4
		indices->push_back(i*4 + 1);	// i=1 => 5
		indices->push_back(i*4 + 3);	// i=1 => 7

		indices->push_back(i*4 + 1);	// i=1 => 5
		indices->push_back(i*4 + 2);	// i=1 => 6
		indices->push_back(i*4 + 3);	// i=1 => 7
	}

	// Element buffer object
	glGenBuffers(1, &ebo); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*indices), &(*indices)[0], GL_STATIC_DRAW);
}
