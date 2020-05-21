#pragma once

namespace HaloSim
{

enum Projection
{
    Stereographic = 0,
    Rectilinear,
    Equidistant,
    EqualArea,
    Orthographic
};

struct Camera
{
    float pitch;
    float yaw;
    float fov;
    Projection projection;
    bool hideSubHorizon;

    float getMaximumFov() const;
    static Camera createDefaultCamera();
};

} // namespace HaloSim
