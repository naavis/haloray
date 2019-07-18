#include <QtTest/QtTest>
#include "../src/simulation/camera.h"

Q_DECLARE_METATYPE(HaloSim::Projection)

using namespace HaloSim;

class CameraTests : public QObject
{
    Q_OBJECT
private slots:
    void maxFov()
    {
        QFETCH(Projection, projection);
        QFETCH(float, expectedMaxFov);

        auto c = Camera();
        c.projection = projection;
        QCOMPARE(c.getMaximumFov(), expectedMaxFov);
    }
    void maxFov_data()
    {
        QTest::addColumn<Projection>("projection");
        QTest::addColumn<float>("expectedMaxFov");

        QTest::newRow("stereographic") << Projection::Stereographic << 350.0f;
        QTest::newRow("rectilinear") << Projection::Rectilinear << 160.0f;
        QTest::newRow("equidistant") << Projection::Equidistant << 360.0f;
        QTest::newRow("equal area") << Projection::EqualArea << 360.0f;
        QTest::newRow("orthographic") << Projection::Orthographic << 180.0f;
    }
};

QTEST_MAIN(CameraTests)
#include "cameraTests.moc"
