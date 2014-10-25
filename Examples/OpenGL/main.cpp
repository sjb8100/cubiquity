// Include standard headers
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>

// Include GLEW
#include <GL/glew.h>

// Include GLFW
#include <glfw3.h>
GLFWwindow* window;

// Include GLM
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
using namespace glm;

#include "shader.hpp"
#include "texture.hpp"
#include "controls.hpp"

#include "CubiquityC.h"

GLuint programID;

class OpenGLOctreeNode
{
public:
	OpenGLOctreeNode(OpenGLOctreeNode* parent)
	{
		noOfIndices = 0;
		indexBuffer = 0;
		vertexBuffer = 0;

		posX = 0;
		posY = 0;
		posZ = 0;

		meshLastUpdated = 0;
		oldestMeshSync = 0xFFFFFFFF;
		renderThisNode = false;
		lastChanged = 0;

		this->parent = parent;

		for (uint32_t z = 0; z < 2; z++)
		{
			for (uint32_t y = 0; y < 2; y++)
			{
				for (uint32_t x = 0; x < 2; x++)
				{
					children[x][y][z] = 0;
				}
			}
		}
	}

	GLuint noOfIndices;
	GLuint indexBuffer;
	GLuint vertexBuffer;
	GLuint vertexArrayObject;

	int32_t posX;
	int32_t posY;
	int32_t posZ;

	uint32_t meshLastUpdated;
	uint32_t oldestMeshSync;
	uint32_t renderThisNode;
	uint32_t lastChanged;

	OpenGLOctreeNode* parent;
	OpenGLOctreeNode* children[2][2][2];
};

void validate(int returnCode)
{
	if (returnCode != CU_OK)
	{
		std::cout << cuGetErrorCodeAsString(returnCode) << " : " << cuGetLastErrorMessage() << std::endl;
		exit(EXIT_FAILURE);
	}
}

