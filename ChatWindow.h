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
#include "networkclient.h"

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWindow(NetworkClient *client, QWidget *parent = nullptr);

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
};

#endif // CHATWINDOW_H
