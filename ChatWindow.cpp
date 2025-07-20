// chatwindow.cpp
#include "chatwindow.h"
#include <QDateTime>
#include <QVBoxLayout>
#include <QMessageBox>
#include <QDebug>
#include <QMap>
#include <QPair>
#include <QPushButton>
#include <QStringList>
#include <QInputDialog>
#include "public.hpp"

using json = nlohmann::json;
static QMap<QPair<int, int>, QStringList> chatHistory;
static QMap<QPair<int, int>, QStringList> groupHistory;
static int oneChat;


ChatWindow::ChatWindow(NetworkClient *client, QWidget *parent)
    : QWidget(parent), m_client(client)
{
    setWindowTitle("Chat Client");
    resize(800, 500);
    setupUI();

    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(m_friendList, &QListWidget::itemClicked, this, &ChatWindow::onFriendSelected);
    connect(m_groupList, &QListWidget::itemClicked, this, &ChatWindow::onGroupSelected);
    connect(m_client, &NetworkClient::jsonReceived, this, &ChatWindow::onMessageReceived);
}

void ChatWindow::setupUI()
{
    m_friendList = new QListWidget;
    m_groupList = new QListWidget;
    m_messageDisplay = new QTextEdit;
    m_messageDisplay->setReadOnly(true);
    m_inputField = new QLineEdit;
    m_sendButton = new QPushButton("Send");
    m_createGroupButton = new QPushButton("Create Group");
    m_joinGroupButton = new QPushButton("Join Group");
    m_currentChatLabel = new QLabel("No friend selected");

    connect(m_createGroupButton, &QPushButton::clicked, this, &ChatWindow::onCreateGroupClicked);
    connect(m_joinGroupButton, &QPushButton::clicked, this, &ChatWindow::onJoinGroupClicked);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    leftLayout->addWidget(new QLabel("Friends"));
    leftLayout->addWidget(m_friendList);
    leftLayout->addWidget(new QLabel("Groups"));
    leftLayout->addWidget(m_groupList);
    leftLayout->addWidget(m_createGroupButton);
    leftLayout->addWidget(m_joinGroupButton);

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
    oneChat=1;
    QListWidgetItem *item = m_friendList->currentItem();
    if (!item) return;

    QString friendInfo = item->text();
    m_currentChatLabel->setText("Chatting with: " + friendInfo);
    m_currentChatId = friendInfo.section(":", 0, 0).toInt();
    m_messageDisplay->clear();
    QPair<int, int> key1(m_selfId, m_currentChatId);
    QPair<int, int> key2(m_currentChatId, m_selfId);

    // 显示与该好友之间的所有消息
    QStringList allMessages;
    allMessages.append(chatHistory[key1]);
    allMessages.append(chatHistory[key2]);

    std::sort(allMessages.begin(), allMessages.end()); // 根据时间顺序（依赖时间格式）

    for (const QString &line : allMessages) {
        m_messageDisplay->append(line);
    }
}

void ChatWindow::onGroupSelected()
{
    oneChat=2;
    QListWidgetItem *item = m_groupList->currentItem();
    if (!item) return;

    QString groupInfo = item->text();
    m_currentChatLabel->setText("Chatting with: " + groupInfo);
    m_currentGroupId = groupInfo.section(":", 0, 0).toInt();
    m_messageDisplay->clear();
    QPair<int, int> key1(m_selfId, m_currentGroupId);
    QPair<int, int> key2(m_currentGroupId, m_selfId);

    // 显示与该好友之间的所有消息
    QStringList allMessages;
    allMessages.append(groupHistory[key1]);
    allMessages.append(groupHistory[key2]);

    std::sort(allMessages.begin(), allMessages.end()); // 根据时间顺序（依赖时间格式）

    for (const QString &line : allMessages) {
        m_messageDisplay->append(line);
    }
}

