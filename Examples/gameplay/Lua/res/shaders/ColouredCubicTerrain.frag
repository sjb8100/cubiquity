#ifdef OPENGL_ES
precision highp float;
#endif

// Inputs
varying vec4 v_color;
varying vec4 v_modelSpacePosition;
varying vec4 v_worldSpacePosition;

// Uniforms
uniform sampler2D u_diffuseTexture;             // Diffuse map texture
uniform sampler2D u_normalmapTexture;       	// Normalmap texture
uniform vec3 u_ambientColor;                    // Ambient color
uniform vec3 u_lightColor;                      // Light color
uniform vec3 u_lightDirection;					// Light direction
uniform float u_specularExponent;				// Specular exponent
uniform vec3 u_cameraPosition;
uniform mat4 u_worldViewMatrix;
uniform mat4 u_inverseTransposeWorldViewMatrix;

// Varyings




// This one is named like a varying parameter as this is how Gameplay
// expects the normal to have been passed in, but actually it is not 
// varying and we are instead calculating the normal in the fragment shader.
vec3 v_normalVector;
vec2 v_texCoord;
vec3 v_cameraDirection;
vec3 v_lightDirection;

vec4 a_position;

vec4 _baseColor;
vec3 _ambientColor;
vec3 _diffuseColor;
vec3 _specularColor;

void applyLight(mat3 tangentSpaceTransformMatrix)
{
    // Transform light direction to tangent space
    v_lightDirection = tangentSpaceTransformMatrix * u_lightDirection;
    
    // Compute the camera direction for specular lighting
	vec4 positionWorldViewSpace = u_worldViewMatrix * a_position;
    v_cameraDirection = u_cameraPosition - positionWorldViewSpace.xyz;
}

vec3 computeLighting(vec3 normalVector, vec3 lightDirection, float attenuation, vec3 cameraDirection)
{
    // Ambient
    _ambientColor = _baseColor.rgb * u_ambientColor;

    // Diffuse
    float ddot = dot(normalVector, lightDirection);
    float diffuseIntensity = attenuation * ddot;
    diffuseIntensity = max(0.0, diffuseIntensity);
    _diffuseColor = u_lightColor * _baseColor.rgb * diffuseIntensity;

    // Specular
    vec3 halfVector = normalize(lightDirection + cameraDirection);
    float specularIntensity = attenuation * max(0.0, pow(dot(normalVector, halfVector), u_specularExponent));
    specularIntensity = max(0.0, specularIntensity);
    _specularColor = u_lightColor * _baseColor.rgb * specularIntensity;
	
	return _ambientColor + _diffuseColor + _specularColor;
}

vec3 getLitPixel()
{
    // Fetch normals from the normal map
    vec3 normalVector = normalize(texture2D(u_normalmapTexture, v_texCoord).rgb * 2.0 - 1.0);
    vec3 lightDirection = normalize(v_lightDirection);
    
    vec3 cameraDirection = normalize(v_cameraDirection);
    return computeLighting(normalVector, -lightDirection, 1.0, cameraDirection);
}

void main()
{
    a_position = v_modelSpacePosition;
    
    // Calculate the normal vector in model space (as would normally be passed into the vertex shader).
    v_normalVector = normalize(cross(dFdx(v_modelSpacePosition.xyz), dFdy(v_modelSpacePosition.xyz))); 
    // This fixes normal corruption which has been seen.
    v_normalVector = floor(v_normalVector + vec3(0.5, 0.5, 0.5));
    
    // Transform the normal, tangent and binormals to view space.
	mat3 inverseTransposeWorldViewMatrix = mat3(u_inverseTransposeWorldViewMatrix[0].xyz, u_inverseTransposeWorldViewMatrix[1].xyz, u_inverseTransposeWorldViewMatrix[2].xyz);
    vec3 normalVector = normalize(inverseTransposeWorldViewMatrix * v_normalVector);
    
    vec3 tangent = normalVector.yzx;
    vec3 binormal = normalVector.zxy;
    
    // Create a transform to convert a vector to tangent space.
    vec3 tangentVector  = normalize(inverseTransposeWorldViewMatrix * tangent);
    vec3 binormalVector = normalize(inverseTransposeWorldViewMatrix * binormal);
    mat3 tangentSpaceTransformMatrix = mat3(tangentVector.x, binormalVector.x, normalVector.x, tangentVector.y, binormalVector.y, normalVector.y, tangentVector.z, binormalVector.z, normalVector.z);
    
    // Apply light.
    applyLight(tangentSpaceTransformMatrix);
    
    //Compute texture coordinates
    v_texCoord = vec2(dot(v_worldSpacePosition.xyz, v_normalVector.yzx), dot(v_worldSpacePosition.xyz, v_normalVector.zxy));
    //v_texCoord /= 9.0;
    v_texCoord += 0.5;
    
    // Compute noise. Ideally we would pull a noise value from a 3D texture based on the position of the voxel,
    // but gameplay only seems to support 2D textures at the moment. Therefore we store the texture 'slices'
    // above each other to give a texture which is x pixels wide and y=x*x pixels high.
    const float noiseTextureBaseSize = 16.0; //Size of our 3D texture, actually the width of our 2D replacement.
    const float noiseStrength = 0.04;
    vec3 voxelCentre = v_worldSpacePosition.xyz - (v_normalVector * 0.5); // Back along normal takes us towards center of voxel.
    voxelCentre = floor(voxelCentre + vec3(0.5)); // 'floor' is more widely supported than 'round'.
    vec2 noiseTextureSmaplePos = vec2(voxelCentre.x, voxelCentre.y + voxelCentre.z * noiseTextureBaseSize);
    noiseTextureSmaplePos = noiseTextureSmaplePos / vec2(noiseTextureBaseSize, noiseTextureBaseSize * noiseTextureBaseSize);
    vec3 noise = texture2D(u_diffuseTexture, noiseTextureSmaplePos).rgb; // Sample the texture.
    noise = noise * 2.0 - 1.0; // Adjust range to be -1.0 to +1.0
    noise *= noiseStrength; // Scale to desired strength.
    
    //Form the base color by applying noise to the colour which was passed in.
    _baseColor = vec4(v_color.rgb + noise, 1.0) ;
    
    //_baseColor *= 0.001;
    //_baseColor += 1.0;

    // Light the pixel
    gl_FragColor.a = _baseColor.a;
    #if defined(TEXTURE_DISCARD_ALPHA)
    if (gl_FragColor.a < 0.5)
        discard;
    #endif
    gl_FragColor.rgb = getLitPixel();
	
	// Global color modulation
	#if defined(MODULATE_COLOR)
	gl_FragColor *= u_modulateColor;
	#endif
	#if defined(MODULATE_ALPHA)
    gl_FragColor.a *= u_modulateAlpha;
    #endif
}