#ifndef INCLUDE_OBJLOADER_CPP
#define INCLUDE_OBJLOADER_CPP

#include <string>
#include <vector>
#include <map>
#include <iostream>
#include <fstream>

#include <stdlib.h>
#include <string.h>

#include <glm/glm.hpp>

class ObjectDetails
{
	friend class ObjectManager;

private:
	std::string objectName;
	std::string objectFilePath;

	GLuint vertexBufferId;
	GLuint uvBufferId;
	GLuint normalBufferId;
	uint bufferSize;

public:
	ObjectDetails(
			std::string objectName,
			std::string objectFilePath,
			GLuint vertexBufferId,
			GLuint uvBufferId,
			GLuint normalBufferId,
			uint bufferCount)
			: objectName(objectName),
				objectFilePath(objectFilePath),
				vertexBufferId(vertexBufferId),
				uvBufferId(uvBufferId),
				normalBufferId(normalBufferId),
				bufferSize(bufferCount)
	{
		std::cout << "[ObjectDetails] "
							<< "Constructing ObjectDetails." << std::endl;
		std::cout << "[ObjectDetails] "
							<< "Constructed ObjectDetails." << std::endl;
	}

	std::string getObjectName()
	{
		return objectName;
	}

	GLuint getVertexBufferId()
	{
		return vertexBufferId;
	}

	GLuint getUvBufferId()
	{
		return uvBufferId;
	}

	GLuint getNormalBufferId()
	{
		return normalBufferId;
	}

	uint getBufferSize()
	{
		return bufferSize;
	}
};

class ObjectManager
{
private:
	static ObjectManager instance;

	std::map<std::string, std::shared_ptr<ObjectDetails>> namedObjects;
	std::map<std::string, int> namedObjectReferences;

	template <typename VecType>
	GLuint createBuffer(std::vector<VecType> &bufferData)
	{
		GLuint bufferId;
		glGenBuffers(1, &bufferId);
		glBindBuffer(GL_ARRAY_BUFFER, bufferId);
		glBufferData(GL_ARRAY_BUFFER, bufferData.size() * sizeof(VecType), &bufferData[0], GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return bufferId;
	}

	uint loadObjObject(std::string objectName, std::string objectFilePath, GLuint *vertexBufferId, GLuint *uvBufferId, GLuint *normalBufferId)
	{
		std::cout << "[ObjectManager] "
							<< "Loading object: " << objectName << " (" << objectFilePath << ")" << std::endl;

		// Information that we need to read from the OBJ file.
		std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
		std::vector<glm::vec3> tempVertices;
		std::vector<glm::vec2> tempUvs;
		std::vector<glm::vec3> tempNormals;

		std::vector<glm::vec3> outVertices;
		std::vector<glm::vec2> outUvs;
		std::vector<glm::vec3> outNormals;

		std::cout
				<< "[ObjectManager] "
				<< "Reading OBJ file for object: " << objectName << std::endl
				<< "[ObjectManager] "
				<< "\tFile: " << objectFilePath << std::endl;

		// Open the texture OBJ file.
		auto file = fopen(objectFilePath.c_str(), "r");
		if (file == NULL)
		{
			std::cout << "[ObjectManager] "
								<< "Could not open OBJ file for object: " << objectName << std::endl
								<< "[ObjectManager] "
								<< "\tFile: " << objectFilePath << std::endl;
			exit(1);
		}

		// Read the data from the file.
		while (1)
		{
			char lineHeader[128];
			// Read the first word from the current line.
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
			{
				// If we hit the end of the file, break and quit reading.
				break;
			}

			// The current line contains vertex position information. Read it into the vertex vector.
			if (strcmp(lineHeader, "v") == 0)
			{
				glm::vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				tempVertices.push_back(vertex);
			}
			// The current line contains vertex UV information. Read it into the uv vector.
			else if (strcmp(lineHeader, "vt") == 0)
			{
				glm::vec2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				uv.y = -uv.y; // Invert V coordinate since we will only use DDS texture, which are inverted. Remove if you want to use TGA or BMP loaders.
				tempUvs.push_back(uv);
			}
			// The current line contains vertex normal information. Read it into the normal vector.
			else if (strcmp(lineHeader, "vn") == 0)
			{
				glm::vec3 normal;
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				tempNormals.push_back(normal);
			}
			// This lets us know what vertex positions, UVs, and normals are supposed to go together, so record the indexes.
			else if (strcmp(lineHeader, "f") == 0)
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9)
				{
					std::cout << "[ObjectManager] "
										<< "Complex OBJ file: " << objectFilePath << std::endl
										<< "[ObjectManager] "
										<< "\tCannot continue." << std::endl;
					fclose(file);
					exit(1);
				}

				vertexIndices.push_back(vertexIndex[0]);
				vertexIndices.push_back(vertexIndex[1]);
				vertexIndices.push_back(vertexIndex[2]);

				uvIndices.push_back(uvIndex[0]);
				uvIndices.push_back(uvIndex[1]);
				uvIndices.push_back(uvIndex[2]);

				normalIndices.push_back(normalIndex[0]);
				normalIndices.push_back(normalIndex[1]);
				normalIndices.push_back(normalIndex[2]);
			}
			else
			{
				// If nothing else, the line can be ignored, so skip.
				char stupidBuffer[1000];
				fgets(stupidBuffer, 1000, file);
			}
		}

