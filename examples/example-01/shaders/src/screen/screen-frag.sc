$input v_texcoord0

#include <../mrender.sh>

SAMPLER2D(u_buffer, 0);

void main() 
{
	vec4 color = texture2D(u_buffer, v_texcoord0);

	gl_FragColor = vec4(color.rgb, 1.0);
}