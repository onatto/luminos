#version 440 core
restrict writeonly layout(rgba16f) uniform image2D dstTex;

layout (local_size_x = 16, local_size_y = 16) in;

void main() {
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    imageStore(dstTex, gid, vec4(gid.x/256.0, gid.y/256.0, 0.0, 1.0));
}
