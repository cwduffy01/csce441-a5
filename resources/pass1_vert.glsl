#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 invMV;
uniform bool polar;
uniform float t;
uniform float tOff;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 vPos;
varying vec3 vNor;


void main()
{
	if (aNor.x == 0 && aNor.y == 0 && aNor.z == 0) {
		float x = aPos.x;
		float theta = aPos.y;

		float c = 0.2;
		float s = 5;
		float h = 2;

		float deltax = x + t + tOff;

		float f = c * (cos(s * deltax) + h);
		float y = f * cos(theta);
		float z = f * sin(theta);

		vec3 dpdx = vec3(
			1.0,
			-c * s * sin(s * (deltax)) * cos(theta),
			-c * s * sin(s * (deltax)) * sin(theta)
		);

		vec3 dpdt = vec3(
			0.0,
			-c * (cos(s * (deltax)) + h) * sin(theta),
			 c * (cos(s * (deltax)) + h) * cos(theta)
		);

		vec3 n_hat = normalize(cross(dpdx, dpdt));

		vec3 pos = vec3(x, y, z);
		vec3 nor = vec3(n_hat.x, n_hat.y, n_hat.z);

		gl_Position = P * (MV * vec4(pos, 1.0));
		vPos = vec3(MV * vec4(pos, 1.0));
		vNor = normalize(nor);
	}
	else {
		gl_Position = P * (MV * aPos);

		vPos = vec3(MV * aPos);
		vNor = normalize(mat3(invMV) * aNor);
	}
}
