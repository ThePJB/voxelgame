#version 460 core
in vec3 normal;
in vec3 colour;
in float tvalue;

out vec4 FragColour;

//uniform vec3 light;
const vec3 distant_blue = vec3(0.4, 0.5, 0.8);

vec3 light = normalize(vec3(1, 2, 0));

float texture_w = 10; 


void main(){

    float brightness = dot(light, normal);
    brightness = 0.5 + brightness/2;    

    vec3 adjustedColour = brightness * colour;

    vec3 finalColour = mix(adjustedColour, distant_blue, tvalue);

    FragColour = vec4(finalColour, 1);
}