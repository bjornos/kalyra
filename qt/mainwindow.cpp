#include "mainwindow.h"
#include "ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QFileInfo>
#include <QProcess>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    fwrt = false;
    fetchOnly = false;
    manifest = "";
    ui->statusBar->showMessage("Ready.",0);
    this->setWindowTitle("Kalyra");


}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::loadFile() {
    QString fileName = QFileDialog::getOpenFileName(this,
         "Open build manifest", "",
         "Kalyra Manifest File (*.manifest);;All Files (*)");

    if (fileName.isEmpty())
          return;
      else {
          QFile file(fileName);
          if (!file.open(QIODevice::ReadOnly)) {
              QMessageBox::information(this, "Unable to open file",
                  file.errorString());
              return;
          }
      }

     manifest = fileName;

    QFileInfo info1(fileName);

    ui->statusBar->showMessage("Loaded manifest",2000);
    this->setWindowTitle("Kalyra - " + info1.fileName());

    manifestPath = info1.absolutePath();
    ui->txtCurrManifest->setText(info1.fileName());



}


void MainWindow::on_pushButton_clicked()
{
    loadFile();
}

void MainWindow::on_commandLinkButton_clicked()
{
    if (manifest == "")
        ui->statusBar->showMessage("No manifest loaded",2000);
    else {
        QProcess process;
        QStringList com;

        com.append("-g");
        com.append("-m");
        com.append(manifest);

        //if (fetchOnly)
          //  com.append("-f");

        process.setWorkingDirectory(manifestPath);

        process.start("ff", com);



        process.waitForFinished(-1);

        QString output(process.readAllStandardOutput());
QString stderr(process.readAllStandardError());

ui->textBrowser->setText(output +  stderr);//   ->setText(output + stderr);



        qDebug() << output;
            qDebug() << stderr;
    }
}


void MainWindow::on_pushButton_released()
{
    //ui->label->setText(("Clicled"));
}

void MainWindow::on_checkBox_clicked()
{
    fwrt = true;
}

void MainWindow::on_fetchOnly_clicked()
{
    fetchOnly = true;
}
