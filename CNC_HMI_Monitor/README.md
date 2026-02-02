# CNC HMI Monitor

CNC 設備監控人機介面 - 新代科技面試專案

## 專案說明

本專案是一個模擬 CNC 控制器的人機介面 (HMI) 應用程式，展示以下技術能力：

- **C++ 程式設計** - 使用現代 C++17 標準
- **GUI 開發** - Qt Widgets 人機介面設計
- **網路通訊** - TCP Client-Server 架構
- **即時數據視覺化** - Qt Charts 圖表繪製

## 系統架構

```
┌─────────────────┐     TCP/IP      ┌─────────────────┐
│   HMI Client    │ ◄─────────────► │   Mock Server   │
│   (Qt Widgets)  │   JSON Data     │  (Data Simulator)│
└─────────────────┘                 └─────────────────┘
```

## 功能特色

- 機台狀態即時監控（運行/待機/錯誤）
- X/Y/Z 三軸座標顯示
- 主軸轉速與進給率顯示
- 即時數據折線圖
- TCP 網路連線管理

## 環境需求

- Qt 6.x
- CMake 3.16+
- C++17 編譯器 (MSVC/MinGW/GCC)

## 編譯方式

```bash
# 建立 build 目錄
mkdir build && cd build

# 執行 CMake
cmake ..

# 編譯專案
cmake --build .
```

## 執行方式

1. 先啟動 Mock Server：
```bash
./bin/CNC_Mock_Server
```

2. 再啟動 HMI Client：
```bash
./bin/CNC_HMI_Client
```

3. 點擊「連線」按鈕開始監控

## 專案結構

```
CNC_HMI_Monitor/
├── CMakeLists.txt          # CMake 建置設定
├── README.md               # 本文件
├── docs/
│   ├── PROJECT_PLAN.md     # 專案規劃文檔
│   └── TECHNICAL_TUTORIAL.md # 技術教學文檔
├── src/                    # HMI Client 原始碼
│   ├── main.cpp
│   ├── mainwindow.h/cpp
│   ├── tcpclient.h/cpp
│   └── machinedata.h/cpp
└── server/                 # Mock Server 原始碼
    ├── main.cpp
    └── mockserver.h/cpp
```

## 通訊協定

使用 JSON 格式傳輸機台數據：

```json
{
    "timestamp": 1706860800000,
    "position": { "x": 125.35, "y": 67.82, "z": -15.0 },
    "spindle": { "rpm": 3000, "load": 45.5 },
    "feed": { "rate": 500, "override": 100 },
    "status": { "state": "RUNNING", "mode": "AUTO", "alarm": 0 }
}
```

## 作者

面試專案 - 新代科技 52軟體研發工程師

## 授權

MIT License
