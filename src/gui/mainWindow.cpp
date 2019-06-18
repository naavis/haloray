#include "mainWindow.h"
#include <QMainWindow>
#include <QWidget>
#include "ui_mainWindow.h"

MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent), ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}
