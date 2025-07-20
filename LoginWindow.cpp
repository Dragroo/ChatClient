// loginwindow.cpp
#include "loginwindow.h"
#include <QHBoxLayout>
#include <QIntValidator>
#include <QMessageBox>

LoginWindow::LoginWindow(QWidget *parent) : QWidget(parent)
{
    setWindowTitle("Chat Client - Login/Register");
    setFixedSize(300, 200);

    QLabel *idLabel = new QLabel("User ID:");
    QLabel *nameLabel = new QLabel("Username:");
    QLabel *pwdLabel = new QLabel("Password:");

    m_idEdit = new QLineEdit;
    m_idEdit->setValidator(new QIntValidator(0, 99999999, this));
    m_nameEdit = new QLineEdit;
    m_passwordEdit = new QLineEdit;
    m_passwordEdit->setEchoMode(QLineEdit::Password);

    m_loginButton = new QPushButton("Login");
    m_registerButton = new QPushButton("Register");

    QVBoxLayout *mainLayout = new QVBoxLayout;
    mainLayout->addWidget(idLabel);
    mainLayout->addWidget(m_idEdit);
    mainLayout->addWidget(nameLabel);
    mainLayout->addWidget(m_nameEdit);
    mainLayout->addWidget(pwdLabel);
    mainLayout->addWidget(m_passwordEdit);
    mainLayout->addWidget(m_loginButton);
    mainLayout->addWidget(m_registerButton);
    setLayout(mainLayout);

    connect(m_loginButton, &QPushButton::clicked, this, &LoginWindow::onLoginClicked);
    connect(m_registerButton, &QPushButton::clicked, this, &LoginWindow::onRegisterClicked);
}

void LoginWindow::onLoginClicked()
{
    bool ok;
    int id = m_idEdit->text().toInt(&ok);
    if (!ok || m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please input valid user ID and password.");
        return;
    }
    emit loginRequested(id, m_passwordEdit->text());
}

void LoginWindow::onRegisterClicked()
{
    if (m_nameEdit->text().isEmpty() || m_passwordEdit->text().isEmpty()) {
        QMessageBox::warning(this, "Input Error", "Please input username and password.");
        return;
    }
    emit registerRequested(m_nameEdit->text(), m_passwordEdit->text());
}


