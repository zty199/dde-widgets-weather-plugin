#include "plugin.h"
#include "config.hpp"
#include "weatherdisplaywidget.h"
#include "qweatherapi.h"
#include "settingsdialog.h"

#include <DStandardPaths>

#include <QTranslator>

namespace dwweather {
WeatherWidget::~WeatherWidget()
{
    if (!m_timer.isNull()) {
        m_timer->stop();
    }
}

QWidget *WeatherWidget::view()
{
    return m_widget;
}

void WeatherWidget::typeChanged(const Type type)
{
    Q_UNUSED(type)
    m_widget->setFixedSize(handler()->size());
}

bool WeatherWidget::initialize(const QStringList &arguments)
{
    Q_UNUSED(arguments)

    static bool loaded = false;
    if (!loaded) {
        loaded = loadTranslator();
    }

    m_widget = new WeatherDisplayWidget;
    m_widget->setRadius(handler()->roundedCornerRadius());
    m_widget->setBlurRectXRadius(handler()->roundedCornerRadius());
    m_widget->setBlurRectYRadius(handler()->roundedCornerRadius());

    m_timer.reset(new QBasicTimer);

    Config::instance()->initialize(qApp->organizationName(), qApp->applicationName(), handler()->pluginId());

    m_location = handler()->value(KEY_LOCATION, QString()).toString().trimmed();
    m_lastUpdateTimeStamp = handler()->value(KEY_TIMESTAMP, 0).toLongLong();
    m_updateInterval = handler()->value(KEY_INTERVAL, DEFAULT_UPDATE_INTERVAL).toULongLong();
    m_weatherNow = WeatherNow(handler()->value(KEY_WEATHER, QByteArray()).toByteArray());

    /**
     * FIXME: “编辑小组件”功能提供组件预览，实际会多初始化一个临时实例；
     * 该实例无法保存配置，导致 dde-widgets 每次启动都会重新获取天气
     *
     * 故可以利用该特性添加配置设置，在初始化时判断配置是否被保存，
     * 以此避免预览临时组件频繁重新获取天气
     */
    handler()->setValue(KEY_ID, handler()->id());

    initConnections();

    return true;
}

void WeatherWidget::showWidgets()
{
    /**
     * FIXME: 判断配置是否添加成功，以此区分预览临时组件
     * 预览组件无需进行后续请求数据操作
     */
    if (!handler()->containsValue(KEY_ID)) {
        return;
    }

    if (m_weatherNow.isValid()) {
        m_widget->setData(m_weatherNow);
    }

    if (m_location.isEmpty()) {
        QWeatherAPI::instance()->getLocationFromIP();
    } else {
        updateWeatherNow();
    }
}

void WeatherWidget::hideWidgets()
{
    m_timer->stop();
}

void WeatherWidget::aboutToShutdown()
{
    /**
     * FIXME: 计划在组件移除时存储 API KEY,
     * 方便下次放入组件时使用
     *
     * 目前发现删除小组件时可能写入失败，
     * 可能与配置文件操作顺序相关
     */
    Config::instance()->saveWebAPIKey();
}

bool WeatherWidget::enableSettings()
{
    return true;
}

void WeatherWidget::settings()
{
    SettingsDialog *dialog = new SettingsDialog(handler(), m_widget);
    if (dialog->exec() == QDialog::Accepted) {
        quint64 updateInterval = handler()->value(KEY_INTERVAL, DEFAULT_UPDATE_INTERVAL).toULongLong();
        if (m_updateInterval != updateInterval) {
            m_updateInterval = updateInterval;
            // NOTE: 更新时间间隔修改，重置定时器
            if (m_timer->isActive()) {
                m_timer->start(m_updateInterval, this);
            }
        }

        // NOTE: 仅在区域变化时强制更新
        QString location = handler()->value(KEY_LOCATION, QString()).toString().trimmed();
        updateWeatherNow(m_location != location);
        m_location = location;
    }
    dialog->deleteLater();
}

void WeatherWidget::timerEvent(QTimerEvent *event)
{
    if (event->timerId() == m_timer->timerId()) {
        updateWeatherNow();
    }

    return QObject::timerEvent(event);
}

bool WeatherWidget::loadTranslator()
{
    QString locale = IWidget::userInterfaceLanguage();
    if (locale.isEmpty()) {
        return false;
    }

    QString fileName = QString("dde-widgets-weather_%1").arg(locale);

    // translations dirs.
    QStringList dirs {"./translations/"};
    const auto &genDatas = Dtk::Core::DStandardPaths::standardLocations(
        QStandardPaths::GenericDataLocation);
    for (const auto &path : qAsConst(genDatas)) {
        dirs << path + "/dde-widgets/translations/";
    }

    auto qtl = new QTranslator;
    for (auto dir : dirs) {
        if (qtl->load(fileName, dir)) {
            qApp->installTranslator(qtl);
            qCInfo(dwLog) << QString("load translation [%1] successful.").arg(fileName);
            return true;
        }
    }

    if (qtl->isEmpty()) {
        qCWarning(dwLog) << QString("load translation [%1] error from those dirs.").arg(fileName)
                         << dirs;
        qtl->deleteLater();
    }

    return false;
}

void WeatherWidget::initConnections()
{
    connect(QWeatherAPI::instance(), &QWeatherAPI::sigGetLocationFromIPFinished,
            this, &WeatherWidget::slotGetLocationFromIPFinished, Qt::UniqueConnection);
    connect(QWeatherAPI::instance(), &QWeatherAPI::sigGetWeatherNowFromLocation,
            this, &WeatherWidget::slotGetWeatherNowFromLocation, Qt::UniqueConnection);
}

void WeatherWidget::updateWeatherNow(bool force)
{
    // NOTE: 超过时间间隔 / 跨天 / 缓存数据无效 或 强制 时更新
    QDateTime lastUpdate = QDateTime::fromSecsSinceEpoch(m_lastUpdateTimeStamp);
    QDateTime current = QDateTime::currentDateTime();
    qint64 secs = lastUpdate.secsTo(current);

    if (!force
        && secs >= 0
        && secs < m_updateInterval
        && lastUpdate.daysTo(current) < 1
        && m_weatherNow.isValid()) {
        return;
    }

    QWeatherAPI::instance()->getWeatherNowFromLocation(m_location);
}

void WeatherWidget::slotGetLocationFromIPFinished(const QString &location)
{
    if (!m_location.isEmpty() && m_location == location.trimmed()) {
        return;
    }

    m_location = location.trimmed();
    if (m_location.isEmpty()) {
        m_location = QString("%1,%2").arg(DEFAULT_LONGITUDE).arg(DEFAULT_LATITUDE);
    }

    handler()->setValue(KEY_LOCATION, m_location);
    updateWeatherNow(true);
}

void WeatherWidget::slotGetWeatherNowFromLocation(const WeatherNow &weather)
{
    m_weatherNow = weather;
    handler()->setValue(KEY_WEATHER, m_weatherNow.rawData);
    m_widget->setData(m_weatherNow);

    // NOTE: 获取实时天气数据成功后，更新时间戳状态
    if (weather.isValid()) {
        m_lastUpdateTimeStamp = QDateTime::currentSecsSinceEpoch();
        handler()->setValue(KEY_TIMESTAMP, m_lastUpdateTimeStamp);
        m_timer->start(m_updateInterval * 1000, this);
    }
}

QString WeatherWidgetPlugin::title() const
{
    return tr("Weather");
}

QString WeatherWidgetPlugin::description() const
{
    return tr("Display weather at current geographical location");
}

QString WeatherWidgetPlugin::aboutDescription() const
{
    return tr("Weather data is obtained base on QWeather API");
}

QIcon WeatherWidgetPlugin::logo() const
{
    return QIcon::fromTheme(":/images/WeatherIcon/102.png");
}

QStringList WeatherWidgetPlugin::contributors() const
{
    return {"linuxdeepin / zty199"};
}

IWidget *WeatherWidgetPlugin::createWidget()
{
    return new WeatherWidget;
}
} // namespace dwweather
