#ifndef __OBJLOADER_H_
#define __OBJLOADER_H_
#include <fstream>
#include <vector>
#include <glm/glm.hpp>

class OBJLoader {
private:
public:
    OBJLoader() {}
    ~OBJLoader() {}

    bool tester();

    bool loadOBJ(
        const char* path,
        std::vector < glm::vec3 >& out_vertices,
        std::vector < glm::vec2 >& out_uvs,
        std::vector < glm::vec3 >& out_normals
    );
};

#endif