#version 430

layout (location = 0) in vec3 VertexPosition;
layout (location = 1) in vec3 VertexNormal;
layout (location = 2) in vec2 VertexTex;

uniform vec3 lightPosition;
uniform vec3 cameraPosition;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform mat4 NormalMatrix;
uniform mat4 ModelMatrix;
uniform mat4 MVP;

uniform sampler2D MoonTexColor;
uniform float textureOffset;
uniform float orbitDegree;

uniform float heightFactor;
uniform float imageWidth;
uniform float imageHeight;

out Data
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;


out vec3 LightVector;// Vector from Vertex to Light;
out vec3 CameraVector;// Vector from Vertex to Camera;

void main()
{
    vec4 texColor = texture(MoonTexColor, vec2(mod(textureOffset+ VertexTex.x, 1.0), VertexTex.y));

    mat4 R = mat4(cos(orbitDegree), -sin(orbitDegree), 0.0, 0.0,
                  sin(orbitDegree), cos(orbitDegree), 0.0, 0.0,
                  0.0, 0.0, 1.0, 0.0,
                  0.0, 0.0, 0.0, 1.0);

    data.Normal = vec3(normalize(vec3(transpose(inverse(R)) * vec4(VertexNormal, 0))));
    data.Position = vec3(vec4(VertexPosition, 1));
    data.TexCoord = vec2(VertexTex.x, VertexTex.y);


    CameraVector = -1 * normalize(vec3(ViewMatrix*vec4(cameraPosition,1)) - VertexPosition);
    LightVector = -1 * normalize(vec3(ViewMatrix*vec4(lightPosition,1))  - VertexPosition);

    gl_Position = MVP * R* vec4(VertexPosition, 1);
}



/*
mat4 R = mat4(cos(orbitDegree), -1.0 * sin(orbitDegree), 0.0, 0.0,
                  sin(orbitDegree), cos(orbitDegree), 0.0, 0.0,
                  0.0, 0.0, 1.0, 0.0,
                  0.0, 0.0, 0.0, 1.0);
                  */