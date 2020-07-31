#version 460 core

in vec3 pos;
out vec4 FragColour;

uniform float solar_angle;
uniform float dayness;
uniform float sunriseness;

const float twilight_angle = 12;

const vec4 day_colour = vec4(0.3, 0.5, 0.8, 1);
const vec4 night_colour = vec4(0.05, 0.03, 0.1, 1);

const vec4 sunrise_colour = vec4(0.8, 0.6, 0.3, 1);

void main(){
    vec4 background_colour = mix(night_colour, day_colour, dayness);
    vec3 sun_pos = vec3(0, sin(radians(solar_angle)), cos(radians(solar_angle)));
    vec3 sky_pos = normalize(pos);
    //float L2_dist_to_sun = distance(normalize(pos), sun_pos);
    float custom_dist = abs(0.01 * sky_pos.x - sun_pos.x) + abs(sky_pos.y - sun_pos.y) + abs(sky_pos.z - sun_pos.z);

    FragColour = mix(background_colour, sunrise_colour, sunriseness*(1-custom_dist));
}