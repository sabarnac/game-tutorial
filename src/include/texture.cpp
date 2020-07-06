#ifndef INCLUDE_TEXTURE_CPP
#define INCLUDE_TEXTURE_CPP

#include <string>
#include <map>
#include <iostream>
#include <fstream>
#include <memory>

#include <GL/glew.h>

enum TextureType
{
	BMP
};

class TextureDetails
{
	friend class TextureManager;

private:
	GLuint textureId;
	std::string textureName;
	std::string textureFilePath;
	TextureType textureType;

public:
	TextureDetails(TextureDetails &) = delete;

	TextureDetails(GLuint textureId, std::string textureName, std::string textureFilePath, TextureType textureType)
			: textureId(textureId),
				textureName(textureName),
				textureFilePath(textureFilePath),
				textureType(textureType) {}

	GLuint getTextureId()
	{
		return textureId;
	}

	std::string getTextureName()
	{
		return textureName;
	}

	TextureType getTextureType()
	{
		return textureType;
	}
};

class TextureManager
{
private:
	static TextureManager instance;

	std::map<std::string, std::shared_ptr<TextureDetails>> namedTextures;
	std::map<std::string, int> namedTextureReferences;

	GLuint createTexture(std::string textureName, unsigned char *textureData, unsigned int width, unsigned int height)
	{
		GLuint textureId;
		glGenTextures(1, &textureId);
		glBindTexture(GL_TEXTURE_2D, textureId);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, textureData);

		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		glGenerateMipmap(GL_TEXTURE_2D);

		glBindTexture(GL_TEXTURE_2D, 0);
		return textureId;
	}

	GLuint loadBmpTexture(std::string textureName, std::string textureFilePath)
	{
		unsigned char header[54];
		unsigned int dataPos;
		unsigned int imageSize;
		unsigned int width, height;

		auto file = fopen(textureFilePath.c_str(), "rb");
		if (!file)
		{
			std::cout << "Failed at texture 1" << std::endl;
			exit(1);
		}

		auto readBytes = fread(header, 1, 54, file);
		if (readBytes != 54)
		{
			fclose(file);
			std::cout << "Failed at texture 2" << std::endl;
			exit(1);
		}
		if (header[0] != 'B' || header[1] != 'M')
		{
			fclose(file);
			std::cout << "Failed at texture 3" << std::endl;
			exit(1);
		}
		if (*(int *)&(header[0x1E]) != 0)
		{
			fclose(file);
			std::cout << "Failed at texture 4" << std::endl;
			exit(1);
		}
		if (*(int *)&(header[0x1C]) != 24)
		{
			fclose(file);
			std::cout << "Failed at texture 5" << std::endl;
			exit(1);
		}

		dataPos = *(int *)&(header[0x0A]);
		imageSize = *(int *)&(header[0x22]);
		width = *(int *)&(header[0x12]);
		height = *(int *)&(header[0x16]);

		// Some BMP files are misformatted, guess missing information.
		if (imageSize == 0)
		{
			// Each pixel is multiplied by 3 because each pixel has 3 bytes of information, one for R, G, and B.
			imageSize = width * height * 3;
		}
		if (dataPos == 0)
		{
			// Data should start right after the BMP header, which is located at the start of the file and is 54 bytes in size, which is why we skip all those bytes.
			dataPos = 54;
		}

		// Create a buffer. This will contain the actual RGB data of the texture.
		auto textureData = new unsigned char[imageSize];

		// Read the actual data from the file into the buffer.
		fread(textureData, 1, imageSize, file);

		// Everything is in memory now, the file can be closed.
		fclose(file);

		// Create an OpenGL texture.
		auto textureId = createTexture(textureName, textureData, width, height);
		delete[] textureData;

		return textureId;
	}

	TextureManager()
			: namedTextures({}),
				namedTextureReferences({}) {}

public:
	TextureManager(TextureManager &) = delete;

	std::shared_ptr<TextureDetails> &createTexture(std::string textureName, std::string textureFilePath, TextureType textureType)
	{
		auto existingTexture = namedTextures.find(textureName);
		if (existingTexture != namedTextures.end())
		{
			namedTextureReferences[textureName]++;
			return existingTexture->second;
		}

		GLuint textureId;

		switch (textureType)
		{
		case BMP:
			textureId = loadBmpTexture(textureName, textureFilePath);
		}

		auto newTexture = std::make_shared<TextureDetails>(textureId, textureName, textureFilePath, textureType);

		namedTextures.insert(std::pair<std::string, std::shared_ptr<TextureDetails>>(textureName, newTexture));
		namedTextureReferences[textureName] = 1;

		return namedTextures[textureName];
	}

	std::shared_ptr<TextureDetails> &getTextureDetails(std::string textureName)
	{
		return namedTextures[textureName];
	}

	void destroyTexture(std::shared_ptr<TextureDetails> &textureDetails)
	{
		namedTextureReferences[textureDetails->getTextureName()]--;
		if (namedTextureReferences[textureDetails->getTextureName()] <= 0)
		{
			namedTextureReferences.erase(textureDetails->getTextureName());
			namedTextures.erase(textureDetails->getTextureName());
			glDeleteTextures(1, &textureDetails->textureId);
		}
	}

	static TextureManager &getInstance()
	{
		return instance;
	}
};

TextureManager TextureManager::instance;

#endif