#shader comp
#version 460 core

layout(std430, binding = 0) buffer DensityDistribution
{
    float values[];
};

layout (std430, binding = 1) buffer RayHitData
{
    UnpaddedTriangle hit_triangle;
    float hit;
};

void main()
{
    /*
    Store chunk coordinate in hit data and send in all nine chunks around you (SNIGELFART??)
    Figure out some other way of determining which chunk's density distribution to get
    */
}