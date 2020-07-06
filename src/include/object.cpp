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

#include <glm/glm.hpp>

class ObjectDetails
{
	friend class ObjectManager;

private:
	std::string objectName;
	std::string objectFilePath;

	std::vector<glm::vec3> vertices;

	GLuint vertexBufferId;
	GLuint uvBufferId;
	GLuint normalBufferId;
	uint bufferSize;

public:
	ObjectDetails(
			std::string objectName,
			std::string objectFilePath,
			std::vector<glm::vec3> vertices,
			GLuint vertexBufferId,
			GLuint uvBufferId,
			GLuint normalBufferId,
			uint bufferCount)
			: objectName(objectName),
				objectFilePath(objectFilePath),
				vertices(vertices),
				vertexBufferId(vertexBufferId),
				uvBufferId(uvBufferId),
				normalBufferId(normalBufferId),
				bufferSize(bufferCount) {}

	std::string getObjectName()
	{
		return objectName;
	}

	std::vector<glm::vec3> &getVertices()
	{
		return vertices;
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

	uint loadObjObject(std::string objectName, std::string objectFilePath, std::vector<glm::vec3> &outVertices, GLuint *vertexBufferId, GLuint *uvBufferId, GLuint *normalBufferId)
	{
		std::vector<unsigned int> vertexIndices, uvIndices, normalIndices;
		std::vector<glm::vec3> tempVertices;
		std::vector<glm::vec2> tempUvs;
		std::vector<glm::vec3> tempNormals;

		std::vector<glm::vec2> outUvs;
		std::vector<glm::vec3> outNormals;

		auto file = fopen(objectFilePath.c_str(), "r");
		if (file == NULL)
		{
			std::cout << "Failed at object 1" << std::endl;
			exit(1);
		}

		while (1)
		{
			char lineHeader[128];
			int res = fscanf(file, "%s", lineHeader);
			if (res == EOF)
			{
				break;
			}

			if (strcmp(lineHeader, "v") == 0)
			{
				glm::vec3 vertex;
				fscanf(file, "%f %f %f\n", &vertex.x, &vertex.y, &vertex.z);
				tempVertices.push_back(vertex);
			}
			else if (strcmp(lineHeader, "vt") == 0)
			{
				glm::vec2 uv;
				fscanf(file, "%f %f\n", &uv.x, &uv.y);
				tempUvs.push_back(uv);
			}
			else if (strcmp(lineHeader, "vn") == 0)
			{
				glm::vec3 normal;
				fscanf(file, "%f %f %f\n", &normal.x, &normal.y, &normal.z);
				tempNormals.push_back(normal);
			}
			else if (strcmp(lineHeader, "f") == 0)
			{
				std::string vertex1, vertex2, vertex3;
				unsigned int vertexIndex[3], uvIndex[3], normalIndex[3];
				int matches = fscanf(file, "%d/%d/%d %d/%d/%d %d/%d/%d\n", &vertexIndex[0], &uvIndex[0], &normalIndex[0], &vertexIndex[1], &uvIndex[1], &normalIndex[1], &vertexIndex[2], &uvIndex[2], &normalIndex[2]);
				if (matches != 9)
				{
					fclose(file);
					std::cout << "Failed at object 2" << std::endl;
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
				char stupidBuffer[1000];
				fgets(stupidBuffer, 1000, file);
			}
		}

		for (unsigned int i = 0; i < vertexIndices.size(); i++)
		{
			auto vertexIndex = vertexIndices[i];
			auto uvIndex = uvIndices[i];
			auto normalIndex = normalIndices[i];

			auto vertex = tempVertices[vertexIndex - 1];
			auto uv = tempUvs[uvIndex - 1];
			auto normal = tempNormals[normalIndex - 1];

			outVertices.push_back(vertex);
			outUvs.push_back(uv);
			outNormals.push_back(normal);
		}
		fclose(file);

		*vertexBufferId = createBuffer(outVertices);
		*uvBufferId = createBuffer(outUvs);
		*normalBufferId = createBuffer(outNormals);

		return outVertices.size();
	}

	ObjectManager()
			: namedObjects({}),
				namedObjectReferences({}) {}

public:
	ObjectManager(ObjectManager &) = delete;

	std::shared_ptr<ObjectDetails> &createObject(std::string objectName, std::string objectFilePath)
	{
		auto existingObject = namedObjects.find(objectName);
		if (existingObject != namedObjects.end())
		{
			namedObjectReferences[objectName]++;
			return existingObject->second;
		}

		std::vector<glm::vec3> vertices;
		GLuint vertexBufferId;
		GLuint uvBufferId;
		GLuint normalBufferId;

		uint bufferSize = loadObjObject(objectName, objectFilePath, vertices, &vertexBufferId, &uvBufferId, &normalBufferId);

		auto newObject = std::make_shared<ObjectDetails>(objectName, objectFilePath, vertices, vertexBufferId, uvBufferId, normalBufferId, bufferSize);

		namedObjects.insert(std::pair<std::string, std::shared_ptr<ObjectDetails>>(objectName, newObject));
		namedObjectReferences[objectName] = 1;

		return namedObjects[objectName];
	}

	std::shared_ptr<ObjectDetails> &getObjectDetails(std::string objectName)
	{
		return namedObjects[objectName];
	}

	void destroyObject(std::shared_ptr<ObjectDetails> &objectDetails)
	{
		namedObjectReferences[objectDetails->getObjectName()]--;
		if (namedObjectReferences[objectDetails->getObjectName()] <= 0)
		{
			namedObjectReferences.erase(objectDetails->getObjectName());
			namedObjects.erase(objectDetails->getObjectName());
			glDeleteBuffers(1, &objectDetails->vertexBufferId);
			glDeleteBuffers(1, &objectDetails->uvBufferId);
			glDeleteBuffers(1, &objectDetails->normalBufferId);
		}
	}

	static ObjectManager &getInstance()
	{
		return instance;
	}
};

ObjectManager ObjectManager::instance;

#endif