		std::cout << "[ObjectManager] "
							<< "Storing OBJ data into output vertex attribute vectors: " << objectName << std::endl;

		// For each vertex of each triangle.
		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{

			// Get the indices of a single vertex's attributes.
			auto vertexIndex = vertexIndices[i];
			auto uvIndex = uvIndices[i];
			auto normalIndex = normalIndices[i];

			// Get the attributes of the vertex.
			auto vertex = tempVertices[vertexIndex - 1];
			auto uv = tempUvs[uvIndex - 1];
			auto normal = tempNormals[normalIndex - 1];

			// Store the attributes of the vertex in the output vectors.
			outVertices.push_back(vertex);
			outUvs.push_back(uv);
			outNormals.push_back(normal);
		}
		fclose(file);

		std::cout << "[ObjectManager] "
							<< "Creating object buffers." << std::endl;
		*vertexBufferId = createBuffer(outVertices);
		*uvBufferId = createBuffer(outUvs);
		*normalBufferId = createBuffer(outNormals);

		std::cout << "[ObjectManager] "
							<< "Loaded OBJ object loader." << std::endl;
		return outVertices.size();
	}

	ObjectManager()
			: namedObjects({}),
				namedObjectReferences({})
	{
		std::cout << "[ObjectManager] "
							<< "Constructing ObjectManager." << std::endl;
		std::cout << "[ObjectManager] "
							<< "Constructed ObjectManager." << std::endl;
	}

public:
	ObjectManager(ObjectManager &) = delete;

	std::shared_ptr<ObjectDetails> &createObject(std::string objectName, std::string objectFilePath)
	{
		std::cout << "[ObjectManager] "
							<< "Creating object: " << objectName << "(" << objectFilePath << ")" << std::endl;

		auto existingObject = namedObjects.find(objectName);
		if (existingObject != namedObjects.end())
		{
			std::cout << "[ObjectManager] "
								<< "Already created. Returning existing object." << std::endl;
			namedObjectReferences[objectName]++;
			return existingObject->second;
		}

		GLuint vertexBufferId;
		GLuint uvBufferId;
		GLuint normalBufferId;

		uint bufferSize = loadObjObject(objectName, objectFilePath, &vertexBufferId, &uvBufferId, &normalBufferId);

		auto newObject = std::make_shared<ObjectDetails>(objectName, objectFilePath, vertexBufferId, uvBufferId, normalBufferId, bufferSize);

		namedObjects.insert(std::pair<std::string, std::shared_ptr<ObjectDetails>>(objectName, newObject));
		namedObjectReferences[objectName] = 1;

		std::cout << "[ObjectManager] "
							<< "Created object." << std::endl;
		return namedObjects[objectName];
	}

	std::shared_ptr<ObjectDetails> &getObjectDetails(std::string objectName)
	{
		return namedObjects[objectName];
	}

	void destroyObject(std::shared_ptr<ObjectDetails> &objectDetails)
	{
		std::cout << "[ObjectManager] "
							<< "Dereferencing object: " << objectDetails->getObjectName() << std::endl;

		namedObjectReferences[objectDetails->getObjectName()]--;
		if (namedObjectReferences[objectDetails->getObjectName()] <= 0)
		{
			std::cout << "[ObjectManager] "
								<< "Destroying object: " << objectDetails->getObjectName() << std::endl;

			namedObjectReferences.erase(objectDetails->getObjectName());
			namedObjects.erase(objectDetails->getObjectName());
			glDeleteBuffers(1, &objectDetails->vertexBufferId);
			glDeleteBuffers(1, &objectDetails->uvBufferId);
			glDeleteBuffers(1, &objectDetails->normalBufferId);

			std::cout << "[ObjectManager] "
								<< "Destroyed object." << std::endl;
		}
	}

	static ObjectManager &getInstance()
	{
		return instance;
	}
};

ObjectManager ObjectManager::instance;

#endif