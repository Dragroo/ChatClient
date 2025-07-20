// chatwindow.h
#ifndef CHATWINDOW_H
#define CHATWINDOW_H

#include <QWidget>
#include <QListWidget>
#include <QTextEdit>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QSplitter>
#include <QLabel>
#include <vector>
#include <QString>
#include "networkclient.h"
#include "json.hpp"

struct FriendInfo {
    int id;
    QString name;
    QString state;
};

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWindow(NetworkClient *client, QWidget *parent = nullptr);
    void setFriendList(const std::vector<FriendInfo> &friends);
    void setCurrentUserId(int id);

private slots:
    void onSendClicked();
    void onFriendSelected();
    void onMessageReceived(const nlohmann::json &js);

private:
    void setupUI();

    QListWidget *m_friendList;
    QTextEdit *m_messageDisplay;
    QLineEdit *m_inputField;
    QPushButton *m_sendButton;
    QLabel *m_currentChatLabel;
    NetworkClient *m_client;
    int m_currentChatId = -1;
    int m_selfId = -1;
};

#endif // CHATWINDOW_H
