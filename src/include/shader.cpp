#ifndef INCLUDE_SHADER_CPP
#define INCLUDE_SHADER_CPP

#include <string>
#include <vector>
#include <map>
#include <memory>
#include <iostream>
#include <fstream>
#include <sstream>

#include <GL/glew.h>

class ShaderDetails
{
	friend class ShaderManager;

private:
	GLuint shaderId;
	std::string shaderName;
	std::string vertexShaderFilePath;
	std::string geometryShaderFilePath;
	std::string fragmentShaderFilePath;

public:
	ShaderDetails(GLuint shaderId, std::string shaderName, std::string vertexShaderFilePath, std::string geometryShaderFilePath, std::string fragmentShaderFilePath)
			: shaderId(shaderId),
				shaderName(shaderName),
				vertexShaderFilePath(vertexShaderFilePath),
				geometryShaderFilePath(geometryShaderFilePath),
				fragmentShaderFilePath(fragmentShaderFilePath) {}

	GLuint getShaderId()
	{
		return shaderId;
	}

	std::string getShaderName()
	{
		return shaderName;
	}
};

class ShaderManager
{
private:
	static ShaderManager instance;

	std::map<std::string, std::shared_ptr<ShaderDetails>> namedShaders;
	std::map<std::string, int> namedShaderReferences;

	std::string loadShaderCode(std::string shaderName, std::string shaderFilePath)
	{
		std::string shaderCode;
		std::ifstream shaderStream(shaderFilePath, std::ios::in);
		if (shaderStream.is_open())
		{
			std::stringstream sstr;
			sstr << shaderStream.rdbuf();
			shaderCode = sstr.str();
			shaderStream.close();
		}
		else
		{
			std::cout << shaderName << std::endl
								<< "Failed at shader 1" << std::endl;
			exit(1);
		}

		return shaderCode;
	}

	void compileShader(std::string shaderName, std::string shaderCode, GLuint shaderId)
	{
		auto result = GL_FALSE;
		int infoLogLength;

		auto sourcePointer = shaderCode.c_str();
		glShaderSource(shaderId, 1, &sourcePointer, NULL);
		glCompileShader(shaderId);

		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			std::vector<char> shaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(shaderId, infoLogLength, NULL, &shaderErrorMessage[0]);
			std::cout << shaderName << std::endl
								<< &shaderErrorMessage[0] << std::endl
								<< "Failed at shader 2" << std::endl;
			exit(1);
		}
	}

	GLuint createProgram(std::string shaderName, std::vector<GLuint> shaderIds)
	{
		auto result = GL_FALSE;
		int infoLogLength;

		auto programId = glCreateProgram();
		for (auto shaderId = shaderIds.begin(); shaderId != shaderIds.end(); shaderId++)
		{
			glAttachShader(programId, *shaderId);
		}
		glLinkProgram(programId);

		glGetProgramiv(programId, GL_LINK_STATUS, &result);
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			std::vector<char> programErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
			std::cout << shaderName << std::endl
								<< &programErrorMessage[0] << std::endl
								<< "Failed at shader 3" << std::endl;
			exit(1);
		}

		return programId;
	}

	GLuint loadShaders(std::string shaderName, std::string vertexShaderFilePath, std::string fragmentShaderFilePath)
	{
		auto vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
		auto fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

		auto vertexShaderCode = loadShaderCode(shaderName, vertexShaderFilePath);
		auto fragmentShaderCode = loadShaderCode(shaderName, fragmentShaderFilePath);

		compileShader(shaderName, vertexShaderCode, vertexShaderId);
		compileShader(shaderName, fragmentShaderCode, fragmentShaderId);

		auto programId = createProgram(shaderName, std::vector<GLuint>({vertexShaderId, fragmentShaderId}));

		glDetachShader(programId, vertexShaderId);
		glDetachShader(programId, fragmentShaderId);

		glDeleteShader(vertexShaderId);
		glDeleteShader(fragmentShaderId);

		return programId;
	}

	GLuint loadShaders(std::string shaderName, std::string vertexShaderFilePath, std::string geometryShaderFilePath, std::string fragmentShaderFilePath)
	{
		auto vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
		auto geometryShaderId = glCreateShader(GL_GEOMETRY_SHADER);
		auto fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

		auto vertexShaderCode = loadShaderCode(shaderName, vertexShaderFilePath);
		auto geometryShaderCode = loadShaderCode(shaderName, geometryShaderFilePath);
		auto fragmentShaderCode = loadShaderCode(shaderName, fragmentShaderFilePath);

		compileShader(shaderName, vertexShaderCode, vertexShaderId);
		compileShader(shaderName, geometryShaderCode, geometryShaderId);
		compileShader(shaderName, fragmentShaderCode, fragmentShaderId);

		auto programId = createProgram(shaderName, std::vector<GLuint>({vertexShaderId, geometryShaderId, fragmentShaderId}));

		glDetachShader(programId, vertexShaderId);
		glDetachShader(programId, geometryShaderId);
		glDetachShader(programId, fragmentShaderId);

		glDeleteShader(vertexShaderId);
		glDeleteShader(geometryShaderId);
		glDeleteShader(fragmentShaderId);

		return programId;
	}

	ShaderManager()
			: namedShaders({}),
				namedShaderReferences({}) {}

