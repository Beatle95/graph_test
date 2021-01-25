#include "mainwindow.h"
#include "ui_mainwindow.h"

void appendPoint(float x, float y);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    plotter = QSharedPointer<Plotter>(new Plotter(5, 5, 780, 500, this));
}

MainWindow::~MainWindow()
{
    if(thread){
        if(thread->isRunning()){
            thread->quit();
            thread->wait();
        }
    }
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
    }
    thread = QSharedPointer<QThread>(new QThread);
    fileParser = QSharedPointer<FileParser>(new FileParser(plotter, fileName));
    fileParser->moveToThread(thread.data());
    connect(thread.data(), SIGNAL(started()), fileParser.data(), SLOT(slotRun()));
    connect(fileParser.data(), SIGNAL(sigProcessComplete(int, QString)), this, SLOT(slotDataReady(int, QString)));
    thread->start();
    threadRunning = true;
    ui->pushButton->setEnabled(false);
}

void MainWindow::slotDataReady(int result, QString header)
{
    threadRunning = false;
    switch(result){
        case FileParserState::SUCCESS:
            drawPlot();
            ui->textBrowser->setText(header);
            break;
        case FileParserState::HEADER_ERROR:
            QMessageBox::warning(this, "Error", "There is header in the middle of the data. File is incorrect.", QMessageBox::Ok);
            break;
        case FileParserState::ELEMENTS_COUNT_ERROR:
            QMessageBox::warning(this, "Error", "Wrong elements count in data. File is incorrect.", QMessageBox::Ok);
            break;
        case FileParserState::FILE_CANT_OPEN:
            QMessageBox::warning(this, "Error", "Can't open file.", QMessageBox::Ok);
            break;
    }
    ui->pushButton->setEnabled(true);
}

void MainWindow::drawPlot()
{
    plotter->drawView();
}
