#shader vert
#version 460 core

layout (location = 0) in vec3 a_position;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);
}

#shader frag
#version 460 core

uniform vec3 u_color;

out vec4 o_color;

void main()
{
    o_color = vec4(u_color, 1.0f);
}