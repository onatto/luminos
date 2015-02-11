#version 440 core

layout(location = 0) in vec3 a_pos;
layout(location = 1) in vec3 a_nor;

out gl_PerVertex {
    vec4 gl_Position;
};
out vec3 world_normal;

layout(std140) uniform Transforms
{
    mat4 world;
    mat4 view;
    mat4 proj;
    mat4 proj_view_world;
    vec4 camera_wspos;
};

void main() {
    world_normal = (world * vec4(a_nor,0)).xyz;
    gl_Position = proj_view_world * vec4(a_pos, 1.0);
}
