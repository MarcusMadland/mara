$input v_texcoord0

#define SPOT_LIGHT 1
#include <../mrender-light.sh>

void main()
{
	gl_FragColor = calcLight(v_texcoord0);
}