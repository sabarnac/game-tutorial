#ifndef INCLUDE_TEXT_CPP
#define INCLUDE_TEXT_CPP

#include <iostream>
#include <map>
#include <vector>
#include <memory>

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <ft2build.h>
#include FT_FREETYPE_H

/**
 * Enum of supported text alignments relative to text position.
 */
enum TextAlignment
{
  // Left means that the starting position of the text is at the provided text position.
  LEFT,
  // center means that the middle position of the text is at the provided text position.
  CENTER,
  // Right means that the ending position of the text is at the provided text position.
  RIGHT
};

/**
 * Class containing information about a text character.
 */
class TextCharacter
{
  // Let the text manager access private variables.
  friend class TextManager;
  friend class TextCharacterSet;

private:
  char character;
  unsigned int characterSetLayerId;
  glm::vec2 size;
  glm::vec2 bearing;
  unsigned int advance;
  glm::vec2 maxUv;

public:
  TextCharacter(char character,
                glm::vec2 size,
                glm::vec2 bearing,
                unsigned int advance,
                glm::vec2 maxUv,
                unsigned int characterSetLayerId)
      : character(character),
        size(size),
        bearing(bearing),
        advance(advance),
        maxUv(maxUv),
        characterSetLayerId(characterSetLayerId) {}

  double getCharacter()
  {
    return character;
  }
};

class TextCharacterSet
{
  // Let the text manager access private variables.
  friend class TextManager;

private:
  std::string fontId;
  std::string fontFilePath;

  GLuint characterTextureArrayId;

  std::map<char, TextCharacter> characterMap;

  void loadFont(std::string fontId, std::string fontFilePath)
  {
    FT_Library freeType;
    if (FT_Init_FreeType(&freeType))
    {
      std::cout << fontId << std::endl
                << "Failed at text character set 1" << std::endl;
      exit(1);
    }
    FT_Face fontFace;
    if (FT_New_Face(freeType, fontFilePath.c_str(), 0, &fontFace))
    {
      std::cout << fontId << std::endl
                << "Failed at text character set 2" << std::endl;
      exit(1);
    }

    FT_Set_Pixel_Sizes(fontFace, 0, 100);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    glGenTextures(1, &characterTextureArrayId);
    glBindTexture(GL_TEXTURE_2D_ARRAY, characterTextureArrayId);

    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D_ARRAY, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    unsigned int maxWidth = 0;
    unsigned int maxRows = 0;
    for (char character = 0; character < std::numeric_limits<char>::max(); character++)
    {
      if (FT_Load_Char(fontFace, character, FT_LOAD_RENDER))
      {
        std::cout << fontId << std::endl
                  << "Failed at text character set 3" << std::endl;
        continue;
      }

      maxWidth = glm::max(maxWidth, fontFace->glyph->bitmap.width);
      maxRows = glm::max(maxRows, fontFace->glyph->bitmap.rows);
    }

    std::vector<uint8_t> clearData(maxWidth * maxRows, 0);
    glTexImage3D(GL_TEXTURE_2D_ARRAY, 0, GL_RED, maxWidth, maxRows, std::numeric_limits<char>::max(), 0, GL_RED, GL_UNSIGNED_BYTE, nullptr);

    for (char character = 0; character < std::numeric_limits<char>::max(); character++)
    {
      if (FT_Load_Char(fontFace, character, FT_LOAD_RENDER))
      {
        std::cout << fontId << std::endl
                  << "Failed at text character set 4" << std::endl;
        continue;
      }

      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, (unsigned int)character, maxWidth, maxRows, 1, GL_RED, GL_UNSIGNED_BYTE, &clearData[0]);
      glTexSubImage3D(GL_TEXTURE_2D_ARRAY, 0, 0, 0, (unsigned int)character, fontFace->glyph->bitmap.width, fontFace->glyph->bitmap.rows, 1, GL_RED, GL_UNSIGNED_BYTE, fontFace->glyph->bitmap.buffer);

      TextCharacter textCharacter(
          character,
          glm::vec2(fontFace->glyph->bitmap.width, fontFace->glyph->bitmap.rows),
          glm::vec2(fontFace->glyph->bitmap_left, fontFace->glyph->bitmap_top),
          fontFace->glyph->advance.x,
          glm::vec2(fontFace->glyph->bitmap.width / maxWidth, fontFace->glyph->bitmap.rows / maxRows),
          (unsigned int)character);
      characterMap.insert(std::pair<char, TextCharacter>(character, textCharacter));
    }

    glBindTexture(GL_TEXTURE_2D_ARRAY, 0);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

    FT_Done_Face(fontFace);
    FT_Done_FreeType(freeType);
  }

  TextCharacterSet(std::string fontId, std::string fontFilePath)
      : fontId(fontId),
        fontFilePath(fontFilePath)
  {
    loadFont(fontId, fontFilePath);
  }

public:
  std::string getFontId()
  {
    return fontId;
  }

  TextCharacter getCharacter(char character)
  {
    return characterMap[character];
  }
};

/**
 * Class for containing the details of the text to render.
 */
class TextDetails
{
  // Let the text manager access private variables.
  friend class TextManager;

private:
  // The text content to render.
  std::string content;
  // The position of the text, with the origin being the top-left of the screen.
  glm::vec2 position;
  // The alignment of the text relative to the given text position.
  TextAlignment alignment;

public:
  TextDetails(
      std::string content,
      glm::vec2 position,
      TextAlignment alignment)
      : content(content),
        position(position),
        alignment(alignment) {}

  /**
   * Get the content of the text.
   * 
   * @return The text content.
   */
  std::string getContent()
  {
    return content;
  }

  /**
   * Get the alignment of the text relative to the given text coordinates.
   * 
   * @return The text alignment.
   */
  TextAlignment getAlignment()
  {
    return alignment;
  }

  /**
   * Get the position of the text, with the origin being the top-left of the screen.
   * 
   * @return The text position.
   */
  glm::vec2 &getPosition()
  {
    return position;
  }
};

/**
 * A manager class for managing and trendering text.
 */
class TextManager
{
private:
  static TextManager instance;
  static TextCharacterSet characterSet;

  std::vector<std::shared_ptr<TextDetails>> textToRenderMap;

  void clearTextToRenderMap()
  {
    textToRenderMap.clear();
  }

  TextManager() {}

public:
  /**
   * Returns the singleton instance of the text manager.
   * 
   * @return The text manager singleton instance.
   */
  static TextManager &getInstance()
  {
    return instance;
  }
};

// Initialize the text character set static variable.
TextCharacterSet TextManager::characterSet = TextCharacterSet("Roboto", "assets/fonts/Roboto-Regular.ttf");
// Initialize the text manager singleton instance static variable.
TextManager TextManager::instance;

#endif