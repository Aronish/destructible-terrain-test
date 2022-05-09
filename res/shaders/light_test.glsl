#shader vert
#version 460 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform mat4 u_model = mat4(1.0f);
uniform mat4 u_view;
uniform mat4 u_projection;

out vec3 v_position_W;
out vec3 v_normal_W;

void main()
{
    v_position_W = (u_model * vec4(a_position, 1.0f)).xyz;
    v_normal_W = mat3(u_model) * a_normal;
    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);
}

#shader frag
#version 460 core

const vec3 light_direction = vec3(0.0f, -1.0f, 0.0f);

in vec3 v_position_W;
in vec3 v_normal_W;

uniform float u_points_per_axis;
uniform vec3 u_camera_position_W;
uniform vec3 u_color = vec3(0.22f, 0.42f, 0.046f);

layout (std140, binding = 0) uniform WorldGenerationConfig
{
    int u_octaves_2d, u_octaves_3d;
    float u_frequency_2d, u_lacunarity_2d, u_persistence_2d, u_amplitude_2d, u_exponent_2d;
    float u_frequency_3d, u_lacunarity_3d, u_persistence_3d, u_amplitude_3d, u_exponent_3d;
    float u_water_level;
};

out vec4 o_color;

void main()
{
    vec3 view_direction = normalize(u_camera_position_W - v_position_W);
    vec3 normal = normalize(v_normal_W);
    vec3 direction = normalize(-light_direction);
    vec3 half_way_direction = normalize(direction + view_direction);

    float diffuse_factor = max(dot(normal, direction), 0.0f);
    float steepness = 1.0f - dot(normal, vec3(0.0f, 1.0f, 0.0f));
    vec3 diffuse = vec3(diffuse_factor);
    if (v_position_W.y < 0.01f * u_points_per_axis)
    {
        diffuse *= vec3(0.64f, 0.77f, 0.52f);
    } else if (v_position_W.y >= 0.01f * u_points_per_axis && v_position_W.y < 0.8 * u_points_per_axis)
    {
        diffuse *= vec3(0.22f, 0.42f, 0.046f);// * steepness;
    } else
    {
        diffuse *= vec3(0.3f, 0.3f, 0.3f);
    }
    vec3 ambient = vec3(0.1f * steepness + 0.01f);
    vec3 specular = vec3(0.25f) * pow(max(dot(normal, half_way_direction), 0.0f), 8.0f);
    o_color = vec4(ambient + diffuse + specular, 1.0f);
}
