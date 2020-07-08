#ifndef INCLUDE_OBJECT_CPP
#define INCLUDE_OBJECT_CPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <string.h>

#include <GL/glew.h>
#include <glm/glm.hpp>

/**
 * Class for containing the details of the object.
 */
class ObjectDetails
{
	// Let the object manager access private variables.
	friend class ObjectManager;

private:
	// The name of the object.
	std::string objectName;
	// The file path to the object data.
	std::string objectFilePath;

	// The list of vertices of the object.
	std::vector<glm::vec3> vertices;

	// The ID of the array buffer containing the vertex position data of the object.
	GLuint vertexBufferId;
	// The ID of the array buffer containing the vertex UV coordinates data of the object.
	GLuint uvBufferId;
	// The ID of the array buffer containing the vertex normal vector data of the object.
	GLuint normalBufferId;
	// The size of the buffer/number of vertices of the object.
	unsigned int bufferSize;

public:
	ObjectDetails(
			std::string objectName,
			std::string objectFilePath,
			std::vector<glm::vec3> vertices,
			GLuint vertexBufferId,
			GLuint uvBufferId,
			GLuint normalBufferId,
			unsigned int bufferCount)
			: objectName(objectName),
				objectFilePath(objectFilePath),
				vertices(vertices),
				vertexBufferId(vertexBufferId),
				uvBufferId(uvBufferId),
				normalBufferId(normalBufferId),
				bufferSize(bufferCount) {}

	/**
   * Get the name of the object.
   * 
   * @return The object name.
   */
	std::string getObjectName()
	{
		return objectName;
	}

	/**
   * Get the list of vertices of the object.
   * 
   * @return The object vertices.
   */
	std::vector<glm::vec3> &getVertices()
	{
		return vertices;
	}

	/**
   * Get the array buffer ID of the object vertex positions.
   * 
   * @return The array buffer ID.
   */
	GLuint getVertexBufferId()
	{
		return vertexBufferId;
	}

	/**
   * Get the array buffer ID of the object vertex UV coordinates.
   * 
   * @return The array buffer ID.
   */
	GLuint getUvBufferId()
	{
		return uvBufferId;
	}

	/**
   * Get the array buffer ID of the object vertex normal vectors.
   * 
   * @return The array buffer ID.
   */
	GLuint getNormalBufferId()
	{
		return normalBufferId;
	}

	/**
   * Get the size of the buffer/number of vertices of the object.
   * 
   * @return The number of vertices.
   */
	unsigned int getBufferSize()
	{
		return bufferSize;
	}
};

/**
 * A manager class for managing objects used by models.
 */
class ObjectManager
{
private:
	// Singleton instance of the object manager.
	static ObjectManager instance;

	// A map of created objects.
	std::map<std::string, std::shared_ptr<ObjectDetails>> namedObjects;
	// A map counting the references to the created objects.
	std::map<std::string, int> namedObjectReferences;

