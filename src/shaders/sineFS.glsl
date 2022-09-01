#version 430 core
#extension GL_NV_shadow_samplers_cube : enable
out vec4 f_color;

float Eta = 0.95f;
float ratio_of_reflection_and_refraction = 0.5f;

in V_OUT
{
   vec3 position;
   vec3 normal;
   vec2 texture_coordinate;
   vec4 clipSpace;
   vec3 toCameraVector;
   vec3 fromLightVector;
} f_in;

uniform vec3 cameraPos;

uniform samplerCube skybox;
uniform sampler2D tiles;

vec2 intersectCube(vec3 origin, vec3 ray, vec3 cubeMin, vec3 cubeMax)
{
	vec3 tMin = (cubeMin - origin) / ray;
	vec3 tMax = (cubeMax - origin) / ray;
	vec3 t1 = min(tMin, tMax);
	vec3 t2 = max(tMin, tMax);
	float tNear = max(max(t1.x, t1.y), t1.z);
	float tFar = min(min(t2.x, t2.y), t2.z);
	return vec2(tNear, tFar);
}

vec3 getWallColor(vec3 point)
{
	float scale = 0.5f;

	vec3 wallColor;
	vec3 normal;

	if (abs(point.x) > 0.999f)
	{
		wallColor = texture2D(tiles, point.yz * 0.5f + vec2(1.0f, 0.5f)).rgb;
		normal = vec3(-point.x, 0.0f, 0.0f);
	}
	else if (abs(point.z) > 0.999f)
	{
		wallColor = texture2D(tiles, point.yx * 0.5f + vec2(1.0f, 0.5f)).rgb;
		normal = vec3(0.0f, 0.0f, -point.z);
	}
	else
	{
		wallColor = texture2D(tiles, point.xz * 0.5f + 0.5f).rgb;
		normal = vec3(0.0f, 1.0f, 0.0f);
	}

	scale /= length(point);


	return wallColor * scale;
}

vec3 getSurfaceRayColor(vec3 origin, vec3 ray, vec3 waterColor)
{
	vec3 color;
	if (ray.y < 0.0)
	{
		vec2 temp = intersectCube(origin, ray, vec3(-1.0f, -1.0f, -1.0f), vec3(1.0f, 1.0f, 1.0f));
		color = getWallColor(origin + ray * temp.y);
	}
	else
		color = vec3(textureCube(skybox, ray));

	color *= waterColor;
	return color;
}

void main()
{
	vec2 ndc = (f_in.clipSpace.xy / f_in.clipSpace.w) / 2.0f + 0.5f;
	vec2 refractTexCoords = vec2(ndc.x, ndc.y);

	vec3 I = normalize(f_in.position - cameraPos);
	vec3 reflectionVector = reflect(I, normalize(f_in.normal));
	vec3 refractionVector = refract(I, normalize(f_in.normal), Eta);

	vec3 normal = normalize(f_in.normal);

	vec3 reflectionColor = vec3(texture(skybox, reflectionVector));
	vec3 refractionColor = getSurfaceRayColor(vec3(refractTexCoords.y, 0.0f, refractTexCoords.x), refractionVector, vec3(1.0f)) * vec3(0.0f, 0.8f, 1.0f);

	//if (f_in.normal.y > 0)
	//	f_color = vec4(mix(reflectionColor, refractionColor, ratio_of_reflection_and_refraction), 1.0f);
	//else
	//	f_color = vec4(refractionColor, 1.0f);

	//FragColor = mix(refractionColor, FragColor, 0.5f);

	f_color = vec4(mix(refractionColor, reflectionColor, 0.8f), 1.0f);
	//f_color = vec4(normal, 1.0f);
}