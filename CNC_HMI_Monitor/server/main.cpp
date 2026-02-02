#include <QCoreApplication>
#include <QCommandLineParser>
#include <QDebug>
#include "mockserver.h"

/**
 * @brief CNC Mock Server 主程式進入點
 *
 * 此程式模擬 CNC 機台控制器，
 * 定時發送模擬的機台數據給連線的 HMI 客戶端。
 *
 * 使用方式:
 *   CNC_Mock_Server [port]
 *   預設連接埠: 8888
 */
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);

    // 設定應用程式資訊
    QCoreApplication::setApplicationName("CNC Mock Server");
    QCoreApplication::setApplicationVersion("1.0.0");

    // 命令列參數解析
    QCommandLineParser parser;
    parser.setApplicationDescription("CNC Machine Data Simulator Server");
    parser.addHelpOption();
    parser.addVersionOption();

    // 連接埠參數
    parser.addPositionalArgument("port", "Server port (default: 8888)");

    parser.process(app);

    // 取得連接埠
    quint16 port = 8888;
    const QStringList args = parser.positionalArguments();
    if (!args.isEmpty()) {
        bool ok;
        quint16 customPort = args.first().toUShort(&ok);
        if (ok && customPort > 0) {
            port = customPort;
        }
    }

    // 建立並啟動伺服器
    MockServer server;
    if (!server.start(port)) {
        qCritical() << "Failed to start server on port" << port;
        return 1;
    }

    qDebug() << "";
    qDebug() << "Press Ctrl+C to stop the server";
    qDebug() << "";

    return app.exec();
}
