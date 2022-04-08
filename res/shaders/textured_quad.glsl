#shader vert
#version 460 core

layout (location = 0) in vec2 a_position;
layout (location = 1) in vec2 a_texture_coordinate;

uniform mat4 u_model;
uniform mat4 u_projection;

out vec2 v_texture_coordinate;

void main()
{
    v_texture_coordinate = a_texture_coordinate;
    gl_Position = u_projection * u_model * vec4(a_position, 0.0f, 1.0f);
}

#shader frag
#version 460 core

in vec2 v_texture_coordinate;

layout (binding = 0) uniform sampler2D s_texture;

out vec4 o_color;

void main()
{
    o_color = texture(s_texture, v_texture_coordinate);
}