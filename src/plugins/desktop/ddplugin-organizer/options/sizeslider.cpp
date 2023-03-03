// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "sizeslider.h"
#include "organizer_defines.h"
#include "delegate/collectionitemdelegate.h"
#include "config/configpresenter.h"
#include "utils/organizerutils.h"

#include <dfm-framework/dpf.h>

#include <DIconButton>

#include <QVBoxLayout>
#include <QLabel>
#include <QPainter>

DWIDGET_USE_NAMESPACE
using namespace ddplugin_organizer;

SizeSlider::SizeSlider(QWidget *parent) : ContentBackgroundWidget(parent)
{
    dpfSignalDispatcher->subscribe("ddplugin_canvas", "signal_CanvasManager_IconSizeChanged", this, &SizeSlider::syncIconLevel);
}

SizeSlider::~SizeSlider()
{
    dpfSignalDispatcher->unsubscribe("ddplugin_canvas", "signal_CanvasManager_IconSizeChanged", this, &SizeSlider::syncIconLevel);
}

void SizeSlider::switchMode(SizeSlider::Mode mode)
{
    if (!slider) {
        QVBoxLayout *box = new QVBoxLayout(this);
        box->setContentsMargins(10, 10, 10, 10);
        setLayout(box);

        label = new QLabel(this);
        label->setFixedHeight(30);
        box->addWidget(label);

        slider = new DSlider(Qt::Horizontal, this);
        box->addWidget(slider);

        QIcon empty = QIcon::fromTheme("empty");
        // keep the sequence calling icon related functions.
        slider->setIconSize(QSize(32, 32));
        // create left button
        slider->setLeftIcon(empty);
        // left icon should be 16pix
        // find left button.
        {
            auto leftBtn = findChildren<DIconButton *>();
            if (leftBtn.size() != 1) {
                qCritical() << "can not find left button" << leftBtn.size();
            } else {
                auto btn = leftBtn.first();
                btn->setIconSize(QSize(16, 16));
            }
        }

        // create right button
        slider->setRightIcon(empty);
        slider->setPageStep(1);
        slider->slider()->setSingleStep(1);
        slider->slider()->setTickInterval(1);
        slider->setEnabledAcrossStyle(true);

        connect(slider, &DSlider::valueChanged, this, &SizeSlider::setIconLevel);
        connect(slider, &DSlider::iconClicked, this, &SizeSlider::iconClicked);
    }

    curMode = mode;
    if (mode == View)
        resetToView();
    else
        resetToIcon();
}

void SizeSlider::resetToView()
{
    // setRange may send valueChanged.
    slider->blockSignals(true);
    slider->slider()->setRange(DisplaySize::kSmaller, DisplaySize::kLarger);
    slider->blockSignals(false);

    slider->setBelowTicks(ticks(slider->maximum() - slider->minimum() + 1));

    label->setText(tr("Display size"));
    auto cur = CfgPresenter->displaySize();

    setValue(cur);
}

void SizeSlider::resetToIcon()
{
    int min = CollectionItemDelegate::minimumIconLevel();
    int max = CollectionItemDelegate::maximumIconLevel();
    Q_ASSERT(max >= min && min > -1);

    label->setText(tr("Icon size"));

    // setRange may send valueChanged.
    slider->blockSignals(true);
    slider->slider()->setRange(min, max);
    slider->blockSignals(false);
    slider->setBelowTicks(ticks(max - min + 1));

    int cur = iconLevel();
    if (min > cur || max < cur) {
        qCritical() << QString("canvas icon level %0 is out of range %1 ~ %2.")
                       .arg(cur).arg(min).arg(max);
        cur = min;
    }

    setValue(cur);
}

void SizeSlider::setValue(int v)
{
    if (!slider || slider->value() == v)
        return;

    if (v < slider->minimum() || v > slider->maximum()) {
        qWarning() << "invalid level " << v << "mode" << curMode;
        return;
    }

    slider->blockSignals(true);
    slider->setValue(v);
    slider->blockSignals(false);
}

void SizeSlider::iconClicked(DSlider::SliderIcons icon, bool checked)
{
    int cur = slider->value();
    if (icon == DSlider::LeftIcon) {
        int min = slider->minimum();
        cur--;
        if (cur >= min)
            slider->setValue(cur);
    } else {
      int max = slider->maximum();
      cur++;
      if (cur <= max)
          slider->setValue(cur);
    }
}

int SizeSlider::iconLevel()
{
    return dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasManager_IconLevel").toInt();
}

QStringList SizeSlider::ticks(int count)
{
    QStringList ret;
    for (int i = 0; i < count; ++i)
        ret << "";
    return ret;
}

void SizeSlider::setIconLevel(int lv)
{
    if (curMode == View)
        emit CfgPresenter->changeDisplaySize(static_cast<DisplaySize>(lv));
    else
        dpfSlotChannel->push("ddplugin_canvas", "slot_CanvasManager_SetIconLevel", lv);
}

void SizeSlider::syncIconLevel(int lv)
{
    // covert icon size to display size .
    if (curMode == View)
       lv = OrganizerUtils::covertIconLevel(lv, true);

    setValue(lv);
}
