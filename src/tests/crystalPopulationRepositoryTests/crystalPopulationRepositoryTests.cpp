#include <QtTest/QtTest>
#include "../haloray-core/simulation/crystalPopulationRepository.h"

class CrystalPopulationRepositoryTests : public QObject
{
    Q_OBJECT
private slots:
    void hasThreePopulationsByDefault()
    {
        auto repository = HaloRay::CrystalPopulationRepository();
        QCOMPARE(repository.getCount(), 3);
    }

    void addingPopulationIncreasesCount()
    {
        auto repository = HaloRay::CrystalPopulationRepository();
        auto originalCount = repository.getCount();
        repository.add(HaloRay::CrystalPopulationPreset::Random);
        QCOMPARE(repository.getCount(), originalCount + 1);
    }

    void removingPopulationDecreasesCount()
    {
        auto repository = HaloRay::CrystalPopulationRepository();
        auto originalCount = repository.getCount();
        repository.add(HaloRay::CrystalPopulationPreset::Random);
        repository.remove(0);
        QCOMPARE(repository.getCount(), originalCount);
    }

    void evenProbabilitiesByDefault()
    {
        auto repository = HaloRay::CrystalPopulationRepository();
        QCOMPARE(repository.getProbability(0), 1.0 / 3.0);
        QCOMPARE(repository.getProbability(1), 1.0 / 3.0);
        QCOMPARE(repository.getProbability(2), 1.0 / 3.0);
    }

    void evenWeightsByDefault()
    {
        auto repository = HaloRay::CrystalPopulationRepository();
        QCOMPARE(repository.getWeight(0), 1);
        QCOMPARE(repository.getWeight(1), 1);
        QCOMPARE(repository.getWeight(2), 1);
    }
};

QTEST_MAIN(CrystalPopulationRepositoryTests)
#include "crystalPopulationRepositoryTests.moc"
