// SPDX-FileCopyrightText: 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpaperwindow.h"
#include "wallpaperprovider.h"
#include "docksketch.h"
#include "colordialog.h"

#include <DGuiApplicationHelper>

#include <QFileDialog>
#include <QStandardPaths>
#include <QApplication>
#include <QScreen>
#include <QMenu>

DWIDGET_USE_NAMESPACE
using namespace dfm_wallpapersetting;

#define NEW_ITEM_KEY "add_new_item"
#define SOLID_PREFIX "solid::"

static constexpr char kDefaultWallpaperPath[] = "/usr/share/backgrounds/default_background.jpg";

WallpaperWindow::WallpaperWindow(QWidget *parent) : QWidget(parent)
{

}

WallpaperWindow::~WallpaperWindow()
{
}

void WallpaperWindow::initialize(WallpaperProvider *ptr)
{
    if (provider)
        return;

    Q_ASSERT(ptr);
    provider = ptr;

    mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    setLayout(mainLayout);

    preview = new PreviewWidget(this);
    preview->setFixedSize(PREVIEW_ICON_WIDTH + 2 * PREVIEW_ICON_MARGIN,
                          PREVIEW_ICON_HEIGHT + 2 * PREVIEW_ICON_MARGIN);
    mainLayout->addSpacing(20);
    {
        auto dock = new DockSketch(preview);
        dock->move(PREVIEW_ICON_MARGIN + 8, preview->height() - PREVIEW_ICON_MARGIN - dock->height() - 6);
    }
    mainLayout->addWidget(preview, 0, Qt::AlignCenter);

    modeBox = new ModeButtonBox(this);
    modeBox->setFixedSize(120 * 2, 36);
    modeBox->initialize();

    mainLayout->addSpacing(15);
    mainLayout->addWidget(modeBox, 0, Qt::AlignCenter);

    itemModel = new ItemModel(this);

    listView = new ListView;
    listView->setModel(itemModel);
    listView->initialize();
    listView->setShowDelete(true);
    listView->setContextMenuPolicy(Qt::CustomContextMenu);
    mainLayout->addWidget(listView);

    mainLayout->addSpacing(15);

    //screen
    screenBox = new ScreenComBox;
    mainLayout->addWidget(screenBox);
    screenBox->reset();

    intervalCombox = new IntervalCombox;
    mainLayout->addWidget(intervalCombox);

    auto vSpaceItem = new QSpacerItem(20, 20, QSizePolicy::Minimum, QSizePolicy::Preferred);
    mainLayout->addSpacerItem(vSpaceItem);

    connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged,
            this, &WallpaperWindow::onThmeChanged, Qt::QueuedConnection);

    onThmeChanged();

    connect(modeBox, &ModeButtonBox::switchTo, this, &WallpaperWindow::switchMode);
    connect(screenBox, &ScreenComBox::screenChanged, this, &WallpaperWindow::onScreenChanged);
    connect(screenBox, &ScreenComBox::screenReseted, this, &WallpaperWindow::onScreenRested);

    connect(listView, &ListView::itemClicked, this, &WallpaperWindow::itemClicked);
    connect(listView, &ListView::deleteButtonClicked, this, &WallpaperWindow::onDeleteItem);
    connect(listView, &ListView::customContextMenuRequested, this, &WallpaperWindow::onViewMenu);

    // need retuned data from slot
    connect(intervalCombox, &IntervalCombox::getCurrentSlideShow, provider, &WallpaperProvider::getSlideshow, Qt::DirectConnection);
    connect(intervalCombox, &IntervalCombox::setCurrentSlideshow, provider, &WallpaperProvider::setSlideshow);

    connect(provider, &WallpaperProvider::wallpaperAdded, this, &WallpaperWindow::addNewWallpaper);
    connect(provider, &WallpaperProvider::wallpaperRemoved, this, &WallpaperWindow::removeWallpaper);
    connect(provider, &WallpaperProvider::wallpaperActived, this, &WallpaperWindow::activateWallpaper);

    connect(provider, &WallpaperProvider::imageChanged, this, &WallpaperWindow::repaintItem);
}

