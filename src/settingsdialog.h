#pragma once

#include "global.h"

#include <widgetsinterface.h>

#include <DDialog>
#include <DLabel>
#include <DLineEdit>

class QGridLayout;

WEATHER_WIDGETS_BEGIN_NAMESPACE

class SettingsDialog : public Dtk::Widget::DDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(WIDGETS_NAMESPACE::WidgetHandler *handler, QWidget *parent = nullptr);

private:
    void initUI();
    void initLocationWidget(QGridLayout *layout);
    void initUpdateIntervalWidget(QGridLayout *layout);
    void initWebAPIKeyWidget(QGridLayout *layout);
    void initConnections();

    /**
     * @brief 读取配置
     */
    void initSettings();
    /**
     * @brief 保存配置
     */
    void saveSettings();

    /**
     * @brief 更新按钮状态
     */
    void updateButtonEnabled();

private:
    WIDGETS_NAMESPACE::WidgetHandler *m_handler = nullptr;

    Dtk::Widget::DLineEdit *m_locationLineEdit = nullptr;
    Dtk::Widget::DLineEdit *m_updateIntervalLineEdit = nullptr;
    Dtk::Widget::DLabel *m_webAPIKeyLabel = nullptr;
    Dtk::Widget::DLineEdit *m_webAPIKeyLineEdit = nullptr;
};

WEATHER_WIDGETS_END_NAMESPACE
