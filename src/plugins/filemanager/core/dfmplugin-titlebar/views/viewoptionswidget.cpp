// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/viewoptionswidget_p.h"
#include "views/viewoptionswidget.h"
#include "events/titlebareventcaller.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>

#include <DFontSizeManager>

#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QKeyEvent>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
using DFMBASE_NAMESPACE::Global::ViewMode;

static constexpr int kViewOptionsButtonIconSize { 16 };
static constexpr int kViewOptionsMargin { 10 };
static constexpr int kViewOptionsSpacing { 10 };
static constexpr int kViewOptionsSingleSpacing { 5 };
static constexpr int kViewOptionsFrameMargin { 6 };
static constexpr int kViewOptionsFrameHeight { 40 };
static constexpr int kViewOptionsMinIconSize { 24 };
static constexpr int kViewOptionsMaxIconSize { 512 };
static constexpr int kViewOptionsIconSizeStep { 8 };
static constexpr int kViewOptionsMinGridDensity { 60 };
static constexpr int kViewOptionsMaxGridDensity { 150 };
static constexpr int kViewOptionsGridDensityStep { 10 };

ViewOptionsWidgetPrivate::ViewOptionsWidgetPrivate(ViewOptionsWidget *qq)
    : QObject(qq), q(qq)
{
    initializeUi();
    initConnect();
}

ViewOptionsWidgetPrivate::~ViewOptionsWidgetPrivate()
{
}

