$input v_texcoord0

#define DIRECTIONAL_LIGHT 1
#include <../mrender-light.sh>

void main()
{
	gl_FragColor = calcLight(v_texcoord0);
}