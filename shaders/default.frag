#version 460

in vec3 normal;
in vec2 texcoord;

out vec4 FragColor;

uniform sampler2D albedo;

void main() {
    vec3 lightDir = normalize(vec3(0, -1, -1));
    float cosTheta = max(.0f, dot(normal, -lightDir));
    vec4 color = texture(albedo, texcoord);

    if(color.a < 0.1)
        discard;

    FragColor = color * vec4(cosTheta,cosTheta,cosTheta,cosTheta);
}