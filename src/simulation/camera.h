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

    bool operator==(const struct Camera &other) const
    {
        return pitch == other.pitch &&
               yaw == other.yaw &&
               fov == other.fov &&
               projection == other.projection &&
               hideSubHorizon == other.hideSubHorizon;
    }

    bool operator!=(const struct Camera &other) const
    {
        return !operator==(other);
    }
};

}
