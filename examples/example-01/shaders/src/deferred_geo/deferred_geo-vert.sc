$input a_position, a_normal, a_tangent, a_texcoord0
$output v_wpos, v_normal, v_tangent, v_bitangent, v_texcoord0

#include <../mrender.sh>

void main() 
{
	vec3 wpos = mul(u_model[0], vec4(a_position, 1.0) ).xyz;
	gl_Position = mul(u_viewProj, vec4(wpos, 1.0) );
	
	vec4 normal = a_normal * 2.0 - 1.0;
	vec3 wnormal = mul(u_model[0], vec4(normal.xyz, 0.0) ).xyz;

	vec4 tangent = a_tangent * 2.0 - 1.0;
	vec3 wtangent = mul(u_model[0], vec4(tangent.xyz, 0.0) ).xyz;

	vec3 viewNormal = normalize(mul(u_view, vec4(wnormal, 0.0) ).xyz);
	vec3 viewTangent = normalize(mul(u_view, vec4(wtangent, 0.0) ).xyz);
	vec3 viewBitangent = cross(viewNormal, viewTangent) * tangent.w;

	v_wpos      = wpos;
	v_normal    = viewNormal;
	v_tangent   = viewTangent;
	v_bitangent = viewBitangent;
	v_texcoord0 = vec2(a_texcoord0.x, 1.0 - a_texcoord0.y);
}