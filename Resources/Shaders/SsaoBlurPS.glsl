#version 410

layout(location = 0) in vec2 TexCoord;

out float FragColour;

uniform sampler2D SsaoMap;

void main()
{
  vec2 texelSize = 1.0f / vec2(textureSize(SsaoMap, 0));
  float result = 0.0f;
  for (int x = -2; x < 2; ++x) 
  {
    for (int y = -2; y < 2; ++y)
    {
      vec2 offset = vec2(float(x), float(y)) * texelSize;
      result += texture(SsaoMap, TexCoord + offset).r;
    }
  }
  FragColour = result / (4.0f * 4.0f);
}
