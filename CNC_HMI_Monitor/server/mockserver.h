#ifndef MOCKSERVER_H
#define MOCKSERVER_H

#include <QObject>
#include <QTcpServer>
#include <QTcpSocket>
#include <QTimer>
#include <QList>
#include <QRandomGenerator>

/**
 * @brief Mock Server 類別
 * 模擬 CNC 機台，定時發送機台數據給連線的客戶端
 */
class MockServer : public QObject
{
    Q_OBJECT

public:
    explicit MockServer(QObject *parent = nullptr);
    ~MockServer();

    /**
     * @brief 啟動伺服器
     * @param port 監聽的連接埠
     * @return 啟動是否成功
     */
    bool start(quint16 port);

    /**
     * @brief 停止伺服器
     */
    void stop();

    /**
     * @brief 檢查伺服器是否正在運行
     * @return 運行狀態
     */
    bool isRunning() const;

private slots:
    void onNewConnection();
    void onClientDisconnected();
    void onSendData();

private:
    /**
     * @brief 產生模擬的機台數據 (JSON 格式)
     * @return JSON 格式的位元組陣列
     */
    QByteArray generateMachineData();

    /**
     * @brief 發送資料給所有連線的客戶端
     * @param data 要發送的資料
     */
    void broadcastToClients(const QByteArray &data);

    QTcpServer *m_server;
    QList<QTcpSocket*> m_clients;
    QTimer *m_dataTimer;

    // 模擬數據狀態
    double m_posX;
    double m_posY;
    double m_posZ;
    int m_spindleRpm;
    int m_feedRate;
    int m_state;  // 0: IDLE, 1: RUNNING, 2: PAUSED
    int m_cycleCount;
};

#endif // MOCKSERVER_H
