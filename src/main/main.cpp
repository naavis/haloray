#include <QtGlobal>
#include <QApplication>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QString>
#include <QObject>
#include <QMutex>
#include <QMutexLocker>
#include <QDateTime>
#include <QFile>
#include <QDir>
#include <QStandardPaths>
#include <stdexcept>
#include "gui/mainWindow.h"

#ifndef STRINGIFY0
#define STRINGIFY0(v) #v
#endif
#ifndef STRINGIFY
#define STRINGIFY(v) STRINGIFY0(v)
#endif

const int RequiredOpenGLMajorVersion = 4;
const int RequiredOpenGLMinorVersion = 4;

QFile logFile;
QMutex logFileMutex;

void openLogFile()
{
    const QString &tempFilePath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
    QDir tempFileDir(tempFilePath);
    if (tempFileDir.exists("haloray") || tempFileDir.mkdir("haloray"))
    {
        QString logFilePath = tempFileDir.absoluteFilePath("haloray/haloray.log");
        logFile.setFileName(logFilePath);
        logFile.open(QIODevice::WriteOnly | QIODevice::Text);
    }
}

void logHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QMutexLocker locker(&logFileMutex);

    QString message = qFormatLogMessage(type, context, msg) + "\n";

    fprintf(stderr, "%s", message.toLocal8Bit().constData());

    if (logFile.isOpen())
    {
        logFile.write(message.toUtf8().constData());
        logFile.flush();
    }
}

void initializeLogging()
{
    qSetMessagePattern("%{time} %{type}: %{message} (%{file}:%{line}, %{function})");
    openLogFile();
    qInstallMessageHandler(logHandler);
#ifdef HALORAY_VERSION
    qInfo("HaloRay version: %s", STRINGIFY(HALORAY_VERSION));
#else
    qInfo("HaloRay development build branch \"%s\", commit %s", STRINGIFY(GIT_BRANCH), STRINGIFY(GIT_COMMIT_HASH));
#endif
}

bool supportsOpenGL(int requiredMajorVersion, int requiredMinorVersion)
{
    qInfo("Checking for OpenGL compatibility");
    auto testContext = new QOpenGLContext();
    auto majorVersion = testContext->format().majorVersion();
    auto minorVersion = testContext->format().minorVersion();
    qInfo("OpenGL version: %i.%i", majorVersion, minorVersion);
    delete testContext;
    bool isCompatible = majorVersion > requiredMajorVersion ||
           (majorVersion == requiredMajorVersion && minorVersion >= requiredMinorVersion);

    if (!isCompatible)
    {
        qWarning("OpenGL %i.%i not supported", requiredMajorVersion, requiredMinorVersion);
    }

    return isCompatible;
}

void setDefaultSurfaceFormat()
{
    QSurfaceFormat format;
    format.setVersion(RequiredOpenGLMajorVersion, RequiredOpenGLMinorVersion);
    format.setProfile(QSurfaceFormat::OpenGLContextProfile::CoreProfile);
    format.setSwapBehavior(QSurfaceFormat::SwapBehavior::DoubleBuffer);
    format.setSwapInterval(1);
    QSurfaceFormat::setDefaultFormat(format);
}

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(haloray);
    initializeLogging();

    QGuiApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    setDefaultSurfaceFormat();
    QApplication app(argc, argv);

    if (!supportsOpenGL(RequiredOpenGLMajorVersion, RequiredOpenGLMinorVersion))
    {
        QString errorMessage = QString(QObject::tr("Your graphics processing unit does not support OpenGL %1.%2, which is required to run HaloRay."))
                                   .arg(RequiredOpenGLMajorVersion)
                                   .arg(RequiredOpenGLMinorVersion);
        QMessageBox::critical(nullptr, QObject::tr("Incompatible GPU"), errorMessage);
        return 1;
    }

    try {
        qInfo("Setting up OpenGL context");
        auto openGLContext = QOpenGLContext();
        openGLContext.setShareContext(QOpenGLContext::globalShareContext());
        openGLContext.create();
        qInfo("OpenGL context created");
        qInfo("Setting up offscreen drawing surface");
        auto surface = QOffscreenSurface(nullptr);
        surface.create();
        openGLContext.makeCurrent(&surface);
        qInfo("Offscreen drawing surface created");

        qInfo("Opening main window");
        HaloRay::MainWindow mainWindow;
        mainWindow.showMaximized();

        return app.exec();
    }
    catch (const std::exception &e)
    {
        QMessageBox::critical(nullptr, QObject::tr("Exception thrown"), QString("An error occurred:\n%1").arg(e.what()));
        qFatal("Caught exception: %s", e.what());
        return 1;
    }
}
