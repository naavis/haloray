#include "previewRenderArea.h"
#include <QPainter>
#include <QVector2D>
#include <QVector3D>
#include <QVector4D>
#include <QMatrix4x4>
#include <algorithm>
#include "../../simulation/trigonometryUtilities.h"

namespace HaloRay
{

PreviewRenderArea::PreviewRenderArea(QWidget *parent) : QWidget(parent)
{
    // TODO: Set size policy and size hint
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
}

void PreviewRenderArea::paintEvent(QPaintEvent *)
{
    const int numVertices = 24;
    QVector3D vertices[numVertices];
    initializeGeometry(vertices, numVertices);

    QMatrix4x4 transformMat;
    transformMat.perspective(45.0f, 1.0f, 0.01f, 10000.0f);
    transformMat.scale(50.0f);
    transformMat.lookAt(QVector3D(2.0f, 2.0f, 3.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    QPen pen;
    pen.setWidth(3);
    painter.setPen(pen);

    painter.translate(width() / 2, height() / 2);
    int side = qMin(width(), height());
    painter.scale(side / 500.0, side / 500.0);

    QVector4D mappedVertices[numVertices];
    for (int i = 0; i < numVertices; ++i)
    {
        mappedVertices[i] = transformMat * vertices[i].toVector4D();
    }

    QPoint points[numVertices];
    std::transform(mappedVertices, mappedVertices + numVertices, points, [](QVector4D vertex) {
        return vertex.toPoint();
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
    float caRatioAverage = 0.5f;
    float upperApexAngle = degToRad(56.0f);
    float lowerApexAngle = degToRad(56.0f);
    // TODO: Upper and lower apexes seem to be the wrong way around
    // Maybe whole coordinate system is like that?
    float upperApexHeightAverage = 0.5f;
    float lowerApexHeightAverage = 0.5f;
    float prismFaceDistances[6];
    for (auto i = 0; i < 6; ++i)
    {
        prismFaceDistances[i] = 1.0f;
    }

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

}
