// Shaded：vLocalPos 供局部轴分界；vUV 供采样
// vWorldNormal：世界空间法线，供 morph_blend_lit.frag 的 DIRECTIONAL_LIGHT() 使用（与 Qt 生成的 VAR_WORLD_NORMAL 解耦，避免自定义顶点后默认受光为 0）
VARYING vec3 vLocalPos;
VARYING vec2 vUV;
VARYING vec3 vWorldNormal;

void MAIN()
{
    vLocalPos = VERTEX;
    vUV = UV0;

    vec3 nObj = NORMAL;
    if (dot(nObj, nObj) < 1e-10)
        nObj = vec3(0.0, 0.0, 1.0);
    vWorldNormal = normalize(NORMAL_MATRIX * nObj);

    POSITION = MODELVIEWPROJECTION_MATRIX * vec4(VERTEX, 1.0);
}
