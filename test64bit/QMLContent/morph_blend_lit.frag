// 变脸 + 3D：MAIN() 输出 BASE_COLOR/EMISSIVE/MR；显式实现 DIRECTIONAL_LIGHT()。
// 原因：仅写 MAIN() 依赖引擎默认方向光时，在「自定义顶点 + RuntimeLoader 网格」下 VAR_WORLD_NORMAL 易失效，
// N·L≈0 → 只有自发光可见，调节 DirectionalLight.brightness 无效。此处用顶点 VARYING vWorldNormal 做 Lambert 漫反射。
// 未实现 AMBIENT_LIGHT / IBL_PROBE / POINT_LIGHT 等时仍走引擎默认（与 Principled 一致）。
VARYING vec3 vLocalPos;
VARYING vec2 vUV;
VARYING vec3 vWorldNormal;

vec3 srgbToLinear(vec3 c)
{
    bvec3 lo = lessThanEqual(c, vec3(0.04045));
    vec3 low = c / 12.92;
    vec3 high = pow((c + 0.055) / 1.055, vec3(2.4));
    return mix(high, low, vec3(lo));
}

float pickAxis(vec3 p)
{
    if (uBlendAxis == 0)
        return p.x;
    if (uBlendAxis == 1)
        return -p.y;
    return p.z;
}

vec4 morphBlendColorLinear()
{
    vec2 uv = vUV;
    vec4 leftColor = texture(leftTex, uv);
    vec4 rightColor = texture(rightTex, uv);

    if (uAssumeSrgbTexture > 0.5) {
        leftColor.rgb = srgbToLinear(leftColor.rgb);
        rightColor.rgb = srgbToLinear(rightColor.rgb);
    }

    float t;
    if (uUseMeshAxis > 0.5) {
        float c = pickAxis(vLocalPos);
        t = clamp(c * uMeshScale + uMeshBias, 0.0, 1.0);
    } else {
        t = uv.x;
    }

    float f = max(uFeather, 0.0001);
    float w = smoothstep(uLeftRatio - f, uLeftRatio + f, t);
    return mix(leftColor, rightColor, w);
}

void MAIN()
{
    vec4 blended = morphBlendColorLinear();
    vec3 baseLin = blended.rgb;
    // 与 3DDemo material.frag 一致：略提色度 + 暖色，减轻开灯发灰、偏冷
    float lu = dot(baseLin, vec3(0.2126, 0.7152, 0.0722));
    vec3 chr = baseLin - vec3(lu);
    baseLin = vec3(lu) + chr * uLitChromaBoost;
    baseLin *= uLitWarmTint;
    baseLin = clamp(baseLin, 0.0, 1.0);
    vec3 emiLin = baseLin * uEmissiveFactor;
    if (uApplySceneExposure > 0.5) {
        baseLin *= uSceneExposure;
        emiLin *= uSceneExposure;
    }
    BASE_COLOR = vec4(baseLin, blended.a);
    EMISSIVE_COLOR = emiLin;
    METALNESS = 0.05;
    ROUGHNESS = 0.48;
    SPECULAR_AMOUNT = 0.3;
}

void DIRECTIONAL_LIGHT()
{
    vec3 n = normalize(vWorldNormal);
    float ndotl = max(0.0, dot(n, TO_LIGHT_DIR));
    vec3 kd = BASE_COLOR.rgb * (1.0 - METALNESS);
    DIFFUSE += kd * LIGHT_COLOR * SHADOW_CONTRIB * ndotl;
}
