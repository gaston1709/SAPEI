#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QSerialPort>
#include <QTimer>
#include <QMap>
#include "addcarddialog.h"
#include <QDateTime>
#include <QFileInfo>
#include <QListWidget>
#include <QListWidgetItem>

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void on_chargeButton_clicked();
    void handleReadyRead();
    void enableReading();
    void on_addCardButton_clicked();
    void on_cardListWidget_itemDoubleClicked(QListWidgetItem *item);



private:
    void connectSerialPort();
    void selectSerialPort();  // Declare the function here
    void updateBalanceDisplay();
    void processId(const QString &id);
    void updateCardList();
    void handleAddCardMode(const QString &id);
    void handleChargingMode(const QString &id);
    void verifyFileChanges();
    bool saveFileChanges();
    bool deleteFileLine(const QString &id);
    void sendToArduino(const QString &message);






    Ui::MainWindow *ui;
    QSerialPort *serial;
    QString selectedPortName;
    QTimer *readTimer;
    int availableMoney;
    bool isChargingMode;
    bool isReadingEnabled;
    bool isAddingCardMode;
    QString currentId;
    QMap<QString, int> idBalances;
    QMap<QString, QStringList> idToCardData; // Agrega esta línea para manejar los datos de las tarjetas
    QDateTime lastModification;

    void saveCard(const QString &id, const QString &nombre, const QString &apellido, const QString &dni, const QString &patente, const QString &color);
    void loadData(); // Asegúrate de que esta función esté declarada aquí


};

#endif // MAINWINDOW_H
