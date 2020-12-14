#version 330 core
in vec2 TexCoord;
in vec4 normal;
out vec4 FragColour;

uniform sampler2D ourTexture;
uniform vec3 light;

float ambient = 0.3;

void main(){
   //FragColour = vec4(normal.x, normal.y, normal.z, 1.0);
   //FragColour = texture(ourTexture, TexCoord) * max(ambient, dot(light, normalize(vec3(normal.x, normal.y, normal.z))));
   FragColour = texture(ourTexture, TexCoord) * vec4(TexCoord.x, TexCoord.y, 1, 1);
   //FragColour = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
}