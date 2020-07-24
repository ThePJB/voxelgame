#version 460 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aNormal;
layout (location = 2) in vec3 aColour;

out vec3 normal;
out vec3 colour;

const vec3 distant_blue = vec3(0.1, 0.4, 1.0);

uniform vec3 player_pos;
//uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;

void main(){
   gl_Position = projection * view * vec4(aPos, 1.0);
   normal = vec3(aNormal);
   float d = distance(player_pos, aPos);
   //float d = sqrt((player_pos.x-aPos.x)*(player_pos.x-aPos.x) + (player_pos.y-aPos.y*player_pos.y + player_pos.z*player_pos.z);
   const float bluedist = 1000;
   float t = max(bluedist - d, 0) / bluedist;
   colour = mix(aColour, distant_blue, t);
}
