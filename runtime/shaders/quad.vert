#version 440 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex;

out gl_PerVertex
{
      vec4 gl_Position;
};

uniform mat4 proj;
uniform vec4 offset;

out vec2 v_tex;
void main() {
    v_tex = vec2(a_tex.x, a_tex.y);
    gl_Position = proj * vec4((a_pos * 0.5 + 0.5) * offset.zw + offset.xy, 0.0, 1.0);
}
