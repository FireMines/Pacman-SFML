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
layout (location = 1) in vec3 aNormals;
layout (location = 2) in vec2 inTexCoords;

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

static const std::string VertexShaderSrc = R"(
#version 430 core

layout(location = 0) in vec3 a_Position;
layout(location = 1) in vec3 a_normals;
//layout(location = 2) in vec2 a_texture; Incase we want to add textures to our model later.

//We specify our uniforms. We do not need to specify locations manually, but it helps with knowing what is bound where.
layout(location=0) uniform mat4 u_TransformationMat = mat4(1);
layout(location=1) uniform mat4 camMatrix           = mat4(1);
layout(location=2) uniform mat4 u_LightSpaceMat     = mat4(1);

out vec4 vertexPositions;
out vec3 normals;
out vec4 FragPosLightSpace;

void main() {

//We need these in a different shader later down the pipeline, so we need to send them along. Can't just call in a_Position unfortunately.
vertexPositions = vec4(a_Position, 1.0);

//We also need them in Lightspace, so we compute that here
FragPosLightSpace = u_LightSpaceMat *  u_TransformationMat * vertexPositions;

normals = a_normals;

//We multiply our matrices with our position to change the positions of vertices to their final destinations.
gl_Position = camMatrix * u_TransformationMat * vertexPositions;
}
)";

static const std::string directionalLightFragmentShaderSrc = R"(
#version 430 core

in vec4 vertexPositions;
in vec3 normals;
in vec4 FragPosLightSpace;

out vec4 color;

uniform vec4 u_Color;
uniform vec3 u_LightColor;
uniform vec3 u_LightDirection;
uniform float u_Specularity;

vec3 DirectionalLight(in vec3 color, in vec3 direction, in float shadow) {

    //Ambient lighting. Ambient light is light that is present even when normally no light should shine upon that part of the object. 
    //This is the poor mans way of simulating light reflecting from other surfaces in the room. For those that don't want to get into more advanced lighting models.
    float ambient_strength = 0.1;
    vec3 ambient = ambient_strength * color;
    
    //Diffuse lighting. Light that scatters in all directions after hitting the object.
    vec3 dir_to_light = normalize(-direction);                                          //First we get the direction from the object to the lightsource ( which is of course the opposite of the direction of the light)
    vec3 diffuse = color * max(0.0, dot(normals, dir_to_light));                         //Then we find how strongly the light scatters in different directions, with a minimum of 0.0, via the normals and the direction we just found.
    
    vec3 reflectionDirection = reflect(dir_to_light,normals);                                                         //And then we find the angle between the direction of the light and the direction from surface to camera
    
    vec3 specular = u_Specularity * reflectionDirection * color;                                                           //Finally, multiply with how reflective the surface is and add the color.
    
    //We make sure to mutilply diffuse and specular with how shadowed our vertex is
    //The 1-shadow is not really necessary for this, but the values coming from the ShadowCalculation can be updated to give smoother transitions between shadows
    //In which case this might be usefull
    return ambient +(1.0-shadow) * (diffuse + specular);
}

void main() {

//Then send them to the lightfunction
vec3 light = DirectionalLight(u_LightColor,u_LightDirection, 0.0f);

//Finally, multiply with the color. Make sure the vector still has the same dimensions. Alpha channel is set to 1 here, because our object is not transparent. Might be different if you use a texture.
color = u_Color * vec4(light, 1.0);
}


)";

#endif // __SQUARE_H_

