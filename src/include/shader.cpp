#ifndef INCLUDE_SHADER_CPP
#define INCLUDE_SHADER_CPP

#include <string>
#include <vector>
#include <map>
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
	std::string fragmentShaderFilePath;

public:
	ShaderDetails(GLuint shaderId, std::string shaderName, std::string vertexShaderFilePath, std::string fragmentShaderFilePath)
			: shaderId(shaderId),
				shaderName(shaderName),
				vertexShaderFilePath(vertexShaderFilePath),
				fragmentShaderFilePath(fragmentShaderFilePath)
	{
		std::cout << "[ShaderDetails] "
							<< "Constructing ShaderDetails." << std::endl;
		std::cout << "[ShaderDetails] "
							<< "Constructed ShaderDetails." << std::endl;
	}

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
		std::cout << "[ShaderManager] "
							<< "Reading shader code for shader program: " << shaderName << std::endl
							<< "[ShaderManager] "
							<< "\tFile: " << shaderFilePath << std::endl;

		// Reading shader code.
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
			std::cerr << "[ShaderManager] "
								<< "Cannot read shader code for shader program: " << shaderName << std::endl
								<< "[ShaderManager] "
								<< "\tFile: " << shaderFilePath << std::endl;
			exit(1);
		}

		return shaderCode;
	}

	void compileShader(std::string shaderName, std::string shaderCode, GLuint shaderId)
	{
		std::cout << "[ShaderManager] "
							<< "Compiling shader code for shader program: " << shaderName << std::endl;

		auto result = GL_FALSE;
		int infoLogLength;

		// Compiling shader source code.
		auto sourcePointer = shaderCode.c_str();
		glShaderSource(shaderId, 1, &sourcePointer, NULL);
		glCompileShader(shaderId);

		// Validating sader compilation.
		glGetShaderiv(shaderId, GL_COMPILE_STATUS, &result);
		glGetShaderiv(shaderId, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			std::vector<char> shaderErrorMessage(infoLogLength + 1);
			glGetShaderInfoLog(shaderId, infoLogLength, NULL, &shaderErrorMessage[0]);
			std::cerr << "[ShaderManager] "
								<< "Cannot read shader code for shader program: " << shaderName << std::endl
								<< "[ShaderManager] "
								<< "\tError message: " << &shaderErrorMessage[0] << std::endl;
			exit(1);
		}
	}

	GLuint createProgram(std::string shaderName, GLuint vertexShaderId, GLuint fragmentShaderId)
	{
		std::cout << "[ShaderManager] "
							<< "Linking shaders to shader program for shader program: " << shaderName << std::endl;

		auto result = GL_FALSE;
		int infoLogLength;

		// Creating shader program.
		auto programId = glCreateProgram();
		// Attaching shader components to shader program.
		glAttachShader(programId, vertexShaderId);
		glAttachShader(programId, fragmentShaderId);
		// Linking shader program together.
		glLinkProgram(programId);

		// Checking shader program.
		glGetProgramiv(programId, GL_LINK_STATUS, &result);
		glGetProgramiv(programId, GL_INFO_LOG_LENGTH, &infoLogLength);
		if (infoLogLength > 0)
		{
			std::vector<char> programErrorMessage(infoLogLength + 1);
			glGetProgramInfoLog(programId, infoLogLength, NULL, &programErrorMessage[0]);
			std::cerr << "[ShaderManager] "
								<< "Cannot link shaders" << std::endl
								<< "[ShaderManager] "
								<< "\tError message: " << &programErrorMessage[0] << std::endl;
			exit(1);
		}

		return programId;
	}

	GLuint loadShaders(std::string shaderName, std::string vertexFilePath, std::string fragmentFilePath)
	{
		std::cout << "[ShaderManager] "
							<< "Loading shader program: " << shaderName << " (Vertex: " << vertexFilePath << " | Fragment:" << fragmentFilePath << ")" << std::endl;

		std::cout << "[ShaderManager] "
							<< "Creating shader references for shader program: " << shaderName << std::endl;
		// Create the shader references
		auto vertexShaderId = glCreateShader(GL_VERTEX_SHADER);
		auto fragmentShaderId = glCreateShader(GL_FRAGMENT_SHADER);

		// Read the shader code from the files
		auto vertexShaderCode = loadShaderCode(shaderName, vertexFilePath);
		auto fragmentShaderCode = loadShaderCode(shaderName, fragmentFilePath);

		// Compile shaders
		compileShader(shaderName, vertexShaderCode, vertexShaderId);
		compileShader(shaderName, fragmentShaderCode, fragmentShaderId);

		// Link the program
		auto programId = createProgram(shaderName, vertexShaderId, fragmentShaderId);

		// Detaching shader components from shader program.
		glDetachShader(programId, vertexShaderId);
		glDetachShader(programId, fragmentShaderId);

		// Deleting shader components as they're no longer required.
		glDeleteShader(vertexShaderId);
		glDeleteShader(fragmentShaderId);

		std::cout << "[ShaderManager] "
							<< "Created shader program." << std::endl;
		return programId;
	}

	ShaderManager()
			: namedShaders({}),
				namedShaderReferences({}) {}

public:
	ShaderManager(ShaderManager &) = delete;

	std::shared_ptr<ShaderDetails> &createShaderProgram(std::string shaderName, std::string vertexShaderFilePath, std::string fragmentShaderFilePath)
	{
		std::cout << "[ShaderManager] "
							<< "Creating shader program: " << shaderName << "(" << vertexShaderFilePath << " | " << fragmentShaderFilePath << ")" << std::endl;

		auto existingShader = namedShaders.find(shaderName);
		if (existingShader != namedShaders.end())
		{
			std::cout << "[ShaderManager] "
								<< "Already created. Returning existing shader program." << std::endl;
			namedShaderReferences[shaderName]++;
			return existingShader->second;
		}

		auto shaderProgramId = loadShaders(shaderName, vertexShaderFilePath, fragmentShaderFilePath);

		auto newShader = std::make_shared<ShaderDetails>(shaderProgramId, shaderName, vertexShaderFilePath, fragmentShaderFilePath);

		namedShaders.insert(std::pair<std::string, std::shared_ptr<ShaderDetails>>(shaderName, newShader));
		namedShaderReferences[shaderName] = 1;

		std::cout << "[ShaderManager] "
							<< "Created shader program." << std::endl;
		return namedShaders[shaderName];
	}

	std::shared_ptr<ShaderDetails> &getShaderDetails(std::string shaderName)
	{
		return namedShaders[shaderName];
	}

	void destroyShaderProgram(std::shared_ptr<ShaderDetails> &shaderDetails)
	{
		std::cout << "[ShaderManager] "
							<< "Dereferencing shader program: " << shaderDetails->getShaderName() << std::endl;

		namedShaderReferences[shaderDetails->getShaderName()]--;
		if (namedShaderReferences[shaderDetails->getShaderName()] <= 0)
		{
			std::cout << "[ShaderManager] "
								<< "Destroying shader program: " << shaderDetails->getShaderName() << std::endl;

			namedShaderReferences.erase(shaderDetails->getShaderName());
			namedShaders.erase(shaderDetails->getShaderName());
			glDeleteProgram(shaderDetails->shaderId);

			std::cout << "[ShaderManager] "
								<< "Destroyed shader program." << std::endl;
		}
	}

	static ShaderManager &getInstance()
	{
		return instance;
	}
};

ShaderManager ShaderManager::instance;

#endif