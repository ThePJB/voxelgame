#version 460 core
in vec2 TexCoord;
in vec4 normal;
out vec4 FragColour;

uniform sampler2D ourTexture;
uniform vec3 light;

float ambient = 0.3;

void main(){
   FragColour = texture(ourTexture, TexCoord) * max(ambient, dot(light, normalize(vec3(normal.x, normal.y, normal.z))));
   //FragColour = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
}