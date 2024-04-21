#pragma once

#include "config.hpp"
#include "data.hpp"
#include "global.h"

#include <widgetsinterface.h>

#include <QObject>
#include <QPointer>
#include <QTimer>

WIDGETS_USE_NAMESPACE

WEATHER_WIDGETS_BEGIN_NAMESPACE

class WeatherDisplayWidget;
class WeatherWidget : public QObject
    , public IWidget
{
    Q_OBJECT

public:
    WeatherWidget() = default;
    ~WeatherWidget() override;

    QWidget *view() override;

    void typeChanged(const Type type) override;

    bool initialize(const QStringList &arguments) override;

    void showWidgets() override;
    void hideWidgets() override;

    void aboutToShutdown() override;

    bool enableSettings() override;
    void settings() override;

protected:
    void timerEvent(QTimerEvent *event) override;

private:
    /**
     * @brief 加载翻译文件
     * @note 见 dde-widgets common/helper.hpp
     */
    bool loadTranslator();
    void initConnections();
    /**
     * @brief 更新实时天气数据
     * @param force 强制更新，忽略定时更新限制
     */
    void updateWeatherNow(bool force = false);

private slots:
    void slotGetLocationFromIPFinished(const QString &location);
    void slotGetWeatherNowFromLocation(const WeatherNow &weather);

private:
    QPointer<WeatherDisplayWidget> m_widget;
    QScopedPointer<QBasicTimer> m_timer;

    QString m_location;
    qint64 m_lastUpdateTimeStamp = 0;
    quint64 m_updateInterval = DEFAULT_UPDATE_INTERVAL;
    WeatherNow m_weatherNow;
};

class WeatherWidgetPlugin : public IWidgetPlugin
{
    Q_OBJECT
    Q_PLUGIN_METADATA(IID DdeWidgetsPlugin_iid FILE "plugin.json")
    Q_INTERFACES(WIDGETS_NAMESPACE::IWidgetPlugin)

public:
    QString title() const override;
    QString description() const override;
    QString aboutDescription() const override;
    QIcon logo() const override;
    QStringList contributors() const override;

    IWidget *createWidget() override;

    QVector<IWidget::Type> supportTypes() const override { return {IWidget::Small}; }
};

WEATHER_WIDGETS_END_NAMESPACE
