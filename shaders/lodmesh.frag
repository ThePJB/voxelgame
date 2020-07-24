#version 460 core
in vec3 normal;
in vec3 colour;
out vec4 FragColour;

//uniform vec3 light;
vec3 light = vec3(0, 1, 0);

float texture_w = 10; 

void main(){

    float brightness = dot(light, normal);
    
    vec3 adjustedColour = brightness * colour;
    FragColour = vec4(adjustedColour, 1);
    //FragColour = vec4(1,1,1,1);
    //FragColour = vec4(brightness, brightness, brightness, 1);
}