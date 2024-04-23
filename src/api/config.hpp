#pragma once

#include "global.h"
#include "logging_categories.h"

#include <QObject>
#include <QSettings>
#include <QStandardPaths>

WEATHER_WIDGETS_BEGIN_NAMESPACE

#define DEFAULT_LATITUDE 39.56
#define DEFAULT_LONGITUDE 116.20

#define DEFAULT_UPDATE_INTERVAL 60 * 60 * 4

#define KEY_WEB_API_KEY "webAPIKey"
#define KEY_LOCATION "location"
#define KEY_TIMESTAMP "lastUpdateTimeStamp"
#define KEY_INTERVAL "updateInterval"
#define KEY_WEATHER "weatherNow"
#define KEY_ID "id"

class Config : public QObject
{
    Q_OBJECT
    Q_DISABLE_COPY_MOVE(Config)

public:
    static Config *instance()
    {
        static Config config;
        return &config;
    }

    QSettings *settings()
    {
        return m_settings;
    }

    QString readWebAPIKey()
    {
        if (m_settings == nullptr) {
            qCWarning(dwweatherLog).noquote() << "Config not initialized, call Config::initialize first";
            return m_webAPIKey;
        }

        QByteArray webAPIKey = m_settings->value(KEY_WEB_API_KEY, QByteArray()).toByteArray().trimmed();
        if (webAPIKey.isEmpty()) {
            return m_webAPIKey;
        }

        m_webAPIKey = QString::fromUtf8(QByteArray::fromBase64(webAPIKey));
        return m_webAPIKey;
    }

    void saveWebAPIKey()
    {
        if (m_settings == nullptr) {
            qCWarning(dwweatherLog).noquote() << "Config not initialized, call Config::initialize first";
            return;
        }

        if (m_webAPIKey == DEFAULT_WEB_API_KEY) {
            return;
        }

        m_settings->setValue(KEY_WEB_API_KEY, m_webAPIKey.toUtf8().toBase64());
        m_settings->sync();
    }

    void initialize(const QString &orgName, const QString &appName, const QString &pluginId)
    {
        QString fileName = QString("%1-%2.json").arg(appName, pluginId);
        fileName = QString("%1/%2/%3").arg(QStandardPaths::writableLocation(QStandardPaths::ConfigLocation), orgName, fileName);
        m_settings = new QSettings(fileName, QSettings::IniFormat, this);

        readWebAPIKey();
    }

    QString getWebAPIKey() const
    {
        return m_webAPIKey;
    }

    void setWebAPIKey(const QString &webAPIKey)
    {
        m_webAPIKey = webAPIKey;
    }

private:
    Config() = default;
    ~Config() = default;

private:
    QSettings *m_settings = nullptr;

    QString m_webAPIKey = DEFAULT_WEB_API_KEY;
};

WEATHER_WIDGETS_END_NAMESPACE
