#ifndef TCPCLIENT_H
#define TCPCLIENT_H

#include <QObject>
#include <QTcpSocket>
#include <QHostAddress>

/**
 * @brief TCP 客戶端類別
 * 負責與 Mock Server 建立連線並接收機台數據
 */
class TcpClient : public QObject
{
    Q_OBJECT

public:
    explicit TcpClient(QObject *parent = nullptr);
    ~TcpClient();

    /**
     * @brief 連線到伺服器
     * @param host 主機位址
     * @param port 連接埠
     */
    void connectToServer(const QString &host, quint16 port);

    /**
     * @brief 斷開連線
     */
    void disconnectFromServer();

    /**
     * @brief 檢查是否已連線
     * @return 連線狀態
     */
    bool isConnected() const;

    /**
     * @brief 發送資料到伺服器
     * @param data 要發送的資料
     */
    void sendData(const QByteArray &data);

signals:
    /**
     * @brief 連線成功信號
     */
    void connected();

    /**
     * @brief 斷線信號
     */
    void disconnected();

    /**
     * @brief 接收到資料信號
     * @param data 接收到的資料
     */
    void dataReceived(const QByteArray &data);

    /**
     * @brief 錯誤發生信號
     * @param errorString 錯誤訊息
     */
    void errorOccurred(const QString &errorString);

private slots:
    void onConnected();
    void onDisconnected();
    void onReadyRead();
    void onErrorOccurred(QAbstractSocket::SocketError socketError);

private:
    QTcpSocket *m_socket;
    QByteArray m_buffer;  // 資料緩衝區，處理 TCP 分包
};

#endif // TCPCLIENT_H
