#shader vert
#version 460 core

layout (location = 0) in vec2 a_position;

uniform mat4 u_model;
uniform mat4 u_view;
uniform mat4 u_projection;

out vec2 v_position;

void main()
{
    v_position = a_position;
    gl_Position = u_projection * u_view * u_model * vec4(a_position.x, 0.0, a_position.y, 1.0f);
}

#shader frag
#version 460 core

in vec2 v_position;

out vec4 o_color;

void main()
{
    o_color = vec4(v_position, 0.0f, 1.0f);
}
