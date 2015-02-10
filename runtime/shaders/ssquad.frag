#version 440 core

in vec2 v_tex;

out vec4 fragColor;

uniform sampler2D tex;

void main() {
   fragColor = texture(tex, v_tex);
   //fragColor = vec4(v_tex, 1.0, 1.0); 
}
