#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    serial = new QSerialPort(this);
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    serial->setPortName("COM1");
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::EvenParity);
    serial->setStopBits(QSerialPort::OneStop);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_disconnectButton_clicked()
{
    serial->close();
}

void MainWindow::on_startButton_clicked()
{
    QByteArray data;
    data[0] = 0x01;
    data[1] = 0x06;
    data[2] = 0x21;
    data[3] = 0x35;
    data[4] = 0x00;
    data[5] = 0x0F;
    data[6] = 0xD3;
    data[7] = 0xFC;

    if(serial->open(QIODevice::ReadWrite))
    {
        serial->write(data);
    }
    else
    {
        //error
        qDebug() << serial->errorString();
    }
}


void MainWindow::readData()
{
    QByteArray retrievedData = serial->readAll();
    qDebug() << retrievedData;
}

void MainWindow::on_pushButton_clicked()
{
    QByteArray data2;
    data2[0] = 0x01;
    data2[1] = 0x06;
    data2[2] = 0x21;
    data2[3] = 0x36;
    data2[4] = 0x00;
    data2[5] = 0x10;
    data2[6] = 0x62;
    data2[7] = 0x34;

    if(serial->open(QIODevice::ReadWrite))
    {
        serial->write(data2);
    }
    else
    {
        //error
        qDebug() << serial->errorString();
    }
}
