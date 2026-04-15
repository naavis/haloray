#pragma once
#include <QObject>

namespace HaloRay
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
    float pitch = 0.0f;
    float yaw = 0.0f;
    float fov = 75.0f;
    Projection projection = Projection::Stereographic;
    bool hideSubHorizon = false;

    float getMaximumFov() const;
    float getFocalLength() const;
    bool operator==(const Camera&) const;
    bool operator!=(const Camera&) const;
    static Camera createDefaultCamera();

};

}

Q_DECLARE_METATYPE(HaloRay::Camera)
