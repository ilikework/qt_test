// SPDX-License-Identifier: BSD-3-Clause
// Qt 6 ShaderEffect fragment — compiled to .qsb via qt_add_shaders

#version 440

layout(location = 0) in vec2 qt_TexCoord0;
layout(location = 0) out vec4 fragColor;

layout(binding = 1) uniform sampler2D source;

layout(std140, binding = 0) uniform buf {
    mat4 qt_Matrix;
    float qt_Opacity;
    float centerNX;
    float centerNY;
    float radiusN;
    float zoomMag;
    float texW;
    float texH;
} ubuf;

void main()
{
    vec2 uv = qt_TexCoord0;
    float mn = min(ubuf.texW, ubuf.texH);
    vec2 p = vec2(uv.x * ubuf.texW, uv.y * ubuf.texH);
    vec2 c = vec2(ubuf.centerNX * ubuf.texW, ubuf.centerNY * ubuf.texH);
    float r = ubuf.radiusN * mn;
    float dist = length(p - c);
    float edge = max(1.5, 0.004 * mn);
    float a = 1.0 - smoothstep(r - edge, r + 0.5, dist);
    if (a < 0.001) {
        fragColor = vec4(0.0);
        return;
    }
    vec2 srcUV = vec2(ubuf.centerNX, ubuf.centerNY) + (uv - vec2(ubuf.centerNX, ubuf.centerNY)) / ubuf.zoomMag;
    srcUV = clamp(srcUV, 0.0, 1.0);
    vec4 col = texture(source, srcUV);
    fragColor = vec4(col.rgb, col.a * a) * ubuf.qt_Opacity;
}
