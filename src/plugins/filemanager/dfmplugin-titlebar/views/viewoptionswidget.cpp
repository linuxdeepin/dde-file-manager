// SPDX-FileCopyrightText: 2021 - 2024 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "views/private/viewoptionswidget_p.h"
#include "views/viewoptionswidget.h"
#include "utils/optionbuttonmanager.h"
#include "utils/titlebarhelper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/application/application.h>
#include <dfm-base/base/application/settings.h>
#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <DFontSizeManager>
#include <DPalette>
#include <DPaletteHelper>

#include <QVBoxLayout>
#include <QStandardItemModel>
#include <QKeyEvent>
#include <QApplication>
#include <QScreen>
#include <QToolTip>

DWIDGET_USE_NAMESPACE
using namespace dfmplugin_titlebar;
DFMBASE_USE_NAMESPACE
DFMGLOBAL_USE_NAMESPACE
using DFMBASE_NAMESPACE::Global::ViewMode;
using namespace GlobalDConfDefines::ConfigPath;
using namespace GlobalDConfDefines::BaseConfig;

static constexpr int kViewOptionsButtonIconSize { 16 };
static constexpr int kViewOptionsMargin { 10 };
static constexpr int kViewOptionsWidth { 180 };
static constexpr int kViewOptionsSingleSpacing { 5 };
static constexpr int kViewOptionsFrameMargin { 6 };
static constexpr int kViewOptionsFrameHeight { 40 };

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
    // Find the window associated with this widget
    auto winId = FMWindowsIns.findWindowId(q);
    currentWindow = FMWindowsIns.findWindowById(winId);

    QSize buttonIconSize(kViewOptionsButtonIconSize, kViewOptionsButtonIconSize);
    // Title
    QVBoxLayout *mainLayout = new QVBoxLayout(q);
    mainLayout->setContentsMargins(kViewOptionsMargin, kViewOptionsMargin, kViewOptionsMargin, kViewOptionsMargin);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);
    title = new DLabel(tr("View Options"), q);
    DFontSizeManager::instance()->bind(title, DFontSizeManager::T6, QFont::Normal);
    title->setAlignment(Qt::AlignCenter);
    title->setFixedWidth(kViewOptionsWidth);
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
    DFontSizeManager::instance()->bind(iconSizeTitle, DFontSizeManager::T10, QFont::Normal);
    iconSizeLabelLayout->addWidget(iconSizeLabelFrame);
    iconSizeLabelLayout->addWidget(iconSizeTitle);
    iconSizeLayout->addLayout(iconSizeLabelLayout);
    auto iconSizeWidget = new DFrame(q);
    iconSizeWidget->setFixedHeight(kViewOptionsFrameHeight);
    QHBoxLayout *iconSizeSliderLayout = new QHBoxLayout(q);
    iconSizeSliderLayout->setContentsMargins(kViewOptionsFrameMargin, kViewOptionsFrameMargin,
                                             kViewOptionsFrameMargin, kViewOptionsFrameMargin);
    iconSizeSlider = new DSlider(Qt::Horizontal, iconSizeWidget);
    iconSizeSlider->setMaximum(viewDefines.iconSizeCount() - 1);
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
    DFontSizeManager::instance()->bind(gridDensityTitle, DFontSizeManager::T10, QFont::Normal);
    gridDensityLabelLayout->addWidget(gridDensityLabelFrame);
    gridDensityLabelLayout->addWidget(gridDensityTitle);
    gridDensityLayout->addLayout(gridDensityLabelLayout);
    auto gridDensityWidget = new DFrame(q);
    gridDensityWidget->setFixedHeight(kViewOptionsFrameHeight);
    QHBoxLayout *gridDensitySliderLayout = new QHBoxLayout(q);
    gridDensitySliderLayout->setContentsMargins(kViewOptionsFrameMargin, kViewOptionsFrameMargin,
                                                kViewOptionsFrameMargin, kViewOptionsFrameMargin);
    gridDensitySlider = new DSlider(Qt::Horizontal, gridDensityWidget);
    gridDensitySlider->setMaximum(viewDefines.iconGridDensityCount() - 1);
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
    DFontSizeManager::instance()->bind(listHeightTitle, DFontSizeManager::T10, QFont::Normal);
    listHeightLabelLayout->addWidget(listHeightLabelFrame);
    listHeightLabelLayout->addWidget(listHeightTitle);
    listHeightLayout->addLayout(listHeightLabelLayout);
    auto listHeightWidget = new DFrame(q);
    listHeightWidget->setFixedHeight(kViewOptionsFrameHeight);
    QHBoxLayout *listHeightSliderLayout = new QHBoxLayout(q);
    listHeightSliderLayout->setContentsMargins(kViewOptionsFrameMargin, kViewOptionsFrameMargin,
                                               kViewOptionsFrameMargin, kViewOptionsFrameMargin);
    listHeightSlider = new DSlider(Qt::Horizontal, listHeightWidget);
    listHeightSlider->setMaximum(viewDefines.listHeightCount() - 1);
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
    // Initialize checkbox state from window
    if (currentWindow)
        displayPreviewCheckBox->setChecked(currentWindow->isDetailSpaceVisible());
    QHBoxLayout *displayPreviewLayout = new QHBoxLayout(displayPreviewWidget);
    displayPreviewLayout->setContentsMargins(kViewOptionsFrameMargin, kViewOptionsFrameMargin,
                                             kViewOptionsFrameMargin, kViewOptionsFrameMargin);
    displayPreviewLayout->addWidget(displayPreviewCheckBox);
    displayPreviewWidget->setLayout(displayPreviewLayout);
    mainLayout->addWidget(displayPreviewWidget);
    auto updateLabelColor = [this]() {
        bool isDarkTheme = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;

        DPalette pa = DPaletteHelper::instance()->palette(title);
        QColor textColor = isDarkTheme ? QColor(255, 255, 255, 128)
                                       : QColor(0, 0, 0, 128);

        pa.setColor(DPalette::WindowText, textColor);
        iconSizeTitle->setPalette(pa);
        gridDensityTitle->setPalette(pa);
        listHeightTitle->setPalette(pa);
    };
    updateLabelColor();
    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            title, updateLabelColor);
}

