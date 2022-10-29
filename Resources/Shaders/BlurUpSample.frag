#version 410

// This shader performs upsampling on a texture,
// as taken from Call Of Duty method, presented at ACM Siggraph 2014.

layout(std140) uniform BloomBuffer
{
  vec2 SourceResolution;
  float FilterRadius;
} Constants;

uniform sampler2D Texture;

layout(location = 0) in vec2 TexCoord;
layout(location = 0) out vec3 Output;

void main()
{
    // The filter kernel is applied with a radius, specified in texture
    // coordinates, so that the radius will vary across mip resolutions.
    float x = Constants.FilterRadius;
    float y = Constants.FilterRadius;

    // Take 9 samples around current texel:
    // a - b - c
    // d - e - f
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(Texture, vec2(TexCoord.x - x, TexCoord.y + y)).rgb;
    vec3 b = texture(Texture, vec2(TexCoord.x,     TexCoord.y + y)).rgb;
    vec3 c = texture(Texture, vec2(TexCoord.x + x, TexCoord.y + y)).rgb;

    vec3 d = texture(Texture, vec2(TexCoord.x - x, TexCoord.y)).rgb;
    vec3 e = texture(Texture, vec2(TexCoord.x,     TexCoord.y)).rgb;
    vec3 f = texture(Texture, vec2(TexCoord.x + x, TexCoord.y)).rgb;

    vec3 g = texture(Texture, vec2(TexCoord.x - x, TexCoord.y - y)).rgb;
    vec3 h = texture(Texture, vec2(TexCoord.x,     TexCoord.y - y)).rgb;
    vec3 i = texture(Texture, vec2(TexCoord.x + x, TexCoord.y - y)).rgb;

    // Apply weighted distribution, by using a 3x3 tent filter:
    //  1   | 1 2 1 |
    // -- * | 2 4 2 |
    // 16   | 1 2 1 |
    Output = e*4.0;
    Output += (b+d+f+h)*2.0;
    Output += (a+c+g+i);
    Output *= 1.0 / 16.0;
}