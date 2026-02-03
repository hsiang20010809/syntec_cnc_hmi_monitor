# CNC HMI Monitor 完整解析 - 從零開始理解

寫給完全沒用過 Qt、不熟悉 GUI 程式的你。

---

## 目錄

1. [這個程式到底在幹嘛？](#1-這個程式到底在幹嘛)
2. [整體架構：兩個程式一起跑](#2-整體架構兩個程式一起跑)
3. [CMakeLists.txt：為什麼 Qt Creator 只要讀它？](#3-cmakeliststxt為什麼-qt-creator-只要讀它)
4. [Qt 如何讓程式跑起來？](#4-qt-如何讓程式跑起來)
5. [Signal/Slot：Qt 的靈魂機制](#5-signalslotqt-的靈魂機制)
6. [程式運作流程：從啟動到畫面更新](#6-程式運作流程從啟動到畫面更新)
7. [TCP 網路通訊：兩個程式怎麼對話？](#7-tcp-網路通訊兩個程式怎麼對話)
8. [各檔案職責一覽](#8-各檔案職責一覽)
9. [必讀關鍵程式碼](#9-必讀關鍵程式碼)
10. [開發過程遇到的問題與解法](#10-開發過程遇到的問題與解法)
11. [面試時怎麼講這個專案](#11-面試時怎麼講這個專案)

---

## 1. 這個程式到底在幹嘛？

### 一句話版本

> 模擬一台 CNC 工具機，透過網路把機台數據傳到另一個程式，那個程式把數據「畫」成人看得懂的監控畫面。

### 白話版本

想像你在工廠裡：

```
真實世界：
┌────────────┐                    ┌────────────┐
│  CNC 工具機 │ ──── 網路線 ────► │  監控螢幕   │
│  (切削金屬) │    傳送狀態數據    │  (顯示座標  │
│             │                   │   轉速等)   │
└────────────┘                    └────────────┘

我們的程式：
┌────────────┐                    ┌────────────┐
│ Mock Server │ ──── TCP/IP ────► │ HMI Client │
│ (假裝是機台)│    傳送 JSON 數據  │ (監控畫面)  │
└────────────┘                    └────────────┘
```

因為我們手邊沒有真的 CNC 機台，所以用 Mock Server「假裝」產生數據。
HMI Client 不知道也不在乎數據是真是假——它只負責接收和顯示。

### 這跟新代科技有什麼關係？

新代做的就是 CNC 控制器，他們的軟體工程師每天在做的事情包括：
- 人機介面開發 → 就是我們的 HMI Client
- 網路通訊架構 → 就是我們的 TCP 傳輸
- 應用工具開發 → 就是這類監控/控制軟體

---

## 2. 整體架構：兩個程式一起跑

這個專案編譯後會產生**兩個獨立的 .exe 執行檔**：

```
CNC_Mock_Server.exe     ← 模擬機台（沒有畫面，純背景程式）
CNC_HMI_Client.exe      ← 監控介面（有畫面的那個）
```

### 它們的關係

```
                     網路（TCP port 8888）
                          │
    CNC_Mock_Server       │       CNC_HMI_Client
    ┌──────────────┐      │      ┌──────────────┐
    │              │      │      │              │
    │ 每 0.1 秒    │ ─────┼────► │ 收到數據     │
    │ 產生假數據   │      │      │              │
    │ 送出 JSON    │      │      │ 解析 JSON    │
    │              │      │      │              │
    └──────────────┘      │      │ 更新畫面     │
                          │      │ - 座標數字   │
                          │      │ - 狀態燈號   │
                          │      │ - 圖表曲線   │
                          │      └──────────────┘
```

**重點**：兩個程式是獨立的，透過「網路」溝通。
這代表理論上你可以把 Server 跑在 A 電腦，Client 跑在 B 電腦，一樣能運作。

### Mock Server 產生的數據是怎麼來的？

不是純隨機，是**有規律的模擬**，讓畫面看起來像真的機台在運作：

**座標（圓形路徑）**：
```
X = 100 + 50 × cos(角度)      ← 角度每 0.1 秒增加
Y = 100 + 50 × sin(角度)      ← 所以 XY 會沿著圓形移動
Z = -10 + 5 × sin(角度 × 0.5) ← Z 軸緩慢上下擺動
```
這模擬了 CNC 在做圓弧切削的情境。

**主軸轉速**：基礎值 3000 RPM ± 隨機 200 的波動（模擬切削負載變化）

**機台狀態**：每 200 個週期（約 20 秒）自動循環切換：
```
IDLE（停機）→ RUNNING（運行）→ PAUSED（暫停）→ IDLE → ...

IDLE    : 轉速歸零、進給歸零、座標不動
RUNNING : 轉速 3000、進給 500、座標沿圓形路徑移動
PAUSED  : 轉速逐漸歸零、進給歸零、座標停住
```

### Demo 的時候你要做什麼？

```
你的操作              程式在幹嘛
──────              ────────
1. 啟動 Server       開始每 0.1 秒產生數據（但沒人接收，先放著）
2. 啟動 Client       畫面出現，數字都是 0，圖表是空的
3. 按 Connect        TCP 連上 → 數據湧入 → 畫面自動開始動
                     ↓
   什麼都不用做       座標在變、圖表在畫、狀態燈自動切換顏色
                     你可以跟面試官講解每個面板在顯示什麼
                     ↓
4. 按 Disconnect     斷線，畫面停在最後一筆數據
```

就這樣，不需要任何額外操作。畫面會自己動，你只要負責講解。

---

## 3. CMakeLists.txt：為什麼 Qt Creator 只要讀它？

### 先搞懂：程式是怎麼從原始碼變成 .exe 的？

```
原始碼 (.cpp/.h)  →  編譯器 (g++)  →  執行檔 (.exe)
```

但當專案有很多檔案時，問題來了：
- 哪些 .cpp 要編譯？
- 要連結哪些函式庫？
- 編譯順序是什麼？

**CMake 就是來回答這些問題的工具。**

### CMakeLists.txt 在說什麼？

用白話翻譯我們的 CMakeLists.txt：

```cmake
# 「我叫 CNC_HMI_Monitor，請用 C++17 來編我」
cmake_minimum_required(VERSION 3.16)
project(CNC_HMI_Monitor)
set(CMAKE_CXX_STANDARD 17)

# 「我需要 Qt 的這些模組」
find_package(Qt6 REQUIRED COMPONENTS Core Widgets Network Charts)

# 「把這些檔案編成一個叫 CNC_HMI_Client 的程式」
add_executable(CNC_HMI_Client
    src/main.cpp
    src/mainwindow.cpp
    src/tcpclient.cpp
    src/machinedata.cpp
)

# 「這個程式需要用到 Qt 的這些功能」
target_link_libraries(CNC_HMI_Client PRIVATE
    Qt6::Widgets    # GUI 元件
    Qt6::Network    # 網路功能
    Qt6::Charts     # 圖表功能
)
```

### Qt Creator 讀到 CMakeLists.txt 後會做什麼？

```
1. 看到 find_package(Qt6) → 知道要用 Qt 函式庫
2. 看到 add_executable  → 知道有哪些原始碼檔案
3. 看到 target_link_libraries → 知道要連結哪些模組
4. 自動設定好編譯按鈕、執行按鈕、程式碼補全等
```

所以 CMakeLists.txt 就像是**專案的說明書**，告訴開發工具「這個專案長什麼樣」。

---

## 4. Qt 如何讓程式跑起來？

### 普通的 C++ 程式

```cpp
int main() {
    printf("Hello");
    return 0;  // 印完就結束了
}
```

### Qt 的 GUI 程式

```cpp
int main(int argc, char *argv[]) {
    QApplication app(argc, argv);  // 1. 建立 Qt 應用程式

    MainWindow window;             // 2. 建立視窗
    window.show();                 // 3. 顯示視窗

    return app.exec();             // 4. 進入「事件迴圈」← 關鍵！
}
```

### 什麼是「事件迴圈」(Event Loop)？

`app.exec()` 會啟動一個**無限迴圈**，不斷處理事件：

```
事件迴圈：
┌──────────────────────────────────────────┐
│                                          │
│   等待事件 ─► 處理事件 ─► 等待事件 ─►... │
│                                          │
│   事件的種類：                            │
│   • 使用者點了按鈕                        │
│   • 網路收到資料                          │
│   • 計時器到時間了                        │
│   • 視窗需要重新繪製                      │
│                                          │
│   直到使用者關閉視窗 → 迴圈結束           │
└──────────────────────────────────────────┘
```

**這就是為什麼視窗不會打開後立刻關閉——它在 `app.exec()` 裡一直等待和處理事件。**

### GUI 程式 vs 一般程式的差異

```
一般 C++ 程式：上到下，跑完就結束
    main() → doA() → doB() → doC() → return

Qt GUI 程式：建完視窗後進入無限迴圈，被動回應事件
    main() → 建視窗 → app.exec()
                         ↓
                    等事件...等事件...
                    使用者按了 Connect！
                         ↓
                    呼叫 onConnectClicked()
                         ↓
                    繼續等事件...等事件...
```

---

## 5. Signal/Slot：Qt 的靈魂機制

這是面試**一定會被問的**，一定要懂。

### 問題：物件之間怎麼通知對方？

假設「使用者按了 Connect 按鈕」，按鈕怎麼告訴主視窗「我被按了」？

### 傳統做法：直接呼叫

```cpp
// 按鈕直接呼叫視窗的函式 → 按鈕要知道視窗的存在 → 耦合太高
button.onClick = mainWindow.onConnectClicked;
```

### Qt 的做法：Signal & Slot

```
發送者 (Button)              接收者 (MainWindow)
   │                              │
   │  clicked()   ──connect──►  onConnectClicked()
   │  (Signal)                    (Slot)
   │                              │
```

```cpp
// 「當 m_connectBtn 發出 clicked 信號時，呼叫 this 的 onConnectClicked」
connect(m_connectBtn, &QPushButton::clicked,
        this, &MainWindow::onConnectClicked);
```

### 為什麼這樣比較好？

- Button 不需要知道 MainWindow 的存在
- 一個 Signal 可以連接多個 Slot
- 可以跨執行緒傳遞

### 我們專案中的 Signal/Slot 連接

```
m_connectBtn::clicked ──────────► MainWindow::onConnectClicked
    按鈕被按                          啟動 TCP 連線

TcpClient::connected ──────────► MainWindow::onTcpConnected
    TCP 連上了                        更新 UI 顯示「Connected」

TcpClient::dataReceived ───────► MainWindow::onTcpDataReceived
    收到網路資料                       解析 JSON → 更新畫面

QTcpSocket::readyRead ─────────► TcpClient::onReadyRead
    Socket 有資料可讀                  讀取資料，發出 dataReceived
```

---

## 6. 程式運作流程：從啟動到畫面更新

### 完整流程

```
[使用者啟動 Server]
    │
    ▼
MockServer::start(8888)
    → 開始在 port 8888 監聽
    → 啟動 Timer，每 100ms 產生數據
    │
[使用者啟動 Client]
    │
    ▼
MainWindow 建構子
    → 建立 UI 元件（面板、按鈕、圖表）
    → 建立 TcpClient 物件
    → 連接所有 Signal/Slot
    │
[使用者按 Connect]
    │
    ▼
onConnectClicked()
    → TcpClient::connectToServer("127.0.0.1", 8888)
    → QTcpSocket 發起 TCP 連線
    │
    ▼
[TCP 三方交握完成]
    → TcpClient 發出 connected() 信號
    → MainWindow 更新 UI 為「Connected」
    │
    ▼
[Server 每 100ms 發送 JSON]    ←─── 持續循環
    │
    ▼
QTcpSocket::readyRead 信號觸發
    │
    ▼
TcpClient::onReadyRead()
    → 讀取網路資料到緩衝區
    → 用換行符號切割出完整的 JSON
    → 發出 dataReceived(jsonData) 信號
    │
    ▼
MainWindow::onTcpDataReceived(jsonData)
    → MachineData::parseFromJson(jsonData)    ← 解析 JSON
    → updateUI(machineData)                   ← 更新所有 UI 元件
        → 更新狀態燈號顏色
        → 更新座標數字
        → 更新主軸轉速數字
        → 圖表加入新的數據點
    │
    ▼
[畫面更新完成，等待下一筆資料]  ───► 回到迴圈繼續
```

---

## 7. TCP 網路通訊：兩個程式怎麼對話？

### TCP 是什麼？

TCP 是一種網路協定，保證資料**按順序、完整地**送達。
像是打電話——先建立連線，然後雙方可以持續對話。

### 我們的通訊流程

```
   Mock Server (port 8888)              HMI Client
        │                                  │
        │ ◄── TCP 連線請求 ─────────────── │ connect()
        │                                  │
        │ ── 連線建立 OK ─────────────────► │ connected()
        │                                  │
  每 100ms:                                │
        │ ── {"position":{"x":125}...}\n ► │ readyRead()
        │ ── {"position":{"x":126}...}\n ► │ readyRead()
        │ ── {"position":{"x":127}...}\n ► │ readyRead()
        │         ...持續傳送...            │
        │                                  │
```

### 資料格式

Server 送出的是 JSON 字串，每筆用換行符 `\n` 分隔：

```json
{"timestamp":1706860800,"position":{"x":125.35,"y":67.82,"z":-15.0},"spindle":{"rpm":3000,"load":45.5},"feed":{"rate":500,"override":100},"status":{"state":"RUNNING","mode":"AUTO","alarm":0}}
```

### 為什麼用換行符分隔？（重要概念）

TCP 是「串流」協定，它不管你的訊息邊界在哪。
你送 "Hello\nWorld\n"，對方可能收到：

```
第一次 readyRead: "Hel"
第二次 readyRead: "lo\nWor"
第三次 readyRead: "ld\n"
```

所以我們需要**自己定義怎麼切割訊息**。
我們的做法：用 `\n` 換行符當分隔，收到資料先存到緩衝區，遇到 `\n` 才算一筆完整資料。

這就是 `tcpclient.cpp` 裡 `onReadyRead()` 做的事。

---

## 8. 各檔案職責一覽

```
src/
├── main.cpp           程式進入點，就 5 行，建立視窗然後 app.exec()
├── mainwindow.h/cpp   主視窗，負責：建立 UI、接收資料、更新畫面
├── tcpclient.h/cpp    TCP 連線管理，負責：連線、斷線、接收、緩衝
└── machinedata.h/cpp  數據結構，負責：定義欄位、解析 JSON、狀態轉換

server/
├── main.cpp           Server 進入點，啟動 Server 然後等待
└── mockserver.h/cpp   模擬伺服器，負責：監聽連線、產生假數據、廣播
```

### 各檔案的「大小」和重要程度

```
mainwindow.cpp   ★★★★★  最大最重要，所有 UI 邏輯都在這
tcpclient.cpp    ★★★★☆  網路通訊核心
machinedata.cpp  ★★★☆☆  JSON 解析，比較單純
mockserver.cpp   ★★★☆☆  假數據產生器
main.cpp (x2)    ★☆☆☆☆  超短，只是進入點
```

---

## 9. 必讀關鍵程式碼

不用每行都讀，但這幾段一定要理解：

### (1) Signal/Slot 連接 — `mainwindow.cpp` 建構子

```cpp
// 當 TCP 連上時 → 更新 UI
connect(m_tcpClient, &TcpClient::connected,
        this, &MainWindow::onTcpConnected);

// 當收到資料時 → 解析並更新畫面
connect(m_tcpClient, &TcpClient::dataReceived,
        this, &MainWindow::onTcpDataReceived);
```

### (2) TCP 資料接收與緩衝 — `tcpclient.cpp`

```cpp
void TcpClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());  // 讀到緩衝區

    while (true) {
        int newlineIndex = m_buffer.indexOf('\n');  // 找換行符
        if (newlineIndex == -1) break;              // 沒找到，等下一次

        QByteArray line = m_buffer.left(newlineIndex);  // 取出一行
        m_buffer.remove(0, newlineIndex + 1);           // 移除已處理的

        emit dataReceived(line);  // 發送信號給 MainWindow
    }
}
```

### (3) JSON 解析 — `machinedata.cpp`

```cpp
bool MachineData::parseFromJson(const QByteArray &jsonData)
{
    QJsonDocument doc = QJsonDocument::fromJson(jsonData);
    QJsonObject root = doc.object();

    // 取值就像查字典
    posX = root["position"].toObject()["x"].toDouble();
    spindleRpm = root["spindle"].toObject()["rpm"].toInt();
    // ...
}
```

### (4) UI 更新 — `mainwindow.cpp`

```cpp
void MainWindow::updateUI(const MachineData &data)
{
    // 把數字塞到 Label 上
    m_posXLabel->setText(QString::number(data.posX, 'f', 3));
    m_spindleRpmLabel->setText(QString("%1 RPM").arg(data.spindleRpm));

    // 把數據加到圖表上
    updateChart(data.spindleRpm);
}
```

### (5) Server 產生假數據 — `mockserver.cpp`

```cpp
// 模擬圓形路徑的座標移動
double angle = m_cycleCount * 0.05;
m_posX = 100.0 + 50.0 * qCos(angle);
m_posY = 100.0 + 50.0 * qSin(angle);
```

---

## 10. 開發過程遇到的問題與解法

### 問題 1：switch-case 中宣告變數導致編譯錯誤

**情境**：Mock Server 的 `generateMachineData()` 中，在 `case 1` 宣告了 `double angle`。

**錯誤訊息**：
```
error: jump to case label
note: crosses initialization of 'double angle'
```

**原因**：
C++ 的 switch-case 各個 case 共享同一個作用域。如果在 `case 1` 宣告變數，跳到 `case 2` 時這個變數處於「宣告了但沒初始化」的狀態，編譯器認為這很危險。

```cpp
switch (state) {
case 1:
    double angle = 0.05;  // ← 這裡宣告了
    break;
case 2:                   // ← 跳到這裡時，angle 存在但沒被初始化
    break;
}
```

**解法**：用大括號 `{}` 在 case 內建立獨立作用域。

```cpp
case 1: {  // 加了大括號
    double angle = 0.05;  // angle 只在這個大括號內有效
    break;
}
```

**面試怎麼講**：「我在 switch-case 中遇到變數作用域問題，C++ 的 case 預設共享作用域，需要用大括號限制變數生命週期。」

---

### 問題 2：findChild 回傳 null 導致 UI 元件消失

**情境**：用 `findChild<QGroupBox*>("statusGroup")` 試圖找到子元件，結果回傳 null。

**錯誤訊息**：
```
QLayout: Cannot add a null widget to QHBoxLayout
```

**原因**：
`findChild()` 只能在**物件樹**中搜尋。如果 QGroupBox 建立時沒有設定 parent，它就不在任何物件樹中，自然找不到。

```
MainWindow                    QGroupBox("statusGroup")
  └── centralWidget              (孤兒，沒有 parent)
       └── layout
            └── ???           findChild 找不到它！
```

**解法**：不使用 findChild，改讓函數直接回傳 widget 指標。

```cpp
// 修改前：
void setupStatusPanel();  // 沒有回傳值，靠 findChild 找
// 修改後：
QGroupBox* createStatusPanel();  // 直接回傳指標
```

**面試怎麼講**：「我遇到 Qt 物件樹的問題。Qt 用父子關係管理元件，如果沒有正確設定 parent，findChild 就找不到元件。我改用直接回傳指標的方式，更明確也更安全。」

---

### 問題 3：TCP 分包問題（黏包）

**情境**：TCP 收到的資料不一定剛好是完整的一筆 JSON。

**問題**：
```
預期：收到 {"rpm":3000}\n
實際：第一次收到 {"rpm":30    ← 不完整！
      第二次收到 00}\n{"rpm  ← 兩筆混在一起！
```

**原因**：
TCP 是「串流」協定，不保證訊息邊界。它只保證資料按順序到達，但一次 read 可能讀到半筆、一筆、或好幾筆資料。

**解法**：實作緩衝區機制。

```cpp
void TcpClient::onReadyRead()
{
    m_buffer.append(m_socket->readAll());  // 全部先存起來

    while (true) {
        int idx = m_buffer.indexOf('\n');  // 找分隔符
        if (idx == -1) break;             // 沒有 → 資料不完整，等下一次

        QByteArray line = m_buffer.left(idx);  // 取出完整的一行
        m_buffer.remove(0, idx + 1);           // 從緩衝區移除
        emit dataReceived(line);               // 送出
    }
}
```

**面試怎麼講**：「TCP 是串流協定，沒有訊息邊界的概念。我用換行符作為訊息分隔，並實作了緩衝區來處理 TCP 黏包問題，確保每次只處理完整的 JSON 資料。」

---

### 問題 4：Mock Server 在 CMD 中啟動後立刻退出

**情境**：從 Windows CMD 直接執行 CNC_Mock_Server.exe，程式啟動後立刻結束。

**原因**：
Qt 程式執行時需要載入 Qt 的動態連結函式庫（DLL）。Qt Creator 執行程式時會自動設定好 DLL 的搜尋路徑（PATH），但從 CMD 直接執行時，系統找不到這些 DLL，程式就會載入失敗而退出。

```
從 Qt Creator 執行：
  Qt Creator 設定 PATH → 包含 D:\Qt\6.10.2\mingw_64\bin → 找到 DLL → 正常運作

從 CMD 執行：
  PATH 沒有包含 Qt 目錄 → 找不到 Qt6Core.dll 等 → 程式立刻退出
```

**解法**：
- 方法一：從 Qt Creator 內執行（切換 Run Target）
- 方法二：用 `windeployqt` 工具把需要的 DLL 複製到 exe 旁邊
- 方法三：手動把 Qt 的 bin 目錄加到系統 PATH

**面試怎麼講**：「這是動態連結函式庫的路徑問題。Qt 程式依賴多個 DLL，需要確保執行環境能找到它們。正式部署時可以用 windeployqt 工具自動處理依賴。」

---

## 11. 面試時怎麼講這個專案

### 30 秒版本

> 「我做了一個 CNC 機台的即時監控系統。用 C++ 和 Qt 開發，架構是 Client-Server。Server 透過 TCP 傳送機台數據，Client 接收後即時顯示座標、主軸轉速、機台狀態，並用圖表呈現歷史趨勢。」

### 被問「為什麼這樣設計」時

> 「Client-Server 架構是因為實際工廠中，控制器和監控端通常在不同設備上，用網路通訊比較符合實際情境。選擇 Qt 是因為它在工業界廣泛使用，而且跨平台支援 Linux，符合職缺的技術要求。」

### 被問「最有挑戰的部分」時

> 「TCP 的黏包處理。TCP 是串流協定，一次 read 不一定是完整的一筆訊息，我實作了緩衝區搭配換行分隔符來確保每次處理完整的 JSON 資料。」

### 被問「如果要改進」時

> 「可以加入多台機台同時監控、數據記錄與匯出功能、告警系統，以及通訊協定可以改用工業標準的 Modbus 或 OPC UA。」

---

*文檔建立日期：2026-02-03*
