#include "mainwindow.h"
#include "ui_mainwindow.h"

void appendPoint(float x, float y);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    plotter = new Plotter(5, 5, 780, 500, this);
}

MainWindow::~MainWindow()
{
    if(thread){
        if(thread->isRunning()){
            thread->quit();
            thread->wait();
        }
    }
    if(fileParser)
        delete fileParser;
    if(thread)
        delete thread;
    delete ui;
}

void MainWindow::on_pushButton_clicked()
{
    if(threadRunning){
        QMessageBox::warning(this, "Error", "Wait until process is completed.", QMessageBox::Ok);
        return;
    }
    QString fileName = QFileDialog::getOpenFileName(this, tr("Open File"), "C:/", tr("*.*"));
    if(fileName.isEmpty()){
        return;
    }
    if(thread){
        thread->quit();
        thread->wait();
        delete thread;
        delete fileParser;
    }
    thread = new QThread;
    fileParser = new FileParser(plotter, fileName);
    fileParser->moveToThread(thread);
    connect(thread, SIGNAL(started()), fileParser, SLOT(slotRun()));
    connect(fileParser, SIGNAL(sigProcessComplete(int, QString)), this, SLOT(slotDataReady(int, QString)));
    thread->start();
    threadRunning = true;
    ui->pushButton->setEnabled(false);
}

void MainWindow::slotDataReady(int result, QString header)
{
    threadRunning = false;
    switch(result){
        case 0:
            drawPlot();
            ui->textBrowser->setText(header);
            break;
        case -1:
            QMessageBox::warning(this, "Error", "There is header in the middle of the data. File is incorrect.", QMessageBox::Ok);
            break;
        case -2:
            QMessageBox::warning(this, "Error", "Wrong elements count in data. File is incorrect.", QMessageBox::Ok);
            break;
        case -3:
            QMessageBox::warning(this, "Error", "Can't convert data to float.", QMessageBox::Ok);
            break;
        case -4:
            QMessageBox::warning(this, "Error", "Can't open file.", QMessageBox::Ok);
            break;
    }
    ui->pushButton->setEnabled(true);
}

void MainWindow::drawPlot()
{
    plotter->drawView();
}
