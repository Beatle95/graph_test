#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QThread>
#include <QFileDialog>
#include <QMessageBox>

#include "fileparser.h"
#include "plotter.h"

#include <QDebug>

class FileParser;

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    FileParser* fileParser = nullptr;
    QThread* thread = nullptr;
    Plotter* plotter = nullptr;
    bool threadRunning = false;

    void drawPlot();

public slots:
    void slotDataReady(int result, QString header);

private slots:
    void on_pushButton_clicked();
};
#endif // MAINWINDOW_H
