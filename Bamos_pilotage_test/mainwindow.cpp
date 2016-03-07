#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    once = false;
    ui->freqBox->setDecimals(1);
    ui->freqBox->setSingleStep(0.1);
    ui->freqBox->setMinimum(0.0);
    ui->freqBox->setMaximum(50.0);
    serial = new QSerialPort(this);
    timer = new QTimer(this);
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));            
    serial->setPortName("COM1");
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::EvenParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->open(QIODevice::ReadWrite);
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::on_disconnectButton_clicked()
{
    serial->close();
}

void MainWindow::on_connectButton_clicked()
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

    if (once == false)
    {
        timer->connect(timer, SIGNAL(timeout()), this, SLOT(setSpeed()));
        timer->start(3000);
        once = true;
    }

    serial->write(data);
}


int MainWindow::freqToHex1()
{
    double test = ui->freqBox->value();
    test*=10;
    short test3 = test;
    int final;

    QString test2=QString::number(test3, 16);

    if (test2.length() <= 2)
    {
        final = 0;
    }else if (test2.length() == 3)
    {
        final = test2.mid(0, 1).toInt(0, 16);
    }

    qDebug() << "octet1 : " << final;

    return final;
}



int MainWindow::freqToHex2()
{
    double test = ui->freqBox->value();
    test*=10;
    short test3 = test;
    int final;

    QString test2=QString::number(test3, 16);

    if (test2.length() <= 2)
    {
        final = test2.toInt(0, 16);
    }else if (test2.length()  == 3)
    {
        final = test2.mid(1, 2).toInt(0, 16);
    }

    qDebug() << hex << "octet2 : " << final;

    return final;
}

void MainWindow::readData()
{
    QByteArray retrievedData = serial->readAll();
    qDebug() << retrievedData;
}

unsigned short MainWindow::calcCrc(QByteArray data, int len)
{
    unsigned short crc = 0xFFFF;
    int nbOctets = 0;
    int count = 0;
    unsigned char currentOctet;

    while (nbOctets < len)
    {
        currentOctet = data[count];
        count++;
        crc = crc ^ currentOctet;
        for (int i = 0; i < 8; i++)
        {
            if (crc % 2 == 1)
            {
                crc = (crc/2) ^ (0xA001);
            }else{
                crc/=2;
            }
        }
        nbOctets++;
    }
    //qDebug() << crc;
    return crc;
}

void MainWindow::setSpeed()
{
    QByteArray data2;
    data2[0] = 0x01;
    data2[1] = 0x06;
    data2[2] = 0x21;
    data2[3] = 0x36;
    data2[4] = freqToHex1();
    data2[5] = freqToHex2();
    //data2[6] = 0x62;
    //data2[7] = 0x34;

    unsigned short test = calcCrc(data2, data2.length());
    qDebug() << "test : " << hex << test;

    QString crc = QString::number(test, 16);
    if (crc.length() == 2)
    {
        crc = "00" + crc;
    }else if (crc.length() == 3)
    {
        crc = "0" + crc;
    }

    data2[6] = crc.mid(2, 2).toInt(0, 16);
    data2[7] = crc.mid(0, 2).toInt(0, 16);

    qDebug() << "crc : " << crc;

    serial->write(data2);
}

void MainWindow::on_stopButton_clicked()
{
    QByteArray data4;
    data4[0] = 0x01;
    data4[1] = 0x06;
    data4[2] = 0x21;
    data4[3] = 0x35;
    data4[4] = 0x00;
    data4[5] = 0x07;
    data4[6] = 0xD2;
    data4[7] = 0x3A;

    serial->write(data4);
}
