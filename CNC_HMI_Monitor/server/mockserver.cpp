#include "mockserver.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDateTime>
#include <QDebug>
#include <QtMath>

MockServer::MockServer(QObject *parent)
    : QObject(parent)
    , m_server(new QTcpServer(this))
    , m_dataTimer(new QTimer(this))
    , m_posX(0.0)
    , m_posY(0.0)
    , m_posZ(0.0)
    , m_spindleRpm(0)
    , m_feedRate(0)
    , m_state(0)
    , m_cycleCount(0)
{
    // 連接伺服器信號
    connect(m_server, &QTcpServer::newConnection,
            this, &MockServer::onNewConnection);

    // 連接定時器信號 (每 100ms 發送一次數據)
    connect(m_dataTimer, &QTimer::timeout,
            this, &MockServer::onSendData);
}

MockServer::~MockServer()
{
    stop();
}

bool MockServer::start(quint16 port)
{
    if (m_server->isListening()) {
        qWarning() << "Server is already running";
        return false;
    }

    if (!m_server->listen(QHostAddress::Any, port)) {
        qWarning() << "Failed to start server:" << m_server->errorString();
        return false;
    }

    qDebug() << "===========================================";
    qDebug() << "CNC Mock Server started on port" << port;
    qDebug() << "Waiting for client connections...";
    qDebug() << "===========================================";

    // 啟動數據發送定時器
    m_dataTimer->start(100);  // 10 Hz

    return true;
}

void MockServer::stop()
{
    m_dataTimer->stop();

    // 關閉所有客戶端連線
    for (QTcpSocket *client : m_clients) {
        client->disconnectFromHost();
    }
    m_clients.clear();

    if (m_server->isListening()) {
        m_server->close();
        qDebug() << "Server stopped";
    }
}

bool MockServer::isRunning() const
{
    return m_server->isListening();
}

void MockServer::onNewConnection()
{
    QTcpSocket *client = m_server->nextPendingConnection();
    if (!client) {
        return;
    }

    m_clients.append(client);

    connect(client, &QTcpSocket::disconnected,
            this, &MockServer::onClientDisconnected);

    qDebug() << "New client connected from"
             << client->peerAddress().toString()
             << ":" << client->peerPort();
    qDebug() << "Total clients:" << m_clients.size();
}

void MockServer::onClientDisconnected()
{
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        m_clients.removeOne(client);
        client->deleteLater();
        qDebug() << "Client disconnected. Remaining clients:" << m_clients.size();
    }
}

void MockServer::onSendData()
{
    if (m_clients.isEmpty()) {
        return;
    }

    QByteArray data = generateMachineData();
    broadcastToClients(data);
}

QByteArray MockServer::generateMachineData()
{
    m_cycleCount++;

    // 模擬加工週期
    // 每 100 個週期切換一次狀態
    if (m_cycleCount % 200 == 0) {
        m_state = (m_state + 1) % 3;  // 循環: IDLE -> RUNNING -> PAUSED -> IDLE
    }

    // 根據狀態更新數據
    QString stateStr;
    switch (m_state) {
    case 0:  // IDLE
        stateStr = "IDLE";
        m_spindleRpm = 0;
        m_feedRate = 0;
        break;
    case 1: {  // RUNNING
        stateStr = "RUNNING";
        // 模擬主軸轉速變化 (2800-3200 RPM)
        m_spindleRpm = 3000 + QRandomGenerator::global()->bounded(-200, 201);
        // 模擬進給速率
        m_feedRate = 500 + QRandomGenerator::global()->bounded(-50, 51);
        // 模擬座標移動 (圓形路徑)
        double angle = m_cycleCount * 0.05;
        m_posX = 100.0 + 50.0 * qCos(angle);
        m_posY = 100.0 + 50.0 * qSin(angle);
        m_posZ = -10.0 + 5.0 * qSin(angle * 0.5);
        break;
    }
    case 2:  // PAUSED
        stateStr = "PAUSED";
        // 主軸減速
        m_spindleRpm = qMax(0, m_spindleRpm - 100);
        m_feedRate = 0;
        break;
    }

    // 建立 JSON 物件
    QJsonObject position;
    position["x"] = qRound(m_posX * 1000.0) / 1000.0;  // 保留3位小數
    position["y"] = qRound(m_posY * 1000.0) / 1000.0;
    position["z"] = qRound(m_posZ * 1000.0) / 1000.0;

    QJsonObject spindle;
    spindle["rpm"] = m_spindleRpm;
    spindle["load"] = 30.0 + QRandomGenerator::global()->bounded(0, 30);

    QJsonObject feed;
    feed["rate"] = m_feedRate;
    feed["override"] = 100;

    QJsonObject status;
    status["state"] = stateStr;
    status["mode"] = "AUTO";
    status["alarm"] = 0;

    QJsonObject root;
    root["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    root["position"] = position;
    root["spindle"] = spindle;
    root["feed"] = feed;
    root["status"] = status;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Compact) + "\n";  // 加上換行符作為分隔
}

void MockServer::broadcastToClients(const QByteArray &data)
{
    for (QTcpSocket *client : m_clients) {
        if (client->state() == QAbstractSocket::ConnectedState) {
            client->write(data);
        }
    }
}
