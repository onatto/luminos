#version 440 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex;

out gl_PerVertex
{
      vec4 gl_Position;
};

out vec2 v_tex;
void main() {
    v_tex = a_tex;
    gl_Position = vec4(a_pos, 0.999, 1.0);
}
