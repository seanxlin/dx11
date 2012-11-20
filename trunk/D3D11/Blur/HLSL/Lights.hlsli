///////////////////////////////////////////////////////////////
// Structures and functions for lighting calculations.
///////////////////////////////////////////////////////////////

struct DirectionalLight
{
	float4 mAmbient;
	float4 mDiffuse;
	float4 mSpecular;
	float3 mDirection;
	float mPad;
};

struct PointLight
{ 
	float4 mAmbient;
	float4 mDiffuse;
	float4 mSpecular;

	float3 mPosition;
	float mRange;

	float3 mAttenuation;
	float mPad;
};

struct SpotLight
{
	float4 mAmbient;
	float4 mDiffuse;
	float4 mSpecular;

	float3 mPosition;
	float mRange;

	float3 mDirection;
	float mSpot;

	float3 mAttenuation;
	float mPad;
};

struct Material
{
	float4 mAmbient;
	float4 mDiffuse;
	float4 mSpecular; // w = SpecPower
	float4 mReflect;
};

///////////////////////////////////////////////////////////////// 
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
///////////////////////////////////////////////////////////////
void computeDirectionalLight(Material material, DirectionalLight light, float3 normal, float3 toEye,
					         out float4 ambient, out float4 diffuse, out float4 specular)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	float3 lightVector = -light.mDirection;

	// Add ambient term.
	ambient = material.mAmbient * light.mAmbient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.	
	float diffuseFactor = dot(lightVector, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 lightReflection = reflect(-lightVector, normal);
		float specularFactor = pow( max( dot(lightReflection, toEye), 0.0f), 
            material.mSpecular.w);
					
		diffuse = diffuseFactor * material.mDiffuse * light.mDiffuse;
		specular = specularFactor * material.mSpecular * light.mSpecular;
	}
}

///////////////////////////////////////////////////////////////
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
///////////////////////////////////////////////////////////////
void computePointLight(Material material, PointLight light, float3 position, float3 normal, float3 toEye,
                       out float4 ambient, out float4 diffuse, out float4 specular)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 lightVector = light.mPosition - position;
		
	// The distance from surface to light.
	float surfaceToLightDistance = length(lightVector);
	
	// Range test.
	if (surfaceToLightDistance > light.mRange)
		return;
		
	// Normalize the light vector.
	lightVector /= surfaceToLightDistance; 
	
	// Ambient term.
	ambient = material.mAmbient * light.mAmbient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	float diffuseFactor = dot(lightVector, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 lightReflection = reflect(-lightVector, normal);
		float specularFactor = pow( max( dot(lightReflection, toEye), 0.0f), 
            material.mSpecular.w);
					
		diffuse = diffuseFactor * material.mDiffuse * light.mDiffuse;
		specular = specularFactor * material.mSpecular * light.mSpecular;
	}

	// Attenuate
	float attenuation = 1.0f / dot(light.mAttenuation, float3(1.0f, surfaceToLightDistance, 
        surfaceToLightDistance * surfaceToLightDistance));

	diffuse *= attenuation;
	specular *= attenuation;
}

///////////////////////////////////////////////////////////////
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
///////////////////////////////////////////////////////////////
void computeSpotLight(Material material, SpotLight light, float3 position, float3 normal, 
                      float3 toEye, out float4 ambient, out float4 diffuse, out float4 specular)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 lightVector = light.mPosition - position;
		
	// The distance from surface to light.
	float surfaceToLightDistance = length(lightVector);
	
	// Range test.
	if (surfaceToLightDistance > light.mRange)
		return;
		
	// Normalize the light vector.
	lightVector /= surfaceToLightDistance; 
	
	// Ambient term.
	ambient = material.mAmbient * light.mAmbient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	float diffuseFactor = dot(lightVector, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f)
	{
		float3 lightReflection = reflect(-lightVector, normal);
		float specularFactor = pow( max( dot(lightReflection, toEye), 0.0f), 
            material.mSpecular.w);
					
		diffuse = diffuseFactor * material.mDiffuse * light.mDiffuse;
		specular = specularFactor * material.mSpecular * light.mSpecular;
	}
	
	// Scale by spotlight factor and attenuate.
	float spotLightFactor = pow( max( dot(-lightVector, light.mDirection), 0.0f), light.mSpot);

	// Scale by spotlight factor and attenuate.
	float attenuation = spotLightFactor / dot(light.mAttenuation, float3(1.0f, surfaceToLightDistance, 
        surfaceToLightDistance * surfaceToLightDistance));

	ambient *= spotLightFactor;
	diffuse *= attenuation;
	specular *= attenuation;
}

 
 