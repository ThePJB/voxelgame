#version 460 core
in vec2 TexCoord;
in vec4 normal;
in float texture_x;
in float light_level;
out vec4 FragColour;

uniform sampler2D ourTexture;
uniform vec3 light;

float texture_h = 10; 
float texture_w = 20; 

void main(){
   float texture_y = 1 - normal.y;

   vec2 thisTexCoords = vec2((TexCoord.x + texture_x) / texture_w, (TexCoord.y + texture_y) / texture_h);
   vec4 tex = texture(ourTexture, thisTexCoords);
   FragColour = vec4(tex.xyz*light_level, 1);
}