	/**
	 * Create a array buffer of the given vector type, and store data as static draw use.
	 * 
	 * @param bufferData  The data to store in the buffer.
	 * 
	 * @return The ID of the array buffer.
	 */
	template <typename VecType>
	GLuint createBuffer(std::vector<VecType> &bufferData)
	{
		// Define a variable for storing the buffer ID.
		GLuint bufferId;
		// Create a new buffer and store the ID.
		glGenBuffers(1, &bufferId);
		// Bind the buffer as an array buffer.
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
		// Store the data into the array buffer, with usage set as static draw (won't be modified, and will be used for drawing repeatedly).
		glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(VecType), &bufferData[0], GL_STATIC_DRAW);
		// Unbind the buffer now that we're done.
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		// Return the ID of the created array buffer.
		return bufferId;
	}

	/**
	 * Load the OBJ object file and create array buffers for it.
	 * 
	 * @param objectName      The name of the object being loaded.
	 * @param objectFilePath  The file path to the object data.
	 * @param outVertices     The vector to store the object vertices to.
	 * 
	 * @return The number of vertices in the object.
	 */
	unsigned int loadObjObject(std::string objectName, std::string objectFilePath, std::vector<glm::vec3> &outVertices, GLuint *vertexBufferId, GLuint *uvBufferId, GLuint *normalBufferId)
	{
		// Define vectors for storing the indices to the vertex information.
		std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
		// Define temporary vectors for storing the vertex information stored in the OBJ file.
		std::vector<glm::vec3> tempVertices;
		std::vector<glm::vec2> tempUvs;
		std::vector<glm::vec3> tempNormals;

		// Define vectors for storing the final list of vertex information of the object in order of their use for each triangle.
		std::vector<glm::vec2> outUvs;
		std::vector<glm::vec3> outNormals;

		// Open the OBJ file.
		auto file = fopen(objectFilePath.c_str(), "r");
		// Check if the file is accessible.
		if (file == NULL)
		{
			// Could not read the object file. Time to crash.
			std::cout << objectName << std::endl
								<< "Failed at object 1" << std::endl;
			exit(1);
		}

		// Keep reading the file until we hit a condition.
		while (1)
		{
			// Define a variable for storing the first string in a line
			char lineHeader[128];
			// Read a string from the file.
			int res = fscanf(file, "%s", lineHeader);
			// Check if we hit the end of the file.
			if (res == EOF)
			{
				// Completed reading the file. So break out of the loop.
				break;
			}

			// If the string equals "v".
			if (strcmp(lineHeader, "v") == 0)
			{
				// Line defines a vertex position data.
				// Define a variable for storing the vertex position.
				glm::vec3 vertex;
				// Read the position coordinates of the vertex from the file.
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				// Store it in the temporary vertex position vector.
				tempVertices.push_back(vertex);
			}
			// If the string equals "vt".
			else if (strcmp(lineHeader, "vt") == 0)
			{
				// Line defines a vertex UV coordinates data.
				// Define a variable for storing the vertex UV coordinates.
				glm::vec2 uv;
				// Read the UV coordinates of the vertex from the file.
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				// Store it in the temporary vertex UV coordinates vector.
				tempUvs.push_back(uv);
			}
			// If the string equals "vt".
			else if (strcmp(lineHeader, "vn") == 0)
			{
				// Line defines a vertex normal vector data.
				// Define a variable for storing the vertex normal vector.
				glm::vec3 normal;
				// Read the normal vector of the vertex from the file.
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				// Store it in the temporary vertex normal vector vector.
				tempNormals.push_back(normal);
			}
			// If the string equals "f".
			else if (strcmp(lineHeader, "f") == 0)
			{
				// Line defines the indexes of the vertex information that describes a single face/polygon of the object.
				// Define a variable for storing the indices to the vertex information stored in the temp vectors.
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				// Read the indices to the vertex information.
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9)
				{
					// If we don't manage to read all nine coordinates, then this OBJ file is formatted in a way that we can't support. Close the file and time to crash.
					fclose(file);
					std::cout << objectName << std::endl
										<< "Failed at object 2" << std::endl;
					exit(1);
				}

				// Push the vertex position indices of the face/polygon into the vertex position vector.
				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);

				// Push the vertex UV coordinates indices of the face/polygon into the vertex UV coordinates vector.
				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);

				// Push the vertex normal vector indices of the face/polygon into the vertex normal vector vector.
				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
			else
			{
				// Some information about the object we don't care about. Read the entire line and ignore it.
				char ingoreBuffer[1000];
				fgets(ingoreBuffer, 1000, file);
			}
		}

		// Done reading the file, so close it.
		fclose(file);

		// Loop through the vertex indices of the faces/polygons that we read.
		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			// Grab the indices of the vertex information that represent the face/polygon.
			auto vertexIndex = vertexIndices[i];
			auto uvIndex = uvIndices[i];
			auto normalIndex = normalIndices[i];

			// Grab the actual vertex information that the indices point to.
			auto vertex = tempVertices[vertexIndex - 1];
			auto uv = tempUvs[uvIndex - 1];
			auto normal = tempNormals[normalIndex - 1];

			// Store the vertex information into the final output vectors.
			outVertices.push_back(vertex);
			outUvs.push_back(uv);
			outNormals.push_back(normal);
		}

		// Create buffers for the vertex information, and store them in the buffer ID output variables
		*vertexBufferId = createBuffer(outVertices);
		*uvBufferId = createBuffer(outUvs);
		*normalBufferId = createBuffer(outNormals);

		// Return the number of vertices that were read from the OBJ file.
		return outVertices.size();
	}

	ObjectManager()
			: namedObjects({}),
				namedObjectReferences({}) {}

