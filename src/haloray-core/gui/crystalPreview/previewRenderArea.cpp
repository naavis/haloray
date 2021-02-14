#include "previewRenderArea.h"
#include <QPainter>
#include <QVector2D>
#include <QVector4D>
#include <QMatrix4x4>
#include <algorithm>
#include "../crystalModel.h"
#include "../../simulation/trigonometryUtilities.h"

namespace HaloRay
{

PreviewRenderArea::PreviewRenderArea(CrystalModel *crystals, QWidget *parent)
    : QWidget(parent),
      m_crystals(crystals),
      m_populationIndex(0)
{
    // TODO: Set size policy and size hint
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);

    connect(m_crystals, &CrystalModel::dataChanged, this, [this]() {
        update();
    });
}

void PreviewRenderArea::onPopulationSelectionChange(int index)
{
    m_populationIndex = index;
    update();
}

void PreviewRenderArea::paintEvent(QPaintEvent *)
{
    const int numVertices = 24;
    initializeGeometry(m_vertices, numVertices);
    float largestDimension = qMax(1.0f, getFromModel(m_populationIndex, CrystalModel::CaRatioAverage).toFloat());

    QMatrix4x4 transformMat;
    transformMat.scale(500.0f);
    transformMat.perspective(90.0f, 1.0, 0.01f, 5.0f);
    transformMat.lookAt(QVector3D(-2.0f, 2.0f, 2.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));
    transformMat.scale(1.0f / largestDimension);

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    QPen pen;
    pen.setColor(QColor(0, 0, 0));
    pen.setWidth(3);
    painter.setPen(pen);

    painter.rotate(180.0);
    painter.translate(-width() / 2, -height() / 2);
    int side = qMin(width(), height());
    painter.scale(side / 500.0f, side / 500.0f);

    QMatrix4x4 orientationMatrix = getCrystalOrientationMatrix();

    QVector4D mappedVertices[numVertices];
    for (int i = 0; i < numVertices; ++i)
    {
        mappedVertices[i] = transformMat * orientationMatrix * QVector4D(m_vertices[i], 1.0f);
    }

    QPoint points[numVertices];
    std::transform(
                mappedVertices,
                mappedVertices + numVertices,
                points,
                [](QVector4D vertex) {
        return (vertex / vertex.w()).toPoint();
    });

    painter.drawPolygon(points, 6);
    painter.drawPolygon(points + 6 , 6);
    painter.drawPolygon(points + 12, 6);
    painter.drawPolygon(points + 18, 6);

    for (int i = 0; i < 6; ++i)
    {
        QPolygon edgePoints;
        edgePoints << points[i] << points [i + 6] << points[i + 12] << points[i + 18];
        painter.drawPolyline(edgePoints);
    }
}

QVector2D lineIntersect(QVector2D line1, QVector2D line2)
{
    float p1 = line1.x();
    float theta1 = line1.y();

    float p2 = line2.x();
    float theta2 = line2.y();

    float deltaSine = sin(theta2 - theta1);
    float x = (p1 * sin(theta2) - p2 * sin(theta1)) / deltaSine;
    float y = (p2 * cos(theta1) - p1 * cos(theta2)) / deltaSine;

    return QVector2D(x, y);
}

