#include "mainwindow.h"
#include "ui_mainwindow.h"

/**
 * @brief Constructeur de la classe MainWindow
 * @details Charge tout les reglages devant être initialisés au lancement de l'application.
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    once = false;
    oldValue = 0.0;
    ui->label_status->setStyleSheet("color: #FF0000");
    ui->freqBox->setDecimals(1);
    ui->freqBox->setSingleStep(0.1);
    ui->freqBox->setMinimum(0.0);
    ui->freqBox->setMaximum(50.0);
    serial = new QSerialPort(this);
    timer = new QTimer(this);
    timerReset = new QTimer(this);
    connect(serial, SIGNAL(readyRead()), this, SLOT(readData()));
    serial->setPortName("COM1");
    serial->setBaudRate(QSerialPort::Baud9600);
    serial->setDataBits(QSerialPort::Data8);
    serial->setParity(QSerialPort::EvenParity);
    serial->setStopBits(QSerialPort::OneStop);
    serial->open(QIODevice::ReadWrite);
    connect(timerReset, SIGNAL(timeout()), this, SLOT(on_razButton1_clicked()));
    timerReset->start(3000);
}

/**
 * @brief Destructeur de la classe MainWindow
 * @details On envoie une trame dans le destructeur afin de redefinir la valeur du variateur de vitesse à 0 et ainsi
 * éviter toute valeur résiduelle
 */
MainWindow::~MainWindow()
{
    QByteArray resetSpeed;
    resetSpeed[0] = 0x01;
    resetSpeed[1] = 0x06;
    resetSpeed[2] = 0x21;
    resetSpeed[3] = 0x36;
    resetSpeed[4] = 0x00;
    resetSpeed[5] = 0x00;
    resetSpeed[6] = 0x63;
    resetSpeed[7] = 0xF8;
    serial->write(resetSpeed);
    delete ui;
}

void MainWindow::on_disconnectButton_clicked()
{
    serial->close();
}

/**
 * @brief Boutton "Start", démarre le variateur de vitesse
 * @details envoie une trame de démarage au variateur, stoppe le timer de reset, et lance le timer permetant de rafraichir la vitesse
 */
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
        timerReset->stop();
        timer->connect(timer, SIGNAL(timeout()), this, SLOT(setSpeed()));
        timer->start(2000);
        once = true;
    }
    ui->label_status->setStyleSheet("color: #00FF00");
    ui->label_status->setText("Start");

    serial->write(data);
}

/**
 * @brief Converti l'octet de poid fort de decimal vers hexadecimal.
 * @details Si cette convertion n'est pas faite, alors à partir de 10Hz sur l'IHM, (valeur 100), l'octet de poid fort prend 1, et le variateur admet
 * cette valeur comme haxedecimale, le variateur passe donc à 25.6Hz au lieu de 10. (100 hexa  = 256 decimal)
 * @return L'octet de poid fort converti en hexadecimal.
 */
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

/**
 * @brief Converti l'octet de poid faible de décimal vers hexadecimal.
 * @details Voir Description octet poid fort.
 * @return L'octet de poid faible converti en héxadecimal.
 */
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

/**
 * @brief Slot permettant de lire les données du variateur de vitesse.
 * @details Peu important voire inutile: aucune donnée n'a besoin d'être récupérée, l'affichage se fait donc par un simple qDebug.
 */
void MainWindow::readData()
{
    QByteArray retrievedData = serial->readAll();
    qDebug() << hex << (int)retrievedData[4];
}

/**
 * @brief Algorithme permettant de calculer le CRC16 de chaque trame.
 * @details Il faut faire attention à bien inverser les octets, car la fonction renvoie le permier l'octet de poids faible et en deuxième
 * l'octet de poids fort.
 *
 * @param data Trame dont on veut calculer le CRC16.
 * @param len Longueur de la trame en question.
 *
 * @return CRC16 de la trame.
 */
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

/**
 * @brief Fonction permettant d'envoyer les trames de vitesse (en Hz) au variateur de vitesse.
 * @details On utilise les fonctions freqToHex1() et freqToHex2() pour calculer les octets 4 et 5 car ces derniers doivent
 * pouvoir être modifiés à tout moment par l'utilisateur. Il en va de même avec les octets 6 et 7, ces derniers representant
 * le CRC16, il doit être recalculé à chaque changement de valeur des octets 4 et 5.
 */
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

    //La fonction calcCrc omet les zéros de début de trame, il faut donc tester la longueur du crc et contatenner les zéros manquants si nécessaire.
    if (crc.length() == 2)
    {
        crc = "00" + crc;
    }else if (crc.length() == 3)
    {
        crc = "0" + crc;
    }

    //Inversion des octets
    dataSpeed[6] = crc.mid(2, 2).toInt(0, 16);
    dataSpeed[7] = crc.mid(0, 2).toInt(0, 16);

    serial->write(dataSpeed);
}

/**
 * @brief Permet l'arrêt de moteur.
 * @details Envoie au variateur un trame d'arrêt. L'utilisateur peut choisir entre un arrêt roue libre ou un arrêt "forcé".
 */
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

/**
 * @brief Trame permetant de relancer le variateur de vitesse lorsque celui-ci est en SLF.
 * @details Le boutton est ammené à disparaitre au profit de l'envoi d'une trame avec timer au lancement du programme.
 */
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
