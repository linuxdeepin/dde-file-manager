// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "wallpapersettings.h"
#include "thumbnailmanager.h"
#include "private/wallpapersettings_p.h"
#include "desktoputils/widgetutil.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"

#include <dfm-base/base/schemefactory.h>
#include <dfm-base/utils/windowutils.h>

#include <DPaletteHelper>
#include <DDBusSender>
#include <DSysInfo>

#include <QLabel>
#include <QKeyEvent>
#include <QWindow>
#include <QVBoxLayout>
#include <QPushButton>

#define APPEARANCE_SERVICE "org.deepin.dde.Appearance1"
#define APPEARANCE_PATH "/org/deepin/dde/Appearance1"

DCORE_USE_NAMESPACE
DWIDGET_USE_NAMESPACE
using namespace ddplugin_wallpapersetting;

#define DESKTOP_BUTTON_ID "desktop"
#define LOCK_SCREEN_BUTTON_ID "lock-screen"
#define DESKTOP_AND_LOCKSCREEN_BUTTON_ID "desktop-lockscreen"
#define SCREENSAVER_BUTTON_ID "screensaver"
#define CUSTOMSCREENSAVER_BUTTON_ID "custom-screensaver"

#define BUTTON_NARROW_WIDTH 79
#define BUTTON_WIDE_WIDTH 164

#define CanvasCoreSubscribe(topic, recv, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), recv, func);

#define CanvasCoreUnsubscribe(topic, recv, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), recv, func);

const int WallpaperSettingsPrivate::kHeaderSwitcherHeight = 45;
const int WallpaperSettingsPrivate::kFrameHeight = 130;
const int WallpaperSettingsPrivate::kListHeight = 100;

static constexpr char kDefaultWallpaperPath[] { "/usr/share/backgrounds/default_background.jpg" };

static inline QString covertUrlToLocalPath(const QString &url)
{
    if (url.startsWith("/"))
        return url;
    else
        return QUrl(QUrl::fromPercentEncoding(url.toUtf8())).toLocalFile();
}

DFMBASE_USE_NAMESPACE

WallpaperSettingsPrivate::WallpaperSettingsPrivate(WallpaperSettings *parent)
    : QObject(parent), q(parent)
{
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
    fmDebug() << "create com.deepin.wm interface.";
    wmInter = new WMInter("com.deepin.wm", "/com/deepin/wm",
                          QDBusConnection::sessionBus(), q);
    fmDebug() << "end com.deepin.wm interface.";
#endif

    regionMonitor = new DRegionMonitor(q);
    connect(regionMonitor, &DRegionMonitor::buttonPress, this, &WallpaperSettingsPrivate::onMousePressed);

    fmDebug() << QString("create %1.").arg(APPEARANCE_SERVICE);
    appearanceIfs = new AppearanceIfs(APPEARANCE_SERVICE,
                                      APPEARANCE_PATH,
                                      QDBusConnection::sessionBus(), q);
    appearanceIfs->setTimeout(5000);
    fmDebug() << QString("end %1.").arg(APPEARANCE_SERVICE);

    fmDebug() << "create com.deepin.daemon.ScreenSaver.";
    screenSaverIfs = new ScreenSaverIfs("com.deepin.ScreenSaver",
                                        "/com/deepin/ScreenSaver",
                                        QDBusConnection::sessionBus(), q);
    fmDebug() << "end com.deepin.daemon.ScreenSaver.";

    fmDebug() << "create" << SessionIfs::staticInterfaceName();
    sessionIfs = new SessionIfs(this);
    fmDebug() << "end" << SessionIfs::staticInterfaceName();
    connect(sessionIfs, &SessionIfs::LockedChanged, this, [this]() {
        if (sessionIfs->locked()) {
            fmDebug() << "Session locked, hiding wallpaper settings";
            q->hide();
        }
    });

    reloadTimer.setSingleShot(true);
    connect(&reloadTimer, &QTimer::timeout, q, &WallpaperSettings::refreshList);
}

void WallpaperSettingsPrivate::propertyForWayland()
{
    q->winId();
    if (auto win = q->windowHandle()) {
        fmDebug() << "set wayland role override";
        win->setProperty("_d_dwayland_window-type", "wallpaper-set");
    } else {
        fmCritical() << "wayland role error,windowHandle is nullptr!";
    }
}

void WallpaperSettingsPrivate::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout(q);
    layout->setContentsMargins(0, 0, 0, 0);
    layout->setSpacing(0);

    // wallpaper
    initCloseButton();
    initCarousel();
    layout->addLayout(carouselLayout);

    // screensaver
    initScreenSaver();

    layout->addLayout(toolLayout);
    wallpaperList = new WallpaperList(q);
    connect(wallpaperList, &WallpaperList::mouseOverItemChanged, this, &WallpaperSettingsPrivate::handleNeedCloseButton);
    connect(wallpaperList, &WallpaperList::itemPressed, this, &WallpaperSettingsPrivate::onItemPressed);

    layout->addWidget(wallpaperList);
    layout->addSpacing(10);
    layout->setSizeConstraint(QLayout::SetFixedSize);

    // Use anchors to center this control
    DButtonBoxButton *wallpaperBtn = new DButtonBoxButton(tr("Wallpaper"), q);
    wallpaperBtn->installEventFilter(q);
    wallpaperBtn->setMinimumWidth(40);

    switchModeControl = new DButtonBox(q);
    switchModeControl->setFocusPolicy(Qt::NoFocus);
    // switch wallper mode or screensaver mode
    connect(switchModeControl, &DButtonBox::buttonToggled, this, &WallpaperSettingsPrivate::setMode);

    if (ddplugin_desktop_util::enableScreensaver()) {
        DButtonBoxButton *screensaverBtn = new DButtonBoxButton(tr("Screensaver"), q);
        screensaverBtn->installEventFilter(q);
        screensaverBtn->setMinimumWidth(40);
        switchModeControl->setButtonList({ wallpaperBtn, screensaverBtn }, true);
        if (mode == WallpaperSettings::Mode::ScreenSaverMode)
            screensaverBtn->setChecked(true);
    } else {
        switchModeControl->setButtonList({ wallpaperBtn }, true);
        wallpaperBtn->setChecked(true);
        wallpaperBtn->installEventFilter(q);
    }

    if (mode == WallpaperSettings::Mode::WallpaperMode)
        wallpaperBtn->setChecked(true);

    relaylout();
    layout->addStretch();
    q->setLayout(layout);

    initPreivew();
}

