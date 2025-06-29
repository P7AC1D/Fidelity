#version 410

layout(std140) uniform BloomBuffer
{
  vec2 SourceResolution;
} Constants;

uniform sampler2D Texture;

layout(location = 0) in vec2 TexCoord;
layout(location = 0) out vec3 Output;

void main()
{
    vec2 srcTexelSize = 1.0 / Constants.SourceResolution;
    float x = srcTexelSize.x;
    float y = srcTexelSize.y;

    // Take 13 samples around current texel:
    // a - b - c
    // - j - k -
    // d - e - f
    // - l - m -
    // g - h - i
    // === ('e' is the current texel) ===
    vec3 a = texture(Texture, vec2(TexCoord.x - 2*x, TexCoord.y + 2*y)).rgb;
    vec3 b = texture(Texture, vec2(TexCoord.x,       TexCoord.y + 2*y)).rgb;
    vec3 c = texture(Texture, vec2(TexCoord.x + 2*x, TexCoord.y + 2*y)).rgb;

    vec3 d = texture(Texture, vec2(TexCoord.x - 2*x, TexCoord.y)).rgb;
    vec3 e = texture(Texture, vec2(TexCoord.x,       TexCoord.y)).rgb;
    vec3 f = texture(Texture, vec2(TexCoord.x + 2*x, TexCoord.y)).rgb;

    vec3 g = texture(Texture, vec2(TexCoord.x - 2*x, TexCoord.y - 2*y)).rgb;
    vec3 h = texture(Texture, vec2(TexCoord.x,       TexCoord.y - 2*y)).rgb;
    vec3 i = texture(Texture, vec2(TexCoord.x + 2*x, TexCoord.y - 2*y)).rgb;

    vec3 j = texture(Texture, vec2(TexCoord.x - x, TexCoord.y + y)).rgb;
    vec3 k = texture(Texture, vec2(TexCoord.x + x, TexCoord.y + y)).rgb;
    vec3 l = texture(Texture, vec2(TexCoord.x - x, TexCoord.y - y)).rgb;
    vec3 m = texture(Texture, vec2(TexCoord.x + x, TexCoord.y - y)).rgb;

    // Apply weighted distribution:
    // 0.5 + 0.125 + 0.125 + 0.125 + 0.125 = 1
    // a,b,d,e * 0.125
    // b,c,e,f * 0.125
    // d,e,g,h * 0.125
    // e,f,h,i * 0.125
    // j,k,l,m * 0.5
    // This shows 5 square areas that are being sampled. But some of them overlap,
    // so to have an energy preserving Output we need to make some adjustments.
    // The weights are the distributed, so that the sum of j,k,l,m (e.g.)
    // contribute 0.5 to the final color output. The code below is written
    // to effectively yield this sum. We get:
    // 0.125*5 + 0.03125*4 + 0.0625*4 = 1
    Output = e*0.125;
    Output += (a+c+g+i)*0.03125;
    Output += (b+d+f+h)*0.0625;
    Output += (j+k+l+m)*0.125;
}