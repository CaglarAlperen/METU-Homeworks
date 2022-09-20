#shader vertex
#version 330 core

layout(location = 0) in vec4 position;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

out vec3 texCoord;

void main()
{
    texCoord = position.xyz;
    vec4 pos = ProjectionMatrix * ViewMatrix * position;
    gl_Position = pos.xyww;
};

#shader fragment
#version 330 core

out vec4 FragColor;

in vec3 texCoord;

uniform samplerCube skybox;

void main()
{
    FragColor = texture(skybox, texCoord);
};