void WallpaperSettingsPrivate::relaylout()
{
    if (mode == WallpaperSettings::Mode::ScreenSaverMode) {
        fmDebug() << "Switching to screensaver mode layout";
        waitControlLabel->show();
        waitControl->show();
        lockScreenBox->show();
        carouselCheckBox->hide();
        carouselControl->hide();
        q->layout()->removeItem(carouselLayout);
        static_cast<QBoxLayout *>(q->layout())->insertLayout(0, toolLayout);
    } else {
        fmDebug() << "Switching to wallpaper mode layout";
        waitControlLabel->hide();
        waitControl->hide();
        lockScreenBox->hide();
        // The server version does not support automatic replacement of desktop background
        if (DSysInfo::deepinType() != DSysInfo::DeepinServer) {
            carouselCheckBox->show();
            carouselControl->setVisible(carouselCheckBox->isChecked());
            fmDebug() << "Carousel controls shown, server type:" << DSysInfo::deepinType();
        } else {
            fmDebug() << "Carousel controls hidden for server version";
        }
        q->layout()->removeItem(toolLayout);
        static_cast<QBoxLayout *>(q->layout())->insertLayout(0, carouselLayout);
    }
}

void WallpaperSettingsPrivate::adjustModeSwitcher()
{
    if (!switchModeControl)
        return;

    switchModeControl->adjustSize();

    int toolsWidth = 0;
    if (WallpaperSettings::Mode::ScreenSaverMode == mode) {
        auto margins = toolLayout->contentsMargins();
        int width = waitControlLabel->sizeHint().width() + waitControl->sizeHint().width() + lockScreenBox->sizeHint().width();

        toolsWidth = margins.left() + width + toolLayout->count() * toolLayout->spacing();
    } else if (WallpaperSettings::Mode::WallpaperMode == mode) {
        toolsWidth = carouselCheckBox->sizeHint().width() + carouselLayout->contentsMargins().left() + carouselLayout->contentsMargins().right() + carouselLayout->spacing();

        if (carouselCheckBox->isChecked())
            toolsWidth += carouselControl->sizeHint().width();
    }

    // prevent the toggle control from overlapping the toolbar on the left at low resolution
    int x = q->width() / 2 - switchModeControl->width() / 2;
    if (x < toolsWidth)
        x = q->width() - switchModeControl->width() - 5;

    switchModeControl->move(x, (wallpaperList->y() - switchModeControl->height()) / 2);
}

QString WallpaperSettingsPrivate::timeFormat(int second)
{
    quint8 s = static_cast<quint8>(second % 60);
    int m = static_cast<int>(second / 60);
    int h = m / 60;
    int d = h / 24;

    m = m % 60;
    h = h % 24;

    QString timeString;

    if (d > 0) {
        timeString.append(QString::number(d)).append("d");
    }

    if (h > 0) {
        if (!timeString.isEmpty()) {
            timeString.append(' ');
        }

        timeString.append(QString::number(h)).append("h");
    }

    if (m > 0) {
        if (!timeString.isEmpty()) {
            timeString.append(' ');
        }

        timeString.append(QString::number(m)).append("m");
    }

    if (s > 0 || timeString.isEmpty()) {
        if (!timeString.isEmpty()) {
            timeString.append(' ');
        }

        timeString.append(QString::number(s)).append("s");
    }

    return timeString;
}

void WallpaperSettingsPrivate::carouselTurn(bool checked)
{
    fmInfo() << "Wallpaper carousel turned" << (checked ? "on" : "off");
    carouselControl->setVisible(checked);
    adjustModeSwitcher();

    int checkedIndex = carouselControl->buttonList().indexOf(carouselControl->checkedButton());
    if (!checked) {
        q->setWallpaperSlideShow(QString());
    } else if (checkedIndex >= 0) {
        q->setWallpaperSlideShow(q->availableWallpaperSlide().at(checkedIndex));
    }
}

void WallpaperSettingsPrivate::switchCarousel(QAbstractButton *toggledBtn, bool state)
{
    if (toggledBtn && state)
        q->setWallpaperSlideShow(q->availableWallpaperSlide().at(carouselControl->buttonList().indexOf(toggledBtn)));
}

void WallpaperSettingsPrivate::switchWaitTime(QAbstractButton *toggledBtn, bool state)
{
    Q_UNUSED(state)
    if (!toggledBtn)
        return;

    int index = waitControl->buttonList().indexOf(toggledBtn);
    auto timeArray = q->availableScreenSaverTime();
    if (index < 0 || index >= timeArray.size()) {
        fmWarning() << "invalid index" << index;
        return;
    }

    screenSaverIfs->setBatteryScreenSaverTimeout(timeArray[index]);
    screenSaverIfs->setLinePowerScreenSaverTimeout(timeArray[index]);
}

void WallpaperSettingsPrivate::setMode(QAbstractButton *toggledBtn, bool on)
{
    Q_UNUSED(on);
    int md = switchModeControl->buttonList().indexOf(toggledBtn);
    q->switchMode(WallpaperSettings::Mode(md));
}

