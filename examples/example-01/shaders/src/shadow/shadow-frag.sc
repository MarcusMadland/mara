$input v_color0

#include <../mrender.sh>

void main() {
	vec4 color = v_color0;
        gl_FragColor = vec4_splat(0.0);
}