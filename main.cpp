// main.cpp
#include <QApplication>
#include "loginwindow.h"
#include "networkclient.h"
#include "chatwindow.h"
#include <QObject>
#include <QMessageBox>
#include "json.hpp"
#include "public.hpp"

using json = nlohmann::json;

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LoginWindow loginWin;
    NetworkClient client;
    ChatWindow *chatWin = nullptr;


    // 设置服务器信息
    QString serverIp = "123.57.54.79";
    quint16 serverPort = 6784;

    client.connectToServer(serverIp, serverPort); // 异步连接，由信号判断结果

    QObject::connect(&client, &NetworkClient::connected, [&]() {
        qDebug() << "Connected to server.";
    });

    QObject::connect(&client, &NetworkClient::errorOccurred, [&](QAbstractSocket::SocketError, const QString &errorString) {
        QMessageBox::critical(nullptr, "Connection Error", "Failed to connect: " + errorString);
        app.exit(-1);
    });

    QObject::connect(&loginWin, &LoginWindow::loginRequested, [&](int id, const QString &pwd) {
        json js;
        js["msgid"] = LOGIN_MSG; // LOGIN_MSG
        js["id"] = id;
        js["password"] = pwd.toStdString();
        client.sendJson(js);
    });

    QObject::connect(&loginWin, &LoginWindow::registerRequested, [&](const QString &name, const QString &pwd) {
        json js;
        js["msgid"] = REG_MSG; // REG_MSG
        js["name"] = name.toStdString();
        js["password"] = pwd.toStdString();
        client.sendJson(js);
    });

    QObject::connect(&client, &NetworkClient::jsonReceived, [&](const json &responsejs) {
        if (!responsejs.contains("msgid")) return;
        int msgid = responsejs["msgid"];
        if (msgid == LOGIN_MSG_ACK) { // LOGIN_MSG_ACK
            if (responsejs["errno"].get<int>() != 0) {
                QMessageBox::warning(&loginWin, "Login Failed", QString::fromStdString(responsejs["errmsg"]));
            } else {
                QMessageBox::information(&loginWin, "Login Success", "Welcome back!");
                loginWin.hide();
                chatWin = new ChatWindow(&client);
                // 提取好友列表
                std::vector<FriendInfo> friendList;
                if (responsejs.contains("friends")) {
                    // qDebug() << QString::fromStdString(responsejs["friends"].dump());
                    for (const auto& fstr : responsejs["friends"]) {
                        try {
                            json fjs = json::parse(fstr.get<std::string>());
                            FriendInfo f;
                            f.id = fjs["id"].get<int>();
                            f.name = QString::fromStdString(fjs["name"]);
                            f.state = QString::fromStdString(fjs["state"]);
                            friendList.push_back(f);
                        } catch (...) {
                            qWarning("Failed to parse friend entry");
                        }
                    }
                }
                chatWin->setCurrentUserId(responsejs["id"].get<int>());
                chatWin->setFriendList(friendList);
                chatWin->show();
            }
        } else if (msgid == REG_MSG_ACK) { // REG_MSG_ACK
            if (responsejs["errno"].get<int>() != 0) {
                QMessageBox::warning(&loginWin, "Register Failed", "Username already exists.");
            } else {
                int id = responsejs["id"].get<int>();
                QMessageBox::information(&loginWin, "Register Success", "Your user ID is: " + QString::number(id));
            }
        }
    });

    loginWin.show();
    return app.exec();
}