void WallpaperSettingsPrivate::onListBackgroundReply(QDBusPendingCallWatcher *watch)
{
    if (!watch)
        return;

    watch->deleteLater();
    QDBusPendingCall call = *watch;
    if (call.isError()) {
        fmWarning() << "failed to get all backgrounds: " << call.error().message();
        reloadTimer.start(5000);
    } else {
        q->closeLoading();
        reloadTimer.stop();

        QDBusReply<QString> reply = call.reply();
        QString value = reply.value();
        auto wallapers = processListReply(value);
        fmDebug() << "get available wallpapers" << wallapers;
        actualEffectivedWallpaper = appearanceIfs->GetCurrentWorkspaceBackgroundForMonitor(screenName);
        fmDebug() << "get current wallpaper" << screenName << actualEffectivedWallpaper;
        if (actualEffectivedWallpaper.contains(kDefaultWallpaperPath)) {
            QString errString;
            QUrl currentUrl;
            if (actualEffectivedWallpaper.startsWith("/"))
                currentUrl = QUrl::fromLocalFile(actualEffectivedWallpaper);
            else
                currentUrl = QUrl(actualEffectivedWallpaper);

            FileInfoPointer fileInfo = InfoFactory::create<FileInfo>(currentUrl, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
            while (fileInfo && fileInfo->isAttributes(OptInfoType::kIsSymLink)) {
                QUrl targetUrl = QUrl::fromLocalFile(fileInfo->pathOf(PathInfoType::kSymLinkTarget));
                if (targetUrl == fileInfo->urlOf(UrlInfoType::kUrl))
                    break;

                fileInfo = InfoFactory::create<FileInfo>(targetUrl, Global::CreateFileInfoType::kCreateFileInfoAuto, &errString);
            }

            if (!fileInfo) {
                fmWarning() << errString << "get file info failed:" << currentUrl << actualEffectivedWallpaper;
            } else {
                actualEffectivedWallpaper = fileInfo->urlOf(UrlInfoType::kUrl).toString();
            }
        }
        if (actualEffectivedWallpaper.startsWith("file://"))
            actualEffectivedWallpaper.remove("file://");

        for (auto iter = wallapers.begin(); iter != wallapers.end(); ++iter) {
            QString path = iter->first;
            if (needDelWallpaper.contains(QUrl(path).path()))
                continue;

            WallpaperItem *item = wallpaperList->addItem(covertUrlToLocalPath(path));
            item->setSketch(path);
            // only item is greeterbackground,deletable is false.
            item->setDeletable(iter->second);
            item->addButton(DESKTOP_BUTTON_ID, tr("Desktop", "button"), BUTTON_NARROW_WIDTH, 0, 0, 1, 1);
            item->addButton(LOCK_SCREEN_BUTTON_ID, tr("Lock Screen", "button"), BUTTON_NARROW_WIDTH, 0, 1, 1, 1);
            item->addButton(DESKTOP_AND_LOCKSCREEN_BUTTON_ID, tr("Both"), BUTTON_WIDE_WIDTH, 1, 0, 1, 2);
            item->show();
            connect(item, &WallpaperItem::buttonClicked, this, &WallpaperSettingsPrivate::onItemButtonClicked);
            connect(item, &WallpaperItem::tab, this, &WallpaperSettingsPrivate::onItemTab);
            connect(item, &WallpaperItem::backtab, this, &WallpaperSettingsPrivate::onItemBacktab);

            if (path.remove("file://") == actualEffectivedWallpaper) {   //均有机会出现头部为file:///概率
                emit item->pressed(item);
            }
        }

        wallpaperList->updateItemThumb();
    }
}

void WallpaperSettingsPrivate::onItemPressed(const QString &itemData)
{
    if (itemData.isEmpty()) {
        fmWarning() << "Item pressed with empty data";
        return;
    }

    if (mode == WallpaperSettings::Mode::WallpaperMode) {
        //wmInter->SetTransientBackground(itemData); // it will case gsetting emit valuechanged signal (key is backgroundUris).
        wallpaperPrview->setWallpaper(screenName, itemData);
        currentSelectedWallpaper = itemData;

        // 点击当前壁纸不显示删除按钮
        if (closeButton && closeButton->isVisible())
            closeButton->hide();

    } else {
        screenSaverIfs->Preview(itemData, 1);
        fmDebug() << "screensaver start" << itemData;
        if (wallpaperPrview->isVisible()) {
            QThread::msleep(300);
            wallpaperPrview->setVisible(false);
            fmDebug() << "Hidden wallpaper preview for screensaver mode";
        }
    }
}

void WallpaperSettingsPrivate::onItemButtonClicked(WallpaperItem *item, const QString &id)
{
    if (!item) {
        fmWarning() << "Item button clicked with null item";
        return;
    }

    if (id == DESKTOP_BUTTON_ID) {
        if (!q->isWallpaperLocked()) {
            fmInfo() << "Applying wallpaper to desktop";
            q->applyToDesktop();
        } else {
            fmWarning() << "Cannot apply to desktop - wallpaper is locked";
        }
    } else if (id == LOCK_SCREEN_BUTTON_ID) {
        if (!q->isWallpaperLocked()) {
            fmInfo() << "Applying wallpaper to lock screen";
            q->applyToGreeter();
        } else {
            fmWarning() << "Cannot apply to lock screen - wallpaper is locked";
        }
    } else if (id == DESKTOP_AND_LOCKSCREEN_BUTTON_ID) {
        if (!q->isWallpaperLocked()) {
            fmInfo() << "Applying wallpaper to both desktop and lock screen";
            q->applyToDesktop();
            q->applyToGreeter();
        } else {
            fmWarning() << "Cannot apply to desktop and lock screen - wallpaper is locked";
        }
    } else if (id == SCREENSAVER_BUTTON_ID) {
        fmInfo() << "Setting screensaver:" << item->itemData();
        screenSaverIfs->setCurrentScreenSaver(item->itemData());
    } else if (id == CUSTOMSCREENSAVER_BUTTON_ID) {
        fmInfo() << "Starting custom screensaver config:" << item->itemData();
        screenSaverIfs->StartCustomConfig(item->itemData());
    }

    q->hide();
}

void WallpaperSettingsPrivate::onItemTab(WallpaperItem *item)
{
    Q_UNUSED(item);
    // if the third area sends a tab signal, it jumps to the first control of the first area
    if (mode == WallpaperSettings::Mode::WallpaperMode) {
        carouselCheckBox->setFocus();
    } else {
        waitControl->buttonList().first()->setFocus();
    }
}

void WallpaperSettingsPrivate::onItemBacktab(WallpaperItem *item)
{
    Q_UNUSED(item);
    //if the third area sends a backtab signal, it jumps to the first control in the second area
    switchModeControl->buttonList().first()->setFocus();
}

void WallpaperSettingsPrivate::handleNeedCloseButton(const QString &itemData, const QPoint &pos)
{
    closeButton->setProperty("background", itemData);
    if (!itemData.isEmpty()
        && itemData != currentSelectedWallpaper
        && itemData != actualEffectivedWallpaper) {
        closeButton->adjustSize();
        closeButton->move(pos.x() - 10, pos.y() - 10);
        closeButton->show();
        closeButton->raise();
    } else {
        closeButton->hide();
    }
}

void WallpaperSettingsPrivate::onCloseButtonClicked()
{
    QString itemData = closeButton->property("background").toString();
    fmDebug() << "delete background" << itemData;
    if (!itemData.isEmpty()) {
        appearanceIfs->Delete("background", itemData);   // 当前自定义壁纸不一定能删成功
        needDelWallpaper << itemData;
        wallpaperList->removeItem(itemData);
        closeButton->hide();
    }
}

void WallpaperSettingsPrivate::onMousePressed(const QPoint &pos, int button)
{
    if (button == 4) {
        wallpaperList->prevPage();
    } else if (button == 5) {
        wallpaperList->nextPage();
    } else {
        qreal scale = q->devicePixelRatioF();
        if (auto screen = ddplugin_desktop_util::screenProxyScreen(screenName)) {
            const QRect sRect = screen->geometry();
            QRect nativeRect = q->geometry();

            // 获取窗口真实的geometry
            nativeRect.moveTopLeft((nativeRect.topLeft() - sRect.topLeft()) * scale + sRect.topLeft());
            nativeRect.setSize(nativeRect.size() * scale);

            if (!nativeRect.contains(pos)) {
                fmDebug() << "button pressed on blank area quit.";
                q->hide();
            } else {
                if (!q->isActiveWindow()) {
                    // activate window in mousepress event will case button can not emit clicked signal;
                    fmDebug() << "activate WallpaperSettings by mouse pressed." << button;
                    q->activateWindow();
                }
            }
        } else {
            fmCritical() << "lost screen " << screenName << "closed";
            q->hide();
            return;
        }
    }
}

void WallpaperSettingsPrivate::onScreenChanged()
{
    wallpaperPrview->buildWidgets();
    // apply to widget created for new screen.
    wallpaperPrview->updateWallpaper();
    wallpaperPrview->setVisible(wallpaperPrview->isVisible());

    auto wid = wallpaperPrview->widget(screenName);
    if (wid.get()) {
        wid->lower();
        q->onGeometryChanged();
        q->raise();
        fmDebug() << "onScreenChanged focus" << screenName << q->isVisible() << q->geometry();
        q->activateWindow();
    } else {
        fmDebug() << screenName << "lost exit!";
        q->close();
    }
}

void WallpaperSettingsPrivate::initCloseButton()
{
    closeButton = new DIconButton(q);
    closeButton->setIcon(QIcon::fromTheme("dfm_close_round_normal"));
    closeButton->setFixedSize(24, 24);
    closeButton->setIconSize({ 24, 24 });
    closeButton->setFlat(true);
    closeButton->setFocusPolicy(Qt::NoFocus);
    closeButton->hide();
    connect(closeButton, &DIconButton::clicked, this, &WallpaperSettingsPrivate::onCloseButtonClicked, Qt::UniqueConnection);
}

void WallpaperSettingsPrivate::initCarousel()
{
    DPalette pal = DPaletteHelper::instance()->palette(q);
    QColor textColor = pal.color(QPalette::Normal, QPalette::BrightText);

    carouselLayout = new QHBoxLayout(q);
    // auto switch wallpaper
    carouselCheckBox = new QCheckBox(tr("Wallpaper Slideshow"), q);
    // process tab key event.
    carouselCheckBox->installEventFilter(q);
    // process enter to check
    carouselCheckBox->installEventFilter(this);

    // do not support automatic swich background on the server version.
    if (DSysInfo::deepinType() == DSysInfo::DeepinServer) {
        carouselCheckBox->setChecked(false);
        carouselCheckBox->setEnabled(false);
        carouselCheckBox->setVisible(false);
    } else {
        carouselCheckBox->setChecked(true);
    }

    QPalette wccPal = carouselCheckBox->palette();
    wccPal.setColor(QPalette::All, QPalette::WindowText, textColor);
    carouselCheckBox->setPalette(wccPal);
    carouselControl = new DButtonBox(q);
    carouselControl->installEventFilter(q);

    carouselCheckBox->setFocusPolicy(Qt::StrongFocus);
    carouselControl->setFocusPolicy(Qt::NoFocus);

    fmDebug() << "DSysInfo::deepinType = " << QString::number(DSysInfo::DeepinProfessional);

    // create time slide
    {
        QList<DButtonBoxButton *> slideBtns;
        QString curSlide = q->wallpaperSlideShow();
        auto policys = q->availableWallpaperSlide();
        int curIndex = policys.indexOf(curSlide.toLatin1());

        if (curIndex < 0) {
            carouselCheckBox->setChecked(false);
            curIndex = 3;
        }

        for (const QString &time : policys) {
            DButtonBoxButton *btn;
            if (time == "login") {
                btn = new DButtonBoxButton(tr("When login"), q);
            } else if (time == "wakeup") {
                btn = new DButtonBoxButton(tr("When wakeup"), q);
            } else {
                bool ok = false;
                int t = time.toInt(&ok);
                btn = new DButtonBoxButton(ok ? timeFormat(t) : time, q);
            }

            btn->installEventFilter(q);
            btn->setMinimumWidth(40);
            slideBtns.append(btn);
        }

        carouselControl->setButtonList(slideBtns, true);
        slideBtns[curIndex]->setChecked(true);
        carouselControl->setVisible(carouselCheckBox->isChecked());
    }

    carouselLayout->setSpacing(10);
    carouselLayout->setContentsMargins(20, 5, 20, 5);
    carouselLayout->addWidget(carouselCheckBox);
    carouselLayout->addWidget(carouselControl);
    carouselLayout->addItem(new QSpacerItem(1, kHeaderSwitcherHeight));
    carouselLayout->addStretch();

    // enable or disable auto switch wallpaper
    connect(carouselCheckBox, &QCheckBox::clicked, this, &WallpaperSettingsPrivate::carouselTurn);
    // the period for automatically switching
    connect(carouselControl, &DButtonBox::buttonToggled, this, &WallpaperSettingsPrivate::switchCarousel);
}

void WallpaperSettingsPrivate::initScreenSaver()
{
    toolLayout = new QHBoxLayout(q);

    waitControl = new DButtonBox(q);
    waitControl->installEventFilter(q);
    connect(waitControl, &DButtonBox::buttonToggled, this, &WallpaperSettingsPrivate::switchWaitTime);

    lockScreenBox = new QCheckBox(tr("Require a password on wakeup"), q);
    // process tab key
    lockScreenBox->installEventFilter(q);
    connect(lockScreenBox, &QCheckBox::toggled, screenSaverIfs, &ScreenSaverIfs::setLockScreenAtAwake);

    DPalette pal = DPaletteHelper::instance()->palette(q);
    QColor textColor = pal.color(QPalette::Normal, QPalette::BrightText);

    QPalette lsPal = lockScreenBox->palette();
    lsPal.setColor(QPalette::All, QPalette::WindowText, textColor);
    lockScreenBox->setPalette(lsPal);

    QVector<int> timeArray = q->availableScreenSaverTime();
    QList<DButtonBoxButton *> timeArrayBtns;
    const int currentTimeOut = screenSaverIfs->linePowerScreenSaverTimeout();
    int currentIndex = timeArray.indexOf(currentTimeOut);

    // Insert the value when it does not exist in this list
    if (currentIndex < 0) {
        timeArray.prepend(currentTimeOut);
        currentIndex = 0;
    }

    for (const int time : timeArray) {
        if (time > 0) {
            DButtonBoxButton *btn = new DButtonBoxButton(timeFormat(time), q);
            btn->installEventFilter(q);
            btn->setMinimumWidth(40);
            timeArrayBtns.append(btn);
        }
    }

    timeArrayBtns.append(new DButtonBoxButton(tr("Never"), q));
    timeArrayBtns.last()->installEventFilter(q);
    waitControlLabel = new QLabel(tr("Wait:"), q);
    QPalette wcPal = waitControlLabel->palette();
    wcPal.setColor(QPalette::All, QPalette::WindowText, textColor);
    waitControlLabel->setPalette(wcPal);
    waitControl->setButtonList(timeArrayBtns, true);
    timeArrayBtns[currentIndex]->setChecked(true);
    lockScreenBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    lockScreenBox->setChecked(screenSaverIfs->lockScreenAtAwake());

    toolLayout->setSpacing(10);
    toolLayout->setContentsMargins(20, 10, 20, 10);
    toolLayout->addWidget(waitControlLabel);
    toolLayout->addWidget(waitControl);
    toolLayout->addSpacing(10);
    toolLayout->addWidget(lockScreenBox, 1, Qt::AlignLeft);
}

void WallpaperSettingsPrivate::initPreivew()
{
    wallpaperPrview = new WallaperPreview(this);
    wallpaperPrview->init();
}

QList<QPair<QString, bool>> WallpaperSettingsPrivate::processListReply(const QString &reply)
{
    QList<QPair<QString, bool>> result;
    QJsonDocument doc = QJsonDocument::fromJson(reply.toUtf8());
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        foreach (QJsonValue val, arr) {
            QJsonObject obj = val.toObject();
            QString id = obj["Id"].toString();
            result.append(qMakePair(id, obj["Deletable"].toBool()));
        }
    }

    return result;
}

