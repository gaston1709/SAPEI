#include "addcarddialog.h"
#include "ui_addcarddialog.h"

AddCardDialog::AddCardDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AddCardDialog)
{
    ui->setupUi(this);
    connect(ui->buttonBox, &QDialogButtonBox::accepted, this, &AddCardDialog::on_buttonBox_accepted);
    connect(ui->buttonBox, &QDialogButtonBox::rejected, this, &AddCardDialog::reject);
}

AddCardDialog::~AddCardDialog()
{
    delete ui;
}

QString AddCardDialog::getNombre() const {
    return ui->nombreLineEdit->text();
}

QString AddCardDialog::getApellido() const {
    return ui->apellidoLineEdit->text();
}

QString AddCardDialog::getDni() const {
    return ui->dniLineEdit->text();
}

QString AddCardDialog::getPatente() const {
    return ui->patenteLineEdit->text();
}

QString AddCardDialog::getColor() const {
    return ui->colorLineEdit->text();
}

void AddCardDialog::on_buttonBox_accepted()
{
    accept(); // Esto cerrará el diálogo con el resultado QDialog::Accepted
}
