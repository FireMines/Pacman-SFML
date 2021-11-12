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

static const std::string mapFragmentShaderSrc = R"(
#version 430 core

// in vec4	vColor;
in vec2		TexCoords;

out vec4	FragColor;

uniform		sampler2D image;

void main() {		
		// FragColor = vColor;	
		vec4 ColorTest = texture(image, TexCoords);
		FragColor = ColorTest;
}
)";


#endif // __SQUARE_H_
