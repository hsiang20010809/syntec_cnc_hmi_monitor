#include "mainwindow.h"

#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QFrame>
#include <QMessageBox>
#include <QDebug>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , m_tcpClient(new TcpClient(this))
    , m_chartTimeIndex(0)
{
    setWindowTitle("CNC Machine Monitor HMI");
    setMinimumSize(900, 700);

    setupUI();
    applyStyles();

    // 連接 TCP Client 信號
    connect(m_tcpClient, &TcpClient::connected,
            this, &MainWindow::onTcpConnected);
    connect(m_tcpClient, &TcpClient::disconnected,
            this, &MainWindow::onTcpDisconnected);
    connect(m_tcpClient, &TcpClient::dataReceived,
            this, &MainWindow::onTcpDataReceived);
    connect(m_tcpClient, &TcpClient::errorOccurred,
            this, &MainWindow::onTcpError);

    // 初始狀態
    setConnectionState(false);
}

MainWindow::~MainWindow()
{
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);
    mainLayout->setSpacing(15);
    mainLayout->setContentsMargins(20, 20, 20, 20);

    // 標題
    QLabel *titleLabel = new QLabel("CNC Machine Monitor");
    titleLabel->setObjectName("titleLabel");
    titleLabel->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(titleLabel);

    // 上半部：狀態面板 + 座標顯示
    QHBoxLayout *topLayout = new QHBoxLayout();
    topLayout->setSpacing(15);

    topLayout->addWidget(createStatusPanel(), 1);
    topLayout->addWidget(createCoordinatePanel(), 1);
    mainLayout->addLayout(topLayout);

    // 中間：圖表
    setupChartPanel();
    mainLayout->addWidget(m_chartView, 1);

    // 底部：控制面板
    mainLayout->addWidget(createControlPanel());
}

QGroupBox* MainWindow::createStatusPanel()
{
    QGroupBox *group = new QGroupBox("Machine Status", this);

    QGridLayout *layout = new QGridLayout(group);
    layout->setSpacing(10);

    // 狀態指示燈
    m_statusIndicator = new QLabel();
    m_statusIndicator->setFixedSize(30, 30);
    m_statusIndicator->setObjectName("statusIndicator");

    m_statusLabel = new QLabel("IDLE");
    m_statusLabel->setObjectName("statusValue");

    layout->addWidget(new QLabel("Status:"), 0, 0);
    layout->addWidget(m_statusIndicator, 0, 1);
    layout->addWidget(m_statusLabel, 0, 2);

    // 主軸轉速
    layout->addWidget(new QLabel("Spindle:"), 1, 0);
    m_spindleRpmLabel = new QLabel("0 RPM");
    m_spindleRpmLabel->setObjectName("valueLabel");
    layout->addWidget(m_spindleRpmLabel, 1, 1, 1, 2);

    // 進給速率
    layout->addWidget(new QLabel("Feed Rate:"), 2, 0);
    m_feedRateLabel = new QLabel("0 mm/min");
    m_feedRateLabel->setObjectName("valueLabel");
    layout->addWidget(m_feedRateLabel, 2, 1, 1, 2);

    // 運行模式
    layout->addWidget(new QLabel("Mode:"), 3, 0);
    m_modeLabel = new QLabel("AUTO");
    m_modeLabel->setObjectName("valueLabel");
    layout->addWidget(m_modeLabel, 3, 1, 1, 2);

    layout->setColumnStretch(2, 1);
    return group;
}

QGroupBox* MainWindow::createCoordinatePanel()
{
    QGroupBox *group = new QGroupBox("Position", this);

    QGridLayout *layout = new QGridLayout(group);
    layout->setSpacing(15);

    // X 軸
    QLabel *xTitle = new QLabel("X");
    xTitle->setObjectName("axisTitle");
    m_posXLabel = new QLabel("0.000");
    m_posXLabel->setObjectName("coordValue");
    QLabel *xUnit = new QLabel("mm");
    xUnit->setObjectName("unitLabel");

    layout->addWidget(xTitle, 0, 0);
    layout->addWidget(m_posXLabel, 0, 1);
    layout->addWidget(xUnit, 0, 2);

    // Y 軸
    QLabel *yTitle = new QLabel("Y");
    yTitle->setObjectName("axisTitle");
    m_posYLabel = new QLabel("0.000");
    m_posYLabel->setObjectName("coordValue");
    QLabel *yUnit = new QLabel("mm");
    yUnit->setObjectName("unitLabel");

    layout->addWidget(yTitle, 1, 0);
    layout->addWidget(m_posYLabel, 1, 1);
    layout->addWidget(yUnit, 1, 2);

    // Z 軸
    QLabel *zTitle = new QLabel("Z");
    zTitle->setObjectName("axisTitle");
    m_posZLabel = new QLabel("0.000");
    m_posZLabel->setObjectName("coordValue");
    QLabel *zUnit = new QLabel("mm");
    zUnit->setObjectName("unitLabel");

    layout->addWidget(zTitle, 2, 0);
    layout->addWidget(m_posZLabel, 2, 1);
    layout->addWidget(zUnit, 2, 2);

    layout->setColumnStretch(1, 1);
    return group;
}

