#version 440 core

in vec3 world_normal;

out vec4 fragColor;

void main() {
   fragColor = vec4(normalize(world_normal), 1.0); 
}
