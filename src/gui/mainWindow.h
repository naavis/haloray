#pragma once
#include <QMainWindow>
#include <QWidget>
#include "ui_mainWindow.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainWindow(QWidget *parent = 0);

private:
    Ui::MainWindow *ui;
};
