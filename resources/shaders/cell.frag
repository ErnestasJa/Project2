#version 330
uniform vec3 ldir;
uniform mat3 MV3x3;

smooth in vec3 vpos;
smooth in vec3 vnormal;
out vec3 color;

void main()
{
    float lpower = 120.0;
    float cos_nl = dot(MV3x3*ldir, normalize(vnormal));
    float intensity = cos_nl;

    if (intensity > 0.95)
        color = vec3(1.0,0.5,0.5);
    else if (intensity > 0.5)
        color = vec3(0.6,0.3,0.3);
    else if (intensity > 0.25)
        color = vec3(0.4,0.2,0.2);
    else
        color = vec3(0.2,0.1,0.1);

    color = color;

}