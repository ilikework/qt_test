// 变脸、关 3D：Unshaded，完整输出 FRAGCOLOR（不参与场景方向光）
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

vec3 linearToSrgb(vec3 c)
{
    bvec3 lo = lessThan(c, vec3(0.0031308));
    vec3 low = c * 12.92;
    vec3 high = 1.055 * pow(c, vec3(1.0 / 2.4)) - 0.055;
    return mix(high, low, vec3(lo));
}

vec3 ACESFilm(vec3 x)
{
    const float a = 2.51;
    const float b = 0.03;
    const float c = 2.43;
    const float d = 0.59;
    const float e = 0.14;
    return clamp((x * (a * x + b)) / (x * (c * x + d) + e), 0.0, 1.0);
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
    vec3 rgb = blended.rgb;
    rgb *= uEmissiveFactor;
    if (uApplySceneExposure > 0.5) {
        rgb *= uSceneExposure;
    }
    rgb *= uPrincipledMatchGain;
    float l = dot(rgb, vec3(0.2126, 0.7152, 0.0722));
    vec3 chroma = rgb - vec3(l);
    rgb = vec3(l) + chroma * uUnshadedSaturationBoost;
    rgb = clamp(rgb, 0.0, 1.0);

    if (uTonemapFlavor > 0) {
        rgb = ACESFilm(rgb);
    } else {
        rgb = clamp(rgb, 0.0, 1.0);
    }

    if (uEncodeSrgbToDisplay > 0.5) {
        rgb = linearToSrgb(rgb);
    }
    FRAGCOLOR = vec4(rgb, blended.a);
}
