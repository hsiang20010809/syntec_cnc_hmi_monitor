#ifndef MACHINEDATA_H
#define MACHINEDATA_H

#include <QString>
#include <QJsonObject>
#include <QJsonDocument>

/**
 * @brief 機台狀態列舉
 */
enum class MachineState {
    IDLE = 0,       // 待機
    RUNNING = 1,    // 運行中
    PAUSED = 2,     // 暫停
    ERROR = 3,      // 錯誤
    ESTOP = 4       // 緊急停止
};

/**
 * @brief 機台數據結構
 * 儲存從 Server 接收的機台即時數據
 */
struct MachineData {
    // 時間戳記
    qint64 timestamp = 0;

    // 座標位置 (mm)
    double posX = 0.0;
    double posY = 0.0;
    double posZ = 0.0;

    // 主軸數據
    int spindleRpm = 0;
    double spindleLoad = 0.0;

    // 進給數據
    int feedRate = 0;
    int feedOverride = 100;

    // 狀態資訊
    MachineState state = MachineState::IDLE;
    QString mode = "AUTO";
    int alarmCode = 0;

    /**
     * @brief 從 JSON 資料解析
     * @param jsonData JSON 格式的位元組陣列
     * @return 解析是否成功
     */
    bool parseFromJson(const QByteArray &jsonData);

    /**
     * @brief 取得狀態文字描述
     * @return 狀態字串
     */
    QString getStateString() const;

    /**
     * @brief 取得狀態顏色 (用於 UI 顯示)
     * @return 顏色代碼字串
     */
    QString getStateColor() const;
};

#endif // MACHINEDATA_H
