#ifndef __MRENDERLIGHT_SH__
#define __MRENDERLIGHT_SH__

#include <../mrender.sh>
#include <../mrender-common.sh>

SAMPLER2D(u_gnormal, 0);
SAMPLER2D(u_gdepth,  1);
SAMPLER2D(u_shadowMap, 2);

uniform vec4 u_lightPositionRange; // rgb = position, a = range (radius)
uniform vec4 u_lightColorIntensity; // rgb = color, a = intensity
uniform mat4 u_mtx;
uniform mat4 u_lightMtx;

// @todo This is expensive, inverse it on the cpu instead?
mat4 inverse(mat4 m) 
{
    mat4 inv;
    
    inv[0][0] = m[1][1] * m[2][2] * m[3][3] - m[1][1] * m[2][3] * m[3][2] - m[2][1] * m[1][2] * m[3][3] +
                m[2][1] * m[1][3] * m[3][2] + m[3][1] * m[1][2] * m[2][3] - m[3][1] * m[1][3] * m[2][2];
    inv[0][1] = -m[0][1] * m[2][2] * m[3][3] + m[0][1] * m[2][3] * m[3][2] + m[2][1] * m[0][2] * m[3][3] -
                m[2][1] * m[0][3] * m[3][2] - m[3][1] * m[0][2] * m[2][3] + m[3][1] * m[0][3] * m[2][2];
    inv[0][2] = m[0][1] * m[1][2] * m[3][3] - m[0][1] * m[1][3] * m[3][2] - m[1][1] * m[0][2] * m[3][3] +
                m[1][1] * m[0][3] * m[3][2] + m[3][1] * m[0][2] * m[1][3] - m[3][1] * m[0][3] * m[1][2];
    inv[0][3] = -m[0][1] * m[1][2] * m[2][3] + m[0][1] * m[1][3] * m[2][2] + m[1][1] * m[0][2] * m[2][3] -
                m[1][1] * m[0][3] * m[2][2] - m[2][1] * m[0][2] * m[1][3] + m[2][1] * m[0][3] * m[1][2];
    inv[1][0] = -m[1][0] * m[2][2] * m[3][3] + m[1][0] * m[2][3] * m[3][2] + m[2][0] * m[1][2] * m[3][3] -
                m[2][0] * m[1][3] * m[3][2] - m[3][0] * m[1][2] * m[2][3] + m[3][0] * m[1][3] * m[2][2];
    inv[1][1] = m[0][0] * m[2][2] * m[3][3] - m[0][0] * m[2][3] * m[3][2] - m[2][0] * m[0][2] * m[3][3] +
                m[2][0] * m[0][3] * m[3][2] + m[3][0] * m[0][2] * m[2][3] - m[3][0] * m[0][3] * m[2][2];
    inv[1][2] = -m[0][0] * m[1][2] * m[3][3] + m[0][0] * m[1][3] * m[3][2] + m[1][0] * m[0][2] * m[3][3] -
                m[1][0] * m[0][3] * m[3][2] - m[3][0] * m[0][2] * m[1][3] + m[3][0] * m[0][3] * m[1][2];
    inv[1][3] = m[0][0] * m[1][2] * m[2][3] - m[0][0] * m[1][3] * m[2][2] - m[1][0] * m[0][2] * m[2][3] +
                m[1][0] * m[0][3] * m[2][2] + m[2][0] * m[0][2] * m[1][3] - m[2][0] * m[0][3] * m[1][2];
    inv[2][0] = m[1][0] * m[2][1] * m[3][3] - m[1][0] * m[2][3] * m[3][1] - m[2][0] * m[1][1] * m[3][3] +
                m[2][0] * m[1][3] * m[3][1] + m[3][0] * m[1][1] * m[2][3] - m[3][0] * m[1][3] * m[2][1];
    inv[2][1] = -m[0][0] * m[2][1] * m[3][3] + m[0][0] * m[2][3] * m[3][1] + m[2][0] * m[0][1] * m[3][3] -
                m[2][0] * m[0][3] * m[3][1] - m[3][0] * m[0][1] * m[2][3] + m[3][0] * m[0][3] * m[2][1];
    inv[2][2] = m[0][0] * m[1][1] * m[3][3] - m[0][0] * m[1][3] * m[3][1] - m[1][0] * m[0][1] * m[3][3] +
                m[1][0] * m[0][3] * m[3][1] + m[3][0] * m[0][1] * m[1][3] - m[3][0] * m[0][3] * m[1][1];
    inv[2][3] = -m[0][0] * m[1][1] * m[2][3] + m[0][0] * m[1][3] * m[2][1] + m[1][0] * m[0][1] * m[2][3] -
                m[1][0] * m[0][3] * m[2][1] - m[2][0] * m[0][1] * m[1][3] + m[2][0] * m[0][3] * m[1][1];
    inv[3][0] = -m[1][0] * m[2][1] * m[3][2] + m[1][0] * m[2][2] * m[3][1] + m[2][0] * m[1][1] * m[3][2] -
                m[2][0] * m[1][2] * m[3][1] - m[3][0] * m[1][1] * m[2][2] + m[3][0] * m[1][2] * m[2][1];
    inv[3][1] = m[0][0] * m[2][1] * m[3][2] - m[0][0] * m[2][2] * m[3][1] - m[2][0] * m[0][1] * m[3][2] +
                m[2][0] * m[0][2] * m[3][1] + m[3][0] * m[0][1] * m[2][2] - m[3][0] * m[0][2] * m[2][1];
    inv[3][2] = -m[0][0] * m[1][1] * m[3][2] + m[0][0] * m[1][2] * m[3][1] + m[1][0] * m[0][1] * m[3][2] -
                m[1][0] * m[0][2] * m[3][1] - m[3][0] * m[0][1] * m[1][2] + m[3][0] * m[0][2] * m[1][1];
    inv[3][3] = m[0][0] * m[1][1] * m[2][2] - m[0][0] * m[1][2] * m[2][1] - m[1][0] * m[0][1] * m[2][2] +
                m[1][0] * m[0][2] * m[2][1] + m[2][0] * m[0][1] * m[1][2] - m[2][0] * m[0][2] * m[1][1];
    
    float det = m[0][0] * inv[0][0] + m[0][1] * inv[1][0] + m[0][2] * inv[2][0] + m[0][3] * inv[3][0];
    
    if (det == 0.0) {
        // Matrix is not invertible, return identity matrix or handle the error accordingly
        return mat4(1.0, 0.0, 0.0, 0.0,
                0.0, 1.0, 0.0, 0.0,
                0.0, 0.0, 1.0, 0.0,
                0.0, 0.0, 0.0, 1.0);
    }
    
    inv = inv / det;
    
    return inv;
}

