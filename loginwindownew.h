#ifndef LOGINWINDOWNEW_H
#define LOGINWINDOWNEW_H

#include <QDialog>

namespace Ui {
class LoginWindowNew;
}

class LoginWindowNew : public QDialog
{
    Q_OBJECT

signals:
    void loginRequested(int id, const QString &password);
    void registerRequested(const QString &name, const QString &password);

public:
    explicit LoginWindowNew(QWidget *parent = nullptr);
    ~LoginWindowNew();

private slots:
    void on_m_loginButton_clicked();

    void on_m_registerButton_clicked();

private:
    Ui::LoginWindowNew *ui;
};

#endif // LOGINWINDOWNEW_H
