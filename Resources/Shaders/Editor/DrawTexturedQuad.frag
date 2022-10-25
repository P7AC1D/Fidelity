#version 410

layout(std140) uniform TexturedQuadBuffer
{
  bool PerspectiveDepth;
  bool OrthographicDepth;
  bool SingleChannel;
  bool TextureArray;
  float NearClip;
  float FarClip;
  uint ArraySlice;
} TexturedQuad;

uniform sampler2D Texture;
uniform sampler2DArray TextureArray;

layout(location = 0) in vec2 TexCoord;
layout(location = 0) out vec4 FinalColour;

float LinearizeDepth(float depthSample)
{
  float zNear = TexturedQuad.NearClip;
  float zFar  = TexturedQuad.FarClip;
  return (2.0 * zNear) / (zFar + zNear - depthSample * (zFar - zNear));
}

void main()
{
  if (TexturedQuad.PerspectiveDepth)
  {
    float depth = 0.0f;
    if (TexturedQuad.TextureArray)
    {
      depth = texture(TextureArray, vec3(TexCoord, TexturedQuad.ArraySlice)).r;
    }
    else
    {
      depth = texture(Texture, TexCoord).r;      
    }
    
    depth = LinearizeDepth(depth);
    FinalColour = vec4(depth, depth, depth, 1.0f);
  }
  else if (TexturedQuad.OrthographicDepth)
  {
    float depth = 0.0f;
    if (TexturedQuad.TextureArray)
    {
      depth = texture(TextureArray, vec3(TexCoord, TexturedQuad.ArraySlice)).r;
    }
    else
    {
      depth = texture(Texture, TexCoord).r;
    }

    FinalColour = vec4(depth, depth, depth, 1.0f);
  }
  else if (TexturedQuad.SingleChannel)
  {
    vec4 texSample = texture(Texture, TexCoord);
    FinalColour = vec4(texSample.r, 
                       texSample.r,
                       texSample.r,
                       1.0f);
  }
  else
  {
    vec4 texSample = texture(Texture, TexCoord);
    FinalColour = vec4(texSample.r, 
                       texSample.g,
                       texSample.b,
                       1.0f);
  }
}