void ViewOptionsWidgetPrivate::initializeUi()
{
    QSize buttonIconSize(kViewOptionsButtonIconSize, kViewOptionsButtonIconSize);
    // Title
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(kViewOptionsMargin, kViewOptionsMargin, kViewOptionsMargin, kViewOptionsMargin);
    title = new DLabel(tr("View Options"), q);
    DFontSizeManager::instance()->bind(title, DFontSizeManager::T6, QFont::Normal);
    title->setAlignment(Qt::AlignCenter);
    mainLayout->addWidget(title);

    // Icon size
    iconSizeFrame = new QFrame(q);
    iconSizeFrame->setContentsMargins(0, 0, 0, 0);
    auto iconSizeLayout = new QVBoxLayout(iconSizeFrame);
    iconSizeLayout->setContentsMargins(0, 0, 0, 0);
    iconSizeLayout->setSpacing(kViewOptionsSingleSpacing);
    QHBoxLayout *iconSizeLabelLayout = new QHBoxLayout();
    QFrame *iconSizeLabelFrame = new QFrame(q);
    iconSizeLabelFrame->setFixedWidth(10);
    iconSizeTitle = new DLabel(tr("Icon size"), q);
    iconSizeLabelLayout->addWidget(iconSizeLabelFrame);
    iconSizeLabelLayout->addWidget(iconSizeTitle);
    iconSizeLayout->addLayout(iconSizeLabelLayout);
    auto iconSizeWidget = new DFrame(q);
    iconSizeWidget->setFixedHeight(kViewOptionsFrameHeight);
    QHBoxLayout *iconSizeSliderLayout = new QHBoxLayout(q);
    iconSizeSliderLayout->setContentsMargins(kViewOptionsFrameMargin, kViewOptionsFrameMargin,
                                             kViewOptionsFrameMargin, kViewOptionsFrameMargin);
    iconSizeSlider = new DSlider(Qt::Horizontal, iconSizeWidget);
    int iconSizeRange = (kViewOptionsMaxIconSize - kViewOptionsMinIconSize) / kViewOptionsIconSizeStep;
    iconSizeSlider->setMaximum(iconSizeRange);
    iconSizeSlider->setMinimum(0);
    iconSizeSlider->setValue(Application::instance()->appAttribute(Application::kIconSizeLevel).toInt());
    iconSizeSlider->slider()->setPageStep(1);
    iconSizeSlider->slider()->setTickInterval(1);
    iconSizeSlider->setLeftIcon(QIcon::fromTheme("dfm_viewoptions_minicon"));
    iconSizeSlider->setRightIcon(QIcon::fromTheme("dfm_viewoptions_maxicon"));
    iconSizeSlider->setIconSize(buttonIconSize);
    iconSizeSliderLayout->addWidget(iconSizeSlider);
    iconSizeWidget->setLayout(iconSizeSliderLayout);
    iconSizeLayout->addWidget(iconSizeWidget);
    iconSizeFrame->setLayout(iconSizeLayout);
    mainLayout->addWidget(iconSizeFrame);

    // Grid density
    gridDensityFrame = new QFrame(q);
    gridDensityFrame->setContentsMargins(0, 0, 0, 0);
    auto gridDensityLayout = new QVBoxLayout(gridDensityFrame);
    gridDensityLayout->setContentsMargins(0, 0, 0, 0);
    gridDensityLayout->setSpacing(kViewOptionsSingleSpacing);
    QHBoxLayout *gridDensityLabelLayout = new QHBoxLayout();
    QFrame *gridDensityLabelFrame = new QFrame(q);
    gridDensityLabelFrame->setFixedWidth(10);
    gridDensityTitle = new DLabel(tr("Grid density"), q);
    gridDensityLabelLayout->addWidget(gridDensityLabelFrame);
    gridDensityLabelLayout->addWidget(gridDensityTitle);
    gridDensityLayout->addLayout(gridDensityLabelLayout);
    auto gridDensityWidget = new DFrame(q);
    gridDensityWidget->setFixedHeight(kViewOptionsFrameHeight);
    QHBoxLayout *gridDensitySliderLayout = new QHBoxLayout(q);
    gridDensitySliderLayout->setContentsMargins(kViewOptionsFrameMargin, kViewOptionsFrameMargin,
                                                kViewOptionsFrameMargin, kViewOptionsFrameMargin);
    gridDensitySlider = new DSlider(Qt::Horizontal, gridDensityWidget);
    int gridDensityRange = (kViewOptionsMaxGridDensity - kViewOptionsMinGridDensity) / kViewOptionsGridDensityStep;
    gridDensitySlider->setMaximum(gridDensityRange);
    gridDensitySlider->setMinimum(0);
    gridDensitySlider->setValue(Application::instance()->appAttribute(Application::kGridDensityLevel).toInt());
    gridDensitySlider->slider()->setPageStep(1);
    gridDensitySlider->slider()->setTickInterval(1);
    gridDensitySlider->setLeftIcon(QIcon::fromTheme("dfm_viewoptions_mingrid"));
    gridDensitySlider->setRightIcon(QIcon::fromTheme("dfm_viewoptions_maxgrid"));
    gridDensitySlider->setIconSize(buttonIconSize);
    gridDensitySliderLayout->addWidget(gridDensitySlider);
    gridDensityWidget->setLayout(gridDensitySliderLayout);
    gridDensityLayout->addWidget(gridDensityWidget);
    gridDensityFrame->setLayout(gridDensityLayout);
    mainLayout->addWidget(gridDensityFrame);

    // List height
    listHeightFrame = new QFrame(q);
    listHeightFrame->setContentsMargins(0, 0, 0, 0);
    auto listHeightLayout = new QVBoxLayout(listHeightFrame);
    listHeightLayout->setContentsMargins(0, 0, 0, 0);
    listHeightLayout->setSpacing(kViewOptionsSingleSpacing);
    QHBoxLayout *listHeightLabelLayout = new QHBoxLayout();
    QFrame *listHeightLabelFrame = new QFrame(q);
    listHeightLabelFrame->setFixedWidth(10);
    listHeightTitle = new DLabel(tr("List height"), q);
    listHeightLabelLayout->addWidget(listHeightLabelFrame);
    listHeightLabelLayout->addWidget(listHeightTitle);
    listHeightLayout->addLayout(listHeightLabelLayout);
    auto listHeightWidget = new DFrame(q);
    listHeightWidget->setFixedHeight(kViewOptionsFrameHeight);
    QHBoxLayout *listHeightSliderLayout = new QHBoxLayout(q);
    listHeightSliderLayout->setContentsMargins(kViewOptionsFrameMargin, kViewOptionsFrameMargin,
                                               kViewOptionsFrameMargin, kViewOptionsFrameMargin);
    listHeightSlider = new DSlider(Qt::Horizontal, listHeightWidget);
    listHeightSlider->setMaximum(2);
    listHeightSlider->setMinimum(0);
    listHeightSlider->setValue(Application::instance()->appAttribute(Application::kListHeightLevel).toInt());
    listHeightSlider->slider()->setPageStep(1);
    listHeightSlider->slider()->setTickInterval(1);
    listHeightSlider->setLeftIcon(QIcon::fromTheme("dfm_viewoptions_minlist"));
    listHeightSlider->setRightIcon(QIcon::fromTheme("dfm_viewoptions_maxlist"));
    listHeightSlider->setIconSize(buttonIconSize);
    listHeightSliderLayout->addWidget(listHeightSlider);
    listHeightWidget->setLayout(listHeightSliderLayout);
    listHeightLayout->addWidget(listHeightWidget);
    listHeightFrame->setLayout(listHeightLayout);
    mainLayout->addWidget(listHeightFrame);

    // Display preview
    displayPreviewWidget = new DFrame(q);
    displayPreviewWidget->setFixedHeight(kViewOptionsFrameHeight);
    displayPreviewCheckBox = new QCheckBox(tr("Display preview"), displayPreviewWidget);
    displayPreviewCheckBox->setChecked(Application::instance()->appAttribute(Application::kShowedDisplayPreview).toBool());
    QHBoxLayout *displayPreviewLayout = new QHBoxLayout(displayPreviewWidget);
    displayPreviewLayout->setContentsMargins(kViewOptionsFrameMargin, kViewOptionsFrameMargin,
                                             kViewOptionsFrameMargin, kViewOptionsFrameMargin);
    displayPreviewLayout->addWidget(displayPreviewCheckBox);
    displayPreviewWidget->setLayout(displayPreviewLayout);
    mainLayout->addWidget(displayPreviewWidget);
}

