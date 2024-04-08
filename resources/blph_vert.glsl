#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 invMV;
uniform vec3 lightPos;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 position; // Pass to fragment shader
varying vec3 normal; // Pass to fragment shader


void main()
{
	gl_Position = P * (MV * aPos);

	position = vec3(MV * aPos);
	normal = normalize(mat3(invMV) * aNor);
}
