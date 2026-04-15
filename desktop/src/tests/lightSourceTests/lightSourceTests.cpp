#include <QtTest>
#include "simulation/lightSource.h"

using namespace HaloRay;

class LightSourceTests : public QObject
{
    Q_OBJECT
private slots:
    void equality_givenSimilarLightSources_returnsTrue()
    {
        LightSource lightSource;
        LightSource otherLightSource;

        QVERIFY(lightSource == otherLightSource);
    }

    void equality_givenDifferentAltitude_returnsFalse()
    {
        LightSource lightSource;
        LightSource otherLightSource;

        otherLightSource.altitude = lightSource.altitude + 1.0f;

        QVERIFY(lightSource != otherLightSource);
    }

    void equality_givenDifferentDiameter_returnsFalse()
    {
        LightSource lightSource;
        LightSource otherLightSource;

        otherLightSource.diameter = lightSource.diameter + 1.0f;

        QVERIFY(lightSource != otherLightSource);
    }
};

QTEST_APPLESS_MAIN(LightSourceTests)
#include "lightSourceTests.moc"
