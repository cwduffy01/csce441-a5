#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 invMV;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 vPos;
varying vec3 vNor;


void main()
{
	gl_Position = P * (MV * aPos);

	vPos = vec3(MV * aPos);
	vNor = normalize(mat3(invMV) * aNor);
}