bool WallpaperSettingsPrivate::eventFilter(QObject *watched, QEvent *event)
{
    if (watched == carouselCheckBox && event->type() == QEvent::KeyPress) {
        if (auto keyEvent = dynamic_cast<QKeyEvent *>(event)) {
            // Enter and Return to switch check state.
            if (keyEvent->key() == Qt::Key_Enter || keyEvent->key() == Qt::Key_Return) {
                bool checked = carouselCheckBox->isChecked();
                carouselCheckBox->setChecked(!checked);
                emit carouselCheckBox->clicked(!checked);
            }
        }
    }

    return QObject::eventFilter(watched, event);
}

WallpaperSettings::WallpaperSettings(const QString &screenName, Mode mode, QWidget *parent)
    : DBlurEffectWidget(parent), d(new WallpaperSettingsPrivate(this))
{
    fmInfo() << "Creating WallpaperSettings for screen:" << screenName << "mode:" << static_cast<int>(mode);
    d->screenName = screenName;
    d->mode = mode;
    init();
}

WallpaperSettings::~WallpaperSettings()
{
    CanvasCoreUnsubscribe(signal_ScreenProxy_ScreenChanged, d, &WallpaperSettingsPrivate::onScreenChanged);
    CanvasCoreUnsubscribe(signal_ScreenProxy_DisplayModeChanged, d, &WallpaperSettingsPrivate::onScreenChanged);
    CanvasCoreUnsubscribe(signal_ScreenProxy_ScreenGeometryChanged, this, &WallpaperSettings::onGeometryChanged);

    fmDebug() << "WallpaperSettings deleted";
    delete d->loadingLabel;
    d->loadingLabel = nullptr;

    delete d->toolLayout;
    d->toolLayout = nullptr;

    delete d->carouselLayout;
    d->carouselLayout = nullptr;
}

