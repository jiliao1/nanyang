#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>

class TcpClient : public QObject
{
    Q_OBJECT
public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();

signals:
    // 当成功连接到服务器时发出
    void connectedToServer();
    // 当与服务器断开连接时发出
    void disconnectedFromServer();
    // (核心信号) 当成功接收并解析出一条完整的JSON数据时发出
    void jsonDataReceived(const QByteArray &jsonData);
    // 用于向UI发送状态消息
    void statusMessage(const QString &message);

public slots:
    // 连接到服务器
    void connectToServer(const QString &ipAddress, quint16 port);
    // 主动断开连接
    void disconnectFromServer();

private slots:
    // 当socket连接成功时被调用
    void onConnected();
    // 当socket断开连接时被调用
    void onDisconnected();
    // 当socket有新数据可读时被调用
    void onReadyRead();
    // 当socket发生错误时被调用
    void onError(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_tcpSocket; // 负责所有TCP通信的socket
    QByteArray m_buffer;     // 接收数据的缓冲区，用于处理“粘包”和“半包”
    qint32 m_expectedDataSize; // 期望接收的数据体大小，从数据头中解析得到
};

#endif // TCPCLIENT_H

