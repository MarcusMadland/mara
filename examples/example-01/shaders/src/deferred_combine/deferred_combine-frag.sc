$input v_texcoord0

#include <../mrender.sh>

SAMPLER2D(u_gdiffuse, 0);
SAMPLER2D(u_light, 1);

void main()
{
	// Diffuse
	vec4 diffuse = texture2D(u_gdiffuse, v_texcoord0);
	
	// Light
	vec4 light = texture2D(u_light, v_texcoord0);
	
	// Combine
	gl_FragColor = vec4(vec4(diffuse * light).rgb, 1.0);
}