void processOctreeNode(uint32_t octreeNodeHandle, OpenGLOctreeNode* openGLOctreeNode)
{
	CuOctreeNode octreeNode;
	validate(cuGetOctreeNode(octreeNodeHandle, &octreeNode));

	if ((octreeNode.structureLastChangedRecursive > openGLOctreeNode->lastChanged) || (octreeNode.meshLastChangedRecursive > openGLOctreeNode->oldestMeshSync))
	{
		if (octreeNode.meshLastChanged > openGLOctreeNode->meshLastUpdated)
		{
			if (octreeNode.hasMesh == 1)
			{
				// These will point to the index and vertex data
				uint32_t noOfIndices;
				uint16_t* indices;
				uint16_t noOfVertices;
				void* vertices;

				// Get the index and vertex data
				validate(cuGetNoOfIndices(octreeNodeHandle, &noOfIndices));
				validate(cuGetIndices(octreeNodeHandle, &indices));

				validate(cuGetNoOfVertices(octreeNodeHandle, &noOfVertices));
				validate(cuGetVertices(octreeNodeHandle, &vertices));

				uint32_t volumeType;
				validate(cuGetVolumeType(octreeNodeHandle, &volumeType));

				// Pass it to the OpenGL node.
				openGLOctreeNode->posX = octreeNode.posX;
				openGLOctreeNode->posY = octreeNode.posY;
				openGLOctreeNode->posZ = octreeNode.posZ;

				openGLOctreeNode->noOfIndices = noOfIndices;

				glGenVertexArrays(1, &(openGLOctreeNode->vertexArrayObject));
				glBindVertexArray(openGLOctreeNode->vertexArrayObject);

				glGenBuffers(1, &(openGLOctreeNode->indexBuffer));
				glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, openGLOctreeNode->indexBuffer);
				glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(uint16_t)* noOfIndices, indices, GL_STATIC_DRAW);

				glGenBuffers(1, &(openGLOctreeNode->vertexBuffer));
				glBindBuffer(GL_ARRAY_BUFFER, openGLOctreeNode->vertexBuffer);

				if (volumeType == CU_COLORED_CUBES)
				{
					glBufferData(GL_ARRAY_BUFFER, sizeof(CuColoredCubesVertex)* noOfVertices, vertices, GL_STATIC_DRAW);

					// We pack the encoded position and the encoded normal into a single 
					// vertex attribute to save space: http://stackoverflow.com/a/21680009
					glEnableVertexAttribArray(0);
					glVertexAttribIPointer(0, 4, GL_UNSIGNED_BYTE, sizeof(CuColoredCubesVertex), (GLvoid*)(offsetof(CuColoredCubesVertex, encodedPosX)));

					glEnableVertexAttribArray(1); // Attrib '1' is the first four materials
					glVertexAttribIPointer(1, 1, GL_UNSIGNED_INT, sizeof(CuColoredCubesVertex), (GLvoid*)(offsetof(CuColoredCubesVertex, data)));
				}
				else if (volumeType == CU_TERRAIN)
				{
					glBufferData(GL_ARRAY_BUFFER, sizeof(CuTerrainVertex)* noOfVertices, vertices, GL_STATIC_DRAW);

					// We pack the encoded position and the encoded normal into a single 
					// vertex attribute to save space: http://stackoverflow.com/a/21680009
					glEnableVertexAttribArray(0);
					glVertexAttribIPointer(0, 4, GL_UNSIGNED_SHORT, sizeof(CuTerrainVertex), (GLvoid*)(offsetof(CuTerrainVertex, encodedPosX)));

					glEnableVertexAttribArray(1); // Attrib '1' is the first four materials
					glVertexAttribIPointer(1, 4, GL_UNSIGNED_BYTE, sizeof(CuTerrainVertex), (GLvoid*)(offsetof(CuTerrainVertex, material0)));
				}

				glBindVertexArray(0);
			}

			openGLOctreeNode->meshLastUpdated = octreeNode.meshLastChanged;
		}

		openGLOctreeNode->renderThisNode = octreeNode.renderThisNode;

		openGLOctreeNode->oldestMeshSync = octreeNode.meshLastChanged;

		for (uint32_t z = 0; z < 2; z++)
		{
			for (uint32_t y = 0; y < 2; y++)
			{
				for (uint32_t x = 0; x < 2; x++)
				{
					if (octreeNode.childHandles[x][y][z] != 0xFFFFFFFF && openGLOctreeNode->renderThisNode == 0)
					{
						if (!openGLOctreeNode->children[x][y][z])
						{
							//std::cout << "Adding mesh" << std::endl;
							openGLOctreeNode->children[x][y][z] = new OpenGLOctreeNode(openGLOctreeNode);
						}

						// Recursivly call the octree traversal
						processOctreeNode(octreeNode.childHandles[x][y][z], openGLOctreeNode->children[x][y][z]);

						openGLOctreeNode->oldestMeshSync = (std::min)(openGLOctreeNode->oldestMeshSync, openGLOctreeNode->children[x][y][z]->oldestMeshSync);
					}
					else
					{
						if (openGLOctreeNode->children[x][y][z])
						{
							//std::cout << "Deleting mesh" << std::endl;
							delete openGLOctreeNode->children[x][y][z];
							openGLOctreeNode->children[x][y][z] = nullptr;
						}
					}
				}
			}
		}

		openGLOctreeNode->lastChanged = octreeNode.structureLastChangedRecursive;
	}
}

void renderOpenGLOctreeNode(OpenGLOctreeNode* openGLOctreeNode)
{
	if (openGLOctreeNode->noOfIndices > 0 && openGLOctreeNode->renderThisNode)
	{
		glm::mat4 modelMatrix = glm::translate(glm::mat4(1.0f), glm::vec3(openGLOctreeNode->posX, openGLOctreeNode->posY, openGLOctreeNode->posZ));

		GLuint modelMatrixID = glGetUniformLocation(programID, "modelMatrix");
		glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

		glBindVertexArray(openGLOctreeNode->vertexArrayObject);

		// Draw the triangles!
		glDrawElements(GL_TRIANGLES, openGLOctreeNode->noOfIndices, GL_UNSIGNED_SHORT, 0);

		glBindVertexArray(0);
	}

	for (uint32_t z = 0; z < 2; z++)
	{
		for (uint32_t y = 0; y < 2; y++)
		{
			for (uint32_t x = 0; x < 2; x++)
			{
				if (openGLOctreeNode->children[x][y][z])
				{
					renderOpenGLOctreeNode(openGLOctreeNode->children[x][y][z]);
				}
			}
		}
	}
}

