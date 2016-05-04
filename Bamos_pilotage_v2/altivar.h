#ifndef ALTIVAR_H
#define ALTIVAR_H

#include <QtSerialPort/QSerialPort>
#include <QTimer>
#include <QDebug>
#include <QThread>
#include <QCloseEvent>
#include <QMessageBox>

class altivar
{

public:
    altivar(QString portName, QSerialPort::BaudRate baudrate, QSerialPort::DataBits databits, QSerialPort::Parity parity, QSerialPort::StopBits stopBits, QSerialPort::OpenMode openMode, QSerialPort *serial);
    virtual ~altivar();
    bool start();
    void stop(bool force);
    void reset();
    void setSpeed(double freqBox);

private:
    QSerialPort *localSerial;
    bool once;
    float oldValue;
    int freqToHex1(double octetSpeed1);
    int freqToHex2(double octetSpeed2);
    unsigned short calcCrc(QByteArray data);
};

#endif // ALTIVAR_H
