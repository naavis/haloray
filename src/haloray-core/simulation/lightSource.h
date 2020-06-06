#pragma once
#include <QObject>

namespace HaloRay
{

struct LightSource
{
    float altitude;
    float diameter;

    static LightSource createDefaultLightSource();
};

}

Q_DECLARE_METATYPE(HaloRay::LightSource)
