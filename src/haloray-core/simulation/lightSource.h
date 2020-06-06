#pragma once
#include <QObject>

namespace HaloRay
{

struct LightSource
{
    float altitude = 30.0f;
    float diameter = 0.5f;

    bool operator==(const LightSource&) const;
    bool operator!=(const LightSource&) const;

    static LightSource createDefaultLightSource();
};

}

Q_DECLARE_METATYPE(HaloRay::LightSource)
