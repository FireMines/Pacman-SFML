#ifndef __SQUARE_H_
#define __SQUARE_H_

#include <string>

static const std::string spriteVertexShaderSrc = R"(
#version 430 core

layout (location = 0) in vec2 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 inTexCoords;

/**Matixer trengt til kamera og transformasjoner*/
uniform mat4 u_TransformationMat = mat4(1);
uniform mat4 u_ViewMat           = mat4(1);
uniform mat4 u_ProjectionMat     = mat4(1);

out vec4 vColor;
out vec2 TexCoords;

void main() {
	/**Posisjon basert på transforamtions of kamera*/
	gl_Position = u_ProjectionMat * u_ViewMat * u_TransformationMat * vec4(aPos, 0.0f, 1.0f);
	vColor		= vec4(aColor, 1.0f);
	TexCoords	= inTexCoords;
}
)";

static const std::string spriteGeometryShaderSrc = R"(
#version 430 core

layout (points) in;
layout (points, max_verticies = 1) out;

void main(){
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	EndPrimitive();
}
)";

static const std::string spriteFragmentShaderSrc = R"(
#version 430 core

in vec4		vColor;
in vec2		TexCoords;
out vec4	FragColor;

uniform		sampler2D image;

void main() {
		
		vec4 colorTest = texture(image, TexCoords);
		if(colorTest.a < 0.1) discard;
		else{FragColor = colorTest;}
		
	}
	)";

static const std::string mapVertexShaderSrc = R"(
#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;
layout (location = 2) in vec2 inTexCoords;

/**Matixer trengt til kamera og transformasjoner*/
uniform mat4 u_TransformationMat = mat4(1);
uniform mat4 u_ViewMat           = mat4(1);
uniform mat4 u_ProjectionMat     = mat4(1);

out vec4 vColor;
out vec2 TexCoords;

void main() {
	/**Posisjon basert på transformations av kamera*/
	gl_Position = u_ProjectionMat * u_ViewMat * u_TransformationMat * vec4(aPos, 1.0f);
	vColor		= vec4(aColor, 1.0f);
	TexCoords	= inTexCoords;
	
}
)";

static const std::string mapGeometryShaderSrc = R"(
#version 430 core

layout (points) in;
layout (points, max_verticies = 1) out;

void main(){
	gl_Position = gl_in[0].gl_Position;
	EmitVertex();
	EndPrimitive();
}
)";

static const std::string mapFragmentShaderSrc = R"(
#version 430 core

in vec4		vColor;
in vec2		TexCoords;

out vec4	FragColor;

uniform		sampler2D image;

void main() {		
		vec4 ColorTest = texture(image, TexCoords);
		FragColor = ColorTest;
		
}
)";

static const std::string pelletVertexShaderSrc = R"(
#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColor;

/**Matixer trengt til kamera og transformasjoner*/
uniform mat4 u_TransformationMat = mat4(1);
uniform mat4 u_ViewMat           = mat4(1);
uniform mat4 u_ProjectionMat     = mat4(1);

out vec4 vColor;
out mat4 vTransformation;

void main() {
	/**Posisjon basert på transformations av kamera*/
	gl_Position = vec4(aPos, 1.0f);
	vColor		= vec4(aColor, 1.0f);
	
	vTransformation = u_ProjectionMat * u_ViewMat * u_TransformationMat;
}
)";

static const std::string pelletGeometryShaderSrc = R"(
#version 430 core

layout (points) in;
layout (triangle_strip, max_vertices=384) out;

in vec4 vColor[];
in mat4 vTransformation[];

out vec3 gNormals;
out vec4 gColor;