void ViewOptionsWidgetPrivate::initConnect()
{
    connect(displayPreviewCheckBox, &QCheckBox::checkStateChanged, this, [this](Qt::CheckState state) {
        bool isChecked = (state == Qt::Checked);
        // Check against window state instead of DConfig
        if (currentWindow) {
            bool currentVisible = currentWindow->isDetailSpaceVisible();
            if (isChecked == currentVisible)
                return;
        }
        fmDebug() << "Display preview state changed to:" << isChecked;
        Q_EMIT q->displayPreviewVisibleChanged(isChecked, true);   // User action from checkbox
    });

    // Sync checkbox state when window's detailSpace visibility changes (e.g., drag-to-hide)
    if (currentWindow) {
        connect(currentWindow, &FileManagerWindow::detailSpaceVisibilityChanged, this,
                [this](bool visible) {
                    if (displayPreviewCheckBox->isChecked() != visible) {
                        displayPreviewCheckBox->setChecked(visible);
                    }
                });
    }

    connect(iconSizeSlider, &DSlider::valueChanged, this, [this](int value) {
        fmDebug() << "iconSizeSlider value changed: " << value;
        TitleBarHelper::setFileViewStateValue(fileUrl, "iconSizeLevel", value);
        Application::appObtuselySetting()->sync();
        fmDebug() << "Icon size level saved to settings for URL:" << fileUrl.toString();
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
        TitleBarHelper::setFileViewStateValue(fileUrl, "gridDensityLevel", value);
        Application::appObtuselySetting()->sync();
        fmDebug() << "Grid density level saved to settings for URL:" << fileUrl.toString();
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
        TitleBarHelper::setFileViewStateValue(fileUrl, "listHeightLevel", value);
        Application::appObtuselySetting()->sync();
        fmDebug() << "List height level saved to settings for URL:" << fileUrl.toString();
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

    connectSliderTip(iconSizeSlider, &ViewDefines::getIconSizeList);
    connectSliderTip(gridDensitySlider, &ViewDefines::getIconGridDensityList);
    connectSliderTip(listHeightSlider, &ViewDefines::getListHeightList);
}

void ViewOptionsWidgetPrivate::setUrl(const QUrl &url)
{
    fmDebug() << "Setting URL for view options:" << url.toString();
    fileUrl = url;

    QVariant defaultIconSize = Application::instance()->appAttribute(Application::kIconSizeLevel).toInt();
    QVariant iconSizeValue = TitleBarHelper::getFileViewStateValue(fileUrl, "iconSizeLevel", defaultIconSize);
    iconSizeSlider->blockSignals(true);
    iconSizeSlider->setValue(iconSizeValue.toInt());
    iconSizeSlider->blockSignals(false);
    fmDebug() << "iconSizeLevel: " << iconSizeValue.toInt();

    QVariant defaultGridDensity = Application::instance()->appAttribute(Application::kGridDensityLevel).toInt();
    QVariant gridDensityValue = TitleBarHelper::getFileViewStateValue(fileUrl, "gridDensityLevel", defaultGridDensity);
    gridDensitySlider->blockSignals(true);
    gridDensitySlider->setValue(gridDensityValue.toInt());
    gridDensitySlider->blockSignals(false);
    fmDebug() << "gridDensityLevel: " << gridDensityValue.toInt();

    QVariant defaultListHeight = Application::instance()->appAttribute(Application::kListHeightLevel).toInt();
    QVariant listHeightValue = TitleBarHelper::getFileViewStateValue(fileUrl, "listHeightLevel", defaultListHeight);
    listHeightSlider->blockSignals(true);
    listHeightSlider->setValue(listHeightValue.toInt());
    listHeightSlider->blockSignals(false);
    fmDebug() << "listHeightLevel: " << listHeightValue.toInt();
}

void ViewOptionsWidgetPrivate::switchMode(ViewMode mode)
{
    fmDebug() << "Switching view options mode to:" << static_cast<int>(mode);
    bool iconVisible = (mode == ViewMode::kIconMode);
    bool listVisible = (mode == ViewMode::kListMode || mode == ViewMode::kTreeMode);
    if (OptionButtonManager::instance()->hasVsibleState(fileUrl.scheme())) {
        auto state = OptionButtonManager::instance()->optBtnVisibleState(fileUrl.scheme());
        bool hideListHeightOpt = state & OptionButtonManager::kHideListHeightOpt;
        listVisible = listVisible && !hideListHeightOpt;
        fmDebug() << "Option button visibility state applied, hideListHeightOpt:" << hideListHeightOpt;
    }
    iconSizeFrame->setVisible(iconVisible);
    gridDensityFrame->setVisible(iconVisible);
    listHeightFrame->setVisible(listVisible);
}

void ViewOptionsWidgetPrivate::showSliderTips(Dtk::Widget::DSlider *slider, int pos, const QVariantList &valList)
{
    if (pos >= valList.count() || valList.count() <= 1) {
        fmWarning() << "Invalid slider tip position:" << pos << "or insufficient values:" << valList.count();
        return;
    }
    int offset = (pos * (slider->slider()->width() - 28)) / (valList.count() - 1);
    QPoint showPoint = slider->slider()->mapToGlobal(QPoint(offset, -52));
    QToolTip::showText(showPoint, valList.at(pos).toString(), slider);
}

QList<QString> ViewOptionsWidgetPrivate::getStringListByIntList(const QList<int> &intList)
{
    QList<QString> stringList;
    for (int value : intList) {
        stringList << QString::number(value);
    }
    return stringList;
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

    // Calculate appropriate display position to ensure widget stays within screen bounds
    QPoint showPos = pos;

    if (QApplication::screenAt(pos)) {

        QRect screenRect = QApplication::screenAt(pos)->availableGeometry();

        // Check right boundary
        if (pos.x() + width() > screenRect.right()) {
            showPos.setX(screenRect.right() - width());
        }

        // Check left boundary
        if (showPos.x() < screenRect.left()) {
            showPos.setX(screenRect.left());
        }

        // Check bottom boundary
        if (pos.y() + height() > screenRect.bottom()) {
            showPos.setY(screenRect.bottom() - height());
        }

        // Check top boundary
        if (showPos.y() < screenRect.top()) {
            showPos.setY(screenRect.top());
        }
    } else {
        fmWarning() << "Could not determine screen for position:" << pos;
    }

    move(showPos);
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

template<typename Func>
void ViewOptionsWidgetPrivate::connectSliderTip(Dtk::Widget::DSlider *slider, Func getValueList)
{
    auto valList = (viewDefines.*getValueList)();
    connect(slider, &DSlider::sliderMoved, this, [this, slider, valList](int pos) {
        showSliderTips(slider, pos, valList);
    });
    connect(slider, &DSlider::sliderPressed, this, [this, slider, valList] {
        int position = slider->slider()->sliderPosition();
        showSliderTips(slider, position, valList);
    });
}