void ChatWindow::onSendClicked()
{
    if(oneChat==1){
        QString msg = m_inputField->text().trimmed();
        if (msg.isEmpty() || m_currentChatId == -1 || m_selfId == -1) return;

        json js;
        js["msgid"] = ONE_CHAT_MSG; // ONE_CHAT_MSG
        js["id"] = m_selfId;
        js["toid"] = m_currentChatId;
        js["msg"] = msg.toStdString();
        js["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
        QString time = QString::fromStdString(js["time"]);
        // 存储发送记录
        QString line =time+ " [Me] " + msg;
        QPair<int, int> key(m_selfId, m_currentChatId);
        chatHistory[key].append(line);

        m_client->sendJson(js);
        m_messageDisplay->append(line);
        m_inputField->clear();
    }
    if(oneChat==2){
        QString msg = m_inputField->text().trimmed();
        if (msg.isEmpty() || m_currentGroupId == -1 || m_selfId == -1) return;

        json js;
        js["msgid"] = GROUP_CHAT_MSG; // GROUP_CHAT_MSG
        js["id"] = m_selfId;
        js["groupid"] = m_currentGroupId;
        js["msg"] = msg.toStdString();
        js["time"] = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss").toStdString();
        QString time = QString::fromStdString(js["time"]);
        // 存储发送记录
        QString line =time + " [" + QString::number(m_currentGroupId) + "-" + QString::number(m_selfId) + "] " + msg;
        QPair<int, int> key(m_selfId, m_currentChatId);
        chatHistory[key].append(line);

        m_client->sendJson(js);
        m_messageDisplay->append(line);
        m_inputField->clear();
    }
}

void ChatWindow::onMessageReceived(const json &js)
{
    qDebug() << "[ChatWindow] received JSON:" << QString::fromStdString(js.dump());

    if (!js.contains("msgid")) return;

    try {
        int msgid = js["msgid"];
        if (msgid == ONE_CHAT_MSG && js.contains("id") && js.contains("msg") && js.contains("time") && js.contains("toid")) // ONE_CHAT_MSG
        {
            int senderId = js["id"].get<int>();
            int toId = js["toid"].get<int>();
            if(toId==m_selfId){
                QString msg = QString::fromStdString(js["msg"]);
                QString time = QString::fromStdString(js["time"]);
                QString line = time + " [" + QString::number(senderId) + "] " + msg;
                QPair<int, int> key(senderId, toId);
                chatHistory[key].append(line);
                if (senderId==m_currentChatId) {
                    m_messageDisplay->append(line);
                }
            }
        }
        else if (msgid == GROUP_CHAT_MSG && js.contains("id") && js.contains("msg") && js.contains("time") && js.contains("groupid")) // ONE_CHAT_MSG
        {
            int groupId = js["groupid"].get<int>();
            int senderId = js["id"].get<int>();
            if(userGroupIds.contains(groupId)){
                QString msg = QString::fromStdString(js["msg"]);
                QString time = QString::fromStdString(js["time"]);
                QString line = time + " [" + QString::number(groupId) + "-" + QString::number(senderId) + "] " + msg;
                QPair<int, int> key(groupId, m_selfId);
                groupHistory[key].append(line);
                if (groupId==m_currentGroupId) {
                    m_messageDisplay->append(line);
                }
            }
        }
    } catch (const std::exception &e) {
        qDebug() << "[ChatWindow] JSON parse/display error:" << e.what();
    }
}

void ChatWindow::setFriendList(const std::vector<FriendInfo> &friends)
{
    m_friendList->clear();
    for (const auto &f : friends)
    {
        QString display = QString::number(f.id) + ":" + f.name + " [" + f.state + "]";
        m_friendList->addItem(display);
    }
}

void ChatWindow::loadGroupsFromLoginResponse(const std::vector<GroupInfo> &groups)
{
    m_groupList->clear();
    for (const auto &g : groups)
    {
        QString display = QString::number(g.groupid) + ":" + g.groupname + " [" + g.groupdesc + "]";
        m_groupList->addItem(display);
    }
}

void ChatWindow::setCurrentUserId(int id)
{
    m_selfId = id;
}

void ChatWindow::setCroupIds(int groupId)
{
    userGroupIds.insert(groupId);
}

void ChatWindow::onCreateGroupClicked()
{
    bool ok1, ok2;
    QString groupName = QInputDialog::getText(this, "Create Group", "Group Name:", QLineEdit::Normal, "", &ok1);
    if (!ok1 || groupName.isEmpty()) return;

    QString groupDesc = QInputDialog::getText(this, "Create Group", "Group Description:", QLineEdit::Normal, "", &ok2);
    if (!ok2) return;

    json js;
    js["msgid"] = CREATE_GROUP_MSG;
    js["id"] = m_selfId;
    js["groupname"] = groupName.toStdString();
    js["groupdesc"] = groupDesc.toStdString();
    m_client->sendJson(js);
}

void ChatWindow::onJoinGroupClicked()
{
    bool ok;
    int groupId = QInputDialog::getInt(this, "Join Group", "Group ID:", 0, 0, 999999, 1, &ok);
    if (!ok) return;

    json js;
    js["msgid"] = ADD_GROUP_MSG;
    js["id"] = m_selfId;
    js["groupid"] = groupId;
    m_client->sendJson(js);
}
