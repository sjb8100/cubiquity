// Inputs
attribute vec4 a_position;									// Vertex Position							(x, y, z, w)
attribute vec3 a_normal;									// Vertex Normal							(x, y, z)
attribute vec3 a_color;										// Output Vertex Color

// Outputs
varying vec3 v_color;										// Output Vertex Color 
varying vec3 v_normalVector;

// Uniforms
uniform mat4 u_worldViewProjectionMatrix;					// Matrix to transform a position to clip space.
uniform mat4 u_inverseTransposeModelToWorldMatrix;				// Matrix to transform a normal to view space

void main()
{
    // Transform position to clip space.
    gl_Position = u_worldViewProjectionMatrix * a_position;

    // Transform normal to world space.
    mat3 inverseTransposeModelToWorldMatrix = mat3(u_inverseTransposeModelToWorldMatrix[0].xyz, u_inverseTransposeModelToWorldMatrix[1].xyz, u_inverseTransposeModelToWorldMatrix[2].xyz);
    v_normalVector = inverseTransposeModelToWorldMatrix * a_normal;
    
    // Pass the vertex color to fragment shader
	v_color = a_color;
}