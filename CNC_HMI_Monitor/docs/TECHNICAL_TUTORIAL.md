# CNC HMI Monitor 技術教學文檔

本文檔提供邊做邊學的技術指南，涵蓋 Qt 開發、網路通訊、GUI 設計等核心知識。

---

## 目錄

1. [環境建置](#1-環境建置)
2. [Qt 基礎概念](#2-qt-基礎概念)
3. [GUI 開發教學](#3-gui-開發教學)
4. [TCP 網路通訊](#4-tcp-網路通訊)
5. [JSON 數據處理](#5-json-數據處理)
6. [Qt Charts 圖表](#6-qt-charts-圖表)
7. [實戰練習](#7-實戰練習)

---

## 1. 環境建置

### 1.1 安裝 Qt

**Windows 安裝步驟：**

1. 前往 [Qt 官網](https://www.qt.io/download-qt-installer)
2. 下載 Qt Online Installer
3. 執行安裝程式，選擇：
   - Qt 6.x (最新穩定版)
   - MinGW 編譯器 或 MSVC
   - Qt Charts 模組
   - Qt Network 模組

**建議安裝元件：**
```
Qt
├── Qt 6.6.x
│   ├── MinGW 11.2.0 64-bit
│   ├── Qt Charts
│   ├── Qt Network
│   └── Sources (選配)
└── Developer and Designer Tools
    ├── Qt Creator
    ├── MinGW 11.2.0 64-bit
    └── CMake
```

### 1.2 驗證安裝

開啟 Qt Creator，建立測試專案：

1. File → New Project
2. Application (Qt) → Qt Widgets Application
3. 設定專案名稱與路徑
4. 選擇 Build System: CMake
5. 完成後按 Ctrl+R 編譯執行

如果看到空白視窗，環境建置成功！

### 1.3 專案建置指令（命令列）

```bash
# 建立 build 目錄
mkdir build && cd build

# 執行 CMake
cmake ..

# 編譯
cmake --build .

# 或使用 make (Linux)
make
```

---

## 2. Qt 基礎概念

### 2.1 Qt 核心機制：Signal & Slot

Signal/Slot 是 Qt 的核心通訊機制，用於物件間的事件傳遞。

**概念圖：**
```
┌──────────┐  Signal   ┌──────────┐
│ Sender   │ ────────► │ Receiver │
│ Object   │  emit     │ Object   │
└──────────┘           └──────────┘
     │                      │
 clicked()              onClicked()
  (signal)                (slot)
```

**程式碼範例：**

```cpp
// 定義一個帶有 Signal 的類別
class Sender : public QObject {
    Q_OBJECT  // 必須加入這個巨集
public:
    void doSomething() {
        emit dataReady("Hello");  // 發射信號
    }

signals:
    void dataReady(const QString &data);  // 宣告信號
};

// 定義一個帶有 Slot 的類別
class Receiver : public QObject {
    Q_OBJECT
public slots:
    void onDataReady(const QString &data) {  // 宣告槽函數
        qDebug() << "Received:" << data;
    }
};

// 連接 Signal 與 Slot
Sender sender;
Receiver receiver;
connect(&sender, &Sender::dataReady,
        &receiver, &Receiver::onDataReady);
```

### 2.2 Qt 物件樹與記憶體管理

Qt 使用父子關係自動管理記憶體：

```cpp
// parent 刪除時，child 會自動刪除
QWidget *parent = new QWidget();
QPushButton *child = new QPushButton("Click", parent);

delete parent;  // child 也會被自動刪除
```

**重點：**
- 設定 parent 後，不需手動 delete 子物件
- UI 元件通常以 MainWindow 為 parent

### 2.3 常用 Qt 模組

| 模組 | 用途 | 引用方式 |
|------|------|----------|
| Qt Widgets | GUI 元件 | `#include <QWidget>` |
| Qt Network | 網路通訊 | `#include <QTcpSocket>` |
| Qt Charts | 圖表繪製 | `#include <QChart>` |
| Qt Core | 核心功能 | `#include <QString>` |

---

## 3. GUI 開發教學

### 3.1 MainWindow 結構

```cpp
// mainwindow.h
#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QLabel>
#include <QPushButton>

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onConnectClicked();

private:
    void setupUI();

    QLabel *statusLabel;
    QPushButton *connectBtn;
};

#endif
```

### 3.2 Layout 布局系統

Qt 提供多種 Layout 管理元件位置：

**垂直布局 (QVBoxLayout)：**
```cpp
QVBoxLayout *layout = new QVBoxLayout();
layout->addWidget(new QLabel("Line 1"));
layout->addWidget(new QLabel("Line 2"));
layout->addWidget(new QLabel("Line 3"));

// 結果：
// ┌─────────┐
// │ Line 1  │
// │ Line 2  │
// │ Line 3  │
// └─────────┘
```

**水平布局 (QHBoxLayout)：**
```cpp
QHBoxLayout *layout = new QHBoxLayout();
layout->addWidget(new QLabel("A"));
layout->addWidget(new QLabel("B"));
layout->addWidget(new QLabel("C"));

// 結果：
// ┌───┬───┬───┐
// │ A │ B │ C │
// └───┴───┴───┘
```

**格子布局 (QGridLayout)：**
```cpp
QGridLayout *layout = new QGridLayout();
layout->addWidget(new QLabel("X:"), 0, 0);
layout->addWidget(new QLabel("100.00"), 0, 1);
layout->addWidget(new QLabel("Y:"), 1, 0);
layout->addWidget(new QLabel("200.00"), 1, 1);

// 結果：
// ┌────┬────────┐
// │ X: │ 100.00 │
// │ Y: │ 200.00 │
// └────┴────────┘
```

### 3.3 常用 GUI 元件

**QLabel - 文字/圖片顯示：**
```cpp
QLabel *label = new QLabel("Hello World");
label->setAlignment(Qt::AlignCenter);
label->setStyleSheet("font-size: 24px; color: green;");
```

**QPushButton - 按鈕：**
```cpp
QPushButton *btn = new QPushButton("Connect");
connect(btn, &QPushButton::clicked, this, &MainWindow::onConnect);
```

**QLineEdit - 文字輸入：**
```cpp
QLineEdit *input = new QLineEdit();
input->setPlaceholderText("Enter IP address...");
QString text = input->text();  // 取得輸入內容
```

**QGroupBox - 群組框：**
```cpp
QGroupBox *group = new QGroupBox("Machine Status");
QVBoxLayout *layout = new QVBoxLayout(group);
layout->addWidget(new QLabel("Status: Running"));
```

### 3.4 樣式設定 (QSS)

Qt Style Sheet 類似 CSS：

```cpp
// 單一元件樣式
label->setStyleSheet(
    "QLabel {"
    "   background-color: #2d2d2d;"
    "   color: #00ff00;"
    "   font-size: 18px;"
    "   font-family: 'Consolas';"
    "   padding: 10px;"
    "   border-radius: 5px;"
    "}"
);

// 全域樣式設定
qApp->setStyleSheet(
    "QMainWindow { background-color: #1e1e1e; }"
    "QPushButton { background-color: #0078d4; color: white; }"
    "QPushButton:hover { background-color: #1084d8; }"
);
```

### 3.5 定時器 (QTimer)

用於定時更新 UI：

```cpp
#include <QTimer>

// 建立定時器
QTimer *timer = new QTimer(this);

// 連接 timeout 信號
connect(timer, &QTimer::timeout, this, &MainWindow::updateUI);

// 啟動定時器 (每 100ms 觸發一次)
timer->start(100);

// 停止定時器
timer->stop();
```

---

## 4. TCP 網路通訊

### 4.1 TCP 基礎概念

```
┌────────────┐                    ┌────────────┐
│   Client   │                    │   Server   │
├────────────┤                    ├────────────┤
│ 1. 建立    │ ──── SYN ────────► │            │
│    連線    │ ◄─── SYN+ACK ───── │ 2. 接受    │
│            │ ──── ACK ────────► │    連線    │
├────────────┤                    ├────────────┤
│ 3. 發送/   │ ◄──── Data ──────► │ 3. 發送/   │
│    接收    │                    │    接收    │
├────────────┤                    ├────────────┤
│ 4. 關閉    │ ──── FIN ────────► │ 4. 關閉    │
│    連線    │ ◄─── ACK ───────── │    連線    │
└────────────┘                    └────────────┘
```

### 4.2 QTcpSocket (Client 端)

```cpp
#include <QTcpSocket>

class TcpClient : public QObject {
    Q_OBJECT
public:
    TcpClient(QObject *parent = nullptr) : QObject(parent) {
        socket = new QTcpSocket(this);

        // 連接信號
        connect(socket, &QTcpSocket::connected,
                this, &TcpClient::onConnected);
        connect(socket, &QTcpSocket::disconnected,
                this, &TcpClient::onDisconnected);
        connect(socket, &QTcpSocket::readyRead,
                this, &TcpClient::onReadyRead);
        connect(socket, &QTcpSocket::errorOccurred,
                this, &TcpClient::onError);
    }

    void connectToServer(const QString &host, quint16 port) {
        socket->connectToHost(host, port);
    }

    void disconnect() {
        socket->disconnectFromHost();
    }

signals:
    void dataReceived(const QByteArray &data);
    void connectionStateChanged(bool connected);

private slots:
    void onConnected() {
        qDebug() << "Connected to server!";
        emit connectionStateChanged(true);
    }

    void onDisconnected() {
        qDebug() << "Disconnected from server";
        emit connectionStateChanged(false);
    }

    void onReadyRead() {
        QByteArray data = socket->readAll();
        emit dataReceived(data);
    }

    void onError(QAbstractSocket::SocketError error) {
        qDebug() << "Socket error:" << socket->errorString();
    }

private:
    QTcpSocket *socket;
};
```

### 4.3 QTcpServer (Server 端)

```cpp
#include <QTcpServer>
#include <QTcpSocket>

class MockServer : public QObject {
    Q_OBJECT
public:
    MockServer(QObject *parent = nullptr) : QObject(parent) {
        server = new QTcpServer(this);

        connect(server, &QTcpServer::newConnection,
                this, &MockServer::onNewConnection);
    }

    bool start(quint16 port) {
        if (server->listen(QHostAddress::Any, port)) {
            qDebug() << "Server listening on port" << port;
            return true;
        }
        return false;
    }

    void sendToAll(const QByteArray &data) {
        for (QTcpSocket *client : clients) {
            client->write(data);
        }
    }

private slots:
    void onNewConnection() {
        QTcpSocket *client = server->nextPendingConnection();
        clients.append(client);

        connect(client, &QTcpSocket::disconnected, [this, client]() {
            clients.removeOne(client);
            client->deleteLater();
        });

        qDebug() << "New client connected";
    }

private:
    QTcpServer *server;
    QList<QTcpSocket*> clients;
};
```

### 4.4 網路除錯技巧

**使用 netcat 測試：**
```bash
# 啟動簡易 TCP Server (Linux/Mac)
nc -l 8888

# 連接到 Server
nc localhost 8888

# Windows 可使用 telnet
telnet localhost 8888
```

**常見問題：**

| 問題 | 可能原因 | 解決方法 |
|------|----------|----------|
| 連線失敗 | 防火牆阻擋 | 開放對應 Port |
| 收不到資料 | 未連接 readyRead | 檢查 Signal/Slot 連接 |
| 資料不完整 | TCP 分包 | 實作資料緩衝區 |

---

## 5. JSON 數據處理

### 5.1 Qt JSON 類別

Qt 提供完整的 JSON 支援：

- `QJsonDocument` - JSON 文檔
- `QJsonObject` - JSON 物件
- `QJsonArray` - JSON 陣列
- `QJsonValue` - JSON 值

### 5.2 解析 JSON

```cpp
#include <QJsonDocument>
#include <QJsonObject>

void parseData(const QByteArray &data) {
    // 解析 JSON 文檔
    QJsonDocument doc = QJsonDocument::fromJson(data);

    if (doc.isNull()) {
        qDebug() << "Invalid JSON";
        return;
    }

    // 取得根物件
    QJsonObject root = doc.object();

    // 讀取數值
    double posX = root["position"].toObject()["x"].toDouble();
    double posY = root["position"].toObject()["y"].toDouble();
    double posZ = root["position"].toObject()["z"].toDouble();

    int rpm = root["spindle"].toObject()["rpm"].toInt();
    int feedRate = root["feed"].toObject()["rate"].toInt();

    QString state = root["status"].toObject()["state"].toString();

    qDebug() << "Position:" << posX << posY << posZ;
    qDebug() << "Spindle RPM:" << rpm;
    qDebug() << "Status:" << state;
}
```

### 5.3 產生 JSON

```cpp
QByteArray generateMachineData() {
    QJsonObject position;
    position["x"] = 125.350;
    position["y"] = 67.820;
    position["z"] = -15.000;

    QJsonObject spindle;
    spindle["rpm"] = 3000;
    spindle["load"] = 45.5;

    QJsonObject feed;
    feed["rate"] = 500;
    feed["override"] = 100;

    QJsonObject status;
    status["state"] = "RUNNING";
    status["mode"] = "AUTO";
    status["alarm"] = 0;

    QJsonObject root;
    root["timestamp"] = QDateTime::currentMSecsSinceEpoch();
    root["position"] = position;
    root["spindle"] = spindle;
    root["feed"] = feed;
    root["status"] = status;

    QJsonDocument doc(root);
    return doc.toJson(QJsonDocument::Compact);
}
```

---

## 6. Qt Charts 圖表

### 6.1 CMake 設定

```cmake
# 加入 Qt Charts 模組
find_package(Qt6 REQUIRED COMPONENTS Charts)
target_link_libraries(${PROJECT_NAME} PRIVATE Qt6::Charts)
```

### 6.2 基本折線圖

```cpp
#include <QChart>
#include <QChartView>
#include <QLineSeries>
#include <QValueAxis>

using namespace Qt;

QChartView* createChart() {
    // 建立資料序列
    QLineSeries *series = new QLineSeries();
    series->setName("Spindle RPM");

    // 加入資料點
    series->append(0, 2800);
    series->append(1, 3000);
    series->append(2, 3100);
    series->append(3, 2950);
    series->append(4, 3000);

    // 建立圖表
    QChart *chart = new QChart();
    chart->addSeries(series);
    chart->setTitle("Spindle Speed Monitor");
    chart->setAnimationOptions(QChart::NoAnimation);

    // 設定座標軸
    QValueAxis *axisX = new QValueAxis();
    axisX->setRange(0, 60);
    axisX->setTitleText("Time (s)");

    QValueAxis *axisY = new QValueAxis();
    axisY->setRange(0, 5000);
    axisY->setTitleText("RPM");

    chart->addAxis(axisX, Qt::AlignBottom);
    chart->addAxis(axisY, Qt::AlignLeft);
    series->attachAxis(axisX);
    series->attachAxis(axisY);

    // 建立視圖
    QChartView *chartView = new QChartView(chart);
    chartView->setRenderHint(QPainter::Antialiasing);

    return chartView;
}
```

### 6.3 即時更新圖表

```cpp
class RealtimeChart : public QObject {
    Q_OBJECT
public:
    RealtimeChart(QObject *parent = nullptr) : QObject(parent) {
        series = new QLineSeries();
        timeIndex = 0;
        maxPoints = 60;  // 顯示最近60個點
    }

    void addDataPoint(double value) {
        series->append(timeIndex++, value);

        // 移除舊資料點，保持固定數量
        if (series->count() > maxPoints) {
            series->remove(0);
        }

        // 更新 X 軸範圍
        if (timeIndex > maxPoints) {
            axisX->setRange(timeIndex - maxPoints, timeIndex);
        }
    }

private:
    QLineSeries *series;
    QValueAxis *axisX;
    int timeIndex;
    int maxPoints;
};
```

---

## 7. 實戰練習

### 練習 1：Hello Qt (Day 1)

建立一個簡單視窗，顯示 "CNC HMI Monitor"：

```cpp
// main.cpp
#include <QApplication>
#include <QMainWindow>
#include <QLabel>

int main(int argc, char *argv[]) {
    QApplication app(argc, argv);

    QMainWindow window;
    window.setWindowTitle("CNC HMI Monitor");
    window.resize(800, 600);

    QLabel *label = new QLabel("Welcome to CNC HMI Monitor", &window);
    label->setAlignment(Qt::AlignCenter);
    label->setStyleSheet("font-size: 24px;");
    window.setCentralWidget(label);

    window.show();
    return app.exec();
}
```

### 練習 2：座標顯示元件 (Day 2)

```cpp
QWidget* createCoordinateDisplay() {
    QGroupBox *group = new QGroupBox("Position");
    QGridLayout *layout = new QGridLayout(group);

    // 樣式
    QString labelStyle = "font-size: 16px; font-weight: bold;";
    QString valueStyle = "font-size: 20px; font-family: Consolas; color: #00ff00;";

    // X 軸
    QLabel *xLabel = new QLabel("X:");
    xLabel->setStyleSheet(labelStyle);
    QLabel *xValue = new QLabel("0.000 mm");
    xValue->setStyleSheet(valueStyle);

    // Y 軸
    QLabel *yLabel = new QLabel("Y:");
    yLabel->setStyleSheet(labelStyle);
    QLabel *yValue = new QLabel("0.000 mm");
    yValue->setStyleSheet(valueStyle);

    // Z 軸
    QLabel *zLabel = new QLabel("Z:");
    zLabel->setStyleSheet(labelStyle);
    QLabel *zValue = new QLabel("0.000 mm");
    zValue->setStyleSheet(valueStyle);

    layout->addWidget(xLabel, 0, 0);
    layout->addWidget(xValue, 0, 1);
    layout->addWidget(yLabel, 1, 0);
    layout->addWidget(yValue, 1, 1);
    layout->addWidget(zLabel, 2, 0);
    layout->addWidget(zValue, 2, 1);

    return group;
}
```

### 練習 3：TCP Echo Server (Day 3)

```cpp
// 簡單的 Echo Server
void MockServer::onReadyRead() {
    QTcpSocket *client = qobject_cast<QTcpSocket*>(sender());
    if (client) {
        QByteArray data = client->readAll();
        qDebug() << "Received:" << data;
        client->write(data);  // Echo back
    }
}
```

### 練習 4：整合測試 (Day 4-5)

1. 啟動 Mock Server
2. 啟動 HMI Client
3. 點擊「連線」按鈕
4. 觀察數據更新
5. 觀察圖表繪製

---

## 常見問題 FAQ

### Q1: MOC 編譯錯誤

**問題：** `undefined reference to vtable for MyClass`

**原因：** 使用 `Q_OBJECT` 但未正確執行 MOC

**解決：**
```cmake
# CMakeLists.txt 加入
set(CMAKE_AUTOMOC ON)
```

### Q2: 找不到 Qt 模組

**問題：** `fatal error: QChart: No such file or directory`

**解決：** 確認 CMakeLists.txt 有加入對應模組：
```cmake
find_package(Qt6 REQUIRED COMPONENTS Widgets Network Charts)
```

### Q3: Signal/Slot 不觸發

**檢查清單：**
1. 類別是否有 `Q_OBJECT` 巨集
2. `connect()` 是否成功（檢查回傳值）
3. Signal/Slot 參數型別是否匹配
4. 物件是否已被刪除

### Q4: 中文顯示亂碼

**解決：**
```cpp
// 確保原始碼使用 UTF-8 編碼
// 或使用 QString::fromLocal8Bit()
QLabel *label = new QLabel(QString::fromUtf8("中文"));
```

---

## 參考資源

### 官方文檔
- [Qt 6 Documentation](https://doc.qt.io/qt-6/)
- [Qt Network](https://doc.qt.io/qt-6/qtnetwork-index.html)
- [Qt Charts](https://doc.qt.io/qt-6/qtcharts-index.html)

### 教學網站
- [Qt Tutorial - Widgets](https://doc.qt.io/qt-6/qtwidgets-tutorials.html)
- [Qt Examples](https://doc.qt.io/qt-6/qtexamplesandtutorials.html)

### CNC 相關知識
- [G-Code Reference](https://www.cnccookbook.com/g-code-m-code-reference/)
- [CNC Basics](https://www.thomasnet.com/articles/custom-manufacturing-fabricating/understanding-cnc-machining/)

---

*文檔建立日期：2026-02-02*
*版本：1.0*