int main( void )
{
	// Initialise GLFW
	if( !glfwInit() )
	{
		fprintf( stderr, "Failed to initialize GLFW\n" );
		return -1;
	}

	glfwWindowHint(GLFW_SAMPLES, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	// Open a window and create its OpenGL context
	window = glfwCreateWindow( 1024, 768, "Tutorial 0 - Keyboard and Mouse", NULL, NULL);
	if( window == NULL ){
		fprintf( stderr, "Failed to open GLFW window. If you have an Intel GPU, they are not 3.3 compatible. Try the 2.1 version of the tutorials.\n" );
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window);

	// Initialize GLEW
	glewExperimental = true; // Needed for core profile
	if (glewInit() != GLEW_OK) {
		fprintf(stderr, "Failed to initialize GLEW\n");
		return -1;
	}

	// Ensure we can capture the escape key being pressed below
	glfwSetInputMode(window, GLFW_STICKY_KEYS, GL_TRUE);
	glfwSetCursorPos(window, 1024/2, 768/2);

	// Dark blue background
	glClearColor(0.0f, 0.0f, 0.4f, 0.0f);

	// Enable depth test
	glEnable(GL_DEPTH_TEST);
	// Accept fragment if it closer to the camera than the former one
	glDepthFunc(GL_LESS); 

	// Cull triangles which normal is not towards the camera
	glEnable(GL_CULL_FACE);

	uint32_t volumeHandle;
	bool coloredCubesMode = false;
	if (coloredCubesMode)
	{
		validate(cuNewColoredCubesVolumeFromVDB("C:/code/cubiquity/Data/Volumes/Version 0/VoxeliensTerrain.vdb", CU_READONLY, 32, &volumeHandle));
		programID = LoadShaders("ColoredCubesVertexShader.glsl", "ColoredCubesFragmentShader.glsl");
	}
	else
	{
		validate(cuNewTerrainVolumeFromVDB("C:/code/cubiquity/Data/Volumes/Version 0/SmoothVoxeliensTerrain.vdb", CU_READONLY, 32, &volumeHandle));
		programID = LoadShaders("TerrainVertexShader.glsl", "TerrainFragmentShader.glsl");
	}
	
	// Get a handle for our "MVP" uniform
	GLuint viewMatrixID = glGetUniformLocation(programID, "viewMatrix");
	GLuint projectionMatrixID = glGetUniformLocation(programID, "projectionMatrix");

	OpenGLOctreeNode* rootOpenGLOctreeNode = 0;
	

	do
	{
		// The framework we're using here doesn't seem to provide easy access to the camera position. The following lines compute it.
		glm::mat4 ViewMatrix = getViewMatrix();
		glm::vec4 eyeSpaceEyePos(0.0, 0.0, 0.0, 1.0);
		glm::mat4 InverseViewMatrix = glm::inverse(ViewMatrix);
		glm::vec4 worldSpaceEyePos = InverseViewMatrix * eyeSpaceEyePos;
		worldSpaceEyePos /= worldSpaceEyePos.w;

		
		validate(cuUpdateVolume(volumeHandle, worldSpaceEyePos[0], worldSpaceEyePos[1], worldSpaceEyePos[2], 1.0f));

		uint32_t hasRootNode;
		validate(cuHasRootOctreeNode(volumeHandle, &hasRootNode));
		if (hasRootNode == 1) // FIXME - Maybe it's easier if there is always a root node?
		{
			if (!rootOpenGLOctreeNode)
			{
				rootOpenGLOctreeNode = new OpenGLOctreeNode(0);
			}

			uint32_t octreeNodeHandle;
			cuGetRootOctreeNode(volumeHandle, &octreeNodeHandle);
			processOctreeNode(octreeNodeHandle, rootOpenGLOctreeNode);
		}
		else
		{
			if (rootOpenGLOctreeNode)
			{
				delete rootOpenGLOctreeNode;
				rootOpenGLOctreeNode = nullptr;
			}
		}
		

		// Clear the screen
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// Use our shader
		glUseProgram(programID);

		// Compute the MVP matrix from keyboard and mouse input
		computeMatricesFromInputs();
		glm::mat4 viewMatrix = getViewMatrix();
		glm::mat4 projectionMatrix = getProjectionMatrix();



		// Send our transformations to the currently bound shader
		glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, &viewMatrix[0][0]);
		glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, &projectionMatrix[0][0]);

		if (rootOpenGLOctreeNode)
		{
			renderOpenGLOctreeNode(rootOpenGLOctreeNode);
		}

		// Swap buffers
		glfwSwapBuffers(window);
		glfwPollEvents();

	} // Check if the ESC key was pressed or the window was closed
	while( glfwGetKey(window, GLFW_KEY_ESCAPE ) != GLFW_PRESS &&
		   glfwWindowShouldClose(window) == 0 );

	// Cleanup VBO and shader
	glDeleteProgram(programID);

	// Close OpenGL window and terminate GLFW
	glfwTerminate();

	// Delete the volume from memory (doesn't delete from disk).
	validate(cuDeleteVolume(volumeHandle));

	return 0;
}

