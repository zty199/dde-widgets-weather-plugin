#include "qweatherapi.h"
#include "config.hpp"
#include "logging_categories.h"

#include <widgetsinterface.h>

#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QEventLoop>
#include <QTimer>
#include <QJsonDocument>
#include <QJsonParseError>
#include <QJsonObject>
#include <QJsonArray>

WEATHER_WIDGETS_BEGIN_NAMESPACE

Q_GLOBAL_STATIC(QWeatherAPI, weatherAPI)

QWeatherAPI::QWeatherAPI()
    : m_thread(new QThread)
    , m_worker(new QWeatherAPIWorker)
{
    initConnections();
    start();
}

QWeatherAPI::~QWeatherAPI()
{
    stop();
    m_thread->deleteLater();
}

QWeatherAPI *QWeatherAPI::instance()
{
    return weatherAPI;
}

void QWeatherAPI::getLocationFromIP()
{
    QMetaObject::invokeMethod(m_worker, &QWeatherAPIWorker::slotGetLocationFromIP, Qt::QueuedConnection);
}

void QWeatherAPI::getCityIdFromLocation(const QString &location)
{
    QMetaObject::invokeMethod(m_worker, std::bind(&QWeatherAPIWorker::slotGetCityIdFromLocation, m_worker, location), Qt::QueuedConnection);
}

void QWeatherAPI::getWeatherNowFromLocation(const QString &location)
{
    QMetaObject::invokeMethod(m_worker, std::bind(&QWeatherAPIWorker::slotGetWeatherNowFromLocation, m_worker, location), Qt::QueuedConnection);
}

void QWeatherAPI::initConnections()
{
    connect(m_thread, &QThread::finished, m_worker, &QWeatherAPIWorker::deleteLater);

    connect(m_worker, &QWeatherAPIWorker::sigGetLocationFromIPFinished, this, &QWeatherAPI::sigGetLocationFromIPFinished, Qt::QueuedConnection);
    connect(m_worker, &QWeatherAPIWorker::sigGetCityIdFromLocationFinished, this, &QWeatherAPI::sigGetCityIdFromLocationFinished, Qt::QueuedConnection);
    connect(m_worker, &QWeatherAPIWorker::sigGetWeatherNowFromLocation, this, &QWeatherAPI::sigGetWeatherNowFromLocation, Qt::QueuedConnection);
}

void QWeatherAPI::start()
{
    m_worker->moveToThread(m_thread);
    m_thread->start();
}

void QWeatherAPI::stop()
{
    m_thread->requestInterruption();
    m_thread->quit();
    m_thread->wait();
}

QPair<bool, QString> QWeatherAPIWorker::parseLocation(const QByteArray &replyData)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dwweatherLog).noquote() << "Parse Location failed:" << replyData;
        return {false, QString()};
    }

    QJsonObject jsonObj = jsonDoc.object().value("data").toObject().value("location").toObject();
    QString latitude = QString::number(jsonObj.value("latitude").toDouble(DEFAULT_LATITUDE), 'f', 2);
    QString longitude = QString::number(jsonObj.value("longitude").toDouble(DEFAULT_LONGITUDE), 'f', 2);
    return {true, longitude + "," + latitude};
}

QPair<bool, QString> QWeatherAPIWorker::parseCityID(const QByteArray &replyData)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dwweatherLog).noquote() << "Parse City ID failed:" << replyData;
        return {false, QString()};
    }

    QJsonObject jsonObj = jsonDoc.object();
    int code = jsonObj.value("code").toString().toInt();
    if (code != statusCode(StatusCode::OK)) {
        qCWarning(dwweatherLog).noquote() << "Parse City ID failed: StatusCode:" << code;
        return {false, QString()};
    }

    QJsonArray jsonArray = jsonObj.value("location").toArray();
    if (jsonArray.size() < 1) {
        qCWarning(dwweatherLog).noquote() << "Parse City ID failed: empty location:" << jsonArray;
        return {false, QString()};
    }

    QString cityID = jsonArray.first().toObject().value("id").toString();
    return {cityID.isEmpty(), cityID};
}

QPair<bool, WeatherNow> QWeatherAPIWorker::parseWeatherNow(const QByteArray &replyData)
{
    QJsonParseError error;
    QJsonDocument jsonDoc = QJsonDocument::fromJson(replyData, &error);
    if (error.error != QJsonParseError::NoError) {
        qCWarning(dwweatherLog).noquote() << "Parse Weather failed:" << replyData;
        return {false, WeatherNow()};
    }

    QJsonObject jsonObj = jsonDoc.object();
    int code = jsonObj.value("code").toString().toInt();
    if (code != statusCode(StatusCode::OK)) {
        qCWarning(dwweatherLog).noquote() << "Parse Weather failed: StatusCode:" << code;
        return {false, WeatherNow()};
    }

    WeatherNow weather(jsonObj.value("now").toObject());
    return {weather.isValid(), weather};
}

