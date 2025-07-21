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
static QMap<int, QStringList> groupMembers;
static int oneChat;


ChatWindow::ChatWindow(NetworkClient *client, QWidget *parent)
    : QWidget(parent), m_client(client)
{
    setWindowTitle("Chat Client");
    resize(900, 600); // 增加窗口尺寸
    setupUI();

    connect(m_sendButton, &QPushButton::clicked, this, &ChatWindow::onSendClicked);
    connect(m_friendList, &QListWidget::itemClicked, this, &ChatWindow::onFriendSelected);
    connect(m_groupList, &QListWidget::itemClicked, this, &ChatWindow::onGroupSelected);
    connect(m_client, &NetworkClient::jsonReceived, this, &ChatWindow::onMessageReceived);
    connect(m_groupList, &QListWidget::itemDoubleClicked, this, &ChatWindow::onGroupItemDoubleClicked);
}

void ChatWindow::setupUI()
{
    // 应用全局样式
    this->setStyleSheet(""
                        "QWidget { background-color: #f5f7fa; }"
                        "QListWidget {"
                        "   background-color: white;"
                        "   border: 1px solid #e4e7eb;"
                        "   border-radius: 8px;"
                        "   padding: 5px;"
                        "   font-size: 13px;"
                        "}"
                        "QListWidget::item {"
                        "   padding: 8px 10px;"
                        "   border-bottom: 1px solid #f0f2f5;"
                        "}"
                        "QListWidget::item:selected {"
                        "   background-color: #e6f0ff;"
                        "   color: #1a73e8;"
                        "   border-radius: 4px;"
                        "}"
                        "QTextEdit {"
                        "   background-color: white;"
                        "   border: 1px solid #e4e7eb;"
                        "   border-radius: 8px;"
                        "   padding: 12px;"
                        "   font-size: 14px;"
                        "}"
                        "QLineEdit {"
                        "   background-color: white;"
                        "   border: 1px solid #e4e7eb;"
                        "   border-radius: 8px;"
                        "   padding: 12px;"
                        "   font-size: 14px;"
                        "}"
                        "QLineEdit:focus { border: 1px solid #1a73e8; }"
                        "QLabel {"
                        "   color: #1c1e21;"
                        "   font-size: 14px;"
                        "}");
    m_friendList = new QListWidget;
    m_groupList = new QListWidget;
    m_messageDisplay = new QTextEdit;
    m_friendList->setStyleSheet("QListWidget { min-width: 200px; }");
    m_groupList->setStyleSheet("QListWidget { min-width: 200px; }");
    m_messageDisplay->setReadOnly(true);

    m_inputField = new QLineEdit;
    m_inputField->setPlaceholderText("Type a message...");

    m_sendButton = new QPushButton("Send");
    m_sendButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #1a73e8;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 10px 20px;"
        "   font-weight: 600;"
        "   min-width: 80px;"
        "}"
        "QPushButton:hover { background-color: #1669d6; }"
        "QPushButton:pressed { background-color: #1457b8; }"
        );

    m_createGroupButton = new QPushButton("Create Group");
    m_createGroupButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #34a853;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #2d9347; }"
        "QPushButton:pressed { background-color: #267d3b; }"
        );

    m_joinGroupButton = new QPushButton("Join Group");
    m_joinGroupButton->setStyleSheet(
        "QPushButton {"
        "   background-color: #fbbc05;"
        "   color: white;"
        "   border: none;"
        "   border-radius: 8px;"
        "   padding: 10px;"
        "   font-weight: 600;"
        "}"
        "QPushButton:hover { background-color: #e6ab04; }"
        "QPushButton:pressed { background-color: #d19b03; }"
        );

    m_currentChatLabel = new QLabel("No friend selected");
    m_currentChatLabel->setStyleSheet(
        "QLabel {"
        "   font-size: 16px;"
        "   font-weight: 600;"
        "   color: #1c1e21;"
        "   padding: 10px 0;"
        "   border-bottom: 1px solid #e4e7eb;"
        "}"
        );

    connect(m_createGroupButton, &QPushButton::clicked, this, &ChatWindow::onCreateGroupClicked);
    connect(m_joinGroupButton, &QPushButton::clicked, this, &ChatWindow::onJoinGroupClicked);

    QVBoxLayout *leftLayout = new QVBoxLayout;
    // 好友列表部分
    QLabel *friendsLabel = new QLabel("Friends");
    friendsLabel->setStyleSheet("font-weight: 600; font-size: 15px; color: #1c1e21;");
    leftLayout->addWidget(friendsLabel);
    leftLayout->addWidget(m_friendList);

    // 群组列表部分
    QLabel *groupsLabel = new QLabel("Groups");
    groupsLabel->setStyleSheet("font-weight: 600; font-size: 15px; color: #1c1e21;");
    leftLayout->addWidget(groupsLabel);
    leftLayout->addWidget(m_groupList);

    // 按钮容器
    QHBoxLayout *buttonLayout = new QHBoxLayout;
    buttonLayout->setSpacing(10);
    buttonLayout->addWidget(m_createGroupButton);
    buttonLayout->addWidget(m_joinGroupButton);
    leftLayout->addLayout(buttonLayout);

    leftLayout->setSpacing(15);
    leftLayout->setContentsMargins(10, 10, 10, 10);

    // 左侧容器框架
    QFrame *leftFrame = new QFrame;
    leftFrame->setLayout(leftLayout);
    leftFrame->setStyleSheet(
        "QFrame {"
        "   background-color: white;"
        "   border-radius: 12px;"
        "   border: 1px solid #e4e7eb;"
        "}"
        );



    // 聊天区域布局
    QVBoxLayout *chatLayout = new QVBoxLayout;
    chatLayout->setSpacing(15);
    chatLayout->setContentsMargins(15, 15, 15, 15);

    // 聊天区域容器框架
    QFrame *chatFrame = new QFrame;
    chatFrame->setLayout(chatLayout);
    chatFrame->setStyleSheet(
        "QFrame {"
        "   background-color: white;"
        "   border-radius: 12px;"
        "   border: 1px solid #e4e7eb;"
        "}"
        );
    chatLayout->addWidget(m_currentChatLabel);
    chatLayout->addWidget(m_messageDisplay);


    // 输入区域布局
    QHBoxLayout *inputLayout = new QHBoxLayout;
    inputLayout->setSpacing(10);
    inputLayout->addWidget(m_inputField, 1); // 输入框可扩展
    inputLayout->addWidget(m_sendButton);
    chatLayout->addLayout(inputLayout);

    // 主布局
    QHBoxLayout *mainLayout = new QHBoxLayout(this);
    mainLayout->setSpacing(20);
    mainLayout->setContentsMargins(20, 20, 20, 20);
    mainLayout->addWidget(leftFrame, 2);
    mainLayout->addWidget(chatFrame, 5);

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
        // QStringList members;
        // json userjs = json::parse(g.groupmembers.toStdString());
        // for (const auto &userStr : userjs) {
        //     json userJson = json::parse(userStr.get<std::string>());
        //     int uid = userJson["id"].get<int>();
        //     QString uname = QString::fromStdString(userJson["name"]);
        //     QString ustate = QString::fromStdString(userJson["state"]);
        //     QString urole = QString::fromStdString(userJson["role"]);
        //     members.append(QString("%1 (%2) [%3]").arg(uname).arg(urole).arg(ustate));
        // }
        // groupMembers[g.groupid] = members;
        QString display = QString::number(g.groupid) + ":" + g.groupname + " [" + g.groupdesc + "]";
        m_groupList->addItem(display);
        QStringList memberList;
        for (const auto &u : g.members) {
            memberList.append(QString("%1 (%2) [%3]").arg(u.name).arg(u.role).arg(u.state));
        }
        groupMembers[g.groupid] = memberList;
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

void ChatWindow::onGroupItemDoubleClicked(QListWidgetItem *item)
{
    if (!item) return;
    QString text = item->text();
    int groupId = text.section(":", 0, 0).toInt();

    if (!groupMembers.contains(groupId)) {
        QMessageBox::information(this, "Group Members", "No member info available.");
        return;
    }

    QStringList members = groupMembers[groupId];
    QString info = members.join("\n");
    QMessageBox::information(this, QString("Group %1 Members").arg(groupId), info);
}
