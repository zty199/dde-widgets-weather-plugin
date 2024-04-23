#include "weatherdisplaywidget.h"

#include <DFontSizeManager>

#include <QPainter>
#include <QFile>

DGUI_USE_NAMESPACE

namespace dwweather {
const int nIconSize = 84;

WeatherDisplayWidget::WeatherDisplayWidget(QWidget *parent)
    : DBlurEffectWidget(parent)
{
    initUI();
    initConnections();

    slotThemeTypeChanged(DGuiApplicationHelper::instance()->themeType());
}

void WeatherDisplayWidget::setData(const WeatherNow &weather)
{
    m_weather = weather;
    update();
}

void WeatherDisplayWidget::paintEvent(QPaintEvent *event)
{
    DBlurEffectWidget::paintEvent(event);

    QPainter painter(this);
    paintWeatherIcon(painter);

    QRect rect = this->rect();
    paintWindDirAndScale(painter, rect);
    paintText(painter, rect);
    paintTemperature(painter, rect);
}

void WeatherDisplayWidget::initUI()
{
    setBlurEnabled(true);
    setBlendMode(DBlurEffectWidget::InWindowBlend);
    setFull(true);
}

void WeatherDisplayWidget::initConnections()
{
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, &WeatherDisplayWidget::slotThemeTypeChanged);
}

void WeatherDisplayWidget::paintWeatherIcon(QPainter &painter)
{
    QString icon(":/images/WeatherIcon/%1.png");
    icon = QFile::exists(icon.arg(m_weather.icon)) ? icon.arg(m_weather.icon) : icon.arg(kNAIconName);

    QRect rect = this->rect();
    rect.setSize(QSize(nIconSize, nIconSize));
    rect.moveTopRight(this->rect().topRight() - QPoint(blurRectXRadius(), -blurRectYRadius() - 15));

    painter.drawImage(rect, QImage(icon, "PNG"));
}

void WeatherDisplayWidget::paintTemperature(QPainter &painter, QRect &rect)
{
    painter.save();

    painter.setFont(Dtk::Widget::DFontSizeManager::instance()->t1());
    QString text = m_weather.isValid() ? QString::number(m_weather.temp) : "--";
    text = painter.fontMetrics().elidedText(text + "°", Qt::ElideRight, this->rect().width() - blurRectXRadius() * 2);
    int width = painter.fontMetrics().horizontalAdvance(text) + blurRectXRadius() * 2;
    int height = painter.fontMetrics().lineSpacing();
    rect.setSize(QSize(width, height));
    rect.moveBottomLeft(rect.topLeft() - QPoint(0, 10));
    painter.drawText(rect, Qt::AlignLeft, text);

    painter.restore();
}

void WeatherDisplayWidget::paintText(QPainter &painter, QRect &rect)
{
    painter.save();

    painter.setFont(Dtk::Widget::DFontSizeManager::instance()->t4());
    QString text = m_weather.isValid() ? m_weather.text : tr("Unknown");
    text = painter.fontMetrics().elidedText(text, Qt::ElideRight, this->rect().width() - blurRectXRadius() * 2);
    int width = painter.fontMetrics().horizontalAdvance(text) + blurRectXRadius() * 2;
    int height = painter.fontMetrics().lineSpacing();
    rect.setSize(QSize(width, height));
    rect.moveBottomLeft(rect.topLeft() - QPoint(0, 10));
    painter.drawText(rect, Qt::AlignLeft, text);

    painter.restore();
}

void WeatherDisplayWidget::paintWindDirAndScale(QPainter &painter, QRect &rect)
{
    painter.save();

    painter.setFont(Dtk::Widget::DFontSizeManager::instance()->t8());
    QString text = m_weather.isValid()
                       ? tr("Wind Dir: %1 Scale: %2").arg(m_weather.windDir).arg(m_weather.windScale)
                       : tr("Unknown");
    text = painter.fontMetrics().elidedText(text, Qt::ElideRight, this->rect().width() - blurRectXRadius() * 2);
    int width = painter.fontMetrics().horizontalAdvance(text) + blurRectXRadius() * 2;
    int height = painter.fontMetrics().lineSpacing();
    rect.setSize(QSize(width, height));
    rect.moveBottomLeft(QPoint(blurRectXRadius(), this->rect().bottom() - blurRectYRadius() - 10));
    painter.drawText(rect, Qt::AlignLeft, text);

    painter.restore();
}

void WeatherDisplayWidget::slotThemeTypeChanged(DGuiApplicationHelper::ColorType themeType)
{
    setMaskColor(themeType == DGuiApplicationHelper::DarkType ? DBlurEffectWidget::DarkColor : DBlurEffectWidget::LightColor);
    setMaskAlpha(0);
}
} // namespace dwweather
