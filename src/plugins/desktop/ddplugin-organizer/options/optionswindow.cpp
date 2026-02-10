// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "optionswindow.h"
#include "optionswindow_p.h"
#include "config/configpresenter.h"

#include <dfm-framework/dpf.h>

#include <DTitlebar>
#include <DLabel>

#include <QApplication>
#include <QScreen>
#include <QSize>

using namespace ddplugin_organizer;
DWIDGET_USE_NAMESPACE

OptionsWindowPrivate::OptionsWindowPrivate(OptionsWindow *qq)
    : QObject(qq), q(qq)
{
    dpfSignalDispatcher->subscribe("ddplugin_canvas", "signal_CanvasManager_AutoArrangeChanged", this, &OptionsWindowPrivate::autoArrangeChanged);
}

OptionsWindowPrivate::~OptionsWindowPrivate()
{
    dpfSignalDispatcher->unsubscribe("ddplugin_canvas", "signal_CanvasManager_AutoArrangeChanged", this, &OptionsWindowPrivate::autoArrangeChanged);
}

bool OptionsWindowPrivate::isAutoArrange()
{
    return dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasManager_AutoArrange").toBool();
}

void OptionsWindowPrivate::setAutoArrange(bool on)
{
    dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasManager_SetAutoArrange", on);
}

void OptionsWindowPrivate::autoArrangeChanged(bool on)
{
    // sync sate that changed on canvas.
    if (autoArrange && autoArrange->checked() != on) {
        autoArrange->setChecked(on);
    }
}

void OptionsWindowPrivate::enableChanged(bool enable)
{
    if (organization) {
        organization->reset();

        // adjust size when subwidgets size changed.
        contentWidget->adjustSize();
        q->adjustSize();

        sizeSlider->resetToIcon();
    }
}

OptionsWindow::OptionsWindow(QWidget *parent)
    : DAbstractDialog(parent), d(new OptionsWindowPrivate(this))
{
}

OptionsWindow::~OptionsWindow()
{
}

bool OptionsWindow::initialize()
{
    Q_ASSERT(!layout());
    Q_ASSERT(!d->mainLayout);
    setFocusPolicy(Qt::FocusPolicy::StrongFocus);   // 为了单击widget时，清除其他控件上的焦点

    // main layout
    auto mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->setSizeConstraint(QLayout::SetFixedSize);   // update size when subwidget is removed.
    setLayout(mainLayout);
    d->mainLayout = mainLayout;

    // title
    auto titleBar = new DTitlebar(this);
    titleBar->setMenuVisible(false);
    titleBar->setBackgroundTransparent(true);
    mainLayout->addWidget(titleBar, 0, Qt::AlignTop);
    DLabel *titleLabel = new DLabel(tr("Desktop Settings"), this);
    auto font = titleLabel->font();
    font.setWeight(QFont::Medium);
    titleLabel->setFont(font);
    mainLayout->addWidget(titleLabel, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(10);

    // content
    d->contentWidget = new QWidget(this);
    mainLayout->addWidget(d->contentWidget);

    auto contentLayout = new QVBoxLayout(d->contentWidget);
    contentLayout->setContentsMargins(10, 0, 10, 0);
    contentLayout->setSpacing(0);
    contentLayout->setSizeConstraint(QLayout::SetFixedSize);
    d->contentLayout = contentLayout;
    d->contentWidget->setLayout(contentLayout);

    // auto arrange
    d->autoArrange = new SwitchWidget(tr("Auto arrange icons"), this);
    d->autoArrange->setChecked(d->isAutoArrange());
    d->autoArrange->setFixedHeight(48);
    d->autoArrange->setRoundEdge(SwitchWidget::kBoth);
    contentLayout->addWidget(d->autoArrange);
    connect(d->autoArrange, &SwitchWidget::checkedChanged, this, [this](bool check) {
        d->setAutoArrange(check);
    });
    contentLayout->addSpacing(10);

    // size slider
    d->sizeSlider = new SizeSlider(this);
    d->sizeSlider->setMinimumWidth(400);
    d->sizeSlider->setRoundEdge(SwitchWidget::kBoth);
    d->sizeSlider->setFixedHeight(94);
    d->sizeSlider->init();
    contentLayout->addWidget(d->sizeSlider);
    contentLayout->addSpacing(10);

    // enable desktop organizer
    d->enableOrganize = new SwitchWidget(tr("Enable desktop organizer"), this);
    d->enableOrganize->setChecked(CfgPresenter->isEnable());
    d->enableOrganize->setFixedHeight(48);
    d->enableOrganize->setRoundEdge(SwitchWidget::kBoth);
    contentLayout->addWidget(d->enableOrganize);
    connect(d->enableOrganize, &SwitchWidget::checkedChanged, this, [](bool check) {
        CfgPresenter->changeEnableState(check);
    });
    contentLayout->addSpacing(10);

    // organization
    d->organization = new OrganizationGroup(d->contentWidget);
    d->organization->reset();
    contentLayout->addWidget(d->organization);
    adjustSize();

    // must be queued
    connect(CfgPresenter, &ConfigPresenter::changeEnableState, d, &OptionsWindowPrivate::enableChanged, Qt::QueuedConnection);
    return true;
}

void OptionsWindow::moveToCenter(const QPoint &cursorPos)
{
    if (auto screen = QGuiApplication::screenAt(cursorPos)) {
        auto pos = (screen->size() - size()) / 2.0;
        if (!pos.isValid()) {
            move(screen->geometry().topLeft());
        } else {
            auto topleft = screen->geometry().topLeft();
            move(topleft.x() + pos.width(), topleft.y() + pos.height());
        }
    }
}
