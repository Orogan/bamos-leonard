#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "altivar.h"

/**
 * @brief MainWindow::MainWindow
 * @param parent
 */
MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    serial = new QSerialPort(this);
    timer = new QTimer(this);
    timerReset = new QTimer(this);
    ui->setupUi(this);
    ui->freqBox->setDecimals(1);
    ui->freqBox->setSingleStep(0.1);
    ui->freqBox->setMinimum(0.0);
    ui->freqBox->setMaximum(50.0);
    altivar1 = new altivar("COM1", QSerialPort::Baud9600, QSerialPort::Data8,
                                  QSerialPort::EvenParity, QSerialPort::OneStop,
                                  QIODevice::ReadWrite, serial);
    connect(timerReset, SIGNAL(timeout()), this, SLOT(reset()));
    timerReset->start(500);
}

/**
 * @brief Destructeur de la classe MainWindow
 */
MainWindow::~MainWindow()
{
    delete altivar1;
    delete ui;
}

/**
 * @brief Fonction associée au boutton "Start", permet de démarrer le variateur.
 * @details Appelle la méthode start de la classe altivar, stoppe le timer de reset, et lance le timer permetant de rafraichir la vitesse
 */
void MainWindow::on_startButton_clicked()
{
    if (altivar1->start() == true)
    {
        timerReset->stop();
        timer->connect(timer, SIGNAL(timeout()), this, SLOT(setSpeed()));
        timer->start(2000);
    }
}

/**
 * @brief Fonction associée au boutton "Stop", permet l'arrêt de moteur.
 * @details Appelle la méthode stop de la classe altivar. L'utilisateur peut choisir entre un arrêt roue libre ou un arrêt "forcé" (representé par le booléen "force" en parametre de la méthode stop).
 */
void MainWindow::on_stopButton_clicked()
{
    if (ui->comboBox->currentText() == "Arrêt roue libre")
    {
        altivar1->stop(false);
    }else {
        altivar1->stop(true);
    }
}

/**
 * @brief Permet de vérifier que la fréquence est à 0 à la fermeture de l'application.
 * @details On vérifie que la fréquence est bien à 0 lors de la fermeture de l'application afin d'éviter toute valeur résiduelle
 * lors du prochain lancement. On empêche l'utilisateur de fermer l'application grace à la méthode "ignore()" et on l'informe grace à
 * une QMessageBox.
 *
 * @param event Correspond à l'evenement "appuyer sur la croix"
 */
void MainWindow::closeEvent (QCloseEvent *event)
{
    if (ui->freqBox->value() > 0.0)
    {
        QMessageBox msgBox;
        msgBox.setIcon(QMessageBox::Warning);
        msgBox.setText("Veuillez redefinir la fréquence à 0 avant de quitter l'application.");
        msgBox.exec();

        if (msgBox.Ok)
        {
            event->ignore();
        }
    }
}

/**
 * @brief Fonction appellée par le timer "timerReset", permet de relancer le variateur lorsque celui-ci ce trouve en mode SLF.
 * @details Appelle la méthode reset de la classe altivar.
 */
void MainWindow::reset()
{
    altivar1->reset();
}

/**
 * @brief Fonction appellée par le timer "timerSpeed", permet de rafraichir la valeur de vitesse afin que celle-ci puisse être changée à la volée.
 * @details  Appelle la méthode setSpeed de la classe altivar. On lui passe en parametre la valeur de vitesse, ici stockée dans la DoubleSpinBox de l'IHM.
*/
void MainWindow::setSpeed()
{
    altivar1->setSpeed(ui->freqBox->value());
}
