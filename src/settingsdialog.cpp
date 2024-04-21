#include "settingsdialog.h"
#include "config.hpp"

#include <QGridLayout>
#include <QAbstractButton>
#include <QRegExpValidator>
#include <QDesktopServices>
#include <QUrl>

#include <DWidgetUtil>
#include <DFrame>

WEATHER_WIDGETS_BEGIN_NAMESPACE

const QString websiteLinkTemplate = "<a href='%1' style='text-decoration: none; color: #0081FF;'>%2</a>";

SettingsDialog::SettingsDialog(WIDGETS_NAMESPACE::WidgetHandler *handler, QWidget *parent)
    : DDialog(parent)
    , m_handler(handler)
{
    initUI();
    initConnections();

    initSettings();
}

void SettingsDialog::initUI()
{
    Dtk::Widget::moveToCenter(this);

    setTitle(tr("Settings"));

    QWidget *widget = new QWidget(this);
    QGridLayout *layout = new QGridLayout(widget);
    layout->setMargin(0);
    layout->setSpacing(8);

    initLocationWidget(layout);
    initUpdateIntervalWidget(layout);
    Dtk::Widget::DHorizontalLine *seperator = new Dtk::Widget::DHorizontalLine(widget);
    layout->addWidget(seperator, 2, 0, 1, 3);
    initWebAPIKeyWidget(layout);

    addSpacing(16);
    addContent(widget, Qt::AlignTop);
    addButton(tr("Cancel"));
    addButton(tr("Save"), true, DDialog::ButtonRecommend);
}

void SettingsDialog::initLocationWidget(QGridLayout *layout)
{
    Dtk::Widget::DLabel *label = new Dtk::Widget::DLabel(tr("Location"), layout->parentWidget());
    label->setToolTip(tr("Longitude and latitude of your location"));
    layout->addWidget(label, 0, 0, 1, 1, Qt::AlignRight);

    m_locationLineEdit = new Dtk::Widget::DLineEdit(layout->parentWidget());
    QString location = QString("%1,%2").arg(QString::number(DEFAULT_LONGITUDE, 'f', 2), QString::number(DEFAULT_LATITUDE, 'f', 2));
    m_locationLineEdit->setPlaceholderText(tr("e.g. %1").arg(location));
    layout->addWidget(m_locationLineEdit, 0, 1, 1, 2);
}

void SettingsDialog::initUpdateIntervalWidget(QGridLayout *layout)
{
    Dtk::Widget::DLabel *label = new Dtk::Widget::DLabel(tr("Update Interval"), layout->parentWidget());
    label->setToolTip(tr("Unit: secs"));
    layout->addWidget(label, 1, 0, 1, 1, Qt::AlignRight);

    m_updateIntervalLineEdit = new Dtk::Widget::DLineEdit(layout->parentWidget());
    m_updateIntervalLineEdit->setPlaceholderText(tr("e.g. %1").arg(DEFAULT_UPDATE_INTERVAL));
    layout->addWidget(m_updateIntervalLineEdit, 1, 1, 1, 2);
}

void SettingsDialog::initWebAPIKeyWidget(QGridLayout *layout)
{
    m_webAPIKeyLabel = new Dtk::Widget::DLabel(layout->parentWidget());
    m_webAPIKeyLabel->setText(websiteLinkTemplate.arg("https://dev.qweather.com/", tr("Web API KEY")));
    m_webAPIKeyLabel->setToolTip(tr("Click to register your own Web API KEY"));
    layout->addWidget(m_webAPIKeyLabel, 3, 0, 1, 1, Qt::AlignRight);

    m_webAPIKeyLineEdit = new Dtk::Widget::DLineEdit(layout->parentWidget());
    m_webAPIKeyLineEdit->setPlaceholderText(tr("regex: %1").arg("^[0-9a-z]{32}$"));
    layout->addWidget(m_webAPIKeyLineEdit, 3, 1, 1, 2);
}

void SettingsDialog::initSettings()
{
    m_locationLineEdit->setText(m_handler->value(KEY_LOCATION, QString()).toString());

    m_updateIntervalLineEdit->setText(m_handler->value(KEY_INTERVAL, DEFAULT_UPDATE_INTERVAL).toString());

    QString webAPIKey = Config::instance()->getWebAPIKey();
    m_webAPIKeyLineEdit->setText(webAPIKey == DEFAULT_WEB_API_KEY ? QString() : webAPIKey);
}

void SettingsDialog::saveSettings()
{
    m_handler->setValue(KEY_LOCATION, m_locationLineEdit->text());
    m_handler->setValue(KEY_INTERVAL, m_updateIntervalLineEdit->text());

    Config::instance()->setWebAPIKey(m_webAPIKeyLineEdit->text());
}

void SettingsDialog::updateButtonEnabled()
{
    bool enabled = !m_locationLineEdit->isAlert()
                   && !m_updateIntervalLineEdit->isAlert()
                   && !m_webAPIKeyLineEdit->isAlert();

    getButton(QDialog::Accepted)->setEnabled(enabled);
}

void SettingsDialog::initConnections()
{
    connect(m_locationLineEdit, &Dtk::Widget::DLineEdit::textChanged, this, [=](const QString &text) {
        static QRegularExpression regex(
            R"(^[\-\+]?(?:0?\d{1,2}(?:\.\d{1,2})?|1[0-7]?\d{1}(?:\.\d{1,2})?|180(\.0{1,2})?),[\-\+]?(?:[0-8]?\d{1}(?:\.\d{1,2})?|90(?:\.0{1,2})?)$)");
        m_locationLineEdit->setAlert(!regex.match(text).hasMatch());
        updateButtonEnabled();
    });

    connect(m_updateIntervalLineEdit, &Dtk::Widget::DLineEdit::textChanged, this, [=](const QString &text) {
        static QRegularExpression regex(R"(^[1-9][\d]*$)");
        m_updateIntervalLineEdit->setAlert(!regex.match(text).hasMatch() && !text.isEmpty());
        updateButtonEnabled();
    });

    connect(m_webAPIKeyLabel, &Dtk::Widget::DLabel::linkActivated, this, [=](const QString &link) {
        QDesktopServices::openUrl(link);
        close();
    });

    connect(m_webAPIKeyLineEdit, &Dtk::Widget::DLineEdit::textChanged, this, [=](const QString &text) {
        static QRegularExpression regex(R"(^[\da-z]{32}$)");
        m_webAPIKeyLineEdit->setAlert(!regex.match(text).hasMatch() && !text.isEmpty());
        updateButtonEnabled();
    });

    connect(this, &Dtk::Widget::DDialog::buttonClicked, this, [=](int index) {
        switch (index) {
        case QDialog::Rejected:
            close();
            break;
        case QDialog::Accepted:
            saveSettings();
            close();
            break;
        default:
            break;
        }
    });
}

WEATHER_WIDGETS_END_NAMESPACE