void ViewOptionsWidgetPrivate::initConnect()
{
    connect(displayPreviewCheckBox, &QCheckBox::stateChanged, this, [this](int state) {
        bool isChecked = (state == Qt::Checked);
        Q_EMIT q->displayPreviewVisibleChanged(isChecked);
        Application::instance()->setAppAttribute(Application::kShowedDisplayPreview, isChecked);
    });

    connect(iconSizeSlider, &DSlider::valueChanged, this, [this](int value) {
        fmDebug() << "iconSizeSlider value changed: " << value;
        QVariantMap map = Application::appObtuselySetting()->value("FileViewState", fileUrl).toMap();
        map["iconSizeLevel"] = value;
        Application::appObtuselySetting()->setValue("FileViewState", fileUrl, map);
        Application::appObtuselySetting()->sync();
    });
    connect(iconSizeSlider, &DSlider::iconClicked, this, [this](DSlider::SliderIcons icon, bool checked) {
        if (icon == DSlider::LeftIcon) {
            int newValue = iconSizeSlider->value() - 1;
            if (newValue >= iconSizeSlider->minimum())
                iconSizeSlider->slider()->setValue(newValue);
        } else if (icon == DSlider::RightIcon) {
            int newValue = iconSizeSlider->value() + 1;
            if (newValue <= iconSizeSlider->maximum())
                iconSizeSlider->slider()->setValue(newValue);
        }
    });
    connect(gridDensitySlider, &DSlider::valueChanged, this, [this](int value) {
        fmDebug() << "gridDensitySlider value changed: " << value;
        QVariantMap map = Application::appObtuselySetting()->value("FileViewState", fileUrl).toMap();
        map["gridDensityLevel"] = value;
        Application::appObtuselySetting()->setValue("FileViewState", fileUrl, map);
        Application::appObtuselySetting()->sync();
    });
    connect(gridDensitySlider, &DSlider::iconClicked, this, [this](DSlider::SliderIcons icon, bool checked) {
        if (icon == DSlider::LeftIcon) {
            int newValue = gridDensitySlider->value() - 1;
            if (newValue >= gridDensitySlider->minimum())
                gridDensitySlider->slider()->setValue(newValue);
        } else if (icon == DSlider::RightIcon) {
            int newValue = gridDensitySlider->value() + 1;
            if (newValue <= gridDensitySlider->maximum())
                gridDensitySlider->slider()->setValue(newValue);
        }
    });
    connect(listHeightSlider, &DSlider::valueChanged, this, [this](int value) {
        fmDebug() << "listHeightSlider value changed: " << value;
        QVariantMap map = Application::appObtuselySetting()->value("FileViewState", fileUrl).toMap();
        map["listHeightLevel"] = value;
        Application::appObtuselySetting()->setValue("FileViewState", fileUrl, map);
        Application::appObtuselySetting()->sync();
    });
    connect(listHeightSlider, &DSlider::iconClicked, this, [this](DSlider::SliderIcons icon, bool checked) {
        if (icon == DSlider::LeftIcon) {
            int newValue = listHeightSlider->value() - 1;
            if (newValue >= listHeightSlider->minimum())
                listHeightSlider->slider()->setValue(newValue);
        } else if (icon == DSlider::RightIcon) {
            int newValue = listHeightSlider->value() + 1;
            if (newValue <= listHeightSlider->maximum())
                listHeightSlider->slider()->setValue(newValue);
        }
    });
}

