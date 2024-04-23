#pragma once

#include "global.h"
#include "logging_categories.h"

#include <QString>
#include <QDateTime>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>

#include <zlib.h>

WEATHER_WIDGETS_BEGIN_NAMESPACE

/**
 * @brief 错误状态码
 * @indexpage https://dev.qweather.com/docs/resource/status-code/
 */
enum class StatusCode : int {
    OK = 200, // 请求成功
    NoData = 204, // 请求成功，但你查询的地区暂时没有你需要的数据
    BadRequest = 400, // 请求错误，可能包含错误的请求参数或缺少必选的请求参数
    Unauthorized = 401, // 认证失败，可能使用了错误的 KEY、数字签名错误、KEY 的类型错误（如使用 SDK 的 KEY 去访问 Web API）
    PaymentRequired = 402, // 超过访问次数或余额不足以支持继续访问服务，你可以充值、升级访问量或等待访问量重置
    Forbidden = 403, // 无访问权限，可能是绑定的 PackageName、BundleID、域名 IP 地址不一致，或者是需要额外付费的数据
    NotFound = 404, // 查询的数据或地区不存在
    QPMOverload = 429, // 超过限定的 QPM（每分钟访问次数）
    InternalServerError = 500 // 无响应或超时，接口服务异常
};

template<typename T = int>
inline T statusCode(StatusCode statusCode)
{
    return static_cast<T>(statusCode);
}

constexpr char kNAIconName[] = "999";
/**
 * @brief 实时天气
 * @indexpage https://dev.qweather.com/docs/api/weather/weather-now/
 * @example
 * "now": {
    "obsTime": "2020-06-30T21:40+08:00",
    "temp": "24",
    "feelsLike": "26",
    "icon": "101",
    "text": "多云",
    "wind360": "123",
    "windDir": "东南风",
    "windScale": "1",
    "windSpeed": "3",
    "humidity": "72",
    "precip": "0.0",
    "pressure": "1003",
    "vis": "16",
    "cloud": "10",
    "dew": "21"
  }
 */
struct WeatherNow {
    WeatherNow() { }

    WeatherNow(const QByteArray &raw)
    {
        if (raw.trimmed().isEmpty()) {
            return;
        }

        rawData = raw;
        parseRawData();
    }

    WeatherNow(const QJsonObject &jsonObj)
    {
        rawData = QJsonDocument(jsonObj).toJson(QJsonDocument::Compact);
        fromJsonObject(jsonObj);
    }

    void parseRawData()
    {
        QJsonParseError error;
        QJsonDocument jsonDoc = QJsonDocument::fromJson(rawData, &error);
        if (error.error != QJsonParseError::NoError || !jsonDoc.isObject()) {
            qCWarning(dwweatherLog).noquote() << "Parse WeatherNow failed:" << rawData;
            rawData.clear();
            return;
        }

        fromJsonObject(jsonDoc.object());
    }

    void fromJsonObject(const QJsonObject &jsonObj)
    {
        obsTime = QDateTime::fromString(jsonObj.value("obsTime").toString(), Qt::ISODate);
        temp = jsonObj.value("temp").toString().toInt();
        feelsLike = jsonObj.value("feelsLike").toString().toInt();
        icon = jsonObj.value("icon").toString();
        text = jsonObj.value("text").toString();
        wind360 = jsonObj.value("wind360").toString().toInt();
        windDir = jsonObj.value("windDir").toString();
        windScale = jsonObj.value("windScale").toString().toUInt();
        windSpeed = jsonObj.value("windSpeed").toString().toUInt();
        humidity = jsonObj.value("humidity").toString().toUInt();
        precip = jsonObj.value("precip").toString().toDouble();
        pressure = jsonObj.value("pressure").toString().toUInt();
        vis = jsonObj.value("vis").toString().toUInt();

        if (jsonObj.contains("cloud")) {
            cloud = jsonObj.value("cloud").toString().toUInt();
        }
        if (jsonObj.contains("dew")) {
            dew = jsonObj.value("dew").toString().toInt();
        }
    }

