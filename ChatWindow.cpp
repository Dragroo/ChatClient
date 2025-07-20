// chatwindow.cpp
#include "chatwindow.h"
#include <QDateTime>
#include <QVBoxLayout>
#include <QMessageBox>
#include "public.hpp"

using json = nlohmann::json;

ChatWindow::ChatWindow(NetworkClient *client, QWidget *parent)
    : QWidget(parent), m_client(client)
{
    setWindowTitle("Chat Client");
    resize(800, 500);
    setupUI();

    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(m_friendList, &QListWidget::itemClicked, this, &ChatWindow::onFriendSelected);
    connect(m_client, &NetworkClient::jsonReceived, this, &ChatWindow::onMessageReceived);
}

void ChatWindow::setupUI()
{
    m_friendList = new QListWidget;
    m_messageDisplay = new QTextEdit;
    m_messageDisplay->setReadOnly(true);
    m_inputField = new QLineEdit;
    m_sendButton = new QPushButton("Send");
    m_currentChatLabel = new QLabel("No friend selected");

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel("Friends"));
    leftLayout->addWidget(m_friendList);

    QVBoxLayout *chatLayout = new QVBoxLayout;
    chatLayout->addWidget(m_currentChatLabel);
    chatLayout->addWidget(m_messageDisplay);
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->addWidget(m_inputField);
    inputLayout->addWidget(m_sendButton);
    chatLayout->addLayout(inputLayout);

    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->addLayout(leftLayout, 2);
    mainLayout->addLayout(chatLayout, 5);

    setLayout(mainLayout);
}

void ChatWindow::onFriendSelected()
{
    QListWidgetItem *item = m_friendList->currentItem();
    if (!item) return;

    QString friendInfo = item->text();
    m_currentChatLabel->setText("Chatting with: " + friendInfo);
    m_currentChatId = friendInfo.section(":", 0, 0).toInt();
}

void ChatWindow::onSendClicked()
{
    QString msg = m_inputField->text().trimmed();
    if (msg.isEmpty() || m_currentChatId == -1) return;

    json js;
    js["msgid"] = ONE_CHAT_MSG; // ONE_CHAT_MSG
    js["id"] = 0; // 当前用户ID，待集成
    js["toid"] = m_currentChatId;
    js["msg"] = msg.toStdString();
    js["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();

    m_client->sendJson(js);
    m_messageDisplay->append("Me: " + msg);
    m_inputField->clear();
}

void ChatWindow::onMessageReceived(const json &js)
{
    if (js.contains("msgid") && js["msgid"] == ONE_CHAT_MSG) // ONE_CHAT_MSG
    {
        QString sender = QString::fromStdString(js["name"]);
        QString msg = QString::fromStdString(js["msg"]);
        QString time = QString::fromStdString(js["time"]);
        m_messageDisplay->append(time + " [" + sender + "] " + msg);
    }
}
