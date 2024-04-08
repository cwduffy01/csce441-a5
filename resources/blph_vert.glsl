#version 120

uniform mat4 P;
uniform mat4 MV;
uniform mat4 invMV;
uniform vec3 lightPos;
uniform int polar;
uniform float t;

attribute vec4 aPos; // in object space
attribute vec3 aNor; // in object space

varying vec3 position; // Pass to fragment shader
varying vec3 normal; // Pass to fragment shader


void main()
{

	if (polar == 1) {
		float theta = aPos.y;
		float x = aPos.x;

		float f = 0.4 * (cos(5 * (x + t)) + 2);
		vec4 pos = vec4(x, f * cos(theta), f * sin(theta), aPos.w);
		gl_Position = P * (MV * pos);

		vec3 dpdx = vec3(1.0, -2.0 * sin(5 * (x + t)) * cos(theta), -2.0 * sin(5 * (x + t)) * sin(theta));
		vec3 dpdt = vec3(0.0, -1.0 * f * sin(theta), f * cos(theta));

		vec3 n = cross(dpdx, dpdt);
		vec3 n_hat = n / length(n);
		normal = normalize(vec3(invMV * vec4(n_hat, 0.0)));
	}
	else {
		gl_Position = P * (MV * aPos);

		position = vec3(MV * aPos);
		normal = normalize(mat3(invMV) * aNor);
	}
}
