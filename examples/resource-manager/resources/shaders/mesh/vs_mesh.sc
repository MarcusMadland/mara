$input a_position, a_normal

#include "../common.sh"

void main()
{
	vec3 pos = a_position;
	vec3 normal = a_normal.xyz*2.0 - 1.0;

	pos = pos + normal;

	gl_Position = mul(u_modelViewProj, vec4(pos, 1.0) );
}