void WallpaperSettings::switchMode(WallpaperSettings::Mode mode)
{
    if (mode == d->mode) {
        fmDebug() << "Mode switch requested but already in mode:" << static_cast<int>(mode);
        return;
    }

    if (d->mode == Mode::ScreenSaverMode) {
        d->wallpaperPrview->setVisible(true);
        d->screenSaverIfs->Stop();
    }

    d->mode = mode;
    d->relaylout();
    adjustGeometry();

    refreshList();
}

QString WallpaperSettings::wallpaperSlideShow() const
{
    if (nullptr == d->appearanceIfs) {
        fmWarning() << "appearanceIfs is nullptr";
        return QString();
    }

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(d->screenName);

    // com::deepin::daemon::Appearance do not export GetWallpaperSlideShow.
    QString wallpaperSlideShow = QDBusPendingReply<QString>(d->appearanceIfs->asyncCallWithArgumentList(QStringLiteral("GetWallpaperSlideShow"), argumentList));

    fmDebug() << "dbus Appearance GetWallpaperSlideShow is called, result: " << wallpaperSlideShow;
    return wallpaperSlideShow;
}

void WallpaperSettings::setWallpaperSlideShow(const QString &period)
{
    if (nullptr == d->appearanceIfs) {
        fmWarning() << "appearanceIfs is nullptr";
        return;
    }

    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(d->screenName) << QVariant::fromValue(period);
    d->appearanceIfs->asyncCallWithArgumentList(QStringLiteral("SetWallpaperSlideShow"), argumentList);
}

