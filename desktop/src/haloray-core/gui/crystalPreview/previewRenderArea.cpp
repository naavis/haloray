#include "previewRenderArea.h"
#include <QPainter>
#include <QVector2D>
#include <QVector4D>
#include <QMatrix4x4>
#include <QSize>
#include <algorithm>
#include <cmath>
#include "../../simulation/trigonometryUtilities.h"

namespace HaloRay
{

PreviewRenderArea::PreviewRenderArea(CrystalModel *crystals, QWidget *parent)
    : QWidget(parent),
      m_crystals(crystals),
      m_populationIndex(0)
{
    setBackgroundRole(QPalette::Base);
    setAutoFillBackground(true);
    setSizePolicy(QSizePolicy::MinimumExpanding, QSizePolicy::MinimumExpanding);

    connect(m_crystals, &CrystalModel::dataChanged, this, [this]() {
        update();
    });
}

void PreviewRenderArea::onPopulationSelectionChange(int index)
{
    m_populationIndex = index;
    update();
}

QSize PreviewRenderArea::sizeHint() const
{
    return QSize(300, 300);
}

void PreviewRenderArea::paintEvent(QPaintEvent *)
{
    const int numVertices = 24;
    initializeGeometry(m_vertices);

    QMatrix4x4 perspectiveMat;
    perspectiveMat.perspective(90.0f, 1.0, 0.01f, 100.0f);

    QMatrix4x4 viewMat;
    viewMat.lookAt(QVector3D(5.0f, 5.0f, 5.0f), QVector3D(0.0f, 0.0f, 0.0f), QVector3D(0.0f, 1.0f, 0.0f));

    float largestDimension = getFurthestVertexDistance(m_vertices, numVertices);
    QMatrix4x4 orientationMat = getCrystalOrientationMatrix();
    QMatrix4x4 modelMat;
    modelMat.scale(1.0f / largestDimension);

    QVector4D mappedVertices[numVertices];
    for (int i = 0; i < numVertices; ++i)
    {
        mappedVertices[i] = perspectiveMat * viewMat * orientationMat * modelMat * QVector4D(m_vertices[i], 1.0f);
    }

    QPointF points[numVertices];
    std::transform(
                mappedVertices,
                mappedVertices + numVertices,
                points,
                [](QVector4D vertex) {
        return (vertex / vertex.w()).toPointF();
    });

    QPainter painter(this);
    painter.setRenderHint(QPainter::RenderHint::Antialiasing);
    painter.translate(width() / 2, height() / 2);
    int side = qMin(width(), height());
    painter.scale(side, -side);
    painter.scale(3.5, 3.5);

    QPen pen;
    pen.setColor(QColor(0, 0, 0));
    pen.setWidthF(0.5/side);
    painter.setPen(pen);

    painter.drawPolygon(points, 6);
    painter.drawPolygon(points + 6 , 6);
    painter.drawPolygon(points + 12, 6);
    painter.drawPolygon(points + 18, 6);

    for (int i = 0; i < 6; ++i)
    {
        QPolygonF edgePoints;
        edgePoints << points[i] << points [i + 6] << points[i + 12] << points[i + 18];
        painter.drawPolyline(edgePoints);
    }

    // Drawing the axis lines is helpful for debugging
    //drawAxisLines(perspectiveMat * viewMat, &painter);
}

void PreviewRenderArea::drawAxisLines(QMatrix4x4 viewMat, QPainter *painter)
{
    auto axisLength = 10.0f;
    auto origin = viewMat * QVector4D(0.0f, 0.0f, 0.0f, 1.0f);

    QPen pen;
    pen.setWidth(0);
    pen.setColor(QColor(255, 0, 0));
    painter->setPen(pen);
    auto xAxis = viewMat * QVector4D(axisLength, 0.0f, 0.0f, 1.0f);
    painter->drawLine((origin / origin.w()).toPoint(), (xAxis / xAxis.w()).toPointF());

    pen.setColor(QColor(0, 255, 0));
    painter->setPen(pen);
    auto yAxis = viewMat * QVector4D(0.0f, axisLength, 0.0f, 1.0f);
    painter->drawLine((origin / origin.w()).toPoint(), (yAxis / yAxis.w()).toPointF());

    pen.setColor(QColor(0, 0, 255));
    painter->setPen(pen);
    auto zAxis = viewMat * QVector4D(0.0f, 0.0f, axisLength, 1.0f);
    painter->drawLine((origin / origin.w()).toPoint(), (zAxis / zAxis.w()).toPointF());
}

int getPrevious(int i)
{
    return (((i - 1) % 6) + 6) % 6;
}

int getNext(int i)
{
    return (i + 1) % 6;
}

QVector2D rotate(double angle, double x, double y)
{
    return QVector2D(cos(angle) * x - sin(angle) * y, sin(angle) * x + cos(angle) * y);
}

float determinant3x3(QMatrix3x3 mat)
{
    auto a = mat.constData()[0];
    auto b = mat.constData()[1];
    auto c = mat.constData()[2];
    auto d = mat.constData()[3];
    auto e = mat.constData()[4];
    auto f = mat.constData()[5];
    auto g = mat.constData()[6];
    auto h = mat.constData()[7];
    auto i = mat.constData()[8];
    return a * (e * i - h * f) - b * (d * i - g * f) + c * (d * h - g * e);
}

void generateApexNormals(double apexAngle, QVector3D *apexNormals)
{
    auto halfApex = apexAngle / 2.0;
    QMatrix4x4 normalTiltMat;
    normalTiltMat.rotate(-halfApex * 180.0f / PI, 1.0f, 0.0f, 0.0f);
    for (auto i = 0; i < 6; ++i)
    {
        auto rotAngle = i * PI / 3.0;
        /* Initial normal vector is first tilted around the X axis
         * according to the apex angle and then rotated around the Y
         * axis in 60 degree increments. */
        QMatrix4x4 normalRotationMat;
        normalRotationMat.rotate(rotAngle * 180.0f / PI, 0.0f, 1.0f, 0.0f);
        apexNormals[i] = normalRotationMat * normalTiltMat * QVector3D(0.0f, 0.0f, 1.0f);
    }
}

float getMaximumApexHeight(QVector3D *normals, QVector3D *vertices, float *prismFaceDistances, float apexAngle, int vertexOffset)
{
    float maxApexHeight = std::numeric_limits<float>::max();
    // Intersect three adjacent pyramid faces to find maximum height of pyramid cap based on degenerating faces
    for (auto face = 0; face < 6; ++face)
    {
        auto prevFace = getPrevious(face);
        auto nextFace = getNext(face);

        auto nPrev = normals[prevFace];
        auto nCurr = normals[face];
        auto nNext = normals[nextFace];

        auto prevVert = vertices[prevFace + vertexOffset];
        auto nextVert = vertices[face + vertexOffset];

        auto numerator = QVector3D::dotProduct(prevVert, nPrev) * QVector3D::crossProduct(nCurr, nNext) +
                QVector3D::dotProduct(prevVert, nCurr) * QVector3D::crossProduct(nNext, nPrev) +
                QVector3D::dotProduct(nextVert, nNext) * QVector3D::crossProduct(nPrev, nCurr);
        float detMatrixContents[] = {
            nPrev.x(), nCurr.x(), nNext.x(),
            nPrev.y(), nCurr.y(), nNext.y(),
            nPrev.z(), nCurr.z(), nNext.z()
        };
        auto detMatrix = QMatrix3x3(detMatrixContents);
        auto denominator = determinant3x3(detMatrix);
        auto faceHeight = abs((numerator / denominator).y());
        maxApexHeight = fminf(faceHeight, maxApexHeight);
    }

    // Find maximum height of pyramid cap based on angle and face distances
    for (auto i = 0; i < 3; ++i)
    {
        auto dist = prismFaceDistances[i];
        auto distOpposite = prismFaceDistances[i + 3];
        auto h = (dist + distOpposite) / (2.0 * tan(apexAngle / 2.0));
        maxApexHeight = fminf(h, maxApexHeight);
    }
    return maxApexHeight;
}

void PreviewRenderArea::initializeGeometry(QVector3D *vertices)
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

