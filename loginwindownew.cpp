#include "loginwindownew.h"
#include "ui_loginwindownew.h"
#include <QIntValidator>
#include <QMessageBox>

LoginWindowNew::LoginWindowNew(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::LoginWindowNew)
{
    ui->setupUi(this);
    setFixedSize(400, 240);
    ui->m_idEdit->setValidator(new QIntValidator(0, 99999999, this));
    ui->m_idEdit->setStyleSheet("QLineEdit {"
                            "padding: 10px;"
                            "border: 1px solid #dddfe2;"
                            "border-radius: 4px;"
                            "font-size: 18px;"
                            "}");
    ui->m_nameEdit->setStyleSheet("QLineEdit {"
                              "padding: 10px;"
                              "border: 1px solid #dddfe2;"
                              "border-radius: 4px;"
                              "font-size: 18px;"
                              "}");

    ui->m_passwordEdit->setEchoMode(QLineEdit::Password);
    ui->m_passwordEdit->setStyleSheet("QLineEdit {"
                              "padding: 10px;"
                              "border: 1px solid #dddfe2;"
                              "border-radius: 4px;"
                              "font-size: 18px;"
                              "}");

    ui->m_loginButton->setStyleSheet("QPushButton {"
                                     "background-color: #42b72a;"
                                     "color: white;"
                                     "border: none;"
                                     "border-radius: 4px;"
                                     "padding: 1px;"
                                     "font-weight: bold;"
                                     "font-size: 14px;"
                                     "}"
                                     "QPushButton:hover {"
                                     "background-color: #3aa322;"
                                     "}"
                                     "QPushButton:pressed {"
                                     "background-color: #2f8c1a;"
                                     "}");
    ui->m_registerButton->setStyleSheet("QPushButton {"
                                    "background-color: #42b72a;"
                                    "color: white;"
                                    "border: none;"
                                    "border-radius: 4px;"
                                    "padding: 1px;"
                                    "font-weight: bold;"
                                    "font-size: 14px;"
                                    "}"
                                    "QPushButton:hover {"
                                    "background-color: #3aa322;"
                                    "}"
                                    "QPushButton:pressed {"
                                    "background-color: #2f8c1a;"
                                    "}");


}

LoginWindowNew::~LoginWindowNew()
{
    delete ui;
}

void LoginWindowNew::on_m_loginButton_clicked()
{
    bool ok;
    int id = ui->m_idEdit->text().toInt(&ok);
    if (!ok || ui->m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please input valid user ID and password.");
        return;
    }
    emit loginRequested(id, ui->m_passwordEdit->text());
}


void LoginWindowNew::on_m_registerButton_clicked()
{
    if (ui->m_nameEdit->text().isEmpty() || ui->m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please input username and password.");
        return;
    }
    emit registerRequested(ui->m_nameEdit->text(), ui->m_passwordEdit->text());
}

