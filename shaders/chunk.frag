#version 460 core
in vec2 TexCoord;
in vec4 normal;
in float texture_x;
out vec4 FragColour;

uniform sampler2D ourTexture;
uniform vec3 light;

float texture_w = 10; 

void main(){
   float texture_y = 1 - normal.y;

   vec2 thisTexCoords = vec2(TexCoord.x + texture_x, TexCoord.y + texture_y) / texture_w;
   //vec2 thisTexCoords = vec2(TexCoord.x, TexCoord.y);

   //FragColour = vec4(normal.x, normal.y, normal.z, 1.0);
   //FragColour = texture(ourTexture, TexCoord) * max(ambient, dot(light, normalize(vec3(normal.x, normal.y, normal.z))));
   FragColour = texture(ourTexture, thisTexCoords);
   //FragColour = vec4(TexCoord.x, TexCoord.y, 0.0, 1.0);
   
   //FragColour = vec4(1.0 , 1.0, 0.0, 1.0);
}