QGroupBox* MainWindow::createControlPanel()
{
    QGroupBox *group = new QGroupBox("Connection", this);

    QHBoxLayout *layout = new QHBoxLayout(group);
    layout->setSpacing(15);

    // 主機輸入
    layout->addWidget(new QLabel("Host:"));
    m_hostInput = new QLineEdit("127.0.0.1");
    m_hostInput->setFixedWidth(120);
    layout->addWidget(m_hostInput);

    // 連接埠輸入
    layout->addWidget(new QLabel("Port:"));
    m_portInput = new QLineEdit("8888");
    m_portInput->setFixedWidth(80);
    layout->addWidget(m_portInput);

    // 連線按鈕
    m_connectBtn = new QPushButton("Connect");
    m_connectBtn->setObjectName("connectBtn");
    connect(m_connectBtn, &QPushButton::clicked,
            this, &MainWindow::onConnectClicked);
    layout->addWidget(m_connectBtn);

    // 斷線按鈕
    m_disconnectBtn = new QPushButton("Disconnect");
    m_disconnectBtn->setObjectName("disconnectBtn");
    connect(m_disconnectBtn, &QPushButton::clicked,
            this, &MainWindow::onDisconnectClicked);
    layout->addWidget(m_disconnectBtn);

    layout->addStretch();

    // 連線狀態顯示
    m_connectionStatusLabel = new QLabel("Disconnected");
    m_connectionStatusLabel->setObjectName("connectionStatus");
    layout->addWidget(m_connectionStatusLabel);
    return group;
}

void MainWindow::setupChartPanel()
{
    // 建立資料序列
    m_spindleSeries = new QLineSeries();
    m_spindleSeries->setName("Spindle RPM");

    // 建立圖表
    m_chart = new QChart();
    m_chart->addSeries(m_spindleSeries);
    m_chart->setTitle("Spindle Speed Monitor");
    m_chart->setAnimationOptions(QChart::NoAnimation);
    m_chart->legend()->setVisible(true);
    m_chart->legend()->setAlignment(Qt::AlignBottom);

    // 設定 X 軸 (時間)
    m_axisX = new QValueAxis();
    m_axisX->setRange(0, MAX_CHART_POINTS);
    m_axisX->setTitleText("Time (s)");
    m_axisX->setLabelFormat("%d");
    m_chart->addAxis(m_axisX, Qt::AlignBottom);
    m_spindleSeries->attachAxis(m_axisX);

    // 設定 Y 軸 (RPM)
    m_axisY = new QValueAxis();
    m_axisY->setRange(0, 5000);
    m_axisY->setTitleText("RPM");
    m_axisY->setLabelFormat("%d");
    m_chart->addAxis(m_axisY, Qt::AlignLeft);
    m_spindleSeries->attachAxis(m_axisY);

    // 建立圖表視圖
    m_chartView = new QChartView(m_chart);
    m_chartView->setRenderHint(QPainter::Antialiasing);
    m_chartView->setMinimumHeight(250);
}

void MainWindow::applyStyles()
{
    // 全域樣式
    QString globalStyle = R"(
        QMainWindow {
            background-color: #1e1e1e;
        }
        QGroupBox {
            font-size: 14px;
            font-weight: bold;
            color: #ffffff;
            border: 2px solid #3d3d3d;
            border-radius: 8px;
            margin-top: 10px;
            padding-top: 10px;
        }
        QGroupBox::title {
            subcontrol-origin: margin;
            left: 15px;
            padding: 0 5px;
        }
        QLabel {
            color: #cccccc;
            font-size: 13px;
        }
        QLineEdit {
            background-color: #2d2d2d;
            color: #ffffff;
            border: 1px solid #3d3d3d;
            border-radius: 4px;
            padding: 5px;
            font-size: 13px;
        }
        QLineEdit:focus {
            border-color: #0078d4;
        }
        QPushButton {
            background-color: #0078d4;
            color: #ffffff;
            border: none;
            border-radius: 4px;
            padding: 8px 20px;
            font-size: 13px;
            font-weight: bold;
        }
        QPushButton:hover {
            background-color: #1084d8;
        }
        QPushButton:pressed {
            background-color: #006cbd;
        }
        QPushButton:disabled {
            background-color: #4d4d4d;
            color: #808080;
        }
        #disconnectBtn {
            background-color: #d41a1a;
        }
        #disconnectBtn:hover {
            background-color: #e02020;
        }
        #titleLabel {
            font-size: 28px;
            font-weight: bold;
            color: #ffffff;
            padding: 10px;
        }
        #statusValue {
            font-size: 18px;
            font-weight: bold;
            color: #00ff00;
        }
        #valueLabel {
            font-size: 16px;
            color: #00ccff;
            font-family: 'Consolas', monospace;
        }
        #axisTitle {
            font-size: 20px;
            font-weight: bold;
            color: #ffcc00;
        }
        #coordValue {
            font-size: 28px;
            font-weight: bold;
            color: #00ff00;
            font-family: 'Consolas', monospace;
        }
        #unitLabel {
            font-size: 14px;
            color: #808080;
        }
        #statusIndicator {
            border-radius: 15px;
            background-color: #808080;
        }
        #connectionStatus {
            font-size: 14px;
            font-weight: bold;
            padding: 5px 15px;
            border-radius: 4px;
        }
    )";

    setStyleSheet(globalStyle);

    // 圖表樣式
    m_chart->setBackgroundBrush(QBrush(QColor("#2d2d2d")));
    m_chart->setTitleBrush(QBrush(QColor("#ffffff")));
    m_chart->setTitleFont(QFont("Arial", 12, QFont::Bold));

    m_axisX->setLabelsColor(QColor("#cccccc"));
    m_axisX->setTitleBrush(QBrush(QColor("#cccccc")));
    m_axisX->setGridLineColor(QColor("#3d3d3d"));

    m_axisY->setLabelsColor(QColor("#cccccc"));
    m_axisY->setTitleBrush(QBrush(QColor("#cccccc")));
    m_axisY->setGridLineColor(QColor("#3d3d3d"));

    QPen seriesPen(QColor("#00ff00"));
    seriesPen.setWidth(2);
    m_spindleSeries->setPen(seriesPen);
}

