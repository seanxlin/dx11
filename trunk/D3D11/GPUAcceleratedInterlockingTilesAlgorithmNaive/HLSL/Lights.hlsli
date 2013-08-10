///////////////////////////////////////////////////////////////
//
// Structures and functions for lighting calculations.
//
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
//
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a directional light.  We need to output the terms separately because
// later we will modify the individual terms.
//
///////////////////////////////////////////////////////////////
void computeDirectionalLight(const Material material, 
                             const DirectionalLight light, 
                             const float3 normal, 
                             const float3 toEye,
					         out float4 ambient, 
                             out float4 diffuse, 
                             out float4 specular)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The light vector aims opposite the direction the light rays travel.
	const float3 lightVector = -light.mDirection;

	// Add ambient term.
	ambient = material.mAmbient * light.mAmbient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.	
	const float diffuseFactor = dot(lightVector, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f) {
		const float3 lightReflection = reflect(-lightVector, normal);
		const float specularFactor = 
            pow(max(dot(lightReflection, toEye), 0.0f),
                material.mSpecular.w);
					
		diffuse = diffuseFactor * material.mDiffuse * light.mDiffuse;
		specular = specularFactor * material.mSpecular * light.mSpecular;
	}
}

///////////////////////////////////////////////////////////////
//
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a point light.  We need to output the terms separately because
// later we will modify the individual terms.
//
///////////////////////////////////////////////////////////////
void computePointLight(const Material material, 
                       const PointLight light, 
                       const float3 position, 
                       const float3 normal, 
                       const float3 toEye,
                       out float4 ambient, 
                       out float4 diffuse, 
                       out float4 specular)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 lightVector = light.mPosition - position;
		
	// The distance from surface to light.
	const float surfaceToLightDistance = length(lightVector);
	
	// Range test.
	if (surfaceToLightDistance > light.mRange) {
		return;
    }

	// Normalize the light vector.
	lightVector /= surfaceToLightDistance; 
	
	// Ambient term.
	ambient = material.mAmbient * light.mAmbient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	const float diffuseFactor = dot(lightVector, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f) {
		const float3 lightReflection = reflect(-lightVector, normal);
		const float specularFactor = 
            pow(max(dot(lightReflection, toEye), 0.0f),
                material.mSpecular.w);
					
		diffuse = diffuseFactor * material.mDiffuse * light.mDiffuse;
		specular = specularFactor * material.mSpecular * light.mSpecular;
	}

	// Attenuate
	float attenuation = 
        1.0f / dot(light.mAttenuation, 
                   float3(1.0f, 
                          surfaceToLightDistance, 
                          surfaceToLightDistance * surfaceToLightDistance));

	diffuse *= attenuation;
	specular *= attenuation;
}

///////////////////////////////////////////////////////////////
//
// Computes the ambient, diffuse, and specular terms in the lighting equation
// from a spotlight.  We need to output the terms separately because
// later we will modify the individual terms.
//
///////////////////////////////////////////////////////////////
void computeSpotLight(const Material material, 
                      const SpotLight light, 
                      const float3 position, 
                      const float3 normal, 
                      const float3 toEye, 
                      out float4 ambient,
                      out float4 diffuse, 
                      out float4 specular)
{
	// Initialize outputs.
	ambient = float4(0.0f, 0.0f, 0.0f, 0.0f);
	diffuse = float4(0.0f, 0.0f, 0.0f, 0.0f);
	specular = float4(0.0f, 0.0f, 0.0f, 0.0f);

	// The vector from the surface to the light.
	float3 lightVector = light.mPosition - position;
		
	// The distance from surface to light.
	const float surfaceToLightDistance = length(lightVector);
	
	// Range test.
	if (surfaceToLightDistance > light.mRange) {
		return;
    }

	// Normalize the light vector.
	lightVector /= surfaceToLightDistance; 
	
	// Ambient term.
	ambient = material.mAmbient * light.mAmbient;	

	// Add diffuse and specular term, provided the surface is in 
	// the line of site of the light.
	const float diffuseFactor = dot(lightVector, normal);

	// Flatten to avoid dynamic branching.
	[flatten]
	if (diffuseFactor > 0.0f) {
		const float3 lightReflection = reflect(-lightVector, normal);
		const float specularFactor = 
            pow(max(dot(lightReflection, toEye), 0.0f),
                material.mSpecular.w);
					
		diffuse = diffuseFactor * material.mDiffuse * light.mDiffuse;
		specular = specularFactor * material.mSpecular * light.mSpecular;
	}
	
	// Scale by spotlight factor and attenuate.
	const float spotLightFactor = 
        pow(max(dot(-1.0f * lightVector, light.mDirection), 0.0f), 
            light.mSpot);

	// Scale by spotlight factor and attenuate.
	const float attenuation = 
        spotLightFactor / dot(light.mAttenuation, 
                              float3(1.0f, 
                                     surfaceToLightDistance,
                                     surfaceToLightDistance * surfaceToLightDistance));

	ambient *= spotLightFactor;
	diffuse *= attenuation;
	specular *= attenuation;
}

///////////////////////////////////////////////////////////////
//
// Transforms a normal map sample to world space.
//
///////////////////////////////////////////////////////////////
float3 normalSampleToWorldSpace(const float3 normalMapSample, 
                                const float3 unitNormalW, 
                                const float3 tangentW)
{
	// Uncompress each component from [0,1] to [-1,1].
	const float3 normalT = 2.0f * normalMapSample - 1.0f;

	// Build orthonormal basis.
	const float3 N = unitNormalW;
	const float3 T = normalize(tangentW - dot(tangentW, N) * N);
	const float3 B = cross(N, T);

	const float3x3 TBN = float3x3(T, B, N);

	// Transform from tangent space to world space.
	const float3 bumpedNormalW = mul(normalT, TBN);

	return bumpedNormalW;
}

 
 