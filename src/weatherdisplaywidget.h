#pragma once

#include "data.hpp"
#include "global.h"

#include <DBlurEffectWidget>
#include <DGuiApplicationHelper>

WEATHER_WIDGETS_BEGIN_NAMESPACE

class WeatherDisplayWidget : public Dtk::Widget::DBlurEffectWidget
{
    Q_OBJECT

public:
    explicit WeatherDisplayWidget(QWidget *parent = nullptr);

    void setData(const WeatherNow &weather);

protected:
    void paintEvent(QPaintEvent *event) override;

private:
    void initUI();
    void initConnections();

    void paintWeatherIcon(QPainter &painter);
    void paintTemperature(QPainter &painter, QRect &rect);
    void paintText(QPainter &painter, QRect &rect);
    void paintWindDirAndScale(QPainter &painter, QRect &rect);

private slots:
    void slotThemeTypeChanged(Dtk::Gui::DGuiApplicationHelper::ColorType themeType);

private:
    WeatherNow m_weather;
};

WEATHER_WIDGETS_END_NAMESPACE