void WallpaperWindow::reset(QString screen, int mode)
{
    if (screen.isEmpty())
        screen = qApp->primaryScreen()->name();

    currentScreen = screen;
    intervalCombox->reset(screen);
    screenBox->setCurrentScreen(screen);

    // current wallpaper
    QString cur = provider->getBackgroundForMonitor(screen);
    if (cur == QString(kDefaultWallpaperPath)) {
        int findCount = 3;
        while (findCount-- > 0) {
            QFileInfo info(cur);
            if (info.isSymLink())
                cur = info.readLink();
            else
                break;
        }
    }

    // current mode
    bool isColor = provider->isColor(cur);

    // wants mode
    if (mode == ModeButtonBox::SolidColor || mode == ModeButtonBox::Picture)
        isColor = mode == ModeButtonBox::SolidColor;

    qDebug() << "current wallpaper" << cur << isColor << mode;

    modeBox->switchMode(isColor ? ModeButtonBox::SolidColor : ModeButtonBox::Picture);

    qDebug() << "wait wallpapers...";
    // wait 500ms for data
    QList<ItemNodePtr> items;
    if (!provider->waitWallpaper(5000)) {
        qWarning() << "the wallpapers has not gotten...";
    } else {
        items = isColor ? provider->colors() : provider->pictures();
        qDebug() << "get wallpapers" << items.size();
    }

    items.insert(0, createAddButton());
    itemModel->resetData(items);

    // get current wallpaper
    auto idx = itemModel->itemIndex(cur);
    auto ptr = itemModel->itemNode(idx);
    if (ptr) {
        listView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
        listView->setCurrentIndex(idx);
        listView->scrollTo(idx);
        listView->update();
        previewItem(ptr->item, ptr->color);
    }
}

void WallpaperWindow::scrollToCurrent()
{
    listView->scrollTo(listView->currentIndex());
}

void WallpaperWindow::addNewWallpaper(const ItemNodePtr &ptr)
{
    if (!ptr)
        return;

    if (provider->isColor(ptr->item) == isPictureMode())
        return;

    itemModel->insertItem(1, ptr);

    // check wheather to select
    QString cur = provider->getBackgroundForMonitor(currentScreen);
    if (cur == ptr->item) {
        auto idx = itemModel->itemIndex(ptr->item);
        listView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
        listView->setCurrentIndex(idx);
        listView->update();
        previewItem(ptr->item, ptr->color);
    }
}

void WallpaperWindow::removeWallpaper(const QString &path)
{
    if (path.isEmpty())
        return;

    if (provider->isColor(path) == isPictureMode())
        return;

    itemModel->removeItem(path);
}

void WallpaperWindow::activateWallpaper(const QString &path)
{
    if (path.isEmpty())
        return;

    if (provider->isColor(path) == isPictureMode())
        return;

    // check wheather to select
    if (provider->getBackgroundForMonitor(currentScreen) != path)
        return;

    auto idx = itemModel->itemIndex(path);
    if (!idx.isValid())
        return;

    listView->selectionModel()->select(idx, QItemSelectionModel::ClearAndSelect);
    listView->setCurrentIndex(idx);
    listView->update();

    auto ptr = itemModel->itemNode(idx);
    previewItem(ptr->item, ptr->color);
}

void WallpaperWindow::switchMode(int m)
{
    modeBox->switchMode(m == ModeButtonBox::SolidColor ? ModeButtonBox::SolidColor : ModeButtonBox::Picture);
    reset(currentScreen, m);
}

void WallpaperWindow::onThmeChanged()
{
    bool dark = DGuiApplicationHelper::instance()->themeType() == DGuiApplicationHelper::DarkType;
    if (preview) {
        preview->setBoder(dark ? QColor(255, 255, 255, 26) : QColor(0, 0, 0, 26));
    }

    if (isPictureMode()) {
        if (auto ptr = itemModel->itemNode(itemModel->itemIndex(NEW_ITEM_KEY))) {
            QIcon icon = QIcon::fromTheme("dfm-add-wallpaper");
            ptr->pixmap = icon.pixmap(LISTVIEW_ICON_WIDTH, LISTVIEW_ICON_HEIGHT);
        }
    }
}