    QVector2D hexagonCorners[6];

    // Calculate initial hexagon corners
    for (auto i = 0; i < 6; ++i)
    {
        auto d1 = prismFaceDistances[i];
        auto d2 = prismFaceDistances[getNext(i)];

        auto angle = -i * PI / 3.0;
        auto x_stat = 2.0 * d2 / sqrt(3.0) - d1 / sqrt(3.0);
        auto y_stat = d1;
        hexagonCorners[i] = rotate(angle, x_stat, y_stat);
    }

    // Fix denegerate prism faces
    for (auto face = 0; face < 6; ++face)
    {
        auto prevFace = getPrevious(face);
        auto nextFace = getNext(face);
        auto angle = -face * PI / 3.0;

        auto d1 = prismFaceDistances[face];
        auto d2 = prismFaceDistances[nextFace];
        auto d3 = prismFaceDistances[prevFace];
        if (d1 > d2 + d3)
        {
            auto x_stat = d2 / sqrt(3.0) - d3 / sqrt(3.0);
            auto y_stat = d2 + d3;
            auto rotatedPoint = rotate(angle, x_stat, y_stat);
            hexagonCorners[face] = rotatedPoint;
            hexagonCorners[prevFace] = rotatedPoint;
        }
    }

    /* Scaling value makes sure eventual A axis length is 2.0, so C/A ratio
     * can be easily corrected. */
    auto hexagonScaler = (hexagonCorners[1] - hexagonCorners[4]).length();
    for (auto i = 0; i < 6; ++i)
    {
        hexagonCorners[i] *= 2.0f / hexagonScaler;
    }

