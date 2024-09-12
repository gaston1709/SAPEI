#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "addcarddialog.h"
#include <QDebug>
#include <QByteArray>
#include <QMessageBox>
#include <QInputDialog>
#include <QElapsedTimer>
#include <QFile>
#include <QTextStream>
#include <QSerialPortInfo>
#include <QComboBox>
#include <QVBoxLayout>
#include <QDialogButtonBox>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    serial(new QSerialPort(this)),
    readTimer(new QTimer(this)),
    availableMoney(10000),
    isChargingMode(false),
    isReadingEnabled(true),
    isAddingCardMode(false)
{
    ui->setupUi(this);
    connectSerialPort();
    updateCardList();

    readTimer->setSingleShot(true);
    connect(readTimer, &QTimer::timeout, this, &MainWindow::enableReading);

    connect(ui->addCardButton, &QPushButton::clicked, this, &MainWindow::on_addCardButton_clicked);

    loadData();
    lastModification = QFileInfo("tarjetas.csv").lastModified();

    QTimer *fileCheckTimer = new QTimer(this);
    connect(fileCheckTimer, &QTimer::timeout, this, &MainWindow::verifyFileChanges);
    fileCheckTimer->start(5000);

    connect(ui->cardListWidget, &QListWidget::itemDoubleClicked,
            this, &MainWindow::on_cardListWidget_itemDoubleClicked);

    connect(ui->selectPortButton, &QPushButton::clicked, this, &MainWindow::selectSerialPort);  // Add this line to connect the new button
}


MainWindow::~MainWindow() {
    delete ui;
}

void MainWindow::connectSerialPort() {
    if (!selectedPortName.isEmpty()) {
        serial->setPortName(selectedPortName);
        serial->setBaudRate(QSerialPort::Baud115200);
        serial->setDataBits(QSerialPort::Data8);
        serial->setParity(QSerialPort::NoParity);
        serial->setStopBits(QSerialPort::OneStop);
        serial->setFlowControl(QSerialPort::NoFlowControl);

        if (serial->open(QIODevice::ReadWrite)) {
            connect(serial, &QSerialPort::readyRead, this, &MainWindow::handleReadyRead);
            qDebug() << "Conexión serial abierta en" << selectedPortName;
        } else {
            qDebug() << "No se pudo abrir la conexión serial en" << selectedPortName;
        }
    } else {
        qDebug() << "No se ha seleccionado ningún puerto COM.";
    }
}

