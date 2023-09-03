#include <../mrender.sh>

uniform vec4 u_debugColor;

void main() 
{
	gl_FragColor = vec4(u_debugColor.rgb, 1.0);
}