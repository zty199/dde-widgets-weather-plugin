#pragma once

#include "data.hpp"
#include "global.h"

#include <QObject>
#include <QThread>

WEATHER_WIDGETS_BEGIN_NAMESPACE

class QWeatherAPIWorker;
class QWeatherAPI : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QWeatherAPI)

public:
    QWeatherAPI();
    ~QWeatherAPI() override;

    static QWeatherAPI *instance();

    /**
     * @brief 获取 IP 所在位置经纬度
     * @indexpage https://api.ipbase.com/v2/info
     */
    void getLocationFromIP();
    /**
     * @brief 获取经纬度对应地区 id
     * @param location 经纬度：${longitude},${latitude}
     * @indexpage https://geoapi.qweather.com/v2/city/lookup
     */
    Q_DECL_UNUSED void getCityIdFromLocation(const QString &location);
    /**
     * @brief 获取经纬度对应地区实时天气
     * @param location 经纬度：${longitude},${latitude}
     * @indexpage https://devapi.qweather.com/v7/weather/now
     */
    void getWeatherNowFromLocation(const QString &location);

private:
    void initConnections();

    void start();
    void stop();

signals:
    void sigGetLocationFromIPFinished(const QString &location);
    void sigGetCityIdFromLocationFinished(const QString &id);
    void sigGetWeatherNowFromLocation(const WeatherNow &weather);

private:
    QThread *m_thread = nullptr;

    QWeatherAPIWorker *m_worker = nullptr;
};

class QWeatherAPIWorker : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(QWeatherAPIWorker)

private:
    QWeatherAPIWorker() = default;
    ~QWeatherAPIWorker() = default;

    QPair<bool, QString> parseLocation(const QByteArray &replyData);
    QPair<bool, QString> parseCityID(const QByteArray &replyData);
    QPair<bool, WeatherNow> parseWeatherNow(const QByteArray &replyData);

signals:
    void sigGetLocationFromIPFinished(const QString &location);
    void sigGetCityIdFromLocationFinished(const QString &id);
    void sigGetWeatherNowFromLocation(const WeatherNow &weather);

private slots:
    void slotGetLocationFromIP();
    void slotGetCityIdFromLocation(const QString &location);
    void slotGetWeatherNowFromLocation(const QString &location);

private:
    friend class QWeatherAPI;
};

WEATHER_WIDGETS_END_NAMESPACE