void MainWindow::onConnectClicked()
{
    QString host = m_hostInput->text();
    quint16 port = m_portInput->text().toUShort();

    if (host.isEmpty() || port == 0) {
        QMessageBox::warning(this, "Error", "Please enter valid host and port");
        return;
    }

    m_tcpClient->connectToServer(host, port);
    m_connectBtn->setEnabled(false);
    m_connectionStatusLabel->setText("Connecting...");
}

void MainWindow::onDisconnectClicked()
{
    m_tcpClient->disconnectFromServer();
}

void MainWindow::onTcpConnected()
{
    setConnectionState(true);
}

void MainWindow::onTcpDisconnected()
{
    setConnectionState(false);
}

void MainWindow::onTcpDataReceived(const QByteArray &data)
{
    if (m_machineData.parseFromJson(data)) {
        updateUI(m_machineData);
    }
}

void MainWindow::onTcpError(const QString &errorString)
{
    setConnectionState(false);
    QMessageBox::warning(this, "Connection Error", errorString);
}

void MainWindow::updateUI(const MachineData &data)
{
    // 更新狀態
    updateStatusIndicator(data.state);
    m_statusLabel->setText(data.getStateString());
    m_statusLabel->setStyleSheet(QString("color: %1; font-size: 18px; font-weight: bold;")
                                      .arg(data.getStateColor()));

    // 更新主軸與進給
    m_spindleRpmLabel->setText(QString("%1 RPM").arg(data.spindleRpm));
    m_feedRateLabel->setText(QString("%1 mm/min").arg(data.feedRate));
    m_modeLabel->setText(data.mode);

    // 更新座標
    m_posXLabel->setText(QString::number(data.posX, 'f', 3));
    m_posYLabel->setText(QString::number(data.posY, 'f', 3));
    m_posZLabel->setText(QString::number(data.posZ, 'f', 3));

    // 更新圖表
    updateChart(data.spindleRpm);
}

void MainWindow::updateStatusIndicator(MachineState state)
{
    QString color;
    switch (state) {
    case MachineState::IDLE:
        color = "#808080";
        break;
    case MachineState::RUNNING:
        color = "#00ff00";
        break;
    case MachineState::PAUSED:
        color = "#ffff00";
        break;
    case MachineState::ERROR:
    case MachineState::ESTOP:
        color = "#ff0000";
        break;
    default:
        color = "#ffffff";
    }

    m_statusIndicator->setStyleSheet(QString(
        "border-radius: 15px; background-color: %1;"
    ).arg(color));
}

void MainWindow::updateChart(int rpm)
{
    m_spindleSeries->append(m_chartTimeIndex++, rpm);

    // 移除舊資料點
    if (m_spindleSeries->count() > MAX_CHART_POINTS) {
        m_spindleSeries->remove(0);
    }

    // 更新 X 軸範圍
    if (m_chartTimeIndex > MAX_CHART_POINTS) {
        m_axisX->setRange(m_chartTimeIndex - MAX_CHART_POINTS, m_chartTimeIndex);
    }
}

void MainWindow::setConnectionState(bool connected)
{
    m_connectBtn->setEnabled(!connected);
    m_disconnectBtn->setEnabled(connected);
    m_hostInput->setEnabled(!connected);
    m_portInput->setEnabled(!connected);

    if (connected) {
        m_connectionStatusLabel->setText("Connected");
        m_connectionStatusLabel->setStyleSheet(
            "color: #00ff00; background-color: #1a3d1a; "
            "padding: 5px 15px; border-radius: 4px; font-weight: bold;"
        );
    } else {
        m_connectionStatusLabel->setText("Disconnected");
        m_connectionStatusLabel->setStyleSheet(
            "color: #ff6666; background-color: #3d1a1a; "
            "padding: 5px 15px; border-radius: 4px; font-weight: bold;"
        );
        m_connectBtn->setEnabled(true);
    }
}
