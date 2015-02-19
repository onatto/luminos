#version 440 core
restrict readonly layout(rgba16f, binding = 0) uniform image2D srcTex;
restrict writeonly layout(rgba16f, binding = 1) uniform image2D dstTex;

layout (local_size_x = 16, local_size_y = 16) in;

float weights[5] = {0.2270270270, 0.1945945946, 0.121621621607, 0.0540540541, 0.0162162162};
void main() {
    ivec2 gid = ivec2(gl_GlobalInvocationID.xy);
    ivec2 dim = imageSize(srcTex);
    vec4 color = imageLoad(srcTex, gid) * weights[0];
    for (int i=1; i<5; i++)
    {
        color += imageLoad(srcTex, gid + ivec2( i, 0)) * weights[i];
        color += imageLoad(srcTex, gid + ivec2(-i, 0)) * weights[i];
    }
    imageStore(dstTex, gid, color * 2.0);
}