void QWeatherAPIWorker::slotGetLocationFromIP()
{
    QUrl url("https://api.ipbase.com/v2/info");

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    if (reply == nullptr) {
        qCWarning(dwweatherLog).noquote() << "GET NetworkReply failed:" << url;
        emit sigGetLocationFromIPFinished(QString());
        return;
    }

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    // QTimer timer;
    // timer.setSingleShot(true);
    // timer.setInterval(5 * 1000);
    // connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    // timer.start();
    loop.exec();

    if (reply->isFinished() && reply->error() == QNetworkReply::NoError) {
        QByteArray replyData = reply->readAll();
        reply->deleteLater();
        reply = nullptr;

        auto [ret, location] = parseLocation(replyData);
        qCDebug(dwweatherLog).noquote() << "Current Location:" << (ret ? location : "Unknown");
        emit sigGetLocationFromIPFinished(ret ? location : QString());
        return;
    }

    reply->deleteLater();
    reply = nullptr;
    emit sigGetLocationFromIPFinished(QString());
}

void QWeatherAPIWorker::slotGetCityIdFromLocation(const QString &location)
{
    QString requestURL("https://geoapi.qweather.com/v2/city/lookup");
    QStringList queryParams;
    queryParams.append(QString("location=%1").arg(location));
    queryParams.append(QString("key=%1").arg(Config::instance()->getWebAPIKey()));
    queryParams.append(QString("lang=%1").arg(QLocale::system().bcp47Name()));
    QUrl url(QString("%1?%2").arg(requestURL, queryParams.join("&")));

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    if (reply == nullptr) {
        qCWarning(dwweatherLog).noquote() << "GET NetworkReply failed:" << requestURL;
        emit sigGetCityIdFromLocationFinished(QString());
        return;
    }

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    // QTimer timer;
    // timer.setSingleShot(true);
    // timer.setInterval(5 * 1000);
    // connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    // timer.start();
    loop.exec();

    if (reply->isFinished() && reply->error() == QNetworkReply::NoError) {
        QByteArray replyData = reply->readAll();
        reply->deleteLater();
        reply = nullptr;

        auto [ret, id] = parseCityID(replyData);
        qCDebug(dwweatherLog).noquote() << "Current City ID:" << (ret ? id : "Unknown");
        emit sigGetCityIdFromLocationFinished(ret ? id : QString());
        return;
    }

    reply->deleteLater();
    reply = nullptr;
    emit sigGetCityIdFromLocationFinished(QString());
}

void QWeatherAPIWorker::slotGetWeatherNowFromLocation(const QString &location)
{
    QString requestURL("https://devapi.qweather.com/v7/weather/now");
    QStringList queryParams;
    queryParams.append(QString("location=%1").arg(location));
    queryParams.append(QString("key=%1").arg(Config::instance()->getWebAPIKey()));
    queryParams.append(QString("lang=%1").arg(QLocale::system().bcp47Name()));
    QUrl url(QString("%1?%2").arg(requestURL, queryParams.join("&")));

    QNetworkAccessManager manager;
    QNetworkRequest request(url);
    QNetworkReply *reply = manager.get(request);

    if (reply == nullptr) {
        qCWarning(dwweatherLog).noquote() << "GET NetworkReply failed:" << requestURL;
        emit sigGetWeatherNowFromLocation(WeatherNow());
        return;
    }

    QEventLoop loop;
    connect(reply, &QNetworkReply::finished, &loop, &QEventLoop::quit);
    // QTimer timer;
    // timer.setSingleShot(true);
    // timer.setInterval(5 * 1000);
    // connect(&timer, &QTimer::timeout, &loop, &QEventLoop::quit);
    // timer.start();
    loop.exec();

    if (reply->isFinished() && reply->error() == QNetworkReply::NoError) {
        QByteArray replyData = reply->readAll();
        reply->deleteLater();
        reply = nullptr;

        auto [ret, weather] = parseWeatherNow(replyData);
        qCDebug(dwweatherLog).noquote() << "Current Weather:" << (ret ? weather.rawData : "Unknown");
        emit sigGetWeatherNowFromLocation(weather);
        return;
    }

    reply->deleteLater();
    reply = nullptr;
    emit sigGetWeatherNowFromLocation(WeatherNow());
}

WEATHER_WIDGETS_END_NAMESPACE
