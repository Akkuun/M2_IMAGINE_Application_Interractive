// --------------------------------------------------
// shader definition
// --------------------------------------------------

#version 130

// --------------------------------------------------
// Uniform variables:
// --------------------------------------------------
	uniform float xCutPosition;
	uniform float yCutPosition;
	uniform float zCutPosition;

	uniform float xMax;
	uniform float yMax;
	uniform float zMax;
	
	uniform float xMin;
	uniform float yMin;
	uniform float zMin;

	uniform mat4 mv_matrix;
	uniform mat4 proj_matrix;

// --------------------------------------------------
// varying variables
// --------------------------------------------------
varying vec3 position;
varying vec3 textCoord;
// --------------------------------------------------
// Vertex-Shader
// --------------------------------------------------


void main()
{
	gl_Position = proj_matrix * mv_matrix * gl_Vertex;
	position = gl_Vertex.xyz;

	//Todo : compute textCoord DONE - normalisation simplifiée car xMin=yMin=zMin=0
	textCoord = vec3(
		position.x / xMax, 
		position.y / yMax, 
		position.z / zMax
	);

}
