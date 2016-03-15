#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    once = false;
    ui->label_status->setStyleSheet("color: #FF0000");
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

    /*
    QByteArray startFrame;
    startFrame[0] = 0x01;
    startFrame[1] = 0x06;
    startFrame[2] = 0x21;
    startFrame[3] = 0x35;
    startFrame[4] = 0x00;
    startFrame[5] = 0x86;//86
    startFrame[6] = 0x12;
    startFrame[7] = 0x5A;
    serial->write(startFrame);
    */
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
        timer->start(2000);
        once = true;
    }
    ui->label_status->setStyleSheet("color: #00FF00");
    ui->label_status->setText("Start");

    serial->write(data);
}


int MainWindow::freqToHex1()
{
    double octetSpeed1 = ui->freqBox->value();
    octetSpeed1*=10;
    short octetSpeed1_short = octetSpeed1;
    int final;

    QString octetSpeed1_string = QString::number(octetSpeed1_short, 16);

    if (octetSpeed1_string.length() <= 2)
    {
        final = 0;
    }else if (octetSpeed1_string.length() == 3)
    {
        final = octetSpeed1_string.mid(0, 1).toInt(0, 16);
    }

    //qDebug() << "octet1 : " << final;

    return final;
}

int MainWindow::freqToHex2()
{
    double octetSpeed2 = ui->freqBox->value();
    octetSpeed2*=10;
    short octetSpeed2_short = octetSpeed2;
    int final;

    QString octetSpeed2_string = QString::number(octetSpeed2_short, 16);

    if (octetSpeed2_string.length() <= 2)
    {
        final = octetSpeed2_string.toInt(0, 16);
    }else if (octetSpeed2_string.length()  == 3)
    {
        final = octetSpeed2_string.mid(1, 2).toInt(0, 16);
    }

    //qDebug() << hex << "octet2 : " << final;

    return final;
}

void MainWindow::readData()
{
    QByteArray retrievedData = serial->readAll();
    qDebug() << hex << (int)retrievedData[4];
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
    QByteArray dataSpeed;
    dataSpeed[0] = 0x01;
    dataSpeed[1] = 0x06;
    dataSpeed[2] = 0x21;
    dataSpeed[3] = 0x36;
    dataSpeed[4] = freqToHex1();
    dataSpeed[5] = freqToHex2();

    unsigned short crc_short = calcCrc(dataSpeed, dataSpeed.length());

    QString crc = QString::number(crc_short, 16);
    if (crc.length() == 2)
    {
        crc = "00" + crc;
    }else if (crc.length() == 3)
    {
        crc = "0" + crc;
    }

    dataSpeed[6] = crc.mid(2, 2).toInt(0, 16);
    dataSpeed[7] = crc.mid(0, 2).toInt(0, 16);

    serial->write(dataSpeed);
}

void MainWindow::on_stopButton_clicked()
{
    QString value = ui->comboBox_stopType->currentText();
    QByteArray dataStop;
    dataStop[0] = 0x01;
    dataStop[1] = 0x06;
    dataStop[2] = 0x21;
    dataStop[3] = 0x35;
    if (value == "Arrêt roue libre")
    {
        dataStop[4] = 0x00;
        dataStop[5] = 0x07;
        dataStop[6] = 0xD2;
        dataStop[7] = 0x3A;
    }else{
        dataStop[4] = 0x10;
        dataStop[5] = 0x0F;
        dataStop[6] = 0xDE;
        dataStop[7] = 0x3C;
    }



    ui->label_status->setStyleSheet("color: #FF0000");
    ui->label_status->setText("Stop");
    serial->write(dataStop);
}

void MainWindow::on_razButton1_clicked()
{
    QByteArray dataReset;
    dataReset[0] = 0x01;
    dataReset[1] = 0x06;
    dataReset[2] = 0x21;
    dataReset[3] = 0x35;
    dataReset[4] = 0x00;
    dataReset[5] = 0x86;//86
    dataReset[6] = 0x12;
    dataReset[7] = 0x5A;
    serial->write(dataReset);
}
