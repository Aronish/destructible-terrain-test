#shader vert
#version 460 core

layout (location = 0) in vec3 a_position;
layout (location = 1) in vec3 a_normal;

out GS_DATA
{
    vec3 normal;
} gs_data;

void main()
{
    gs_data.normal = a_normal;
    gl_Position = vec4(a_position, 1.0f);
}

#shader geom
#version 460 core

layout (points) in;
layout (line_strip, max_vertices = 2) out;

in GS_DATA
{
    vec3 normal;
} gs_data[];

uniform mat4 u_model = mat4(1.0f);
uniform mat4 u_view;
uniform mat4 u_projection;

const float NORMAL_LENGTH = 0.01f;

void main()
{
    vec3 position = gl_in[0].gl_Position.xyz;
    mat4 combined = u_projection * u_view * u_model;

    gl_Position = combined * vec4(position, 1.0f);
    EmitVertex();

    gl_Position = combined * vec4(position + gs_data[0].normal * NORMAL_LENGTH, 1.0f);
    EmitVertex();
    EndPrimitive();
}

#shader frag
#version 460 core

out vec4 o_color;

void main()
{
    o_color = vec4(0.0f, 0.0f, 0.7f, 1.0f);
}