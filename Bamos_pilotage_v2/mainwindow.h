#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "altivar.h"
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QThread>
#include <QCloseEvent>
#include <QMessageBox>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:
    void on_startButton_clicked();
    
    void on_stopButton_clicked();

    void closeEvent(QCloseEvent *event);

    void reset();

    void setSpeed();
    
private:
    Ui::MainWindow *ui;
    altivar *altivar1;
    QSerialPort *serial;
    QTimer *timer;
    QTimer *timerReset;

};

#endif // MAINWINDOW_H
