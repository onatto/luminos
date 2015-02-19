#version 440 core
restrict readonly layout(rgba16f, binding = 0) uniform image2D srcTex;
restrict writeonly layout(rgba16f, binding = 1) uniform image2D dstTex;

layout (local_size_x = 16, local_size_y = 16) in;
void main() {
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dim = imageSize(srcTex);
    vec4 color = vec4(0.0);
    color += imageLoad(srcTex, gid + ivec2(-1, -1)) * 0.094;
    color += imageLoad(srcTex, gid + ivec2( 0, -1)) * 0.118;
    color += imageLoad(srcTex, gid + ivec2( 1, -1)) * 0.094;
    color += imageLoad(srcTex, gid + ivec2(-1,  0)) * 0.118;
    color += imageLoad(srcTex, gid + ivec2( 0,  0)) * 0.147;
    color += imageLoad(srcTex, gid + ivec2( 1,  0)) * 0.118;
    color += imageLoad(srcTex, gid + ivec2(-1,  1)) * 0.094;
    color += imageLoad(srcTex, gid + ivec2( 0,  1)) * 0.118;
    color += imageLoad(srcTex, gid + ivec2( 1,  1)) * 0.094;
    //imageStore(dstTex, gid, vec4(color.r, 1.0, 0.0. 1.0));
    imageStore(dstTex, gid, color);
}
