#include <QtTest/QtTest>
#include "simulation/camera.h"

Q_DECLARE_METATYPE(HaloRay::Projection)

using namespace HaloRay;

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

    void equality_givenCamerasWithSameMembers_returnsTrue()
    {
        Camera camera;
        Camera otherCamera;

        QVERIFY(camera == otherCamera);
    }

    void equality_givenDifferentFov_returnsFalse()
    {
        Camera camera;
        Camera otherCamera;
        otherCamera.fov = camera.fov + 1.0f;

        QVERIFY(camera != otherCamera);
    }

    void equality_givenDifferentYaw_returnsFalse()
    {
        Camera camera;
        Camera otherCamera;
        otherCamera.yaw = camera.yaw + 1.0f;

        QVERIFY(camera != otherCamera);
    }

    void equality_givenDifferentPitch_returnsFalse()
    {
        Camera camera;
        Camera otherCamera;
        otherCamera.pitch = camera.pitch + 1.0f;

        QVERIFY(camera != otherCamera);
    }

    void equality_givenDifferentProjection_returnsFalse()
    {
        Camera camera;
        Camera otherCamera;
        otherCamera.projection = Projection::EqualArea;

        QVERIFY(camera != otherCamera);
    }

    void equality_givenDifferentHideSubHorizonProperty_returnsFalse()
    {
        Camera camera;
        Camera otherCamera;
        otherCamera.hideSubHorizon = !camera.hideSubHorizon;

        QVERIFY(camera != otherCamera);
    }
};

QTEST_MAIN(CameraTests)
#include "cameraTests.moc"
