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

const vec3 light_direction = vec3(1.0f, -1.0f, 0.0f);

in vec3 v_position_W;
in vec3 v_normal_W;

uniform vec3 u_camera_position_W;
uniform vec3 u_color = vec3(0.22f, 0.42f, 0.046f);

layout (binding = 0) uniform sampler2D s_grass;
layout (binding = 1) uniform sampler2D s_dirt;

out vec4 o_color;

void main()
{
    vec3 normal = normalize(v_normal_W);
    // Triplanar mapping
    vec3 blending = normalize(max(abs(normal), 0.00001f));
    float b = blending.x + blending.y + blending.z;
    blending /= vec3(b);
    blending *= normalize(vec3(1.0f, 4.0f, 1.0f));

    const float scale_factor = 0.2f;
    vec3 x_axis = texture(s_dirt, v_position_W.yz * scale_factor).xyz;
    vec3 y_axis = texture(s_grass, v_position_W.xz * scale_factor).xyz;
    vec3 z_axis = texture(s_dirt, v_position_W.xy * scale_factor).xyz;

    vec3 color = x_axis * blending.x + y_axis * blending.y + z_axis * blending.z;
    //color *= normalize(vec3(0.4f, 1.25f, 1.0f));

    // Directional lighting
    vec3 view_direction = normalize(u_camera_position_W - v_position_W);
    vec3 direction = normalize(-light_direction);
    vec3 half_way_direction = normalize(direction + view_direction);
    
    float diffuse_factor = max(dot(normal, direction), 0.4f);
    float steepness = 1.0f - dot(normal, vec3(0.0f, 1.0f, 0.0f));
    vec3 diffuse = color * diffuse_factor;
    vec3 ambient = vec3(0.05f * steepness);
    vec3 specular = vec3(0.05f) * pow(max(dot(normal, half_way_direction), 0.0f), 16.0f);
    o_color = vec4((ambient + diffuse + specular) * 1.6f, 1.0f);
}
