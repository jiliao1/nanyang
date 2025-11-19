#include "tcpclient.h"
#include <QDataStream>
#include <QHostAddress>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent), m_expectedDataSize(0)
{
    m_tcpSocket = new QTcpSocket(this);

    // 连接QTcpSocket内置的信号到我们自己的处理槽函数
    connect(m_tcpSocket, &QTcpSocket::connected, this, &TcpClient::onConnected);
    connect(m_tcpSocket, &QTcpSocket::disconnected, this, &TcpClient::onDisconnected);
    connect(m_tcpSocket, &QTcpSocket::readyRead, this, &TcpClient::onReadyRead);
    // 注意：onError是Qt5的旧式信号槽语法，对于重载函数这样写更清晰
    connect(m_tcpSocket, QOverload<QAbstractSocket::SocketError>::of(&QAbstractSocket::error), this, &TcpClient::onError);
}

TcpClient::~TcpClient()
{
}

void TcpClient::connectToServer(const QString &ipAddress, quint16 port)
{
    if (m_tcpSocket->state() == QAbstractSocket::UnconnectedState) {
        emit statusMessage(QString("正在连接到 %1:%2...").arg(ipAddress).arg(port));
        m_tcpSocket->connectToHost(ipAddress, port);
    }
}

void TcpClient::disconnectFromServer()
{
    if (m_tcpSocket->state() != QAbstractSocket::UnconnectedState) {
        m_tcpSocket->disconnectFromHost();
    }
}

void TcpClient::onConnected()
{
    emit statusMessage("已成功连接到服务器。");
    emit connectedToServer();
}

void TcpClient::onDisconnected()
{
    emit statusMessage("已与服务器断开连接。");
    emit disconnectedFromServer();
}

void TcpClient::onError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError);
    emit statusMessage("网络错误: " + m_tcpSocket->errorString());
}


// --- 这是最核心的数据接收和解析逻辑 ---
void TcpClient::onReadyRead()
{
    // 将socket缓冲区中所有可读的数据追加到我们自己的缓冲区m_buffer中
    m_buffer.append(m_tcpSocket->readAll());

    // 循环处理，因为缓冲区中可能有多条完整的数据（粘包）
    while (true) {
        // --- 第一步：解析数据头 ---
        // 如果我们还不知道下一条数据的大小 (m_expectedDataSize为0)
        // 并且缓冲区大小足够容纳一个4字节的整数头
        if (m_expectedDataSize == 0 && m_buffer.size() >= sizeof(qint32))
        {
            // 使用QDataStream来安全地从字节数组中解析出整数
            QDataStream stream(m_buffer.left(sizeof(qint32)));
            stream.setByteOrder(QDataStream::BigEndian); // 假设服务器用大端字节序，需与服务器端统一
            stream >> m_expectedDataSize;

            // 从缓冲区中移除已经处理过的数据头
            m_buffer.remove(0, sizeof(qint32));
        }

        // --- 第二步：解析数据体 ---
        // 如果我们已经知道了数据大小，并且缓冲区中已经接收到了完整的数据体
        if (m_expectedDataSize > 0 && m_buffer.size() >= m_expectedDataSize)
        {
            // 提取出一条完整的JSON数据
            QByteArray jsonData = m_buffer.left(m_expectedDataSize);

            // 从缓冲区中移除已经提取的数据
            m_buffer.remove(0, m_expectedDataSize);

            // 重置期望大小，准备接收下一条数据
            m_expectedDataSize = 0;

            // 发出信号，通知其他模块我们收到了一条完整的JSON数据
            emit jsonDataReceived(jsonData);
        }
        else
        {
            // 如果数据不满足处理条件（头部或身体不完整），则退出循环，等待下一次readyRead信号
            break;
        }
    }
}