public:
	ShaderManager(ShaderManager &) = delete;

	std::shared_ptr<ShaderDetails> &createShaderProgram(std::string shaderName, std::string vertexShaderFilePath, std::string fragmentShaderFilePath)
	{
		auto existingShader = namedShaders.find(shaderName);
		if (existingShader != namedShaders.end())
		{
			namedShaderReferences[shaderName]++;
			return existingShader->second;
		}

		auto shaderProgramId = loadShaders(shaderName, vertexShaderFilePath, fragmentShaderFilePath);

		auto newShader = std::make_shared<ShaderDetails>(shaderProgramId, shaderName, vertexShaderFilePath, "", fragmentShaderFilePath);

		namedShaders.insert(std::pair<std::string, std::shared_ptr<ShaderDetails>>(shaderName, newShader));
		namedShaderReferences[shaderName] = 1;

		return namedShaders[shaderName];
	}

	std::shared_ptr<ShaderDetails> &createShaderProgram(std::string shaderName, std::string vertexShaderFilePath, std::string geometryShaderFilePath, std::string fragmentShaderFilePath)
	{
		auto existingShader = namedShaders.find(shaderName);
		if (existingShader != namedShaders.end())
		{
			namedShaderReferences[shaderName]++;
			return existingShader->second;
		}

		auto shaderProgramId = loadShaders(shaderName, vertexShaderFilePath, geometryShaderFilePath, fragmentShaderFilePath);

		auto newShader = std::make_shared<ShaderDetails>(shaderProgramId, shaderName, vertexShaderFilePath, geometryShaderFilePath, fragmentShaderFilePath);

		namedShaders.insert(std::pair<std::string, std::shared_ptr<ShaderDetails>>(shaderName, newShader));
		namedShaderReferences[shaderName] = 1;

		return namedShaders[shaderName];
	}

	std::shared_ptr<ShaderDetails> &getShaderDetails(std::string shaderName)
	{
		return namedShaders[shaderName];
	}

	void destroyShaderProgram(std::shared_ptr<ShaderDetails> &shaderDetails)
	{
		namedShaderReferences[shaderDetails->getShaderName()]--;
		if (namedShaderReferences[shaderDetails->getShaderName()] <= 0)
		{
			namedShaderReferences.erase(shaderDetails->getShaderName());
			namedShaders.erase(shaderDetails->getShaderName());
			glDeleteProgram(shaderDetails->shaderId);
		}
	}

	static ShaderManager &getInstance()
	{
		return instance;
	}
};

ShaderManager ShaderManager::instance;

#endif