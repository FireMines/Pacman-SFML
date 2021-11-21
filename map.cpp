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

/*
*	Draw the Pellets
*/
void Map::drawPellets() {
	glBindVertexArray(p_vao);			// Tell the code which VAO to use 
	glDrawArrays(GL_POINTS, 0 , p_points->size() / 6);
}

/**
 *	Draws the map
 */
void Map::drawMap() { 
	glBindVertexArray(vao);				// Tell the code which VAO to use 
	glDrawElements(GL_TRIANGLES, indices->size(), GL_UNSIGNED_INT, 0);
}

/**
 *
 */
void Map::deletePellet(std::pair<int, int> position) {
	p_active[position.second][position.first] = false;
	p_count--;

	//std::cout << p_count << std::endl;

	int range = 6;
	int index = p_positions[position];

	/*CleanVAO(p_vao);

	for (int i = index; i < index + range; i++)
	{
		p_points->at(i) = 0.f;
	}

	glGenVertexArrays(1, &p_vao);
	glBindVertexArray(p_vao);

	// location=0 -> position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 0));

	// location=1 -> Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));
	*/
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
	float levitationHeight = 0.5f;

	// Makes Vertex Buffer Object
	p_points = new std::vector<float>;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			if (mapArr[y][x] != 0) continue;

			p_count++;
			p_positions.insert(std::pair<std::pair<int, int>, int>({ x, y }, p_points->size()));

			std::pair<float, float> origo = getScreenCoords(x + (tileSize / 2.f), y - (tileSize / 2.f));

			p_points->push_back(origo.first);
			p_points->push_back(origo.second);
			p_points->push_back(levitationHeight);
			p_points->push_back(1.f); p_points->push_back(1.f); p_points->push_back(0.f);

			/*float	degw = 2.f * PI / (float)p_slices;

			for (int i = 0; i < p_slices; i++) {
				float	deg0 = degw * (float)i,
					deg = degw * (float)(i + 1);

				p_points->push_back(origo.first);
				p_points->push_back(origo.second);
				p_points->push_back(levitationHeight);
				p_points->push_back(1.f); p_points->push_back(1.f); p_points->push_back(0.f);
				p_points->push_back(0.f); p_points->push_back(0.f);

				p_points->push_back(origo.first + cos(deg0) * p_radius);
				p_points->push_back(origo.second + sin(deg0) * p_radius);
				p_points->push_back(levitationHeight);
				p_points->push_back(1.f); p_points->push_back(1.f); p_points->push_back(0.f);
				p_points->push_back(0.f); p_points->push_back(0.f);

				p_points->push_back(origo.first + cos(deg) * p_radius);
				p_points->push_back(origo.second + sin(deg) * p_radius);
				p_points->push_back(levitationHeight);
				p_points->push_back(1.f); p_points->push_back(1.f); p_points->push_back(0.f);
				p_points->push_back(0.f); p_points->push_back(0.f);

				p_indices->push_back(p_indices->size());
				p_indices->push_back(p_indices->size());
				p_indices->push_back(p_indices->size());
			}*/
		}
	}

	glGenBuffers(1, &p_vbo);
	glBindBuffer(GL_ARRAY_BUFFER, p_vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof_v(*p_points), &(*p_points)[0], GL_STATIC_DRAW);

	// location=0 -> position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 0));

	// location=1 -> Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 6, (void*)(sizeof(float) * 3));


}

/**
 *	Initializes the walls in the game
 */
