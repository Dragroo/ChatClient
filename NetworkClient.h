// networkclient.h
#ifndef NETWORKCLIENT_H
#define NETWORKCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QByteArray>
#include "json.hpp"  // 假设使用 nlohmann/json 库

using json = nlohmann::json;

class NetworkClient : public QObject {
    Q_OBJECT

public:
    explicit NetworkClient(QObject *parent = nullptr);
    ~NetworkClient() override;

    // 连接服务器（异步，通过信号返回结果）
    void connectToServer(const QString &ip, quint16 port);
    // 发送 JSON 数据（需在连接状态下调用）
    void sendJson(const json &js);
    // 断开连接
    void closeConnection();

signals:
    // 连接成功信号
    void connected();
    // 连接断开信号
    void disconnected();
    // 接收到 JSON 数据信号
    void jsonReceived(const json &js);
    // 错误信号（错误类型 + 描述）
    void errorOccurred(QAbstractSocket::SocketError error, const QString &errorString);

private slots:
    // 处理 socket 连接成功
    void onSocketConnected();
    // 处理 socket 断开连接
    void onSocketDisconnected();
    // 处理接收到数据
    void onSocketReadyRead();
    // 处理 socket 错误
    void onSocketError(QAbstractSocket::SocketError error);

private:
    QTcpSocket *m_socket = nullptr;  // Qt TCP socket
    QByteArray m_recvBuffer;         // 接收缓冲区（解决粘包问题）
};

#endif // NETWORKCLIENT_H
