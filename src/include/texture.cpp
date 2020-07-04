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
				textureType(textureType)
	{
		std::cout << "[TextureDetails] "
							<< "Constructing TextureDetails." << std::endl;
		std::cout << "[TextureDetails] "
							<< "Constructed TextureDetails." << std::endl;
	}

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
		std::cout << "[TextureManager] "
							<< "Creating texture reference for texture: " << textureName << std::endl;

		// Create an OpenGL texture.
		GLuint textureId;
		glGenTextures(1, &textureId);

		// Bind the texture to the current OpenGL context so that we can start modifying it and writing texture data to it.
		glBindTexture(GL_TEXTURE_2D, textureId);

		// Provide OpenGL with the texture data to copy to the current texture context.
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, textureData);

		// Define some parameters regarding the texture (behaviour when reading outside the texture, magnification and minification behaviour)
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
		// The last option tells that for minified versions to use mip-map textures, so need to generate those as well.
		glGenerateMipmap(GL_TEXTURE_2D);

		return textureId;
	}

	GLuint loadBmpTexture(std::string textureName, std::string textureFilePath)
	{
		std::cout << "[TextureManager] "
							<< "Loading texture: " << textureName << " (" << textureFilePath << ")" << std::endl;

		// Information that we need to read from the BMP file.
		unsigned char header[54];
		unsigned int dataPos;
		unsigned int imageSize;
		unsigned int width, height;

		std::cout << "[TextureManager] "
							<< "Reading texture file for texture: " << textureName << std::endl
							<< "[TextureManager] "
							<< "\tFile: " << textureFilePath << std::endl;

		// Open the texture BMP file.
		auto file = fopen(textureFilePath.c_str(), "rb");
		if (!file)
		{
			std::cout << "[TextureManager] "
								<< "Could not open BMP file for texture: " << textureName << std::endl
								<< "[TextureManager] "
								<< "\tFile: " << textureFilePath << std::endl;
			exit(1);
		}

		// Read the header, i.e. the 54 first bytes. If less than 54 bytes are read, throw an error.
		auto readBytes = fread(header, 1, 54, file);
		if (readBytes != 54)
		{
			std::cout << "[TextureManager] "
								<< "Invalid BMP file: " << textureFilePath << std::endl;
			fclose(file);
			exit(1);
		}
		// A BMP files always begins with "BM". If it doesn't, throw an error
		if (header[0] != 'B' || header[1] != 'M')
		{
			std::cout << "[TextureManager] "
								<< "Invalid BMP file: " << textureFilePath << std::endl;
			fclose(file);
			exit(1);
		}
		// Make sure this is a 24bpp file. If it isn't, throw an error
		if (*(int *)&(header[0x1E]) != 0)
		{
			std::cout << "[TextureManager] "
								<< "Invalid BMP file: " << textureFilePath << std::endl;
			fclose(file);
			exit(1);
		}
		if (*(int *)&(header[0x1C]) != 24)
		{
			std::cout << "[TextureManager] "
								<< "Invalid BMP file: " << textureFilePath << std::endl;
			fclose(file);
			exit(1);
		}

		// Read the information about the image.
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

		std::cout << "[TextureManager] "
							<< "Created texture." << std::endl;
		return textureId;
	}

	TextureManager()
			: namedTextures({}),
				namedTextureReferences({}) {}

public:
	TextureManager(TextureManager &) = delete;

	std::shared_ptr<TextureDetails> &createTexture(std::string textureName, std::string textureFilePath, TextureType textureType)
	{
		std::cout << "[TextureManager] "
							<< "Creating texture: " << textureName << "(" << textureFilePath << ")" << std::endl;

		auto existingTexture = namedTextures.find(textureName);
		if (existingTexture != namedTextures.end())
		{
			std::cout << "[TextureManager] "
								<< "Already created. Returning existing texture." << std::endl;
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

		std::cout << "[TextureManager] "
							<< "Created texture." << std::endl;

		return namedTextures[textureName];
	}

	std::shared_ptr<TextureDetails> &getTextureDetails(std::string textureName)
	{
		return namedTextures[textureName];
	}

	void destroyTexture(std::shared_ptr<TextureDetails> &textureDetails)
	{
		std::cout << "[TextureManager] "
							<< "Dereferencing texture: " << textureDetails->getTextureName() << std::endl;

		namedTextureReferences[textureDetails->getTextureName()]--;
		if (namedTextureReferences[textureDetails->getTextureName()] <= 0)
		{
			std::cout << "[TextureManager] "
								<< "Destroying texture: " << textureDetails->getTextureName() << std::endl;

			namedTextureReferences.erase(textureDetails->getTextureName());
			namedTextures.erase(textureDetails->getTextureName());
			glDeleteTextures(1, &textureDetails->textureId);

			std::cout << "[TextureManager] "
								<< "Destroyed texture." << std::endl;
		}
	}

	static TextureManager &getInstance()
	{
		return instance;
	}
};

TextureManager TextureManager::instance;

#endif