#pragma once
#include <string>
#include <vector>

#include "../Core/Types.hpp"

struct Character
{
  /** ASCII character number.*/
  uint32 Id;
  /** X-position of the character image.*/
  int32 XPos;
  /** Y-position of the character image.*/
  int32 YPos;
  /** Width of the character image.*/
  int32 Width;
  /** Height of the character image.*/
  int32 Height;
  /** The amount to offset the cursor position along the x-axis when drawing the character image.*/
  int32 XOffset;
  /** The amount to offset the cursor position along the y-axis when drawing the character image.*/
  int32 YOffset;
  /** The amount to advance the cursor position after each character image has been drawn*/
  int32 XAdvance;
};

struct Font
{
  /** The name of the font.*/
  std::string Name;
  /** The name of the texture atlas image file that this font uses.*/
  std::string TextureFileName;
  /** The amount the cursor should move when going to a new line.*/
  uint32 LineHeight;
  /** The amount of offset from the top of line to the base of each character.*/
  uint32 Base;
  /** The width of the font texture atlas.*/
  uint32 TextureWidth;
  /** The height of the font texture atlas.*/
  uint32 TextureHeight;
  /** The total number of characters contained in this font.*/
  uint32 Count;
  /** The collection of characters contained in this font.*/
  std::vector<Character> Characters;
};