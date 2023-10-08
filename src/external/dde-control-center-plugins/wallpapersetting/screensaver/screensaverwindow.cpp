// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "screensaverwindow.h"
#include "screensaverprovider.h"

#include <DGuiApplicationHelper>

DWIDGET_USE_NAMESPACE
using namespace dfm_wallpapersetting;

ScreensaverWindow::ScreensaverWindow(QWidget *parent) : QWidget(parent)
{

}

ScreensaverWindow::~ScreensaverWindow()
{
    if (region)
        region->unregisterRegion();

    if (provider)
        provider->stopPreview();
}

void ScreensaverWindow::initialize(ScreensaverProvider *ptr)
{
    if (provider)
        return;

    Q_ASSERT(ptr);
    provider = ptr;

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    setLayout(mainLayout);

    preview = new ScreensaverPreview(this);
    preview->setFixedSize(PREVIEW_ICON_WIDTH + 2 * PREVIEW_ICON_MARGIN,
                          PREVIEW_ICON_HEIGHT + 2 * PREVIEW_ICON_MARGIN);
    mainLayout->addSpacing(20);
    mainLayout->addWidget(preview, 0, Qt::AlignCenter);

    itemModel = new ItemModel(this);

    listView = new ListView;
    listView->setModel(itemModel);
    listView->initialize();
    listView->setShowDelete(false);
    mainLayout->addWidget(listView);

    mainLayout->addSpacing(10);
    customBtn = new CustomButton(this);
    customBtn->setEnabled(false);
    mainLayout->addWidget(customBtn);

    mainLayout->addSpacing(10);
    idle = new IdleTime(this);
    connect(idle, &IdleTime::getCurrentIdle, provider, &ScreensaverProvider::getCurrentIdle, Qt::DirectConnection);
    connect(idle, &IdleTime::setCurrentIdle, provider, &ScreensaverProvider::setCurrentIdle);
    connect(idle, &IdleTime::getIsLock, provider, &ScreensaverProvider::getIsLock, Qt::DirectConnection);
    connect(idle, &IdleTime::setIsLock, provider, &ScreensaverProvider::setIsLock);

    mainLayout->addWidget(idle);

    auto vSpaceItem = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred);
    mainLayout->addSpacerItem(vSpaceItem);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &ScreensaverWindow::onThmeChanged, Qt::QueuedConnection);

    onThmeChanged();

    connect(listView, &ListView::itemClicked, this, &ScreensaverWindow::itemClicked);
    connect(customBtn, &CustomButton::clicked, this, &ScreensaverWindow::configItem);
    connect(preview, &ScreensaverPreview::clicked, this, &ScreensaverWindow::startPreview);

    region = new DRegionMonitor(this);
    // ignore mouse moving
    region->setRegisterFlags(DRegionMonitor::Button | DRegionMonitor::Key);

    // delay work by queue
    connect(region, &DRegionMonitor::buttonPress, this, &ScreensaverWindow::stopPreview, Qt::QueuedConnection);
    connect(region, &DRegionMonitor::keyPress, this, &ScreensaverWindow::stopPreview, Qt::QueuedConnection);

    connect(provider, &ScreensaverProvider::imageChanged, this, &ScreensaverWindow::repaintItem);
}

void ScreensaverWindow::reset()
{
    idle->reset();

    QList<ItemNodePtr> items;
   // wait 500ms for data
   if (!provider->waitScreensaver(5000)) {
       qWarning() << "the screensaver has not gotten...";
   } else {
       items = provider->allScreensaver();
       qDebug() << "get screensavers" << items.size();
   }

   itemModel->resetData(items);

   // get current wallpaper
   QString cur = provider->current();
   auto idx = itemModel->itemIndex(cur);
   auto ptr = qSharedPointerCast<ScreensaverItem>(itemModel->itemNode(idx));
   if (ptr) {
       listView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
       listView->setCurrentIndex(idx);
       listView->scrollTo(idx);
       listView->update();
       customBtn->setEnabled(ptr->configurable);

       previewItem(ptr->coverPath, ptr->color);
   }
}

void ScreensaverWindow::scrollToCurrent()
{
    listView->scrollTo(listView->currentIndex());
}

void ScreensaverWindow::previewItem(const QString &img, const QColor &bkg)
{
    preview->setBackground(bkg);
    preview->setImage(img);
    preview->update();
}

void ScreensaverWindow::configItem()
{
    auto idx = listView->currentIndex();
    if (auto ptr = itemModel->itemNode(idx))
        provider->configure(ptr->item);
}

void ScreensaverWindow::startPreview()
{
    auto idx = listView->currentIndex();
    if (auto ptr = itemModel->itemNode(idx)) {
        provider->startPreview(ptr->item);
        region->registerRegion();
    }
}

void ScreensaverWindow::stopPreview()
{
    region->unregisterRegion();
    provider->stopPreview();
}

void ScreensaverWindow::onThmeChanged()
{
    bool dark = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    if (preview) {
        preview->setBoder(dark ? QColor(255, 255, 255, 26) : QColor(0, 0, 0, 26));
    }
}

void ScreensaverWindow::repaintItem(const QString &it)
{
    if (!isVisible())
        return;

    listView->update(itemModel->itemIndex(it));
}

void ScreensaverWindow::itemClicked(const ItemNodePtr &ptr)
{
    auto item = qSharedPointerCast<ScreensaverItem>(ptr);
    if (!item)
        return;

    previewItem(item->coverPath, ptr->color);
    customBtn->setEnabled(item->configurable);

    if (provider->current() == item->item)
        return;

    provider->setCurrent(item->item);
}
