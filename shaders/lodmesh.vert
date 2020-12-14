#version 330 core
layout (location = 0) in vec3 aPos;
layout (location = 1) in vec3 aColour;
layout (location = 2) in vec3 aNormal;


out vec3 normal;
out vec3 colour;

out float tvalue;

const float bluedist = 1000;

uniform mat4 view;
uniform mat4 projection;
uniform vec3 player_pos;

void main(){
   gl_Position = projection * view * vec4(aPos, 1.0);
   
   normal = aNormal;
   colour = aColour;

   vec3 dv = aPos - player_pos;
   float d = sqrt(dot(dv,dv));

   tvalue = max(d - bluedist, 0) / d;
}