void Map::initVerts() {
	// Makes the Vertex Array Object
	glGenVertexArrays(1, &vao);
	glBindVertexArray(vao);
	float wallHeight = 2.5f;
	int i = 0;
	indices = new std::vector<unsigned int>;

	// Makes the Vertex Buffer Objecy
	points = new std::vector<float>;

	for (int y = 0; y < height; y++) {
		for (int x = 0; x < width; x++) {
			std::pair<float, float> botLeft = getScreenCoords(x, y);

			// Checks if index is a wall
			float isBlue = 0.f;
			if (mapArr[y][x] == 1) isBlue = 1.f;		

			/* Tile wall */
			/*
			// South Left FLOOR
			points->push_back(botLeft.first); points->push_back(botLeft.second); points->push_back(0.f);	//X, Y and Z Coordinates
			points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);						//RGB
			points->push_back(0.f);	points->push_back(0.f);													//Tex coords

			// South Right FLOOR
			points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second); points->push_back(0.f);
			points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
			points->push_back(0.f);	points->push_back(0.f);

			// North Left FLOOR
			points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second + tileSize); points->push_back(0.f);
			points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
			points->push_back(0.f);	points->push_back(0.f);

			// North Right FLOOR
			points->push_back(botLeft.first); points->push_back(botLeft.second + tileSize); points->push_back(0.f);
			points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
			points->push_back(0.f);	points->push_back(0.f);

			// -----FLOOR TILES INDICES-----

			indices->push_back(i + 0);
			indices->push_back(i + 1);	
			indices->push_back(i + 3);	

			indices->push_back(i + 1);
			indices->push_back(i + 2);	
			indices->push_back(i + 3);
			i += 4;
			*/
			// DRAW WALLS IF IT IS A WALL TILE
			if (isBlue == 1.0f) {

				// South
				// South FLOOR R
				points->push_back(botLeft.first); points->push_back(botLeft.second); points->push_back(0.f);	//X, Y and Z Coordinates
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);						//RGB
				points->push_back(1.f);	points->push_back(1.f);													//Tex coords

				// South FLOOR L
				points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second); points->push_back(0.f);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(0.f);	points->push_back(1.f);

				// South ROOF R 
				points->push_back(botLeft.first); points->push_back(botLeft.second); points->push_back(wallHeight);	//X, Y and Z Coordinates
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);							//RGB
				points->push_back(1.f);	points->push_back(0.f);														//Tex coords

				// South ROOF L
				points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second); points->push_back(wallHeight);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(0.f);	points->push_back(0.f);

				// South Indices
				indices->push_back(i + 0);
				indices->push_back(i + 1);
				indices->push_back(i + 2);

				indices->push_back(i + 1);
				indices->push_back(i + 2);
				indices->push_back(i + 3);
				i += 4;

				// North
				// North Left FLOOR
				points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second + tileSize); points->push_back(0.f);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(1.f);	points->push_back(1.f);

				// North Right FLOOR
				points->push_back(botLeft.first); points->push_back(botLeft.second + tileSize); points->push_back(0.f);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(0.f);	points->push_back(1.f);

				// North Left ROOF
				points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second + tileSize); points->push_back(wallHeight);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(1.f);	points->push_back(0.f);

				// North Right ROOF
				points->push_back(botLeft.first); points->push_back(botLeft.second + tileSize); points->push_back(wallHeight);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(0.f);	points->push_back(0.f);

				// North Indices
				indices->push_back(i + 0);
				indices->push_back(i + 1);
				indices->push_back(i + 2);

				indices->push_back(i + 1);
				indices->push_back(i + 2);
				indices->push_back(i + 3);
				i += 4;

				// East
				// South FLOOR L
				points->push_back(botLeft.first); points->push_back(botLeft.second); points->push_back(0.f);	//X, Y and Z Coordinates
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);						//RGB
				points->push_back(1.f);	points->push_back(1.f);													//Tex coords

				// North Right FLOOR
				points->push_back(botLeft.first); points->push_back(botLeft.second + tileSize); points->push_back(0.f);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(0.f);	points->push_back(1.f);

				// South ROOF L 
				points->push_back(botLeft.first); points->push_back(botLeft.second); points->push_back(wallHeight);	//X, Y and Z Coordinates
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);							//RGB
				points->push_back(1.f);	points->push_back(0.f);														//Tex coords

				// North Right ROOF
				points->push_back(botLeft.first); points->push_back(botLeft.second + tileSize); points->push_back(wallHeight);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(0.f);	points->push_back(0.f);

				// East Indices
				indices->push_back(i + 0);
				indices->push_back(i + 1);
				indices->push_back(i + 2);

				indices->push_back(i + 1);
				indices->push_back(i + 2);
				indices->push_back(i + 3);
				i += 4;

				// West
				// South FLOOR L
				points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second); points->push_back(0.f);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(1.f);	points->push_back(1.f);

				// North Left FLOOR
				points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second + tileSize); points->push_back(0.f);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(0.f);	points->push_back(1.f);

				// South ROOF L
				points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second); points->push_back(wallHeight);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(1.f);	points->push_back(0.f);

				// North Left ROOF
				points->push_back(botLeft.first + tileSize); points->push_back(botLeft.second + tileSize); points->push_back(wallHeight);
				points->push_back(0.f); points->push_back(0.f); points->push_back(isBlue);
				points->push_back(0.f);	points->push_back(0.f);

				// West Indices
				indices->push_back(i + 0);
				indices->push_back(i + 1);
				indices->push_back(i + 2);

				indices->push_back(i + 1);
				indices->push_back(i + 2);
				indices->push_back(i + 3);
				i += 4;
			}
		}
	}

	glGenBuffers(1, &vbo);
	glBindBuffer(GL_ARRAY_BUFFER, vbo);
	glBufferData(GL_ARRAY_BUFFER, sizeof_v(*points), &(*points)[0], GL_STATIC_DRAW);

	// location=0 -> position
	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 0) );

	// location=1 -> Color
	glEnableVertexAttribArray(1);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 3) );

	// location = 2 -> textures
	glEnableVertexAttribArray(2);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(float) * 8, (void*)(sizeof(float) * 6) );

	/*
	// Connects indices to points to make two triangles
	indices = new std::vector<unsigned int>;
	for (int i = 0; i < width * height; i++) {
		indices->push_back(i*4 + 0);	// i=1 => 4
		indices->push_back(i*4 + 1);	// i=1 => 5
		indices->push_back(i*4 + 2);	// i=1 => 7

		indices->push_back(i*4 + 1);	// i=1 => 5
		indices->push_back(i*4 + 2);	// i=1 => 6
		indices->push_back(i*4 + 3);	// i=1 => 7
	}*/

	// Element buffer object
	glGenBuffers(1, &ebo); 
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof_v(*indices), &(*indices)[0], GL_STATIC_DRAW);
}
