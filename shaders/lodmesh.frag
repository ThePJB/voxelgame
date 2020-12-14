#version 330 core
in vec3 normal;
in vec3 colour;
in float tvalue;

out vec4 FragColour;

uniform vec3 light_dir;
uniform float dayness;

const vec3 distant_blue = vec3(0.4, 0.5, 0.8);

float texture_w = 10; 


void main(){
    float brightness = 0.3 + dayness * 0.7;
    float facing_ratio = dot(light_dir, normal);
    facing_ratio = 0.7 + 0.3*facing_ratio;    

    vec3 adjustedColour = mix(colour, distant_blue, tvalue);
    adjustedColour = facing_ratio * brightness * adjustedColour;

    FragColour = vec4(adjustedColour, 1);
}