#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QWidget>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QLabel>

class LoginWindow : public QWidget
{
    Q_OBJECT
public:
    explicit LoginWindow(QWidget *parent = nullptr);

signals:
    void loginRequested(int id, const QString &password);
    void registerRequested(const QString &name, const QString &password);

private slots:
    void onLoginClicked();
    void onRegisterClicked();

private:
    QLineEdit *m_idEdit;
    QLineEdit *m_nameEdit;
    QLineEdit *m_passwordEdit;
    QPushButton *m_loginButton;
    QPushButton *m_registerButton;
};

#endif // LOGINWINDOW_H
