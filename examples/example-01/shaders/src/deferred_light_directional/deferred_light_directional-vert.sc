$input a_position, a_texcoord0
$output v_texcoord0

#include <../mrender.sh>

void main() 
{
    gl_Position = vec4(a_position, 1.0);

    v_texcoord0 = vec2(a_texcoord0.x, 1.0 - a_texcoord0.y);
}