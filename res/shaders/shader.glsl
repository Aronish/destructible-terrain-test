#shader vert
#version 460 core

layout (location = 0) in vec2 a_position;

void main()
{
    gl_Position = vec4(a_position, 0.0f, 1.0f);
}

#shader frag
#version 460 core

out vec4 o_color;

void main()
{
    o_color = vec4(0.8f, 0.2f, 0.3f, 1.0f);
}