void WallpaperWindow::getNewWallpaper()
{
    QString fileName = QFileDialog::getOpenFileName(this, tr("Select Wallpaper"),
                                                      QStandardPaths::locate(QStandardPaths::PicturesLocation, ""),
                                                      tr("Images") + QString(" (*.png *.jpg)"));
    if (fileName.isEmpty())
        return;

    QFileInfo file(fileName);
    if (!file.isReadable()) {
        qWarning() << "file is not readable" << fileName;
        return;
    }

    // set to dde
    setWallpaper(fileName);
}

void WallpaperWindow::getNewColor()
{
    //show palete
    ColorDialog dlg;
    dlg.setColor(dlg.palette().color(QPalette::Highlight));
    if (dlg.exec() == QDialog::Rejected)
        return;
    auto color = dlg.color();
    QString path = QDir::tempPath() + QString("/XXXXXX-solid-color-%0%1%2.jpg").arg(QString::number(color.red(), 16))
                                                          .arg(QString::number(color.green(), 16))
                                                          .arg(QString::number(color.blue(), 16));
    // create img
    QImage img(1920, 1080, QImage::Format_ARGB32);
    img.fill(color);
    QTemporaryFile file(path);
    if (!file.open()) {
        qWarning() << "fail to save image" << file.fileName();
        return;
    }

    img.save(&file, "JPG");

    //set to dde, and prefix solid:: to tell dde this is a solid color wallpaper.
    setWallpaper(SOLID_PREFIX + file.fileName());
}

void WallpaperWindow::setWallpaper(const QString &path)
{
    provider->setBackgroundForMonitor(currentScreen, path);
}

void WallpaperWindow::itemClicked(const ItemNodePtr &ptr)
{
    if (!ptr)
        return;

    if (ptr->item == NEW_ITEM_KEY) {
        if (isPictureMode())
            getNewWallpaper();
        else
            getNewColor();
    } else {
        previewItem(ptr->item, ptr->color);
        auto old = provider->getBackgroundForMonitor(currentScreen);
        if (old != ptr->item) {
            setWallpaper(ptr->item);
        }
    }
}

void WallpaperWindow::previewItem(const QString &item, const QColor &bkg)
{
    preview->setBackground(bkg);
    preview->setImage(item);
    preview->update();
}

void WallpaperWindow::onScreenChanged(const QString &name)
{
    if (name.isEmpty() || name == currentScreen)
        return;

    reset(name);
}

void WallpaperWindow::onScreenRested()
{
    if (!currentScreen.isEmpty()) {
        // current screen is existed
        auto list = screenBox->getScreens();
        if (list.contains(currentScreen)) {
            screenBox->setCurrentScreen(currentScreen);
            return;
        }
    }

    // current screen is not existed, set to primary
    reset();
}

void WallpaperWindow::onDeleteItem(const ItemNodePtr &ptr)
{
    if (!ptr)
        return;

    if (provider->deleteBackground(ptr))
        itemModel->removeItem(ptr->item);
}

void WallpaperWindow::onViewMenu(const QPoint &pos)
{
    auto ptr = itemModel->itemNode(listView->indexAt(pos));
    if (ptr.isNull() || ptr->item == NEW_ITEM_KEY)
        return;

    QMenu menu;
    auto ac = menu.addAction(tr("Set to lock screen"));
    if (menu.exec(listView->mapToGlobal(pos)) == ac) {
        provider->setBackgroundToGreeter(ptr->item);
    }
}

void WallpaperWindow::repaintItem(const QString &it)
{
    if (!isVisible())
        return;

    listView->update(itemModel->itemIndex(it));
}

ItemNodePtr WallpaperWindow::createAddButton() const
{
    QIcon icon = QIcon::fromTheme(isPictureMode() ? "dfm-add-wallpaper" : "dfm-select-color");
    ItemNodePtr btn(new ItemNode{NEW_ITEM_KEY, icon.pixmap(LISTVIEW_ICON_WIDTH, LISTVIEW_ICON_HEIGHT),
                                 palette().color(backgroundRole()), false, false});
    return btn;
}