QVector<int> WallpaperSettings::availableScreenSaverTime()
{
    static QVector<int> policy { 60, 300, 600, 900, 1800, 3600, 0 };
    return policy;
}

QStringList WallpaperSettings::availableWallpaperSlide()
{
    static const QStringList policy { "30", "60", "300", "600", "900", "1800", "3600", "login", "wakeup" };
    return policy;
}

void WallpaperSettings::adjustGeometry()
{
    QRect screenRect;
    if (auto sc = ddplugin_desktop_util::screenProxyScreen(d->screenName)) {
        screenRect = sc->geometry();
    } else {
        fmCritical() << "invalid screen name:" << d->screenName;
        screenRect = QRect(0, 0, 1920, 1080);
    }

    int actualHeight = d->kFrameHeight + d->kHeaderSwitcherHeight;
    setFixedSize(screenRect.width() - 20, actualHeight);

    fmDebug() << "move befor: " << this->geometry() << d->wallpaperList->geometry() << height()
              << actualHeight;
    move(screenRect.x() + 10, screenRect.y() + screenRect.height() - actualHeight);
    d->wallpaperList->setFixedSize(screenRect.width() - 20, d->kListHeight);

    // layout all widgets, the wallpaperList->geometry will adjust after this calling.
    layout()->activate();
    fmDebug() << "this move : " << this->geometry() << d->wallpaperList->geometry();

    d->adjustModeSwitcher();
}

void WallpaperSettings::refreshList()
{
    if (!isVisible()) {
        fmDebug() << "Refresh list skipped - widget not visible";
        return;
    }

    d->wallpaperList->hide();
    d->wallpaperList->clear();

    showLoading();
    d->wallpaperList->show();
    if (d->mode == WallpaperSettings::Mode::WallpaperMode)
        loadWallpaper();
    else
        loadScreenSaver();
}

QPair<QString, QString> WallpaperSettings::currentWallpaper() const
{
    return QPair<QString, QString>(d->screenName, d->currentSelectedWallpaper);
}

void WallpaperSettings::onGeometryChanged()
{
    d->wallpaperPrview->updateGeometry();

    adjustGeometry();
    if (!isHidden())
        d->wallpaperList->updateItemThumb();

    fmDebug() << "reset geometry" << this->isVisible() << this->geometry();
    activateWindow();
}

void WallpaperSettings::showEvent(QShowEvent *event)
{
    d->regionMonitor->registerRegion();
    activateWindow();
    DBlurEffectWidget::showEvent(event);
}

void WallpaperSettings::hideEvent(QHideEvent *event)
{
    DBlurEffectWidget::hideEvent(event);

    d->regionMonitor->unregisterRegion();

    if (d->mode == Mode::WallpaperMode) {
#if (QT_VERSION < QT_VERSION_CHECK(6, 0, 0))
        if (!d->currentSelectedWallpaper.isEmpty())
            d->wmInter->SetTransientBackground("");
#endif
        if (ThumbnailManager *manager = ThumbnailManager::instance(devicePixelRatioF())) {
            manager->stop();
            fmDebug() << "Stopped thumbnail manager";
        }
    } else {
        fmDebug() << "Stopping screensaver preview";
        d->screenSaverIfs->Stop();
    }

    emit quit();
}

