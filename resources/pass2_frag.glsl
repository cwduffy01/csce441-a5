#version 120
    
uniform sampler2D posTexture;
uniform sampler2D norTexture;
uniform sampler2D keTexture;
uniform sampler2D kdTexture;
uniform vec2 windowSize;
uniform int renderMode;
    
uniform vec3 lights[256];
uniform int numLights;
uniform mat4 camMV;

uniform bool blur;

vec2 poissonDisk[] = vec2[](
    vec2(-0.220147, 0.976896),
    vec2(-0.735514, 0.693436),
    vec2(-0.200476, 0.310353),
    vec2( 0.180822, 0.454146),
    vec2( 0.292754, 0.937414),
    vec2( 0.564255, 0.207879),
    vec2( 0.178031, 0.024583),
    vec2( 0.613912,-0.205936),
    vec2(-0.385540,-0.070092),
    vec2( 0.962838, 0.378319),
    vec2(-0.886362, 0.032122),
    vec2(-0.466531,-0.741458),
    vec2( 0.006773,-0.574796),
    vec2(-0.739828,-0.410584),
    vec2( 0.590785,-0.697557),
    vec2(-0.081436,-0.963262),
    vec2( 1.000000,-0.100160),
    vec2( 0.622430, 0.680868)
);

vec3 sampleTextureArea(sampler2D texture, vec2 tex0)
{
    const int N = 18; // [1-18]
    const float blur = 0.005;
    vec3 val = vec3(0.0, 0.0, 0.0);
    for(int i = 0; i < N; i++) {
        val += texture2D(texture, tex0.xy + poissonDisk[i]*blur).rgb;
    }
    val /= N;
    return val;
}
    
void main()
{
    vec2 tex;
    tex.x = gl_FragCoord.x/windowSize.x;
    tex.y = gl_FragCoord.y/windowSize.y;

    vec3 pos, nor, ke, kd;
    
    // Fetch shading data

    if (blur) {
        pos = sampleTextureArea(posTexture, tex).rgb;
        nor = sampleTextureArea(norTexture, tex).rgb;
        ke = sampleTextureArea(keTexture, tex).rgb;
        kd = sampleTextureArea(kdTexture, tex).rgb;
    }
    else {
        pos = texture2D(posTexture, tex).rgb;
        nor = texture2D(norTexture, tex).rgb;
        ke = texture2D(keTexture, tex).rgb;
        kd = texture2D(kdTexture, tex).rgb;
    }

    if (renderMode == 0) {
        vec3 fragColor = ke;
	    float s = 10.0;

        for (int i = 0; i < numLights; i++) {
		    vec3 lpos = vec3(camMV *  vec4(lights[2 * i], 1.0f));
		    vec3 lcol = lights[2 * i + 1];

		    vec3 n = normalize(nor);
		    vec3 eye = normalize(-1 * pos);
		    vec3 l = normalize(lpos - pos);
		    vec3 h = normalize(eye + l);

		    float diffuse = max(0, dot(l, n));
		    float specular = pow(max(0, dot(h, n)), s);

		    vec3 color = lcol * (kd * diffuse + specular);

		    float r = length(lpos - pos);
		    float att = 1.0 / (1.0 + 0.0429 * r + 0.9857 * pow(r, 2));

		    fragColor += color * att;
	    }

        gl_FragColor.rgb = fragColor;
    }
    else if (renderMode == 1) { gl_FragColor.rgb = pos; }
    else if (renderMode == 2) { gl_FragColor.rgb = nor; }
    else if (renderMode == 3) { gl_FragColor.rgb = ke; }
    else if (renderMode == 4) { gl_FragColor.rgb = kd; }

    
    //gl_FragColor.rgb = nor;
}