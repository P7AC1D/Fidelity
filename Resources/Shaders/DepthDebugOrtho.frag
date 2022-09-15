#version 410

uniform sampler2D QuadTexture;

layout(location = 0) in vec2 TexCoord;

layout(location = 0) out vec4 FinalColour;

void main()
{             
	float depth =  texture(QuadTexture, TexCoord).x;
	FinalColour = vec4(depth, depth, depth, 1.0f);
}
