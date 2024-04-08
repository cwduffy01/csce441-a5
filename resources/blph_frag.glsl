#version 120

uniform vec3 lightPos;
uniform vec3 lightColor;
uniform vec3 ke;
uniform vec3 kd;
uniform vec3 ks;
uniform float s;

varying vec3 position;
varying vec3 normal;

void main()
{

	vec3 n = normalize(normal);
	vec3 eye = normalize(-1 * position);
	vec3 l = normalize(lightPos - position);
	vec3 h = normalize(eye + l);

	vec3 cd = kd * max(0, dot(l, n));
	vec3 cs = ks * pow(max(0, dot(h, n)), s);

	vec3 color = lightColor * (ke + cd + cs);
	gl_FragColor = vec4(color.xyz, 1.0);
}
