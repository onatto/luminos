#version 440 core

layout(location = 0) in vec2 a_pos;
layout(location = 1) in vec2 a_tex;

out gl_PerVertex
{
      vec4 gl_Position;
};

layout(std140) uniform Transforms
{
    mat4 world;
    mat4 view;
    mat4 proj;
    mat4 proj_view_world;
    vec4 camera_wspos;
};

out vec2 v_tex;

void main() {
    v_tex = a_tex;
    gl_Position = proj * view * world * vec4(a_pos, 0.0, 1.0);
}