void PreviewRenderArea::initializeGeometry(QVector3D *vertices, int numVertices)
{
    float caRatioAverage = getFromModel(m_populationIndex, CrystalModel::CaRatioAverage).toFloat();
    float upperApexAngle = degToRad(getFromModel(m_populationIndex, CrystalModel::UpperApexAngle).toFloat());
    float lowerApexAngle = degToRad(getFromModel(m_populationIndex, CrystalModel::LowerApexAngle).toFloat());
    float upperApexHeightAverage = getFromModel(m_populationIndex, CrystalModel::UpperApexHeightAverage).toFloat();
    float lowerApexHeightAverage = getFromModel(m_populationIndex, CrystalModel::LowerApexHeightAverage).toFloat();
    float prismFaceDistances[6] = {
        getFromModel(m_populationIndex, CrystalModel::PrismFaceDistance1).toFloat(),
        getFromModel(m_populationIndex, CrystalModel::PrismFaceDistance2).toFloat(),
        getFromModel(m_populationIndex, CrystalModel::PrismFaceDistance3).toFloat(),
        getFromModel(m_populationIndex, CrystalModel::PrismFaceDistance4).toFloat(),
        getFromModel(m_populationIndex, CrystalModel::PrismFaceDistance5).toFloat(),
        getFromModel(m_populationIndex, CrystalModel::PrismFaceDistance6).toFloat(),
    };

    float deltaAngle = degToRad(60.0f);
    for (int face = 0; face < 6; face = face + 2)
    {
        int previousFace = face == 0 ? 5 : face - 1;
        int nextFace = face + 1;

        float previousAngle = (face + 1) * deltaAngle;
        float currentAngle = previousAngle + deltaAngle;
        float nextAngle = previousAngle + 2.0 * deltaAngle;

        /* The sqrt(3)/2 multiplier makes the default crystal such
           that the distance of a vertex from the C axis is 1.0 */
        float previousDistance = 0.86602540378443864676372317075294 * prismFaceDistances[previousFace];
        float currentDistance = 0.86602540378443864676372317075294 * prismFaceDistances[face];
        float nextDistance = 0.86602540378443864676372317075294 * prismFaceDistances[nextFace];

        QVector2D previousLine = QVector2D(previousDistance, previousAngle);
        QVector2D currentLine = QVector2D(currentDistance, currentAngle);
        QVector2D nextLine = QVector2D(nextDistance, nextAngle);

        QVector2D previousCurrentIntersection = lineIntersect(previousLine, currentLine);
        QVector2D currentNextIntersection = lineIntersect(currentLine, nextLine);
        QVector2D previousNextIntersection = lineIntersect(previousLine, nextLine);

        float previousCurrentIntersectionDistance = previousCurrentIntersection.length();
        float currentNextIntersectionDistance = currentNextIntersection.length();
        float previousNextIntersectionDistance = previousNextIntersection.length();

        QVector2D v1 = previousCurrentIntersectionDistance < previousNextIntersectionDistance ? previousCurrentIntersection : previousNextIntersection;
        QVector2D v2 = currentNextIntersectionDistance < previousNextIntersectionDistance ? currentNextIntersection : previousNextIntersection;

        // Corresponding vertices of each crystal layer have the same X and Z coordinates
        vertices[face] = QVector3D(v1.x(), 1.0f, v1.y());
        vertices[face + 1] = QVector3D(v2.x(), 1.0f, v2.y());

        vertices[face + 6] = QVector3D(v1.x(), 1.0f, v1.y());
        vertices[face + 6 + 1] = QVector3D(v2.x(), 1.0f, v2.y());

        vertices[face + 12] = QVector3D(v1.x(), -1.0f, v1.y());
        vertices[face + 12 + 1] = QVector3D(v2.x(), -1.0f, v2.y());

        vertices[face + 18] = QVector3D(v1.x(), -1.0f, v1.y());
        vertices[face + 18 + 1] = QVector3D(v2.x(), -1.0f, v2.y());
    }

    // Stretch the crystal to correct C/A ratio
    float caMultiplier = caRatioAverage;
    for (int i = 0; i < numVertices; ++i)
    {
        vertices[i].setY(vertices[i].y() * caMultiplier);
    }

    // Scale pyramid caps
    float upperApexMaxHeight = 1.0 / tan(upperApexAngle / 2.0);
    float lowerApexMaxHeight = 1.0 / tan(lowerApexAngle / 2.0);

    float upperApexHeight = upperApexHeightAverage;
    float lowerApexHeight = lowerApexHeightAverage;

    for (int i = 0; i < 6; ++i)
    {
        vertices[i] = QVector3D(
                    vertices[i].x() * (1.0 - upperApexHeight),
                    vertices[i].y() + upperApexHeight * upperApexMaxHeight,
                    vertices[i].z() * (1.0 - upperApexHeight));
        vertices[numVertices - i - 1] = QVector3D(
                    vertices[numVertices - i - 1].x() * (1.0 - lowerApexHeight),
                    vertices[numVertices - i - 1].y() - lowerApexHeight * lowerApexMaxHeight,
                    vertices[numVertices - i - 1].z() * (1.0 - lowerApexHeight));
    }
}

QMatrix4x4 PreviewRenderArea::getCrystalOrientationMatrix() const
{
    float tilt = getFromModel(m_populationIndex, CrystalModel::TiltAverage).toFloat();
    float rotation = getFromModel(m_populationIndex, CrystalModel::RotationAverage).toFloat();
    QMatrix4x4 orientationMatrix;
    orientationMatrix.rotate(tilt, QVector3D(0.0f, 0.0f, 1.0f));
    orientationMatrix.rotate(rotation, QVector3D(0.0f, 1.0f, 0.0f));
    return orientationMatrix;
}

QVariant PreviewRenderArea::getFromModel(int row, CrystalModel::Columns column) const
{
    return m_crystals->data(m_crystals->index(row, column));
}

}
