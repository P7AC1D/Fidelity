#pragma once

enum class PixelFormat
{
  /// The format has not been initialized.
  Unknown,
  /// 8-bit red channel as unsigned bytes.
  R8,
  /// 8-bit red and green channels stored as unsigned bytes.
  RG8,
  /// 8-bit red, green and blue channels stored as unsigned bytes.
  RGB8,
  /// 8-bit red, green, blue and alpha channels stored as unsigned bytes.
  RGBA8,
  /// 16-bit red, green and blue channels stored as a signed floats.
  RGB16F,
  /// 16-bit red, green, blue and alpha channels stored as a signed floats.
  RGBA16F,
  /// 32-bit depth channel stored as unsigned bytes.
  D32,
  /// 24-bit depth channel with an 8 bit stencil channel stored as unsigned bytes.
  D24S8
};