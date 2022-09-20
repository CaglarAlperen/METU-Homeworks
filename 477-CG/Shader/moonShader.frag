#version 430

in Data
{
    vec3 Position;
    vec3 Normal;
    vec2 TexCoord;
} data;
in vec3 LightVector;
in vec3 CameraVector;

uniform vec3 lightPosition;
uniform sampler2D TexColor;
uniform sampler2D MoonTexColor;
uniform sampler2D TexGrey;
uniform float textureOffset;

out vec4 FragColor;

vec3 ambientReflectenceCoefficient = vec3(0.5f);  
vec3 ambientLightColor = vec3(0.6f);
vec3 specularReflectenceCoefficient= vec3(1.0f);
vec3 specularLightColor = vec3 (1.0f);
float SpecularExponent = 10;
vec3 diffuseReflectenceCoefficient= vec3(1.0f);
vec3 diffuseLightColor =vec3(1.0f);


void main()
{
    vec3 reflect = reflect(-LightVector, data.Normal);

    vec2 offset = vec2(mod(textureOffset +  data.TexCoord.x, 1), data.TexCoord.y);
    vec4 texColor = texture(TexColor, offset);

    diffuseReflectenceCoefficient = vec3(texColor.x, texColor.y, texColor.z);

    vec3 ambient = ambientLightColor * ambientReflectenceCoefficient;
    float costheta = max(dot(data.Normal, LightVector), 0);
    vec3 diffuse = diffuseLightColor * costheta * diffuseReflectenceCoefficient;
    float phongexp = pow(max(dot(CameraVector, reflect), 0), SpecularExponent);
    vec3 spec = specularLightColor * phongexp * specularReflectenceCoefficient;

    
    vec4 color = vec4(ambient + diffuse + spec, 1.0) * texColor;


    FragColor = vec4(clamp(color.xyz, 0.0, 1.0), 1);

}
