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

// GroupInfo.h or wherever你定义的结构体
struct GroupUserInfo {
    int id;
    QString name;
    QString state;
    QString role;
};

struct GroupInfo {
    int groupid;
    QString groupname;
    QString groupdesc;
    std::vector<GroupUserInfo> members;  // 新增
};

class ChatWindow : public QWidget
{
    Q_OBJECT
public:
    explicit ChatWindow(NetworkClient *client, QWidget *parent = nullptr);
    void setFriendList(const std::vector<FriendInfo> &friends);
    void loadGroupsFromLoginResponse(const std::vector<GroupInfo> &groups);
    void setCurrentUserId(int id);
    void setCroupIds(int groupId);

private slots:
    void onSendClicked();
    void onFriendSelected();
    void onGroupSelected();
    void onMessageReceived(const nlohmann::json &js);
    void onCreateGroupClicked();
    void onJoinGroupClicked();
    void onGroupItemDoubleClicked(QListWidgetItem *item);


private:
    void setupUI();

    QListWidget *m_friendList;
    QListWidget *m_groupList;
    QTextEdit *m_messageDisplay;
    QLineEdit *m_inputField;
    QPushButton *m_sendButton;
    QPushButton *m_createGroupButton;
    QPushButton *m_joinGroupButton;
    QSet<int> userGroupIds;
    QLabel *m_currentChatLabel;
    NetworkClient *m_client;
    int m_currentChatId = -1;
    int m_currentGroupId = -1;
    int m_selfId = -1;
};

#endif // CHATWINDOW_H
