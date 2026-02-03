#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>
#include <QLineEdit>
#include <QGroupBox>
#include <QTimer>
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>

#include "tcpclient.h"
#include "machinedata.h"

/**
 * @brief 主視窗類別
 * CNC HMI 監控介面的主要視窗
 */
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    // 按鈕事件
    void onConnectClicked();
    void onDisconnectClicked();

    // 網路事件
    void onTcpConnected();
    void onTcpDisconnected();
    void onTcpDataReceived(const QByteArray &data);
    void onTcpError(const QString &errorString);

private:
    // UI 建立函數
    void setupUI();
    QGroupBox* createStatusPanel();
    QGroupBox* createCoordinatePanel();
    QGroupBox* createControlPanel();
    void setupChartPanel();
    void applyStyles();

    // UI 更新函數
    void updateUI(const MachineData &data);
    void updateStatusIndicator(MachineState state);
    void updateChart(int rpm);
    void setConnectionState(bool connected);

    // 網路元件
    TcpClient *m_tcpClient;

    // 機台數據
    MachineData m_machineData;

    // UI 元件 - 狀態面板
    QLabel *m_statusIndicator;
    QLabel *m_statusLabel;
    QLabel *m_spindleRpmLabel;
    QLabel *m_feedRateLabel;
    QLabel *m_modeLabel;

    // UI 元件 - 座標顯示
    QLabel *m_posXLabel;
    QLabel *m_posYLabel;
    QLabel *m_posZLabel;

    // UI 元件 - 控制面板
    QLineEdit *m_hostInput;
    QLineEdit *m_portInput;
    QPushButton *m_connectBtn;
    QPushButton *m_disconnectBtn;
    QLabel *m_connectionStatusLabel;

    // UI 元件 - 圖表
    QChart *m_chart;
    QChartView *m_chartView;
    QLineSeries *m_spindleSeries;
    QValueAxis *m_axisX;
    QValueAxis *m_axisY;
    int m_chartTimeIndex;
    static const int MAX_CHART_POINTS = 60;
};

#endif // MAINWINDOW_H
