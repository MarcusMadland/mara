$input v_wpos, v_normal, v_tangent, v_bitangent, v_texcoord0

#include <../mrender.sh>
#include <../mrender-common.sh>

SAMPLER2D(u_albedo, 0);
SAMPLER2D(u_normal, 1);
SAMPLER2D(u_specular, 2);

uniform vec4 u_albedoColor;
uniform vec4 u_normalColor;
uniform vec4 u_specularColor;

void main() 
{
	// Diffuse
	vec4 albedoCol = length(texture2D(u_albedo, v_texcoord0).rgb) > 0.0 ? texture2D(u_albedo, v_texcoord0) : u_albedoColor;
	vec4 normalCol = length(texture2D(u_normal, v_texcoord0).rgb) > 0.0 ? texture2D(u_normal, v_texcoord0) : u_normalColor;
	
	// Normal
	vec3 normal;
	normal.xy = normalCol.xy * 2.0 - 1.0;
	normal.z  = sqrt(1.0 - dot(normal.xy, normal.xy) );
	mat3 tbn = mat3(normalize(v_tangent),normalize(v_bitangent),normalize(v_normal));
	normal = normalize(mul(tbn, normal) );
	vec3 wnormal = normalize(mul(u_invView, vec4(normal, 0.0) ).xyz);

	// Output
	gl_FragData[0] = vec4(albedoCol.rgb, 1.0); // Diffuse
	gl_FragData[1] = vec4(encodeNormalUint(wnormal), 1.0); // Normal
	gl_FragData[2] = vec4(0.0, 0.0, 0.0, 1.0); // Specular
	gl_FragData[3] = vec4(0.0, 0.0, 0.0, 1.0); // Irradiance
}