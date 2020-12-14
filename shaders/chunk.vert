#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec2 aTexCoord;
layout (location = 2) in vec3 aNormal;
layout (location = 3) in float aTexture_x;

in float in_light_block;
in float in_light_sky;
in float in_ao;

out vec2 TexCoord;
out vec4 normal;
out float texture_x;
out float light_level;

uniform float dayness;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
   gl_Position = projection * view * model * vec4(aPos, 1.0);
   TexCoord = aTexCoord;
   normal = vec4(aNormal, 1.0);
   texture_x = aTexture_x;

   float dayness_adjusted = 0.2 + dayness * 0.8;

   light_level = max(in_light_block, in_light_sky * dayness_adjusted * in_ao);
}
