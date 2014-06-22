//shader version
#version 150 core

// current drawing landscape type (0 - default, 1 - sea, 2 - land)
uniform int landscapeType;
int SEA = 1;
int LAND = 2;

// current drawing render type (Object = 0, Reflection = 1, Shadow = 2)
uniform int renderType;
int Object = 0;
int Reflection = 1;

uniform vec3 lightPosition;
uniform float maxHeight;
uniform float maxDepth;

//inverse and transpose matrix for normals
uniform mat4 normalMatrix;

//retrieve this data form vertex shader
in VertexData
{
	vec3 position;
	vec3 normal;
	//vec2 texcoord;
	//vec3 color;
	vec3 modelnor;
	vec3 modelpos;
	//float height;
} VertexIn;

out vec4 fragColor;

// Light parameters
vec3 LightColor = vec3(1.0, 1.0, 1.0); // Light source intensity (diffuse and specular light)

//vec3 SeaKd = vec3(0.4, 0.9, 1); // Sea color
vec3 SeaKd = vec3(0.1, 0.225, 0.25);

// Height percents
const float beachMin = -5; // depth
const float beachMax = 16;
const float grassMin = 20;
const float grassMax = 70;
const float snowMin = 60;
const float snowMax = 100;

// Fog info
const vec4 FogColor = vec4(0.53, 0.81, 0.98, 1);
const float FogMaxDist = 3.0;
const float FogMinDist = 1;

float hash(float n)
{
    return fract(sin(n)*43758.5453123);
}

float noise(in vec3 x)
{
    vec3 p = floor(x);
    vec3 f = fract(x);

    f = f*f*(3.0-2.0*f);

    float n = p.x + p.y*57.0 + 113.0*p.z;
    float res = mix(mix(mix(hash(n +  0.0), hash(n +  1.0), f.x),
                        mix(hash(n + 57.0), hash(n + 58.0), f.x), f.y),
                    mix(mix(hash(n + 113.0), hash(n + 114.0), f.x),
                        mix(hash(n + 170.0), hash(n + 171.0), f.x), f.y), f.z);
    return res;
}

// Fractional Brownian Motion for simulate land roughness
float fbm(vec3 p) {
	p *= 20; // tune up (3, 10, 20)
	float final = 1;
	p *= 2.97;
	p *= 1.97; final += noise(p) * final * 0.007;
	p *= 1.99; final += noise(p) * final * 0.002;
	p *= 1.91; final += noise(p) * final * 0.0008;
	return final - 1.0;
}

vec3 roughNormal(vec3 pos, vec3 norm)
{
	const vec2 eps = vec2(0.001, 0.0);
	float fbmpos = fbm(pos);
	if (norm.z == 0)
	{
		return normalize(vec3(
			fbmpos - fbm(pos + eps.xyy) + eps.x * norm.x,
			fbmpos - fbm(pos + eps.yxy) + eps.x * norm.y,
			fbmpos - fbm(pos + eps.yyx)
			));
	}
	else
	{
		return normalize(vec3(
			fbmpos - fbm(pos + eps.xyy) + eps.x * norm.x / norm.z,
			fbmpos - fbm(pos + eps.yxy) + eps.x * norm.y / norm.z,
			fbmpos - fbm(pos + eps.yyx) + eps.x
			));
	}
}

vec4 ComputeLight (const in vec3 lightDir, const in vec3 eyeDir, const in vec3 normal, const in vec3 lightColor, const in vec3 diffColor, const in vec3 specColor, const in float shininess)
{
	vec3 La = vec3(0.4, 0.4, 0.4); // Ambient light
	vec3 ambient = La * diffColor;

	// Convert normal and position to eye coords
	vec4 lPos4 = vec4(lightPosition, 1.0);
	vec3 r = reflect(-lightDir, normal);

	float sDotN = max( dot(lightDir, normal), 0.0 );

	vec3 diffuse = lightColor * diffColor * sDotN; // lambert

	vec3 spec = vec3(0.0);
	if( sDotN > 0.0 )
		spec = lightColor * specColor * pow( max( dot(r, eyeDir), 0.0 ), shininess );
	vec3 LightIntensity = ambient + diffuse + spec;
	return vec4(LightIntensity, 1.0);
}
		
void main()
{	
	// Convert normal and position to eye coords	
	vec4 lPos4 = vec4(lightPosition, 1.0);
	vec3 s = normalize(vec3(lPos4 - VertexIn.position));
	vec3 v = normalize(-VertexIn.position.xyz);
	vec3 Ks = vec3(0, 0, 0); // Specular color
	float height = VertexIn.modelpos.z;
	vec3 tnorm = vec3(0);

	if (landscapeType == SEA)
	{	
		Ks = vec3(1, 1, 1);	
		tnorm = normalize(VertexIn.normal);
		fragColor = ComputeLight(s, v, tnorm, LightColor, SeaKd, Ks, 800);
		fragColor.w = 0.5;
	}
	if (landscapeType == LAND)
	{
		vec3 Kd = vec3(0.36, 0.25, 0.15); // Diffuse reflectivity
		float snowThresh = 1.0 - smoothstep(maxHeight * snowMin / 100, maxHeight * snowMax / 100, height);
		float grassThresh = smoothstep(maxHeight * grassMin / 100, maxHeight * grassMax / 100, height) * 0.95 + 0.1;
		
		// Create small roughness normal
		//vec3 rmodelnorm = VertexIn.modelnor;
		vec3 rmodelnorm = roughNormal(VertexIn.modelpos, VertexIn.modelnor);
		
		float slope = rmodelnorm.z;
		if (slope > grassThresh - 0.1)
		{
			Kd = mix(Kd, vec3(0.4, 0.6, 0.2), smoothstep(grassThresh - 0.1, grassThresh + 0.1, slope));
			Ks = Kd;
		}
		if (slope > snowThresh - 0.1)
		{
			Kd = mix(Kd, vec3(0.8, 0.8, 0.8), smoothstep(snowThresh - 0.1, snowThresh + 0.1, slope));
			Ks = vec3(1, 1, 1);	
		}

		// Fade in 'beach' and add a bit of noise
		Kd = mix(Kd, vec3(0.6, 0.5, 0.2), smoothstep(maxHeight * beachMax / 100, maxHeight * beachMin / 100, height));

		//if (height < 0)
		//	Kd = mix(Kd, SeaKd, smoothstep(0, maxDepth, height));
		// for reflection
		if ((renderType == Reflection) && (height < 0))
			discard;

		//tnorm = normalize(VertexIn.normal);
		tnorm = normalize(vec3(normalMatrix * vec4(rmodelnorm, 1)));
		
		fragColor = ComputeLight(s, v, tnorm, LightColor, Kd, Ks, 100);
		if ((renderType == Object) && (height < 0))
		{
			//fragColor = mix(fragColor, vec4(SeaKd, 0.7), smoothstep(0, maxDepth * 0.8, height));
			//fragColor.w = 0.4;
			discard;
		}
	}
	// calculate fog
	float dist = length(VertexIn.position);
	float fogFactor = (FogMaxDist - dist) / (FogMaxDist - FogMinDist);
	fogFactor = clamp( fogFactor, 0.0, 1.0 );
	fragColor = mix( FogColor, fragColor, fogFactor );
}
