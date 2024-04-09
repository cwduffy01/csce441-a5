#version 120

uniform vec3 lights[256];
uniform int numLights;
uniform mat4 camMV;
uniform vec3 ke;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 position;
varying vec3 normal;

void main()
{
	vec3 fragColor = ke;

	for (int i = 0; i < numLights; i++) {
		vec3 lpos = vec3(camMV *  vec4(lights[2 * i], 1.0f));
		vec3 lcol = lights[2 * i + 1];

		vec3 n = normalize(normal);
		vec3 eye = normalize(-1 * position);
		vec3 l = normalize(lpos - position);
		vec3 h = normalize(eye + l);

		float diffuse = max(0, dot(l, n));
		float specular = pow(max(0, dot(h, n)), s);

		vec3 color = lcol * (kd * diffuse + ks * specular);

		float r = length(lpos - position);
		float att = 1.0 / (1.0 + 0.0429 * r + 0.9857 * pow(r, 2));
		//float att = 1.0;


		fragColor += color * att;
	}

	gl_FragColor = vec4(fragColor, 1.0);
}
