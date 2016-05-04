#include "altivar.h"

/**
 * @brief Constructeur de la classe altivar.
 * @details Permet de configurer les paramètres du port série.
 * @param portName Nom du port série utilisé pour l'altivar.
 * @param baudrate Baudrate utilisé par de port série de l'altivar.
 * @param databits Nombre de bits de données trnasférés par trame par le port série de l'altivar.
 * @param parity Paritée utilisée pour le port série de l'altivar.
 * @param stopBits Nombre de bits de stop utilisés pour le port série de l'altivar.
 * @param openMode Mode d'ouverture du port série de l'altivar.
 * @param serial Port série utilisé et utilisant les configurations précédement définies.
 */
altivar::altivar(QString portName, QSerialPort::BaudRate baudrate,
                 QSerialPort::DataBits databits, QSerialPort::Parity parity,
                 QSerialPort::StopBits stopBits, QSerialPort::OpenMode openMode,
                 QSerialPort *serial)
{
    localSerial = serial;
    once = false;
    oldValue = 0.0;
    localSerial->setPortName(portName);
    localSerial->setBaudRate(baudrate);
    localSerial->setDataBits(databits);
    localSerial->setParity(parity);
    localSerial->setStopBits(stopBits);
    localSerial->open(openMode);
}

/**
 * @brief Destructeur de la classe altivar.
 */
altivar::~altivar()
{
    delete localSerial;
}

/**
 * @brief Fonction permetant de démarrer le variateur de vitesse.
 * @details Envoie une trame de démarage au variateur, stoppe le timer de reset,
 * et lance le timer permetant de rafraichir la vitesse.
 * @return Renvoie true si le booléen "once" est égal à false, sinon renvoie false.
 * Cela permet d'empêcher l'envoie de trames de démarrage si le variateur de vitesse est déjà lancé.
 */
bool altivar::start()
{
    QByteArray data;
    data[0] = 0x01;
    data[1] = 0x06;
    data[2] = 0x21;
    data[3] = 0x35;
    data[4] = 0x00;
    data[5] = 0x0F;

    unsigned short crc_short = calcCrc(data);
    QString crc = QString::number(crc_short, 16);

    if (crc.length() == 2)
    {
        crc = "00" + crc;
    }else if (crc.length() == 3)
    {
        crc = "0" + crc;
    }

    data[6] = crc.mid(2, 2).toInt(0, 16);
    data[7] = crc.mid(0, 2).toInt(0, 16);

    localSerial->write(data);

    if (once == false)
    {
        once = true;
        return true;
    }else{
        return false;
    }


}


/**
 * @brief Algorithme permettant de calculer le CRC16 de chaque trame.
 * @details Il faut faire attention à bien inverser les octets, car la fonction renvoie
 * le permier l'octet de poids faible et en deuxième l'octet de poids fort.
 *
 * @param data Trame dont on veut calculer le CRC16.
 * @return CRC16 de la trame.
 */
unsigned short altivar::calcCrc(QByteArray data)
{
    unsigned short crc = 0xFFFF;
    int len = data.length();
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
 * @brief Converti l'octet de poid fort de decimal vers hexadecimal.
 *
 * @details Si cette convertion n'est pas faite, alors à partir de 10Hz sur l'IHM,
 * (valeur 100), l'octet de poid fort prend 1, et le variateur admet cette valeur comme haxedecimale,
 * le variateur passe donc à 25.6Hz au lieu de 10. (100 hexa  = 256 decimal).
 *
 * @param octetSpeed1 Valeur de la frequence du variateur de vitesse.
 * @return L'octet de poid fort converti en hexadecimal.
 */
int altivar::freqToHex1(double octetSpeed1)
{
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

    return final;
}

/**
 * @brief Converti l'octet de poid faible de décimal vers hexadecimal.
 *
 * @details Voir freqToHex1.
 *
 * @param octetSpeed2 Valeur de la frequence du variateur de vitesse.
 * @return L'octet de poid faible converti en héxadecimal.
 */
int altivar::freqToHex2(double octetSpeed2)
{
    //double octetSpeed2 = ui->freqBox->value();
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
 * @brief Permet l'arrêt de moteur.
 *
 * @details Envoie au variateur un trame d'arrêt. L'utilisateur peut choisir entre
 * un arrêt roue libre ou un arrêt "forcé".
 *
 * @param force booléen permettant de determiner quel mode d'arrêt sera utilisé.
 * Arrêt forcé si force est égal à true, sinon arrêt roue libre.
 */
void altivar::stop(bool force)
{
    unsigned short crc_short;
    QByteArray dataStop;
    dataStop[0] = 0x01;
    dataStop[1] = 0x06;
    dataStop[2] = 0x21;
    dataStop[3] = 0x35;

    if (force == false)
    {
        dataStop[4] = 0x00;
        dataStop[5] = 0x07;
        crc_short = calcCrc(dataStop);
        QString crc = QString::number(crc_short, 16);

        if (crc.length() == 2)
        {
            crc = "00" + crc;
        }else if (crc.length() == 3)
        {
            crc = "0" + crc;
        }

        dataStop[6] = crc.mid(2, 2).toInt(0, 16);
        dataStop[7] = crc.mid(0, 2).toInt(0, 16);

    }else{

        dataStop[4] = 0x10;
        dataStop[5] = 0x0F;
        crc_short = calcCrc(dataStop);
        QString crc = QString::number(crc_short, 16);

        if (crc.length() == 2)
        {
            crc = "00" + crc;
        }else if (crc.length() == 3)
        {
            crc = "0" + crc;
        }

        dataStop[6] = crc.mid(2, 2).toInt(0, 16);
        dataStop[7] = crc.mid(0, 2).toInt(0, 16);
    }

    localSerial->write(dataStop);
}

/**
 * @brief Fonction permettant d'envoyer les trames de vitesse (en Hz) au variateur de vitesse.
 *
 * @details On utilise les fonctions freqToHex1() et freqToHex2() pour calculer les octets 4 et 5
 * car ces derniers doivent pouvoir être modifiés à tout moment par l'utilisateur.
 * Il en va de même avec les octets 6 et 7, ces derniers representant le CRC16,
 * il doit être recalculé à chaque changement de valeur des octets 4 et 5.
 *
 * @param freqBox Valeur de la frequence du variateur de vitesse.
 */
void altivar::setSpeed(double freqBox)
{
    QByteArray dataSpeed;
    dataSpeed[0] = 0x01;
    dataSpeed[1] = 0x06;
    dataSpeed[2] = 0x21;
    dataSpeed[3] = 0x36;
    dataSpeed[4] = freqToHex1(freqBox);
    dataSpeed[5] = freqToHex2(freqBox);

    unsigned short crc_short = calcCrc(dataSpeed);

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

    localSerial->write(dataSpeed);
}

/**
 * @brief Envoie une trame permettant de relancer le variateur lorsque celui-ci ce trouve en
 * mode SLF.
 */
void altivar::reset()
{
    QByteArray dataReset;
    dataReset[0] = 0x01;
    dataReset[1] = 0x06;
    dataReset[2] = 0x21;
    dataReset[3] = 0x35;
    dataReset[4] = 0x00;
    dataReset[5] = 0x86;

    unsigned short crc_short = calcCrc(dataReset);
    QString crc = QString::number(crc_short, 16);

    if (crc.length() == 2)
    {
        crc = "00" + crc;
    }else if (crc.length() == 3)
    {
        crc = "0" + crc;
    }

    dataReset[6] = crc.mid(2, 2).toInt(0, 16);
    dataReset[7] = crc.mid(0, 2).toInt(0, 16);

    localSerial->write(dataReset);
}
