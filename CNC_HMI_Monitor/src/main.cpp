#include <QApplication>
#include "mainwindow.h"

/**
 * @brief CNC HMI Monitor 主程式進入點
 *
 * 本程式是一個 CNC 設備監控的人機介面 (HMI)，
 * 透過 TCP 網路連線接收機台即時數據並視覺化顯示。
 */
int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // 設定應用程式資訊
    QApplication::setApplicationName("CNC HMI Monitor");
    QApplication::setApplicationVersion("1.0.0");
    QApplication::setOrganizationName("Interview Project");

    // 建立並顯示主視窗
    MainWindow mainWindow;
    mainWindow.show();

    return app.exec();
}
