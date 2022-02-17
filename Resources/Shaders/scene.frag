#version 450
#extension GL_ARB_separate_shader_objects : enable

layout(binding = 1000) uniform texture2D light_texture;
layout(binding = 1001) uniform texture2DArray sh_light_map;
layout(binding = 3000) uniform sampler linear_sampler;

layout(location = 0) in vec2 v_uv0;
layout(location = 1) in vec2 v_uv1;
layout(location = 2) in vec4 v_color;
layout(location = 3) in vec3 v_normal;

layout(location = 0) out vec4 out_color;

vec3 GetLightMapData(float layer) {
    vec2 size = vec2(textureSize(sampler2DArray(sh_light_map, linear_sampler), 0));
    vec2 texel_size = vec2(1.0) / size;

    vec3 lm = vec3(0.0);
    lm += texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1 + vec2(0.0, 0.0), layer)).rgb;
    lm += texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1 + vec2(texel_size.x, 0.0), layer)).rgb;
    lm += texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1 + vec2(-texel_size.x, 0.0), layer)).rgb;
    lm += texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1 + vec2(0.0, texel_size.y), layer)).rgb;
    lm += texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1 + vec2(0.0, -texel_size.y), layer)).rgb;
    lm += texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1 + vec2(texel_size.x, texel_size.y), layer)).rgb;
    lm += texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1 + vec2(texel_size.x, -texel_size.y), layer)).rgb;
    lm += texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1 + vec2(-texel_size.x, texel_size.y), layer)).rgb;
    lm += texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1 + vec2(-texel_size.x, -texel_size.y), layer)).rgb;
    return lm / 9.0;
}

void main()
{
    vec3 lm_light_l0 = texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1, 0.0)).rgb;
    vec3 lm_light_l1n1 = texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1, 1.0)).rgb;
    vec3 lm_light_l1_0 = texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1, 2.0)).rgb;
    vec3 lm_light_l1p1 = texture(sampler2DArray(sh_light_map, linear_sampler), vec3(v_uv1, 3.0)).rgb;

    vec3 n = v_normal;
    vec3 ambient_light = lm_light_l0 * 0.282095;
    ambient_light += lm_light_l1n1 * 0.32573 * n.y;
    ambient_light += lm_light_l1_0 * 0.32573 * n.z;
    ambient_light += lm_light_l1p1 * 0.32573 * n.x;
    out_color = vec4(ambient_light, 1.0);
    //out_color = texture(sampler2D(light_texture, linear_sampler), v_uv1);
}