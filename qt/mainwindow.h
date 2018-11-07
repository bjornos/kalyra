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
    bool fwrt;
    bool fetchOnly;

private slots:
    void on_pushButton_clicked();

    void on_fetchOnly_clicked();

    void on_commandLinkButton_clicked();

    void on_pushButton_released();

    void on_checkBox_clicked();

private:
    Ui::MainWindow *ui;
    QString manifestPath;
};

#endif // MAINWINDOW_H
