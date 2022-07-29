#shader vert
#version 460 core

layout (location = 0) in vec3 a_position;

uniform mat4 u_model = mat4(1.0f);
uniform mat4 u_view;
uniform mat4 u_projection;

void main()
{
    gl_Position = u_projection * u_view * u_model * vec4(a_position, 1.0f);
}

#shader frag
#version 460 core

uniform vec4 u_color = vec4(1.0f, 0.0f, 1.0f, 1.0f);

out vec4 o_color;

void main()
{
    o_color = u_color;
}