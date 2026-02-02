#include "machinedata.h"
#include <QJsonObject>
#include <QJsonDocument>
#include <QDebug>

bool MachineData::parseFromJson(const QByteArray &jsonData)
{
    QJsonParseError parseError;
    QJsonDocument doc = QJsonDocument::fromJson(jsonData, &parseError);

    if (parseError.error != QJsonParseError::NoError) {
        qWarning() << "JSON parse error:" << parseError.errorString();
        return false;
    }

    if (!doc.isObject()) {
        qWarning() << "JSON is not an object";
        return false;
    }

    QJsonObject root = doc.object();

    // 解析時間戳記
    timestamp = root["timestamp"].toVariant().toLongLong();

    // 解析座標位置
    QJsonObject position = root["position"].toObject();
    posX = position["x"].toDouble();
    posY = position["y"].toDouble();
    posZ = position["z"].toDouble();

    // 解析主軸數據
    QJsonObject spindle = root["spindle"].toObject();
    spindleRpm = spindle["rpm"].toInt();
    spindleLoad = spindle["load"].toDouble();

    // 解析進給數據
    QJsonObject feed = root["feed"].toObject();
    feedRate = feed["rate"].toInt();
    feedOverride = feed["override"].toInt();

    // 解析狀態資訊
    QJsonObject status = root["status"].toObject();
    QString stateStr = status["state"].toString();
    mode = status["mode"].toString();
    alarmCode = status["alarm"].toInt();

    // 轉換狀態字串為列舉
    if (stateStr == "IDLE") {
        state = MachineState::IDLE;
    } else if (stateStr == "RUNNING") {
        state = MachineState::RUNNING;
    } else if (stateStr == "PAUSED") {
        state = MachineState::PAUSED;
    } else if (stateStr == "ERROR") {
        state = MachineState::ERROR;
    } else if (stateStr == "ESTOP") {
        state = MachineState::ESTOP;
    }

    return true;
}

QString MachineData::getStateString() const
{
    switch (state) {
    case MachineState::IDLE:
        return "IDLE";
    case MachineState::RUNNING:
        return "RUNNING";
    case MachineState::PAUSED:
        return "PAUSED";
    case MachineState::ERROR:
        return "ERROR";
    case MachineState::ESTOP:
        return "E-STOP";
    default:
        return "UNKNOWN";
    }
}

QString MachineData::getStateColor() const
{
    switch (state) {
    case MachineState::IDLE:
        return "#808080";  // 灰色
    case MachineState::RUNNING:
        return "#00ff00";  // 綠色
    case MachineState::PAUSED:
        return "#ffff00";  // 黃色
    case MachineState::ERROR:
        return "#ff0000";  // 紅色
    case MachineState::ESTOP:
        return "#ff0000";  // 紅色
    default:
        return "#ffffff";  // 白色
    }
}
