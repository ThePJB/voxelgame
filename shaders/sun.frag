#version 460 core

out vec4 FragColour;

uniform float solar_angle;

void main(){
    //vec3 colour = vec3(1,0,0);
    vec3 colour = vec3(1,1,1);
    // then try some gl position stuff

    FragColour = vec4(colour, 1);
}