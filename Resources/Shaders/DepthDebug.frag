#version 410

uniform sampler2D QuadTexture;

layout(location = 0) in vec2 TexCoord;

out vec4 FinalColour;

float near = 0.1; 
float far  = 100.0; 
  
float LinearizeDepth(float depth) 
{
    float z = depth * 2.0 - 1.0; // back to NDC 
    return (2.0 * near * far) / (far + near - z * (far - near));	
}

void main()
{             
    float depth = LinearizeDepth(gl_FragCoord.z) / far; // divide by far for demonstration
    FinalColour = vec4(vec3(depth), 1.0);
}
