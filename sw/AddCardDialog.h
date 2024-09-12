#ifndef ADDCARDDIALOG_H
#define ADDCARDDIALOG_H

#include <QDialog>

namespace Ui {
class AddCardDialog;
}

class AddCardDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddCardDialog(QWidget *parent = nullptr);
    ~AddCardDialog();

    QString getNombre() const;
    QString getApellido() const;
    QString getDni() const;
    QString getPatente() const;
    QString getColor() const;

private slots:
    void on_buttonBox_accepted();
private:
    Ui::AddCardDialog *ui;
};

#endif // ADDCARDDIALOG_H
