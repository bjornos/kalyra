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
    this->setWindowTitle("Kalyra");

    manifest = "";

    optBuild = true;
    optFetchOnly = false;
    optClean = false;

    ui->optBuild->setChecked(true);
    ui->optFetchOnly->setChecked(false);
    ui->optClean->setChecked(false);


    ui->statusBar->showMessage("Ready.", 0);
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
    ui->statusBar->showMessage("Loaded manifest", 2000);

    QFileInfo mInfo(fileName);

    this->setWindowTitle("Kalyra - " + mInfo.fileName());
    manifestPath = mInfo.absolutePath();
    ui->txtCurrManifest->setText(mInfo.fileName());
}


void MainWindow::on_pushButton_clicked()
{
    loadFile();
}

void MainWindow::on_commandLinkButton_clicked()
{
    ui->textBrowser->setText("Working. This may take a while...");

    if (manifest == "") {
        ui->statusBar->showMessage("No manifest loaded",2000);
        QMessageBox::critical(NULL, QString(), tr("No manifest loaded.\n"));
        return;
    }
    QProcess process;
    QStringList argList;

    argList.append("-m");
    argList.append(manifest);

    if (optFetchOnly)
        argList.append("-f");

    if (optClean)
        argList.append("-c");

    process.setWorkingDirectory(manifestPath);

    process.start("ff", argList);

    ui->statusBar->showMessage("Running the Firmware Factory - This can take a few minutes...", 0);

    process.waitForFinished(-1);

    QString output(process.readAllStandardOutput());
    if (output == "") {
        ui->statusBar->showMessage("Could not find program ff", 5000);
        QMessageBox::critical(NULL, QString(), tr("Unable to find/run firmware factory\n"));
        return;
    }
    QString errOut(process.readAllStandardError());

    ui->textBrowser->setText(output + errOut);

    ui->statusBar->showMessage("Firmware Factory finished executing.", 10000);

    qDebug() << output;
    qDebug() << errOut;
}


void MainWindow::on_pushButton_released()
{

}

void MainWindow::on_optBuild_clicked()
{
    optBuild = true;
    optFetchOnly = false;
    optClean = false;

    ui->optBuild->setChecked(true);
    ui->optFetchOnly->setChecked(false);
    ui->optClean->setChecked(false);
}

void MainWindow::on_optFetchOnly_clicked()
{
    optBuild = true;
    optFetchOnly = true;
    optClean = false;

    ui->optBuild->setChecked(false);
    ui->optFetchOnly->setChecked(true);
    ui->optClean->setChecked(false);
}

void MainWindow::on_optClean_clicked()
{
    optBuild = false;
    optFetchOnly = false;
    optClean = true;

    ui->optBuild->setChecked(false);
    ui->optFetchOnly->setChecked(false);
    ui->optClean->setChecked(true);
}

