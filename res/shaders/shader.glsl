#shader vert
#version 460 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

uniform mat4 u_model;
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

const vec3 light_direction = vec3(1.0f, 1.0f, 1.0f);

in vec3 v_position_W;
in vec3 v_normal_W;

uniform vec3 u_camera_position_W;

out vec4 o_color;

void main()
{
    vec3 view_direction = normalize(u_camera_position_W - v_position_W);
    vec3 normal = normalize(v_normal_W);
    vec3 direction = normalize(-light_direction);
    vec3 half_way_direction = normalize(direction + view_direction);

    float diffuse_factor = max(dot(normal, direction), 0.0f);
    vec3 diffuse = vec3(0.8f, 0.2f, 0.3f) * diffuse_factor;

    vec3 specular = vec3(1.0f) * pow(max(dot(normal, half_way_direction), 0.0f), 32.0f);

    o_color = vec4(diffuse + specular, 1.0f);
}
