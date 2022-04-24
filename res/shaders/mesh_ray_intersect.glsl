#shader comp
#version 460 core

const uint MAX_INVOCATIONS = 1536;

struct UnpaddedTriangle
{
    float  x_1,  y_1,  z_1;
    float nx_1, ny_1, nz_1;
    float  x_2,  y_2,  z_2;
    float nx_2, ny_2, nz_2;
    float  x_3,  y_3,  z_3;
    float nx_3, ny_3, nz_3;
};

layout (std430, binding = 0) buffer Mesh
{
    UnpaddedTriangle triangles[];
};

layout (std430, binding = 1) buffer RayHitData
{
    UnpaddedTriangle hit_triangle;
    float hit;
};

uniform mat4 u_transform;
uniform vec3 u_ray_origin;
uniform vec3 u_ray_direction;
const float EPSILON = 0.0000001;

layout (local_size_x = MAX_INVOCATIONS, local_size_y = 1, local_size_z = 1) in;

void main()
{
    UnpaddedTriangle triangle = triangles[gl_GlobalInvocationID.x];
    //if (dot(vec3(triangle.nx_1, triangle.ny_1, triangle.nz_1), u_ray_direction) > 0.0f) return;
    vec3 vertex0 = vec3(u_transform * vec4(triangle.x_1, triangle.y_1, triangle.z_1, 1.0f));
    vec3 vertex1 = vec3(u_transform * vec4(triangle.x_2, triangle.y_2, triangle.z_2, 1.0f));
    vec3 vertex2 = vec3(u_transform * vec4(triangle.x_3, triangle.y_3, triangle.z_3, 1.0f));
    vec3 edge1 = vertex1 - vertex0, edge2 = vertex2 - vertex0, h = cross(u_ray_direction, edge2), s, q;
    float a = dot(edge1, h), f, u, v;
    if (a > EPSILON && a < EPSILON) return;
    f = 1.0f / a;
    s = u_ray_origin - vertex0;
    u = f * dot(s, h);
    if (u < 0.0f || u > 1.0f) return;
    q = cross(s, edge1);
    v = f * dot(u_ray_direction, q);
    if (v < 0.0f || u + v > 1.0f) return;
    float t = f * dot(edge2, q);
    if (t > EPSILON && hit < 1.0f)
    {
        hit_triangle = triangle;
        hit_triangle.x_1 = vertex0.x;
        hit_triangle.y_1 = vertex0.y + 0.001f;
        hit_triangle.z_1 = vertex0.z;

        hit_triangle.x_2 = vertex1.x;
        hit_triangle.y_2 = vertex1.y + 0.001f;
        hit_triangle.z_2 = vertex1.z;

        hit_triangle.x_3 = vertex2.x;
        hit_triangle.y_3 = vertex2.y + 0.001f;
        hit_triangle.z_3 = vertex2.z;
        hit = 1.0f;
    }
}