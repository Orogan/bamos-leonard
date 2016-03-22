#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QDebug>
#include <QtSerialPort/QSerialPort>
#include <QTimer>

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

    void on_disconnectButton_clicked();

    void on_connectButton_clicked();

    void readData();

    void setSpeed();

    int freqToHex1();

    int freqToHex2();

    unsigned short calcCrc(QByteArray data, int len);

    void on_stopButton_clicked();

    void on_razButton1_clicked();

private:
    Ui::MainWindow *ui;
    QSerialPort *serial;
    QTimer *timer;
    QTimer *timerReset;
    bool once;
    float oldValue;
};

#endif // MAINWINDOW_H