void MainWindow::selectSerialPort() {
    QDialog dialog(this);
    dialog.setWindowTitle("Seleccionar Puerto Serial");

    QVBoxLayout layout(&dialog);

    QComboBox comboBox;
    const auto ports = QSerialPortInfo::availablePorts();
    for (const QSerialPortInfo &port : ports) {
        comboBox.addItem(port.portName());
    }
    layout.addWidget(&comboBox);

    QDialogButtonBox buttonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    connect(&buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(&buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);
    layout.addWidget(&buttonBox);

    if (dialog.exec() == QDialog::Accepted) {
        selectedPortName = comboBox.currentText();
        connectSerialPort();  // Reconnect using the selected port
    }
}

void MainWindow::sendToArduino(const QString &message) {
    if (serial->isOpen()) {
        QByteArray byteArray = (message + "\r\n").toUtf8();
        serial->write(byteArray);

        if (!serial->waitForBytesWritten(1000)) {
            qDebug() << "Error al enviar datos al Arduino:" << serial->errorString();
        }
    } else {
        qDebug() << "El puerto serie no está abierto.";
    }
}

void MainWindow::handleReadyRead() {
    QByteArray data = serial->readAll();
    QString id = QString::fromUtf8(data).trimmed().toUpper();
    qDebug() << "ID recibido:" << id;

    if (id.length() == 8) {  // Verifica que el ID tenga 4 bytes (8 caracteres hexadecimales)
        if (isAddingCardMode) {
            handleAddCardMode(id);
        } else if (isChargingMode) {
            handleChargingMode(id);
        } else {
            processId(id);
        }
    } else {
        ui->textBrowser->append("ID no válido: " + id);
    }
}

void MainWindow::handleAddCardMode(const QString &id) {
    verifyFileChanges();  // Asegurarse de tener los datos más recientes
    if (!idBalances.contains(id)) {
        isReadingEnabled = false;
        readTimer->stop();

        AddCardDialog dialog(this);
        if (dialog.exec() == QDialog::Accepted) {
            QString nombre = dialog.getNombre();
            QString apellido = dialog.getApellido();
            QString dni = dialog.getDni();
            QString patente = dialog.getPatente();
            QString color = dialog.getColor();
            saveCard(id, nombre, apellido, dni, patente, color);
            idBalances[id] = 0;
            updateCardList();
            ui->textBrowser->append("Tarjeta agregada: " + id);
            sendToArduino("yellow p off");
            QTimer::singleShot(100, this, [&]() {
                sendToArduino("green on");
            });


        }
    } else {
        ui->textBrowser->append("Esta tarjeta ya está registrada: " + id);
        sendToArduino("yellow p off");
        QTimer::singleShot(100, this, [&]() {
        sendToArduino("yellow p");
        });

        isAddingCardMode = false;
    }
    isAddingCardMode = false;
    isReadingEnabled = true;
    readTimer->start(3000);
}

void MainWindow::handleChargingMode(const QString &id) {
    ui->textBrowser->append("Tarjeta escaneada para carga: " + id);

    if (idBalances.contains(id)) {
        currentId = id;
        ui->textBrowser->append("Por favor, ingrese el monto a cargar y presione el botón de carga.");
        sendToArduino("green on");
    } else {
        ui->textBrowser->append("Tarjeta no registrada: " + id);
        sendToArduino("yellow p off");
        QTimer::singleShot(100, this, [&]() {
            sendToArduino("red on");
        });

        isChargingMode = false;
    }

    isReadingEnabled = true;
}

void MainWindow::on_chargeButton_clicked() {
    if (isChargingMode) {
        if (currentId.isEmpty()) {
            ui->textBrowser->append("Por favor, escanee una tarjeta primero.");
            sendToArduino("red on");

            return;
        }
        bool ok;
        int amount = ui->chargeInput->text().toInt(&ok);
        if (ok && amount > 0) {
            availableMoney += amount;
            idBalances[currentId] += amount;
            saveFileChanges();
            updateCardList();
            ui->textBrowser->append("Carga exitosa de " + QString::number(amount) + " para ID: " + currentId);
            sendToArduino("yellow p off");
            sendToArduino("Green on");
            isChargingMode = false;
            currentId.clear();
            ui->chargeInput->clear();
        } else {
            QMessageBox::warning(this, "Error", "Por favor ingrese un monto válido.");
            sendToArduino("red p");

        }
    } else {
        isChargingMode = true;
        ui->textBrowser->append("Modo de carga activado. Por favor escanee una tarjeta.");
        sendToArduino("yellow p on");

    }
}


void MainWindow::saveCard(const QString &id, const QString &nombre, const QString &apellido, const QString &dni, const QString &patente, const QString &color) {
    QStringList cardData;
    cardData << nombre << apellido << dni << patente << color;
    idToCardData[id] = cardData;
    idBalances[id] = 0;  // Saldo inicial

    saveFileChanges();
}

bool MainWindow::saveFileChanges() {
    QFile file("tarjetas.csv");
    if (file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        QTextStream out(&file);
        for (auto it = idToCardData.begin(); it != idToCardData.end(); ++it) {
            QString id = it.key();
            QStringList cardData = it.value();
            out << id << ";" << cardData.join(";") << ";" << idBalances[id] << "\n";
        }
        file.close();
        return true;
    } else {
        qDebug() << "No se pudo abrir el archivo para guardar los datos";
        return false;
    }
}

void MainWindow::loadData() {
    QFile file("tarjetas.csv");
    if (file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        QTextStream in(&file);
        QMap<QString, int> newIdBalances;
        QMap<QString, QStringList> newIdToCardData;
        while (!in.atEnd()) {
            QString line = in.readLine();
            QStringList parts = line.split(";");
            if (parts.size() >= 7) {
                QString id = parts[0];
                int saldo = parts.last().toInt();
                QStringList cardData = parts.mid(1, 5);
                newIdToCardData[id] = cardData;
                newIdBalances[id] = saldo;
            } else {
                qDebug() << "Línea mal formateada en el archivo:" << line;
            }
        }
        file.close();
        // Solo actualizamos los datos principales si la carga fue exitosa
        idBalances = newIdBalances;
        idToCardData = newIdToCardData;
    } else {
        qDebug() << "No se pudo abrir el archivo para cargar los datos";
    }
    updateCardList();
}
void MainWindow::verifyFileChanges() {
    QFileInfo fileInfo("tarjetas.csv");
    if (!fileInfo.exists()) {
        // El archivo ha sido eliminado, limpiamos los datos
        idBalances.clear();
        idToCardData.clear();
        updateCardList();
        lastModification = QDateTime();
    } else if (fileInfo.lastModified() > lastModification) {
        loadData();
        lastModification = fileInfo.lastModified();
    }
}

void MainWindow::enableReading() {
    isReadingEnabled = true;
}

void MainWindow::on_addCardButton_clicked() {
    if (isAddingCardMode) {
        return; // Evitar procesar si ya está en modo de agregar tarjeta
    }
    qDebug() << "Botón Agregar Tarjeta presionado";
    ui->textBrowser->append("Modo de Agregar tarjeta activado.");
    isAddingCardMode = true;
    isReadingEnabled = true;  // Habilitar lectura
    sendToArduino("yellow p on");

}

void MainWindow::processId(const QString &id) {
    if (!idBalances.contains(id)) {
        sendToArduino("red on");
        QMessageBox::warning(this, "Error", "Tarjeta no registrada.");
        return;


    }

    if (idBalances[id] >= 600) {
        idBalances[id] -= 600;
        availableMoney += 600;
        sendToArduino("green on");
        ui->textBrowser->append("ID: " + id + " - Cobro realizado: 600");

        saveFileChanges();  // Actualiza el archivo después del cobro
    } else {
        sendToArduino("red p");
        ui->textBrowser->append("ID: " + id + " - Saldo insuficiente");


    }

    updateCardList();
}

void MainWindow::on_cardListWidget_itemDoubleClicked(QListWidgetItem *item) {
    if (!item) {
        qDebug() << "Error: Item nulo";
        return;
    }

    QString itemText = item->text();
    QStringList parts = itemText.split("|");
    if (parts.isEmpty()) {
        qDebug() << "Error: No se pudo dividir el texto del item";
        return;
    }

    QString idPart = parts.first();
    QStringList idParts = idPart.split(":");
    if (idParts.size() < 2) {
        qDebug() << "Error: Formato de ID incorrecto";
        return;
    }

    QString id = idParts.last().trimmed();
    if (id.isEmpty()) {
        qDebug() << "Error: ID vacío";
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Eliminar tarjeta",
                                  "¿Está seguro que desea eliminar esta tarjeta?",
                                  QMessageBox::Yes|QMessageBox::No);

    if (reply == QMessageBox::Yes) {
        // Desconectar la señal antes de eliminar
        disconnect(ui->cardListWidget, &QListWidget::itemDoubleClicked,
                   this, &MainWindow::on_cardListWidget_itemDoubleClicked);

        if (idBalances.contains(id) && idToCardData.contains(id)) {
            idBalances.remove(id);
            idToCardData.remove(id);

            int row = ui->cardListWidget->row(item);
            QListWidgetItem *itemToDelete = ui->cardListWidget->takeItem(row);
            delete itemToDelete;

            QMessageBox::information(this, "Éxito", "Tarjeta eliminada del programa.");

            if (deleteFileLine(id)) {
                QMessageBox::information(this, "Éxito", "Tarjeta eliminada del archivo CSV.");
            } else {
                QMessageBox::warning(this, "Advertencia", "No se pudo eliminar la tarjeta del archivo CSV.");
            }
        } else {
            QMessageBox::warning(this, "Error", "Esta tarjeta no existe en la base de datos.");
        }

        // Reconectar la señal después de eliminar
        connect(ui->cardListWidget, &QListWidget::itemDoubleClicked,
                this, &MainWindow::on_cardListWidget_itemDoubleClicked);
    }
}



bool MainWindow::deleteFileLine(const QString &id) {
    QFile inputFile("tarjetas.csv");
    QFile tempFile("temp.csv");

    if (!inputFile.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "No se pudo abrir el archivo de entrada";
        return false;
    }

    if (!tempFile.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "No se pudo crear el archivo temporal";
        inputFile.close();
        return false;
    }

    QTextStream in(&inputFile);
    QTextStream out(&tempFile);

    bool lineaBorrada = false;

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (!line.startsWith(id + ";")) {
            out << line << "\n";
        } else {
            qDebug() << "Línea encontrada y borrada: " << line;
            lineaBorrada = true;
        }
    }

    inputFile.close();
    tempFile.close();

    if (lineaBorrada) {
        if (QFile::remove("tarjetas.csv") && QFile::rename("temp.csv", "tarjetas.csv")) {
            qDebug() << "Archivo original reemplazado exitosamente.";
            return true;
        } else {
            qDebug() << "No se pudo reemplazar el archivo original";
            return false;
        }
    } else {
        QFile::remove("temp.csv");
        qDebug() << "No se encontró ninguna línea para borrar.";
        return false;
    }
}


void MainWindow::updateCardList() {
    ui->cardListWidget->clear();

    if (idToCardData.isEmpty()) {
        new QListWidgetItem("No hay tarjetas registradas", ui->cardListWidget);
        return;
    }

    for (auto it = idToCardData.begin(); it != idToCardData.end(); ++it) {
        QString id = it.key();
        QStringList cardData = it.value();

        if (!idBalances.contains(id) || cardData.size() < 5) {
            qDebug() << "Error: Datos incompletos o balance no encontrado para ID" << id;
            continue;
        }

        QString displayText = QString("ID: %1| Nombre: %2 %3| DNI: %4| Patente: %5| Color: %6| Saldo: %7")
                                  .arg(id)
                                  .arg(cardData[0])
                                  .arg(cardData[1])
                                  .arg(cardData[2])
                                  .arg(cardData[3])
                                  .arg(cardData[4])
                                  .arg(idBalances[id]);

        new QListWidgetItem(displayText, ui->cardListWidget);
    }
}