public:
	// Preventing copying the object manager, making sure only one instance can exist.
	ObjectManager(ObjectManager &) = delete;

	/**
	 * Load and create an object from the given object file path. If an object with the same name was already created,
	 * return the same object.
	 * 
	 * @param objectName      The name of the object.
	 * @param objectFilePath  The file path to the object data.
	 * 
	 * @return The details of the loaded object.
	 */
	std::shared_ptr<ObjectDetails> &createObject(std::string objectName, std::string objectFilePath)
	{
		// Check if an object with the name already exists.
		auto existingObject = namedObjects.find(objectName);
		if (existingObject != namedObjects.end())
		{
			// Object already loaded. Increase its reference count and return it.
			namedObjectReferences[objectName]++;
			return existingObject->second;
		}

		// Create variables for storing the object details after loading it.
		std::vector<glm::vec3> vertices;
		GLuint vertexBufferId;
		GLuint uvBufferId;
		GLuint normalBufferId;

		// Load the OBJ object file and store its details.
		unsigned int bufferSize = loadObjObject(objectName, objectFilePath, vertices, &vertexBufferId, &uvBufferId, &normalBufferId);

		// Create a new object details with the captured data.
		auto newObject = std::make_shared<ObjectDetails>(objectName, objectFilePath, vertices, vertexBufferId, uvBufferId, normalBufferId, bufferSize);

		// Insert the newly created object into the map of created objects.
		namedObjects[objectName] = newObject;
		// Set the reference count of the object to 1.
		namedObjectReferences[objectName] = 1;

		// Return the object details.
		return namedObjects[objectName];
	}

	/**
   * Return the object created with the given name.
   * 
   * @param objectName  The name of the object to return.
   * 
   * @return The object created with the given name.
   */
	std::shared_ptr<ObjectDetails> &getObjectDetails(std::string objectName)
	{
		return namedObjects[objectName];
	}

	/**
	 * Delete a reference to the object, and destroy it if no more references are present.
	 * 
	 * @param objectDetails  The details of the object to destroy.
	 */
	void destroyObject(std::shared_ptr<ObjectDetails> &objectDetails)
	{
		// Reduce the reference count of the object.
		namedObjectReferences[objectDetails->getObjectName()]--;
		// Check if there are no more references to the object.
		if (namedObjectReferences[objectDetails->getObjectName()] <= 0)
		{
			// No more references left, so time to clean.
			// Remove the object from the created objects references map.
			namedObjectReferences.erase(objectDetails->getObjectName());
			// Remove the object from the created objects map.
			namedObjects.erase(objectDetails->getObjectName());
			// Delete the array buffer containing the vertex position data of the object.
			glDeleteBuffers(1, &objectDetails->vertexBufferId);
			// Delete the array buffer containing the vertex UV coordinates data of the object.
			glDeleteBuffers(1, &objectDetails->uvBufferId);
			// Delete the array buffer containing the vertex normal vector data of the object.
			glDeleteBuffers(1, &objectDetails->normalBufferId);
		}
	}

	/**
   * Returns the singleton instance of the object manager.
   * 
   * @return The object manager singleton instance.
   */
	static ObjectManager &getInstance()
	{
		return instance;
	}
};

// Initialize the object manager singleton instance static variable.
ObjectManager ObjectManager::instance;

#endif