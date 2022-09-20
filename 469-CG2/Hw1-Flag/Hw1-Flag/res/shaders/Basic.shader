#shader vertex
#version 330 core

layout (location = 0) in vec4 position;
layout (location = 1) in vec4 normal;
layout (location = 2) in vec2 in_texCoord;

uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform vec3 LightPosition;
uniform vec3 LightPosition2;
uniform vec3 CameraPosition;

out vec2 texCoord;
out vec3 LightVector;
out vec3 LightVector2;
out vec3 CameraVector;

out Data
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;

void main()
{
    mat4 NormalMatrix = inverse(transpose(ViewMatrix));

    data.Position = vec3(ViewMatrix * position);
    data.Normal = vec3(normalize(vec3(NormalMatrix * normal)));
    data.TexCoord = vec2(in_texCoord.x, in_texCoord.y);

    CameraVector = normalize(vec3(ViewMatrix * vec4(CameraPosition, 1)) - data.Position);
    LightVector = normalize(vec3(ViewMatrix * vec4(LightPosition, 1)) - data.Position);
    LightVector2 = normalize(vec3(ViewMatrix * vec4(LightPosition, 1)) - data.Position);

    gl_Position = ProjectionMatrix * ViewMatrix * position;
    texCoord = in_texCoord;
};

#shader fragment
#version 330 core

layout (location = 0) out vec4 color;

in Data
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;

in vec3 CameraVector;
in vec3 LightVector;
in vec3 LightVector2;

uniform sampler2D myTexture;

vec3 ambientReflectenceCoefficient = vec3(0.1f);
vec3 ambientLightColor = vec3(0.1f);
vec3 specularReflectenceCoefficient = vec3(0.5f);
vec3 specularLightColor = vec3(1.0f);
float specularExponent = 150;
vec3 diffuseReflectenceCoefficient = vec3(1.0f);
vec3 diffuseLightColor = vec3(1.0f);

void main()
{
    vec3 reflect = reflect(-LightVector, data.Normal);
    vec3 reflect2 = reflect(-LightVector2, data.Normal);
    vec4 texColor = texture(myTexture, data.TexCoord);

    diffuseReflectenceCoefficient = vec3(texColor.x, texColor.y, texColor.z);
    vec3 ambient = ambientLightColor * ambientReflectenceCoefficient;
    float costheta = max(dot(data.Normal, LightVector), 0);
    vec3 diffuse = diffuseLightColor * costheta * diffuseReflectenceCoefficient;
    float costheta2 = max(dot(data.Normal, LightVector2), 0);
    vec3 diffuse2 = diffuseLightColor * costheta2 * diffuseReflectenceCoefficient;
    float phong = pow(max(dot(CameraVector, reflect), 0), specularExponent);
    vec3 specular = specularLightColor * phong * specularReflectenceCoefficient;
    float phong2 = pow(max(dot(CameraVector, reflect2), 0), specularExponent);
    vec3 specular2 = specularLightColor * phong2 * specularReflectenceCoefficient;
    
    vec4 ucolor = vec4(ambient + diffuse + diffuse2 + specular + specular2, 1.0) /* texColor*/;

    //color = vec4(1.0f, 1.0f, 1.0f, 1.0f);
    color = vec4(clamp(ucolor.xyz, 0.0, 1.0), 1);
};