    bool isValid() const
    {
        return !rawData.trimmed().isEmpty();
    }

    /**
     * @brief 数据观测时间
     */
    QDateTime obsTime;
    /**
     * @brief 温度，默认单位：摄氏度
     */
    qint32 temp = 0;
    /**
     * @brief 体感温度，默认单位：摄氏度
     */
    qint32 feelsLike = 0;
    /**
     * @brief 天气状况的图标代码
     */
    QString icon = kNAIconName;
    /**
     * @brief 天气状况的文字描述
     */
    QString text;
    /**
     * @brief 风向360角度
     */
    qint32 wind360 = -1;
    /**
     * @brief 风向
     */
    QString windDir;
    /**
     * @brief 风力等级
     */
    quint32 windScale = 0;
    /**
     * @brief 风速，公里/小时
     */
    quint32 windSpeed = 0;
    /**
     * @brief 相对湿度，百分比数值
     */
    quint32 humidity = 0;
    /**
     * @brief 当前小时累计降水量，默认单位：毫米
     */
    qreal precip = 0.0;
    /**
     * @brief 大气压强，默认单位：百帕
     */
    quint32 pressure = 1000;
    /**
     * @brief 能见度，默认单位：公里
     */
    quint32 vis = 0;
    /**
     * @brief 云量，百分比数值。可能为空
     */
    quint32 cloud = 0;
    /**
     * @brief 露点温度。可能为空
     */
    qint32 dew = 0;

    /**
     * @brief 原始数据
     */
    QByteArray rawData;
};

#define GZIP_ENCODING 16

/**
 * @brief GZip 数据压缩
 * @param source 源数据
 * @return 压缩数据
 */
Q_DECL_UNUSED static QByteArray GZipCompress(const QByteArray &source)
{
    if (source.isEmpty()) {
        return QByteArray();
    }

    z_stream stream;
    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;
    stream.next_in = (Bytef *)source.data();
    stream.avail_in = source.size();

    if (deflateInit2(&stream, Z_DEFAULT_COMPRESSION, Z_DEFLATED,
                     MAX_WBITS + GZIP_ENCODING, 8, Z_DEFAULT_STRATEGY)
        != Z_OK) {
        return QByteArray();
    }

    QByteArray byteArray;
    while (true) {
        char buffer[4096] = {0};
        stream.next_out = (Bytef *)buffer;
        stream.avail_out = 4096;

        int code = deflate(&stream, Z_FINISH);
        byteArray.append(buffer, 4096 - stream.avail_out);
        if (code == Z_STREAM_END || code != Z_OK) {
            break;
        }
    }
    deflateEnd(&stream);

    return byteArray;
}

/**
 * @brief GZip 数据解压缩
 * @param source 源数据
 * @return 解压缩数据
 */
Q_DECL_UNUSED static QByteArray GZipDecompress(const QByteArray &source)
{
    if (source.isEmpty()) {
        return QByteArray();
    }

    z_stream stream;
    stream.zalloc = NULL;
    stream.zfree = NULL;
    stream.opaque = NULL;
    stream.next_in = (Bytef *)source.data();
    stream.avail_in = source.size();

    if (inflateInit2(&stream, MAX_WBITS + GZIP_ENCODING) != Z_OK) {
        return QByteArray();
    }

    QByteArray byteArray;
    while (true) {
        char buffer[4096] = {0};
        stream.next_out = (Bytef *)buffer;
        stream.avail_out = 4096;

        int code = inflate(&stream, Z_FINISH);
        byteArray.append(buffer, 4096 - stream.avail_out);
        if (code == Z_STREAM_END) {
            break;
        }
    }
    inflateEnd(&stream);

    return byteArray;
}

WEATHER_WIDGETS_END_NAMESPACE

Q_DECLARE_METATYPE(WEATHER_WIDGETS_NAMESPACE::WeatherNow)
