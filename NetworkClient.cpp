// networkclient.cpp
#include "networkclient.h"
#include <QDebug>

NetworkClient::NetworkClient(QObject *parent) : QObject(parent) {
    m_socket = new QTcpSocket(this);

    // 连接 Qt 信号槽（核心：异步事件处理）
    connect(m_socket, &QTcpSocket::connected, this, &NetworkClient::onSocketConnected);
    connect(m_socket, &QTcpSocket::disconnected, this, &NetworkClient::onSocketDisconnected);
    connect(m_socket, &QTcpSocket::readyRead, this, &NetworkClient::onSocketReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred, this, &NetworkClient::onSocketError);
}

NetworkClient::~NetworkClient() {
    closeConnection();  // 析构时确保断开连接
}

void NetworkClient::connectToServer(const QString &ip, quint16 port) {
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        qWarning() << "Already connected or connecting. Disconnect first.";
        return;
    }
    // 异步连接（非阻塞）
    m_socket->connectToHost(ip, port);
}

void NetworkClient::sendJson(const json &js) {
    if (m_socket->state() != QAbstractSocket::ConnectedState) {
        qWarning() << "Cannot send data: not connected to server.";
        return;
    }
    // 将 JSON 转换为字节流（不含结尾 '\0'）
    std::string jsonStr = js.dump();  // 生成 JSON 字符串
    QByteArray data(jsonStr.c_str(), jsonStr.size());
    // 发送数据（Qt 内部处理部分发送，无需手动循环）
    m_socket->write(data);
}

void NetworkClient::closeConnection() {
    if (m_socket->state() != QAbstractSocket::UnconnectedState) {
        m_socket->disconnectFromHost();
        // 若处于连接中，等待断开（超时 1 秒）
        if (m_socket->state() == QAbstractSocket::ConnectingState) {
            m_socket->waitForDisconnected(1000);
        }
    }
    m_recvBuffer.clear();  // 清空缓冲区
}

// ------------------------------ 私有槽函数 ------------------------------
void NetworkClient::onSocketConnected() {
    qDebug() << "Connected to server successfully.";
    emit connected();  // 转发连接成功信号
}

void NetworkClient::onSocketDisconnected() {
    qDebug() << "Disconnected from server.";
    emit disconnected();  // 转发断开信号
}

void NetworkClient::onSocketReadyRead() {
    // 读取所有可用数据到缓冲区

    m_recvBuffer.append(m_socket->readAll());
    qDebug() << "Buffer content:" << m_recvBuffer;
    // 解析缓冲区（假设 JSON 消息以换行符 '\0' 分隔，需与服务器约定）
    while (true) {
        int newlinePos = m_recvBuffer.indexOf('\0');
        if (newlinePos == -1){
            break;  // 无完整消息，等待下一次数据
        }
        // 提取单条 JSON 消息（不含换行符）
        QByteArray jsonData = m_recvBuffer.left(newlinePos);
        m_recvBuffer = m_recvBuffer.mid(newlinePos + 1);  // 移除已处理数据

        // 解析 JSON 并转发信号
        try {

            json js = json::parse(jsonData.toStdString());
            emit jsonReceived(js);
        } catch (const json::exception &e) {
            qWarning() << "JSON parse failed:" << e.what()
                       << "Raw data:" << jsonData;
        }
    }
}

void NetworkClient::onSocketError(QAbstractSocket::SocketError error) {
    emit errorOccurred(error, m_socket->errorString());
}
