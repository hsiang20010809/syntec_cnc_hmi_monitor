#include "tcpclient.h"
#include <QDebug>

TcpClient::TcpClient(QObject *parent)
    : QObject(parent)
    , m_socket(new QTcpSocket(this))
{
    // 連接 Socket 信號到內部槽函數
    connect(m_socket, &QTcpSocket::connected,
            this, &TcpClient::onConnected);
    connect(m_socket, &QTcpSocket::disconnected,
            this, &TcpClient::onDisconnected);
    connect(m_socket, &QTcpSocket::readyRead,
            this, &TcpClient::onReadyRead);
    connect(m_socket, &QTcpSocket::errorOccurred,
            this, &TcpClient::onErrorOccurred);
}

TcpClient::~TcpClient()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

void TcpClient::connectToServer(const QString &host, quint16 port)
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        qWarning() << "Already connected, disconnect first";
        return;
    }

    qDebug() << "Connecting to" << host << ":" << port;
    m_socket->connectToHost(host, port);
}

void TcpClient::disconnectFromServer()
{
    if (m_socket->state() == QAbstractSocket::ConnectedState) {
        m_socket->disconnectFromHost();
    }
}

bool TcpClient::isConnected() const
{
    return m_socket->state() == QAbstractSocket::ConnectedState;
}

void TcpClient::sendData(const QByteArray &data)
{
    if (!isConnected()) {
        qWarning() << "Not connected, cannot send data";
        return;
    }
    m_socket->write(data);
}

void TcpClient::onConnected()
{
    qDebug() << "Connected to server";
    m_buffer.clear();
    emit connected();
}

void TcpClient::onDisconnected()
{
    qDebug() << "Disconnected from server";
    m_buffer.clear();
    emit disconnected();
}

void TcpClient::onReadyRead()
{
    // 讀取所有可用資料到緩衝區
    m_buffer.append(m_socket->readAll());

    // 處理緩衝區中的完整 JSON 資料
    // 假設每筆資料以換行符分隔
    while (true) {
        int newlineIndex = m_buffer.indexOf('\n');
        if (newlineIndex == -1) {
            break;  // 沒有完整的一行資料
        }

        // 提取一行完整資料
        QByteArray line = m_buffer.left(newlineIndex);
        m_buffer.remove(0, newlineIndex + 1);

        if (!line.isEmpty()) {
            emit dataReceived(line);
        }
    }
}

void TcpClient::onErrorOccurred(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
    QString errorMsg = m_socket->errorString();
    qWarning() << "Socket error:" << errorMsg;
    emit errorOccurred(errorMsg);
}
