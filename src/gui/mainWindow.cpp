#include "mainWindow.h"
#include <QMainWindow>
#include <QWidget>
#include <QPushButton>
#include "openGLWidget.h"
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    connect(ui->renderButton, &QPushButton::clicked, ui->openGLWidget, &OpenGLWidget::toggleRendering);
}
