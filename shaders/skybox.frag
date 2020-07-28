#version 460 core

in vec3 pos;
out vec4 FragColour;

uniform float solar_angle;
uniform float dayness;
const float twilight_angle = 12;

const vec4 day_colour = vec4(0.3, 0.5, 0.8, 1);
const vec4 night_colour = vec4(0.05, 0.03, 0.1, 1);

void main(){
    FragColour = mix(night_colour, day_colour, dayness);
}