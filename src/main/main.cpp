#include <QtGlobal>
#include <QApplication>
#include <QSurfaceFormat>
#include <QMessageBox>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <QString>
#include <QObject>
#include "gui/mainWindow.h"

const int RequiredOpenGLMajorVersion = 4;
const int RequiredOpenGLMinorVersion = 4;

void logHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg)
{
    QByteArray localMsg = msg.toLocal8Bit();
    const char *file = context.file ? context.file : "";
    const char *function = context.function ? context.function : "";
    switch (type)
    {
    case QtDebugMsg:
        fprintf(stderr, "Debug: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtInfoMsg:
        fprintf(stderr, "Info: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtWarningMsg:
        fprintf(stderr, "Warning: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtCriticalMsg:
        fprintf(stderr, "Critical: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    case QtFatalMsg:
        fprintf(stderr, "Fatal: %s (%s:%u, %s)\n", localMsg.constData(), file, context.line, function);
        break;
    }
}

bool supportsOpenGL(int requiredMajorVersion, int requiredMinorVersion)
{
    auto testContext = new QOpenGLContext();
    auto majorVersion = testContext->format().majorVersion();
    auto minorVersion = testContext->format().minorVersion();
    delete testContext;
    return majorVersion > requiredMajorVersion ||
           (majorVersion == requiredMajorVersion && minorVersion >= requiredMinorVersion);
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
    QGuiApplication::setAttribute(Qt::AA_UseDesktopOpenGL);
    QGuiApplication::setAttribute(Qt::AA_ShareOpenGLContexts);
    QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
    setDefaultSurfaceFormat();
    qInstallMessageHandler(logHandler);
    QApplication app(argc, argv);

    if (!supportsOpenGL(RequiredOpenGLMajorVersion, RequiredOpenGLMinorVersion))
    {
        QString errorMessage = QString(QObject::tr("Your graphics processing unit does not support OpenGL %1.%2, which is required to run HaloRay."))
                                   .arg(RequiredOpenGLMajorVersion)
                                   .arg(RequiredOpenGLMinorVersion);
        QMessageBox::critical(nullptr, QObject::tr("Incompatible GPU"), errorMessage);
        return 1;
    }

    auto openGLContext = QOpenGLContext();
    openGLContext.setShareContext(QOpenGLContext::globalShareContext());
    openGLContext.create();
    auto surface = QOffscreenSurface(nullptr);
    surface.create();
    openGLContext.makeCurrent(&surface);

    HaloRay::MainWindow mainWindow;
    mainWindow.showMaximized();


    return app.exec();
}
