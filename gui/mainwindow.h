#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

    void loadFile();

    QString manifest;
    bool optFetchOnly;
    bool optClean;
    bool optBuild;

private slots:
    void on_pushButton_clicked();

    void on_optFetchOnly_clicked();
    void on_optClean_clicked();
    void on_optBuild_clicked();

    void on_commandLinkButton_clicked();

    void on_pushButton_released();

private:
    Ui::MainWindow *ui;
    QString manifestPath;
};

#endif // MAINWINDOW_H