void WallpaperSettings::keyPressEvent(QKeyEvent *event)
{
    QWidgetList widgetList;   //Controls can be selected on the record page
    if (d->mode == Mode::WallpaperMode) {
        widgetList << d->carouselCheckBox;   //Automatic wallpaper change button
        // Replace cycle button group
        if (d->carouselControl->isVisible()) {
            for (QAbstractButton *button : d->carouselControl->buttonList()) {
                widgetList << qobject_cast<QWidget *>(button);
            }
        }
    } else {
        //Idle time button group
        for (QAbstractButton *button : d->waitControl->buttonList()) {
            widgetList << qobject_cast<QWidget *>(button);
        }
        //button for needing a password
        widgetList << d->lockScreenBox;
    }

    if (!widgetList.contains(focusWidget())) {
        widgetList.clear();

        // Mode switching button group
        for (QAbstractButton *button : d->switchModeControl->buttonList()) {
            widgetList << qobject_cast<QWidget *>(button);
        }
    }

    switch (event->key()) {
    case Qt::Key_Escape:
        hide();
        fmDebug() << "escape key pressed, quit.";
        break;
    case Qt::Key_Right:
        fmDebug() << "Right";
        //Select the next control in the list
        if (widgetList.indexOf(focusWidget(), 0) < widgetList.count() - 1) {
            widgetList.at(widgetList.indexOf(focusWidget(), 0) + 1)->setFocus();
        }
        break;
    case Qt::Key_Left:
        fmDebug() << "Left";
        //Select the previous control in the list
        if (widgetList.indexOf(focusWidget(), 0) > 0) {
            widgetList.at(widgetList.indexOf(focusWidget(), 0) - 1)->setFocus();
        }
        break;
    default:
        DBlurEffectWidget::keyPressEvent(event);
        break;
    }
}

bool WallpaperSettings::eventFilter(QObject *object, QEvent *event)
{
    if (event->type() == QEvent::KeyPress) {
        QKeyEvent *key = dynamic_cast<QKeyEvent *>(event);
        if (!key) {
            fmDebug() << "key is null.";
            return DBlurEffectWidget::eventFilter(object, event);
        }
        fmDebug() << "keyPress" << (Qt::Key)key->key();
        if (key->key() == Qt::Key_Tab) {
            fmDebug() << "Tab";
            //第一个区域发出tab信号，则跳转到第二个区域的第一个控件上
            if (object == d->carouselCheckBox
                || d->carouselControl->buttonList().contains(qobject_cast<QAbstractButton *>(object))
                || object == d->lockScreenBox
                || d->waitControl->buttonList().contains(qobject_cast<QAbstractButton *>(object))) {
                d->switchModeControl->buttonList().first()->setFocus();
                return true;
            }
            //The second area tab jumps to the first control (button) of the current option (wallpaperitem) in the third area
            if (d->switchModeControl->buttonList().contains(qobject_cast<QAbstractButton *>(object))) {
                if (nullptr == d->wallpaperList->currentItem()) {
                    return false;
                }
                QList<QPushButton *> childButtons = d->wallpaperList->currentItem()->findChildren<QPushButton *>();
                if (!childButtons.isEmpty()) {
                    childButtons.first()->setFocus();
                    return true;
                }
            }
            //The tab of the third area jumps to the first area: the tab signal connected to wallpaperitem is processed
        } else if (key->key() == Qt::Key_Backtab) {   //BackTab
            fmDebug() << "BackTab(Shift Tab)";
            //If the first area sends a backtab signal, it will jump to the first control (button) of the current option (wallpaperitem) in the third area
            if (object == d->carouselCheckBox
                || d->carouselControl->buttonList().contains(qobject_cast<QAbstractButton *>(object))
                || object == d->lockScreenBox
                || d->waitControl->buttonList().contains(qobject_cast<QAbstractButton *>(object))) {
                if (nullptr == d->wallpaperList->currentItem()) {
                    return false;
                }
                QList<QPushButton *> childButtons = d->wallpaperList->currentItem()->findChildren<QPushButton *>();
                if (!childButtons.isEmpty()) {
                    childButtons.first()->setFocus();
                    return true;
                }
            }

            //The backtab of the third area jumps to the second area: the backtab signal of the connected wallpaperitem is processed

            //If the second area sends a backtab signal, it jumps to the first control of the first area
            if (d->switchModeControl->buttonList().contains(qobject_cast<QAbstractButton *>(object))) {
                if (d->mode == Mode::WallpaperMode) {
                    d->carouselCheckBox->setFocus();
                } else {
                    d->waitControl->buttonList().first()->setFocus();
                }
                return true;
            }
        } else if (key->key() == Qt::Key_Right || key->key() == Qt::Key_Left) {   //Qcheckbox treats the arrow keys as tab keys
            // Do not send other types, or you will receive two key events
            if (object == d->carouselCheckBox || object == d->lockScreenBox) {
                keyPressEvent(key);
                return true;
            } else {
                return false;
            }
        } else if (key->key() == Qt::Key_Up || key->key() == Qt::Key_Down) {   // 屏蔽上下键
            return true;
        } else {
            return false;
        }
    }
    return DBlurEffectWidget::eventFilter(object, event);
}

void WallpaperSettings::showLoading()
{
    if (d->loadingLabel == nullptr)
        d->loadingLabel = new LoadingLabel;

    d->loadingLabel->resize(d->wallpaperList->size());

    QString lablecontant;
    if (d->mode == Mode::WallpaperMode)
        lablecontant = QString(tr("Loading wallpapers..."));
    else
        lablecontant = QString(tr("Loading screensavers..."));

    d->loadingLabel->setText(lablecontant);
    d->loadingLabel->start();
    d->wallpaperList->setMaskWidget(d->loadingLabel);
}

void WallpaperSettings::closeLoading()
{
    d->wallpaperList->removeMaskWidget();
    delete d->loadingLabel;
    d->loadingLabel = nullptr;
}

void WallpaperSettings::loadWallpaper()
{
    QDBusPendingCall call = d->appearanceIfs->List("background");
    QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
    connect(watcher, &QDBusPendingCallWatcher::finished, d, &WallpaperSettingsPrivate::onListBackgroundReply);
}