void main(){
	gColor = vColor[0];
	mat4 gTransform = vTransformation[0];

	float PI = 3.14159265358979323846264;  

    vec4 pos = gl_in[0].gl_Position;  //introduce a single vertex at the origin

	float x, y, z, xy; 
	float radius = 0.05f;
	float nx, ny, nz, lengthInv = 1.0f / radius;   

	int sectorCount = 8;
	int stackCount = 8;

	float sectorStep = 2 * PI / sectorCount;
	float stackStep = PI / stackCount;
	float sectorAngle, stackAngle;

	vec3 spherePos[8][8];
	vec3 sphereNormals[8][8];

	for(int i = 0; i < stackCount; ++i)
	{
		stackAngle = PI / 2 - i * stackStep;        // starting from pi/2 to -pi/2
		xy = radius * cos(stackAngle);             // r * cos(u)
		z = radius * sin(stackAngle);              // r * sin(u)

		// add (sectorCount+1) vertices per stack
		// the first and last vertices have same position and normal, but different tex coords
		for(int j = 0; j <= sectorCount; ++j)
		{
			sectorAngle = j * sectorStep;           // starting from 0 to 2pi

			// vertex position (x, y, z)
			x = xy * cos(sectorAngle);             // r * cos(u) * cos(v)
			y = xy * sin(sectorAngle);             // r * cos(u) * sin(v)
			spherePos[j][i] = vec3(x, y, z);

			// normalized vertex normal (nx, ny, nz)
			nx = x * lengthInv;
			ny = y * lengthInv;
			nz = z * lengthInv;
			sphereNormals[j][i] = vec3(nx, ny, nz);
		}
	}
	for(int i = 0; i < stackCount; i++){
		for(int j = 0; j <= sectorCount; j++){
			vec3 v0 = spherePos[j][i], 
				 v1 = spherePos[j+1][i], 
				 v2 = spherePos[j+1][i+1], 
				 v3 = spherePos[j][i+1];

			gl_Position = gTransform * vec4(v0.x+pos.x, v0.y+pos.y, v0.z+pos.z, pos.w);
			EmitVertex();
			gl_Position = gTransform * vec4(v1.x+pos.x, v1.y+pos.y, v1.z+pos.z, pos.w);
			EmitVertex();
			gl_Position = gTransform * vec4(v2.x+pos.x, v2.y+pos.y, v2.z+pos.z, pos.w);
			EmitVertex();

			gl_Position = gTransform * vec4(v0.x+pos.x, v0.y+pos.y, v0.z+pos.z, pos.w);
			EmitVertex();
			gl_Position = gTransform * vec4(v2.x+pos.x, v2.y+pos.y, v2.z+pos.z, pos.w);
			EmitVertex();
			gl_Position = gTransform * vec4(v3.x+pos.x, v3.y+pos.y, v3.z+pos.z, pos.w);
			EmitVertex();
		
			
		}
	} 

	EndPrimitive();
}
)";

static const std::string pelletFragmentShaderSrc = R"(
#version 430 core

in vec4		gColor;
in vec3		gNormals;

vec3 ambientColor = vec3(1.f, 1.f, 0.f);

out vec4	FragColor;

void main() {

		//ambient lighting		
		float ambientStrength = 0.1f;
		vec3 ambient = ambientStrength * ambientColor;
	
		FragColor = vec4(ambient, 1.f) * gColor;	
}
)";

static const std::string modelVertexShaderSrc = R"(
#version 430 core

layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 inTexCoords;

/**Matixer trengt til kamera og transformasjoner*/
uniform mat4 u_TransformationMat = mat4(1);
uniform mat4 u_ViewMat           = mat4(1);
uniform mat4 u_ProjectionMat     = mat4(1);

out vec2 TexCoords;

void main() {
	/**Posisjon basert på transforamtions of kamera*/
	gl_Position = u_ProjectionMat * u_ViewMat * u_TransformationMat * vec4(aPos, 1.0f);
	TexCoords	= inTexCoords;
}
)";


static const std::string modelFragmentShaderSrc = R"(
#version 430 core

in vec2		TexCoords;
out vec4	FragColor;

uniform		sampler2D image;

void main() {
		
		vec4 colorTest = texture(image, TexCoords);
		if(colorTest.a < 0.1) discard;
		else{FragColor = colorTest;}
		
}
)";

#endif // __SQUARE_H_

