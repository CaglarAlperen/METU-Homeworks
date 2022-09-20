#shader vertex
#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

uniform mat4 ModelMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;

uniform vec3 CameraPosition;
uniform vec3 LightPosition;
uniform float orbitDegree;

out vec3 Position;
out vec3 Normal;
out vec3 LightVector;
out vec3 CameraVector;

void main()
{
    mat4 R = mat4(cos(orbitDegree), 0.0, sin(orbitDegree), 0.0,
                  0.0, 1.0, 0.0, 0.0,
                  -sin(orbitDegree), 0.0, cos(orbitDegree),  0.0,
                  0.0, 0.0, 0.0, 1.0);

    vec3 viewPos = vec3(ViewMatrix * vec4(inPosition, 1.0f));

    CameraVector = normalize(vec3(ViewMatrix * vec4(CameraPosition, 1)) - viewPos);
    LightVector = normalize(vec3(ViewMatrix * vec4(LightPosition, 1)) - viewPos);

    Normal = mat3(transpose(inverse(R * ModelMatrix))) * inNormal;
    Position = vec3(ModelMatrix * vec4(inPosition, 1.0));
    gl_Position = ProjectionMatrix * ViewMatrix * R * ModelMatrix * vec4(inPosition, 1.0f);
};

#shader fragment
#version 330 core

layout (location = 0) out vec4 color;

in vec3 Normal;
in vec3 Position;
in vec3 CameraVector;
in vec3 LightVector;

vec3 ambientReflectenceCoefficient = vec3(0.1f);
vec3 ambientLightColor = vec3(0.1f);
vec3 specularReflectenceCoefficient = vec3(0.5f);
vec3 specularLightColor = vec3(1.0f);
float specularExponent = 150;
vec3 diffuseReflectenceCoefficient = vec3(1.0f);
vec3 diffuseLightColor = vec3(1.0f);

void main()
{
    vec3 rflct = reflect(-LightVector, Normal);
    vec4 texColor = vec4(0.0f, 1.0f, 0.0f, 1.0f);

    diffuseReflectenceCoefficient = vec3(texColor.x, texColor.y, texColor.z);
    vec3 ambient = ambientLightColor * ambientReflectenceCoefficient;
    float costheta = max(dot(Normal, LightVector), 0);
    vec3 diffuse = diffuseLightColor * costheta * diffuseReflectenceCoefficient;
    float phong = pow(max(dot(CameraVector, rflct), 0), specularExponent);
    vec3 specular = specularLightColor * phong * specularReflectenceCoefficient;
    
    color = vec4(ambient + diffuse + specular, 1.0) * texColor;
};