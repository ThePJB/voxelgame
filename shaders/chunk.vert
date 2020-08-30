#version 460 core
in vec3 aPos;
in vec2 aTexCoord;
in vec3 aNormal;
in float aTexture_x;

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