void ViewOptionsWidgetPrivate::setUrl(const QUrl &url)
{
    fileUrl = url;
    QVariantMap map = Application::appObtuselySetting()->value("FileViewState", fileUrl).toMap();
    QVariant defaultIconSize = Application::instance()->appAttribute(Application::kIconSizeLevel).toInt();
    iconSizeSlider->blockSignals(true);
    iconSizeSlider->setValue(map.value("iconSizeLevel", defaultIconSize).toInt());
    iconSizeSlider->blockSignals(false);
    fmDebug() << "iconSizeLevel: " << map.value("iconSizeLevel", defaultIconSize).toInt();

    QVariant defaultGridDensity = Application::instance()->appAttribute(Application::kGridDensityLevel).toInt();
    gridDensitySlider->blockSignals(true);
    gridDensitySlider->setValue(map.value("gridDensityLevel", defaultGridDensity).toInt());
    gridDensitySlider->blockSignals(false);
    fmDebug() << "gridDensityLevel: " << map.value("gridDensityLevel", defaultGridDensity).toInt();

    QVariant defaultListHeight = Application::instance()->appAttribute(Application::kListHeightLevel).toInt();
    listHeightSlider->blockSignals(true);
    listHeightSlider->setValue(map.value("listHeightLevel", defaultListHeight).toInt());
    listHeightSlider->blockSignals(false);
    fmDebug() << "listHeightLevel: " << map.value("listHeightLevel", defaultListHeight).toInt();
}

void ViewOptionsWidgetPrivate::switchMode(ViewMode mode)
{
    bool iconVisible = (mode == ViewMode::kIconMode);
    bool listVisible = (mode == ViewMode::kListMode || mode == ViewMode::kTreeMode);
    iconSizeFrame->setVisible(iconVisible);
    gridDensityFrame->setVisible(iconVisible);
    listHeightFrame->setVisible(listVisible);
    int widgetHeight = kViewOptionsMargin + kViewOptionsFrameHeight + kViewOptionsSpacing + title->fontMetrics().height();
    int singleHeight = kViewOptionsFrameHeight + iconSizeTitle->fontMetrics().height() + kViewOptionsSingleSpacing + kViewOptionsSpacing;
    if (iconVisible) {
        widgetHeight += 2 * singleHeight;
    }
    if (listVisible) {
        widgetHeight += singleHeight;
    }
    q->setFixedHeight(widgetHeight);
}

ViewOptionsWidget::ViewOptionsWidget(QWidget *parent)
    : DBlurEffectWidget(parent), d(new ViewOptionsWidgetPrivate(this))
{
    setWindowFlag(Qt::Popup);
    setBlurEnabled(true);
    setMode(DBlurEffectWidget::GaussianBlur);
    resize(202, 144);
}

ViewOptionsWidget::~ViewOptionsWidget() = default;

void ViewOptionsWidget::exec(const QPoint &pos, DFMBASE_NAMESPACE::Global::ViewMode mode, const QUrl &url)
{
    d->setUrl(url);
    d->switchMode(mode);
    move(pos);
    show();

    QEventLoop eventLoop;
    connect(this, &ViewOptionsWidget::hidden, &eventLoop, &QEventLoop::quit);
    (void)eventLoop.exec(QEventLoop::DialogExec);
}

void ViewOptionsWidget::hideEvent(QHideEvent *event)
{
    emit hidden();
    DBlurEffectWidget::hideEvent(event);
}
