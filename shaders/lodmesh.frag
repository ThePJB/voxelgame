#version 460 core
in vec3 normal;
in vec3 colour;
out vec4 FragColour;

//uniform vec3 light;
vec3 light = normalize(vec3(1, 2, 0));

float texture_w = 10; 

void main(){

    float brightness = dot(light, normal);
    brightness = 0.5 + brightness/2;    

    vec3 adjustedColour = brightness * colour;
    FragColour = vec4(adjustedColour, 1);
    //FragColour = vec4(1,1,1,1);
    //FragColour = vec4(brightness, brightness, brightness, 1);
}