    for (auto face = 0; face < 6; ++face)
    {
        QVector2D *vertex = &hexagonCorners[face];
        // Corresponding vertices of each crystal layer have the same X and Z coordinates
        // First six vertices are the top apex cap
        // Next six vertices are the top of the base hexagonal crystal
        // Next six vertices are the bottom of the base hexagonal crystal
        // Last six vertices are the bottom apex cap
        vertices[face] = QVector3D(vertex->x(), 0.0f, vertex->y());
        vertices[face + 6] = QVector3D(vertex->x(), 0.0f, vertex->y());
        vertices[face + 12] = QVector3D(vertex->x(), 0.0f, vertex->y());
        vertices[face + 18] = QVector3D(vertex->x(), 0.0f, vertex->y());
    }

    if (upperApexHeightAverage > 0.0 && upperApexAngle < PI && upperApexAngle > 0.0)
    {
        // Generate normals for upper pyramid cap
        QVector3D upperApexNormals[6];
        generateApexNormals(upperApexAngle, upperApexNormals);

        // Set upper pyramid cap vertex positions
        float maxUpperApexHeight = getMaximumApexHeight(upperApexNormals, vertices, prismFaceDistances, upperApexAngle, 0);
        for (auto i = 0; i < 6; ++i)
        {
            auto next = getNext(i);
            auto pyramidEdge = QVector3D::crossProduct(upperApexNormals[i], upperApexNormals[next]);
            vertices[i] += upperApexHeightAverage * maxUpperApexHeight * pyramidEdge / pyramidEdge.y();
        }
    }

    if (lowerApexHeightAverage > 0.0 && lowerApexAngle < PI && lowerApexAngle > 0.0)
    {
        // Generate normals for lower pyramid cap
        QVector3D lowerApexNormals[6];
        generateApexNormals(lowerApexAngle, lowerApexNormals);
        for (auto i = 0; i < 6; ++i)
        {
            lowerApexNormals[i].setY(-lowerApexNormals[i].y());
        }

        // Set lower pyramid cap vertex positions
        float maxLowerApexHeight = getMaximumApexHeight(lowerApexNormals, vertices, prismFaceDistances, lowerApexAngle, 18);
        for (auto i = 0; i < 6; ++i)
        {
            auto next = getNext(i);
            auto pyramidEdge = QVector3D::crossProduct(lowerApexNormals[i], lowerApexNormals[next]);
            vertices[i + 18] -= lowerApexHeightAverage * maxLowerApexHeight * pyramidEdge / pyramidEdge.y();
        }
    }

    // Scale crystal vertically to have correct C/A ratio
    for (auto i = 0; i < 12; ++i)
    {
        vertices[i].setY(vertices[i].y() + caRatioAverage);
        vertices[i + 12].setY(vertices[i + 12].y() - caRatioAverage);
    }

    // Rotate crystal around C-axis so that face numbering follows conventions
    // Prism face 0 (Face 3 in the UI) should be up in a column Parry position
    QMatrix4x4 conventionMatrix;
    conventionMatrix.rotate(-90.0f, 0.0f, 1.0f, 0.0f);
    for (auto i = 0; i < 24; ++i)
    {
        vertices[i] = conventionMatrix * vertices[i];
    }
}

float PreviewRenderArea::getFurthestVertexDistance(QVector3D *vertices, int numVertices) const
{
    return std::max_element(
                vertices,
                vertices + numVertices,
                [](QVector3D a, QVector3D b)
    {
        return a.lengthSquared() < b.lengthSquared();
    })->length();
}

QMatrix4x4 PreviewRenderArea::getCrystalOrientationMatrix() const
{
    float tilt = getFromModel(m_populationIndex, CrystalModel::TiltAverage).toFloat();
    float rotation = getFromModel(m_populationIndex, CrystalModel::RotationAverage).toFloat();
    QMatrix4x4 orientationMatrix;
    // First rotate around Y and tent tile around Z
    orientationMatrix.rotate(-tilt, QVector3D(0.0f, 0.0f, 1.0f));
    orientationMatrix.rotate(rotation, QVector3D(0.0f, 1.0f, 0.0f));

    return orientationMatrix;
}

QVariant PreviewRenderArea::getFromModel(int row, CrystalModel::Columns column) const
{
    return m_crystals->data(m_crystals->index(row, column));
}

}