float hardShadow(vec4 _shadowCoord, float _bias)
{
	vec3 texCoord = _shadowCoord.xyz/_shadowCoord.w;
	return step(texCoord.z-_bias, (texture2D(u_shadowMap, texCoord.xy)).r);
}

float PCF(vec4 _shadowCoord, float _bias, vec2 _texelSize)
{
	vec2 texCoord = _shadowCoord.xy/_shadowCoord.w;

	bool outside = any(greaterThan(texCoord, vec2_splat(1.0)))
				|| any(lessThan   (texCoord, vec2_splat(0.0)))
				 ;

	if (outside)
	{
		//return 1.0;
	}

	float result = 0.0;
	vec2 offset = _texelSize * _shadowCoord.w;

	result += hardShadow(_shadowCoord + vec4(vec2(-1.5, -1.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(-1.5, -0.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(-1.5,  0.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(-1.5,  1.5) * offset, 0.0, 0.0), _bias);

	result += hardShadow(_shadowCoord + vec4(vec2(-0.5, -1.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(-0.5, -0.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(-0.5,  0.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(-0.5,  1.5) * offset, 0.0, 0.0), _bias);

	result += hardShadow(_shadowCoord + vec4(vec2(0.5, -1.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(0.5, -0.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(0.5,  0.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(0.5,  1.5) * offset, 0.0, 0.0), _bias);

	result += hardShadow(_shadowCoord + vec4(vec2(1.5, -1.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(1.5, -0.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(1.5,  0.5) * offset, 0.0, 0.0), _bias);
	result += hardShadow(_shadowCoord + vec4(vec2(1.5,  1.5) * offset, 0.0, 0.0), _bias);

	return result / 16.0;
}

vec4 calcLight(vec2 texcoord0)
{
	// Get position from depth
	float deviceDepth = texture2D(u_gdepth, texcoord0).x;
	float depth       = toClipSpaceDepth(deviceDepth);
	vec3 clip = vec3(texcoord0 * 2.0 - 1.0, depth);
	vec3 wpos = clipToWorld(inverse(u_mtx), clip);
	vec3 lightPosition = u_lightPositionRange.xyz;
	
	// Get normal by decoding buffer
	vec3 normal   = decodeNormalUint(texture2D(u_gnormal, texcoord0).rgb);

	// Get general light information
	float lightIntensity = u_lightColorIntensity.a;

#ifdef POINT_LIGHT
	// Calculate point light
	vec3 lightDir = normalize(lightPosition - wpos);
	float lightDistance = distance(lightPosition, wpos);
	float range = u_lightPositionRange.w;
    float attenuation = smoothstep(range, 0.0, lightDistance / range);
	vec3 lighting = max(dot(normal, lightDir), 0.0) * u_lightColorIntensity.rgb * lightIntensity * attenuation;
	
	// Return final color
	return vec4(lighting.rgb, 1.0);
	
#elif DIRECTIONAL_LIGHT
	// Calculate directional light
	vec3 lightDir = normalize(lightPosition);
	vec3 lighting = max(dot(normal, lightDir), 0.0) * u_lightColorIntensity.rgb * lightIntensity;
	
	// Calculate shadow mapping
	float bias = 0.002;//max(0.05 * (1.0 - dot(normal, lightDir)), 0.005); 
	vec4 fragPosLightSpace = mul(u_lightMtx, vec4(wpos, 1.0));
	vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	projCoords = projCoords * 0.5 + 0.5; 
    float closestDepth  = texture2D(u_shadowMap, projCoords.xy).r;
    float currentDepth  =  projCoords.z - bias;
    float shadowFactor = currentDepth > closestDepth  ? 0.0 : 1.0;  
	
	bool outside = any(greaterThan(projCoords.xy, vec2_splat(0.99)))
				|| any(lessThan   (projCoords.xy, vec2_splat(0.01)))
				 ;
	if(currentDepth > 1.0 || outside)
	{
		shadowFactor = 1.0;
	}
	
	// Return final color
	return vec4(mul(lighting.rgb, shadowFactor), 1.0);;
	
#else
	return vec4(0.0, 0.0, 0.0, 1.0);
#endif
}

#endif // __MRENDERLIGHT_SH__