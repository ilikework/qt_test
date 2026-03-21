// 变脸贴图混合：UV.x < uLeftRatio 用左贴图，否则用右贴图。Shaded 模式设 BASE_COLOR，引擎会加光照
void MAIN()
{
    vec2 uv = UV0;
    vec4 leftColor = texture(leftTex, uv);
    vec4 rightColor = texture(rightTex, uv);
    BASE_COLOR = (uv.x < uLeftRatio) ? leftColor : rightColor;
}
