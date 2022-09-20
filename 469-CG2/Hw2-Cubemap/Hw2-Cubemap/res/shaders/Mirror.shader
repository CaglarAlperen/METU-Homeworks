#shader vertex
#version 330 core

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;

uniform mat4 ModelMatrix;
uniform mat4 ProjectionMatrix;
uniform mat4 ViewMatrix;
uniform float Angle;
//uniform vec3 CameraPosition;

out vec3 Position;
out vec3 Normal;

/*out Data
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;*/

void main()
{
    mat4 R = mat4(cos(Angle), 0.0, sin(Angle), 0.0,
        0.0, 1.0, 0.0, 0.0,
        -sin(Angle), 0.0, cos(Angle), 0.0,
        0.0, 0.0, 0.0, 1.0);
    /*mat4 NormalMatrix = inverse(transpose(ViewMatrix));

    data.Position = vec3(ViewMatrix * position);
    data.Normal = vec3(normalize(vec3(NormalMatrix * normal)));
    data.TexCoord = vec2(in_texCoord.x, in_texCoord.y);

    CameraVector = normalize(vec3(ViewMatrix * vec4(CameraPosition, 1)) - data.Position);
    LightVector = normalize(vec3(ViewMatrix * vec4(LightPosition, 1)) - data.Position);
    LightVector2 = normalize(vec3(ViewMatrix * vec4(LightPosition, 1)) - data.Position);

    gl_Position = ProjectionMatrix * ViewMatrix * position;
    texCoord = in_texCoord;*/

    Normal = mat3(transpose(inverse(R * ModelMatrix))) * inNormal;
    Position = vec3(R * ModelMatrix * vec4(inPosition, 1.0));
    gl_Position = ProjectionMatrix * ViewMatrix * R * ModelMatrix * vec4(inPosition, 1.0f);
};

#shader fragment
#version 330 core

layout (location = 0) out vec4 color;

in vec3 Normal;
in vec3 Position;

uniform vec3 cameraPos;
uniform samplerCube skybox;

//in Data
//{
//    vec3 Position;
//    vec3 Normal;
//    vec2 TexCoord;
//} data;

//in vec3 CameraVector;

//uniform sampler2D myTexture;

//vec3 ambientReflectenceCoefficient = vec3(0.1f);
//vec3 ambientLightColor = vec3(0.1f);
//vec3 specularReflectenceCoefficient = vec3(0.5f);
//vec3 specularLightColor = vec3(1.0f);
//float specularExponent = 150;
//vec3 diffuseReflectenceCoefficient = vec3(1.0f);
//vec3 diffuseLightColor = vec3(1.0f);

void main()
{
    //vec4 texColor = texture(myTexture, data.TexCoord);

    //diffuseReflectenceCoefficient = vec3(texColor.x, texColor.y, texColor.z);
    //vec3 ambient = ambientLightColor * ambientReflectenceCoefficient;
    //float costheta = max(dot(data.Normal, LightVector), 0);
    //vec3 diffuse = diffuseLightColor * costheta * diffuseReflectenceCoefficient;
    //float costheta2 = max(dot(data.Normal, LightVector2), 0);
    //vec3 diffuse2 = diffuseLightColor * costheta2 * diffuseReflectenceCoefficient;
    //float phong = pow(max(dot(CameraVector, reflect), 0), specularExponent);
    //vec3 specular = specularLightColor * phong * specularReflectenceCoefficient;
    //float phong2 = pow(max(dot(CameraVector, reflect2), 0), specularExponent);
    //vec3 specular2 = specularLightColor * phong2 * specularReflectenceCoefficient;
    //
    //vec4 ucolor = vec4(ambient + diffuse + diffuse2 + specular + specular2, 1.0) /* texColor*/;

    vec3 I = normalize(Position - cameraPos);
    vec3 R = reflect(I, normalize(Normal));
    color = vec4(texture(skybox, R).rgb, 1.0);
};