void WallpaperSettings::loadScreenSaver()
{
    const QStringList &screensaverConfigurableItems = d->screenSaverIfs->ConfigurableItems();
    QStringList saverNameList = d->screenSaverIfs->allScreenSaver();
    if (saverNameList.isEmpty() && !d->screenSaverIfs->isValid()) {
        fmWarning() << "com.deepin.ScreenSaver allScreenSaver fail. retry";
        d->reloadTimer.start(5000);
        return;
    }

    closeLoading();
    d->reloadTimer.stop();
    const QString &currentScreensaver = d->screenSaverIfs->currentScreenSaver();
    WallpaperItem *currentItem = nullptr;

    // Supports parameter setting for multiple screensavers
    int customSaverCount = 0;
    for (const QString &name : saverNameList) {
        // The screensaver with the parameter configuration is placed first
        if (screensaverConfigurableItems.contains(name)) {
            saverNameList.move(saverNameList.indexOf(name), customSaverCount);
            customSaverCount++;
        }
    }

    for (const QString &name : saverNameList) {
        //romove
        if ("flurry" == name)
            continue;

        const QString &coverPath = d->screenSaverIfs->GetScreenSaverCover(name);

        WallpaperItem *item = d->wallpaperList->addItem(name);
        item->setSketch(coverPath);
        item->setEnableThumbnail(false);
        item->setDeletable(false);

        if (screensaverConfigurableItems.contains(name)) {
            item->setEntranceIconOfSettings(CUSTOMSCREENSAVER_BUTTON_ID);
            item->addButton(CUSTOMSCREENSAVER_BUTTON_ID, tr("Custom Screensaver"), BUTTON_WIDE_WIDTH, 0, 0, 1, 2);
            item->addButton(SCREENSAVER_BUTTON_ID, tr("Apply", "button"), BUTTON_WIDE_WIDTH, 1, 0, 1, 2);
        } else {
            item->addButton(SCREENSAVER_BUTTON_ID, tr("Apply", "button"), BUTTON_WIDE_WIDTH, 0, 0, 1, 2);
        }

        item->show();

        connect(item, &WallpaperItem::buttonClicked, d, &WallpaperSettingsPrivate::onItemButtonClicked);
        connect(item, &WallpaperItem::tab, d, &WallpaperSettingsPrivate::onItemTab);
        connect(item, &WallpaperItem::backtab, d, &WallpaperSettingsPrivate::onItemBacktab);

        // When entering for the first time, select the current setting screensaver
        if (!currentItem && !name.isEmpty() && name == currentScreensaver) {
            currentItem = item;
        }
    }

    if (currentItem) {
        emit currentItem->pressed(currentItem);
    } else {
        if (d->wallpaperList->count() > 0) {
            fmWarning() << "no screen saver item selected,and select default 0.";
            d->wallpaperList->setCurrentIndex(0);
        }
    }

    d->wallpaperList->updateItemThumb();
}

void WallpaperSettings::applyToDesktop()
{
    if (nullptr == d->appearanceIfs) {
        fmCritical() << "Cannot apply to desktop - appearanceIfs is nullptr";
        return;
    }

    if (d->currentSelectedWallpaper.isEmpty()) {
        fmWarning() << "Cannot apply to desktop - current wallpaper is empty";
        return;
    }

    fmDebug() << "Applying wallpaper to desktop - screen:" << d->screenName << "wallpaper:" << d->currentSelectedWallpaper;
    QList<QVariant> argumentList;
    argumentList << QVariant::fromValue(d->screenName) << QVariant::fromValue(d->currentSelectedWallpaper);
    d->appearanceIfs->asyncCallWithArgumentList(QStringLiteral("SetMonitorBackground"), argumentList);

    emit backgroundChanged();
    fmInfo() << "Desktop wallpaper application completed";
}

void WallpaperSettings::applyToGreeter()
{
    if (nullptr == d->appearanceIfs) {
        fmCritical() << "Cannot apply to greeter - appearanceIfs is nullptr";
        return;
    }

    if (d->currentSelectedWallpaper.isEmpty()) {
        fmWarning() << "Cannot apply to greeter - current wallpaper is empty";
        return;
    }

    fmDebug() << "Applying wallpaper to greeter/lock screen:" << d->currentSelectedWallpaper;
    d->appearanceIfs->Set("greeterbackground", d->currentSelectedWallpaper);
    fmDebug() << "Greeter wallpaper application completed";
}

bool WallpaperSettings::isWallpaperLocked() const
{
    if (QFileInfo::exists("/var/lib/deepin/permission-manager/wallpaper_locked")) {
        QDBusInterface notify("org.freedesktop.Notifications", "/org/freedesktop/Notifications", "org.freedesktop.Notifications");
        notify.asyncCall(QString("Notify"),
                         QString("dde-file-manager"),   // title
                         static_cast<uint>(0),
                         QString("dde-file-manager"),   // icon
                         tr("This system wallpaper is locked. Please contact your admin."),
                         QString(), QStringList(), QVariantMap(), 5000);
        fmDebug() << "wallpaper is locked..";
        return true;
    }

    return false;
}

void WallpaperSettings::init()
{
    // mouse monitor
    d->regionMonitor->setCoordinateType(DRegionMonitor::Original);

    setFocusPolicy(Qt::NoFocus);
    setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    if (WindowUtils::isWayLand()) {
        fmDebug() << "Running on Wayland, setting window properties";
        d->propertyForWayland();
    }

    setBlendMode(DBlurEffectWidget::BehindWindowBlend);

    d->initUI();
    adjustGeometry();

    if (d->mode == Mode::WallpaperMode)
        d->wallpaperPrview->setVisible(true);

    CanvasCoreSubscribe(signal_ScreenProxy_ScreenChanged, d, &WallpaperSettingsPrivate::onScreenChanged);
    CanvasCoreSubscribe(signal_ScreenProxy_DisplayModeChanged, d, &WallpaperSettingsPrivate::onScreenChanged);
    CanvasCoreSubscribe(signal_ScreenProxy_ScreenGeometryChanged, this, &WallpaperSettings::onGeometryChanged);
}
