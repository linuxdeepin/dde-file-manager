/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
 *
 * Author:     dragondjf<dingjiangfeng@deepin.com>
 *
 * Maintainer: dragondjf<dingjiangfeng@deepin.com>
 *             zccrs<zhangjide@deepin.com>
 *             Tangtong<tangtong@deepin.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "frame.h"
#include "constants.h"
#include "wallpaperlist.h"
#include "wallpaperitem.h"
#include "dbus/deepin_wm.h"
#include "thumbnailmanager.h"
#include "appearance_interface.h"
#include "backgroundhelper.h"

#ifndef DISABLE_SCREENSAVER
#include "screensaver_interface.h"
#endif

#include <DThemeManager>
#include <dsegmentedcontrol.h>
#include <DWindowManagerHelper>

#include <QApplication>
#include <QDesktopWidget>
#include <QKeyEvent>
#include <QDebug>
#include <QPainter>
#include <QScrollBar>
#include <QScreen>
#include <QVBoxLayout>
#include <QCheckBox>

#define DESKTOP_BUTTON_ID "desktop"
#define LOCK_SCREEN_BUTTON_ID "lock-screen"
#define SCREENSAVER_BUTTON_ID "screensaver"

static bool previewBackground()
{
    if (DWindowManagerHelper::instance()->windowManagerName() == DWindowManagerHelper::DeepinWM)
        return false;

    return DWindowManagerHelper::instance()->windowManagerName() == DWindowManagerHelper::KWinWM
            || !DWindowManagerHelper::instance()->hasBlurWindow();
}

Frame::Frame(QFrame *parent)
    : DBlurEffectWidget(parent),
      m_wallpaperList(new WallpaperList(this)),
      m_closeButton(new DImageButton(":/images/close_round_normal.svg",
                                 ":/images/close_round_hover.svg",
                                 ":/images/close_round_press.svg", this)),
      m_dbusAppearance(new ComDeepinDaemonAppearanceInterface(AppearanceServ,
                                                              AppearancePath,
                                                              QDBusConnection::sessionBus(),
                                                              this)),
      m_mouseArea(new DRegionMonitor(this))
{
    // 截止到dtkwidget 2.0.10版本，在多个屏幕设置不同缩放比时
    // DRegionMonitor 计算的缩放后的坐标可能是错误的
    m_mouseArea->setCoordinateType(DRegionMonitor::Original);

    setFocusPolicy(Qt::StrongFocus);
    setWindowFlags(Qt::BypassWindowManagerHint | Qt::WindowStaysOnTopHint);
    setAttribute(Qt::WA_TranslucentBackground);

    setBlendMode(DBlurEffectWidget::BehindWindowBlend);
    setMaskColor(DBlurEffectWidget::DarkColor);

    initUI();
    initSize();

    connect(m_mouseArea, &DRegionMonitor::buttonPress, [this](const QPoint &p, const int button){
        if (button == 4) {
            m_wallpaperList->prevPage();
        } else if (button == 5) {
            m_wallpaperList->nextPage();
        } else {
            qDebug() << "button pressed on blank area, quit.";

            qreal scale = devicePixelRatioF();
            const QRect sRect = this->windowHandle()->screen()->geometry();
            QRect nativeRect = geometry();

            // 获取窗口真实的geometry
            nativeRect.setTopLeft((nativeRect.topLeft() - sRect.topLeft()) * scale + sRect.topLeft());
            nativeRect.setSize(nativeRect.size() * scale);

            if (!nativeRect.contains(p)) {
                hide();
            }
        }
    });

    m_closeButton->hide();
    connect(m_wallpaperList, &WallpaperList::mouseOverItemChanged,
            this, &Frame::handleNeedCloseButton);
    connect(m_wallpaperList, &WallpaperList::itemPressed,
            this, &Frame::onItemPressed);

    QTimer::singleShot(0, this, &Frame::initListView);
}

Frame::~Frame()
{

}

void Frame::show()
{
    if (previewBackground()) {
        if (m_dbusDeepinWM) {
            // 销毁不需要的资源
            m_dbusDeepinWM->deleteLater();
            m_dbusDeepinWM = nullptr;
        }

        if (!m_backgroundHelper) {
            m_backgroundHelper = new BackgroundHelper(true, this);
            // 防止壁纸设置窗口被背景窗口覆盖
            connect(m_backgroundHelper, &BackgroundHelper::backgroundAdded, this, &Frame::activateWindow);
        }
    } else if (!m_dbusDeepinWM) {
        if (m_backgroundHelper) {
            // 销毁不需要的资源
            m_backgroundHelper->deleteLater();
            m_backgroundHelper = nullptr;
        }

        m_dbusDeepinWM = new DeepinWM(DeepinWMServ,
                                      DeepinWMPath,
                                      QDBusConnection::sessionBus(),
                                      this);
    }

    if (m_dbusDeepinWM)
        m_dbusDeepinWM->RequestHideWindows();

    m_mouseArea->registerRegion();

    DBlurEffectWidget::show();
}

void Frame::handleNeedCloseButton(QString path, QPoint pos)
{
    if (!path.isEmpty()) {
        m_closeButton->adjustSize();
        m_closeButton->move(pos.x() - 10, pos.y() - 10);
        m_closeButton->show();
        m_closeButton->disconnect();

        connect(m_closeButton, &DImageButton::clicked, this, [this, path] {
            m_dbusAppearance->Delete("background", path);
            m_wallpaperList->removeWallpaper(path);
            m_closeButton->hide();
        }, Qt::UniqueConnection);
    } else {
        m_closeButton->hide();
    }
}

void Frame::showEvent(QShowEvent * event)
{
#ifndef DISABLE_SCREENSAVER
    m_switchModeControl->adjustSize();
#endif

    activateWindow();

    QTimer::singleShot(1, this, &Frame::refreshList);

    DBlurEffectWidget::showEvent(event);
}

void Frame::hideEvent(QHideEvent *event)
{
    DBlurEffectWidget::hideEvent(event);

    if (m_dbusDeepinWM)
        m_dbusDeepinWM->CancelHideWindows();
    m_mouseArea->unregisterRegion();

    if (m_mode == WallpaperMode) {
        if (!m_desktopWallpaper.isEmpty())
            m_dbusAppearance->Set("background", m_desktopWallpaper);
        else if (m_dbusDeepinWM)
            m_dbusDeepinWM->SetTransientBackground("");

        if (!m_lockWallpaper.isEmpty())
            m_dbusAppearance->Set("greeterbackground", m_lockWallpaper);

        ThumbnailManager *manager = ThumbnailManager::instance(devicePixelRatioF());
        manager->stop();
    }
#ifndef DISABLE_SCREENSAVER
    else if (m_mode == ScreenSaverMode) {
        m_dbusScreenSaver->Stop();
    }
#endif

    // 销毁资源
    if (m_dbusDeepinWM) {
        m_dbusDeepinWM->deleteLater();
        m_dbusDeepinWM = nullptr;
    }

    if (m_backgroundHelper) {
        m_backgroundHelper->deleteLater();
        m_backgroundHelper = nullptr;
    }

    emit done();
}

void Frame::keyPressEvent(QKeyEvent * event)
{
    if (event->key() == Qt::Key_Escape) {
        qDebug() << "escape key pressed, quit.";
        hide();
    }

    DBlurEffectWidget::keyPressEvent(event);
}

void Frame::paintEvent(QPaintEvent *event)
{
    DBlurEffectWidget::paintEvent(event);

    QPainter pa(this);

    pa.setCompositionMode(QPainter::CompositionMode_SourceOut);
    pa.setPen(QPen(QColor(255, 255, 255, 20), 1));
    pa.drawLine(QPoint(0, 0), QPoint(width(), 0));
}

bool Frame::event(QEvent *event)
{
#ifndef DISABLE_SCREENSAVER
    if (event->type() == QEvent::LayoutRequest
            || event->type() == QEvent::Resize) {
        adjustModeSwitcherPoint();
    }
#endif

    return DBlurEffectWidget::event(event);
}

#ifndef DISABLE_SCREENSAVER
void Frame::setMode(int mode)
{
    if (m_mode == mode)
        return;

    if (m_mode == ScreenSaverMode) {
        if (m_backgroundHelper) {
            m_backgroundHelper->setVisible(true);
        }

        m_dbusScreenSaver->Stop();
    }

    m_mode = Mode(mode);

    reLayoutTools();
    refreshList();
}

void Frame::reLayoutTools()
{
    if (m_mode == ScreenSaverMode) {
        m_waitControlLabel->show();
        m_waitControl->show();
        m_lockScreenBox->show();
#ifndef DISABLE_WALLPAPER_CAROUSEL
        m_wallpaperCarouselCheckBox->hide();
        m_wallpaperCarouselControl->hide();
        layout()->removeItem(m_wallpaperCarouselLayout);
        static_cast<QBoxLayout*>(layout())->insertLayout(0, m_toolLayout);
#endif
    } else {
        m_waitControlLabel->hide();
        m_waitControl->hide();
        m_lockScreenBox->hide();
#ifndef DISABLE_WALLPAPER_CAROUSEL
        m_wallpaperCarouselCheckBox->show();
        m_wallpaperCarouselControl->setVisible(m_wallpaperCarouselCheckBox->isChecked());
        layout()->removeItem(m_toolLayout);
        static_cast<QBoxLayout*>(layout())->insertLayout(0, m_wallpaperCarouselLayout);
#endif
    }
}
#endif

#if !defined(DISABLE_SCREENSAVER) || !defined(DISABLE_WALLPAPER_CAROUSEL)
void Frame::adjustModeSwitcherPoint()
{
    // 调整模式切换控件的位置
    m_switchModeControl->adjustSize();

    // 自己计算宽度，当控件未显示时无法使用layout的sizeHint
    int tools_width = 0;

#ifndef DISABLE_SCREENSAVER
    {
        auto tools_layout_margins = m_toolLayout->contentsMargins();
        int width = m_waitControlLabel->sizeHint().width() +
                    m_waitControl->sizeHint().width() +
                    m_lockScreenBox->sizeHint().width();

        tools_width = tools_layout_margins.left() + width +
                      m_toolLayout->count() * m_toolLayout->spacing();
    }
#endif

#ifndef DISABLE_WALLPAPER_CAROUSEL
    {
        int width = m_wallpaperCarouselCheckBox->sizeHint().width() +
                    m_wallpaperCarouselControl->sizeHint().width() +
                    m_wallpaperCarouselLayout->contentsMargins().left() +
                    m_wallpaperCarouselControl->count() * m_wallpaperCarouselLayout->spacing();

        if (width > tools_width) {
            tools_width = width;
        }
    }
#endif

    // 防止在低分辨率情况下切换控件和左边的工具栏重叠
    if (width() / 2 < tools_width) {
        m_switchModeControl->move(width() - m_switchModeControl->width() - 10,
                                  (m_wallpaperList->y() - m_switchModeControl->height()) / 2);
    } else {
        m_switchModeControl->move((width() - m_switchModeControl->width()) / 2,
                                  (m_wallpaperList->y() - m_switchModeControl->height()) / 2);
    }
}
#endif

static QString timeFormat(int second)
{
    quint8 s = second % 60;
    quint8 m = second / 60;
    quint8 h = m / 60;
    quint8 d = h / 24;

    m = m % 60;
    h = h % 24;

    QString time_string;

    if (d > 0) {
        time_string.append(QString::number(d)).append("d");
    }

    if (h > 0) {
        if (!time_string.isEmpty()) {
            time_string.append(' ');
        }

        time_string.append(QString::number(h)).append("h");
    }

    if (m > 0) {
        if (!time_string.isEmpty()) {
            time_string.append(' ');
        }

        time_string.append(QString::number(m)).append("m");
    }

    if (s > 0 || time_string.isEmpty()) {
        if (!time_string.isEmpty()) {
            time_string.append(' ');
        }

        time_string.append(QString::number(s)).append("s");
    }

    return time_string;
}

void Frame::initUI()
{
    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setMargin(0);
    layout->setSpacing(0);

    DThemeManager::instance()->setTheme(this, "dark");

#ifndef DISABLE_WALLPAPER_CAROUSEL
    m_wallpaperCarouselLayout = new QHBoxLayout;
    m_wallpaperCarouselCheckBox = new QCheckBox(tr("Wallpaper Slideshow"), this);
    m_wallpaperCarouselCheckBox->setChecked(true);
    m_wallpaperCarouselControl = new DSegmentedControl(this);

    QByteArrayList array_policy {"30", "60", "300", "600", "900", "1800", "3600", "login", "wakeup"};

    {
        int current_policy_index = array_policy.indexOf(m_dbusAppearance->wallpaperSlideShow().toLatin1());

        // 当值不存在此列表时插入此值
        if (current_policy_index < 0) {
            const QString &policy = m_dbusAppearance->wallpaperSlideShow();

            if (!policy.isEmpty()) {
                array_policy.prepend(policy.toLatin1());
            } else {
                m_wallpaperCarouselCheckBox->setChecked(false);
            }

            current_policy_index = 0;
        }

        for (const QByteArray &time : array_policy) {
            int index = 0;

            if (time == "login") {
                index = m_wallpaperCarouselControl->addSegmented(tr("When login"));
            } else if (time == "wakeup") {
                index = m_wallpaperCarouselControl->addSegmented(tr("When wakeup"));
            } else {
                bool ok = false;
                int t = time.toInt(&ok);
                index = ok ? m_wallpaperCarouselControl->addSegmented(timeFormat(t)) : m_wallpaperCarouselControl->addSegmented(time);
            }

            m_wallpaperCarouselControl->at(index)->setMinimumWidth(40);
        }

        m_wallpaperCarouselControl->setCurrentIndex(current_policy_index);
        m_wallpaperCarouselControl->setVisible(m_wallpaperCarouselCheckBox->isChecked());
    }

    m_wallpaperCarouselLayout->setSpacing(10);
    m_wallpaperCarouselLayout->setContentsMargins(20, 10, 20, 10);
    m_wallpaperCarouselLayout->addWidget(m_wallpaperCarouselCheckBox);
    m_wallpaperCarouselLayout->addWidget(m_wallpaperCarouselControl);
    m_wallpaperCarouselLayout->addStretch();

    layout->addLayout(m_wallpaperCarouselLayout);

    connect(m_wallpaperCarouselCheckBox, &QCheckBox::clicked, this, [this, array_policy] (bool checked) {
        m_wallpaperCarouselControl->setVisible(checked);

        if (!checked) {
            m_dbusAppearance->setWallpaperSlideShow(QString());
        } else if (m_wallpaperCarouselControl->currentIndex() >= 0) {
            m_dbusAppearance->setWallpaperSlideShow(array_policy.at(m_wallpaperCarouselControl->currentIndex()));
        }
    });
    connect(m_wallpaperCarouselControl, &DSegmentedControl::currentChanged, this, [this, array_policy] (int index) {
        m_dbusAppearance->setWallpaperSlideShow(array_policy.at(index));
    });
#endif

#ifndef DISABLE_SCREENSAVER
    m_toolLayout = new QHBoxLayout;

    m_waitControl = new DSegmentedControl(this);
    m_lockScreenBox = new QCheckBox(tr("Require a password on wake up"), this);

    QVector<int> time_array {60, 300, 600, 900, 1800, 3600, 0};

    if (!m_dbusScreenSaver) {
        m_dbusScreenSaver = new ComDeepinScreenSaverInterface("com.deepin.ScreenSaver", "/com/deepin/ScreenSaver",
                                                              QDBusConnection::sessionBus(), this);
    }

    int current_wait_time_index = time_array.indexOf(m_dbusScreenSaver->linePowerScreenSaverTimeout());

    // 当值不存在此列表时插入此值
    if (current_wait_time_index < 0) {
        int timeout = m_dbusScreenSaver->linePowerScreenSaverTimeout();
        time_array.prepend(timeout);
        current_wait_time_index = 0;
    }

    for (const int time : time_array) {
        if (time > 0) {
            int index = m_waitControl->addSegmented(timeFormat(time));
            m_waitControl->at(index)->setMinimumWidth(40);
        }
    }

    m_waitControl->addSegmented(tr("Never"));
    m_waitControlLabel = new  QLabel(tr("Wait:"), this);
    m_waitControl->setCurrentIndex(current_wait_time_index);
    m_lockScreenBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    m_lockScreenBox->setChecked(m_dbusScreenSaver->lockScreenAtAwake());

    m_toolLayout->setSpacing(10);
    m_toolLayout->setContentsMargins(20, 10, 20, 10);
    m_toolLayout->addWidget(m_waitControlLabel);
    m_toolLayout->addWidget(m_waitControl);
    m_toolLayout->addSpacing(10);
    m_toolLayout->addWidget(m_lockScreenBox, 1, Qt::AlignLeft);

#ifdef DISABLE_WALLPAPER_CAROUSEL
    // 在布局中占位，保证布局的高度
    QWidget *fake_layout = new QWidget(this);

    fake_layout->setFixedHeight(m_waitControl->height());
    fake_layout->setWindowFlags(Qt::WindowTransparentForInput | Qt::WindowDoesNotAcceptFocus);
    fake_layout->lower();
    m_toolLayout->addWidget(fake_layout);
#endif

    layout->addLayout(m_toolLayout);
    layout->addWidget(m_wallpaperList);

    //###(zccrs): 直接把switModeControl放到布局中始终无法在两种mos模式下都居中
    // 使用anchors使此控件居中
    m_switchModeControl = new DSegmentedControl(this);
    m_switchModeControl->addSegmented({tr("Wallpaper"), tr("Screensaver")});
    m_switchModeControl->at(0)->setMinimumWidth(40);
    m_switchModeControl->at(1)->setMinimumWidth(40);
    m_switchModeControl->setCurrentIndex(m_mode == WallpaperMode ? 0 : 1);

    connect(m_waitControl, &DSegmentedControl::currentChanged, this, [this, time_array] (int index) {
        m_dbusScreenSaver->setBatteryScreenSaverTimeout(time_array[index]);
        m_dbusScreenSaver->setLinePowerScreenSaverTimeout(time_array[index]);
    });

    connect(m_switchModeControl, &DSegmentedControl::currentChanged, this, &Frame::setMode);
    connect(m_lockScreenBox, &QCheckBox::toggled, m_dbusScreenSaver, &ComDeepinScreenSaverInterface::setLockScreenAtAwake);

    reLayoutTools();
#elif !defined(DISABLE_WALLPAPER_CAROUSEL)
    layout->addWidget(m_wallpaperList);
#endif

    layout->addStretch();
}

void Frame::initSize()
{
    const QRect primaryRect = qApp->primaryScreen()->geometry();

#if defined(DISABLE_SCREENSAVER) && defined(DISABLE_WALLPAPER_CAROUSEL)
    setFixedSize(primaryRect.width(), FrameHeight);
#else
    setFixedSize(primaryRect.width(), FrameHeight + 35);
#endif
    qDebug() << "move befor: " << this->geometry() << m_wallpaperList->geometry();
    move(primaryRect.x(), primaryRect.y() + primaryRect.height() - height());
    qDebug() << "this move : " << this->geometry() << m_wallpaperList->geometry();
    m_wallpaperList->setFixedSize(primaryRect.width(), ListHeight);
    m_wallpaperList->move(0, (FrameHeight - ListHeight) / 2);
    qDebug() << "m_wallpaperList move: " << this->geometry() << m_wallpaperList->geometry();
}

void Frame::initListView()
{

}

void Frame::refreshList()
{
    m_wallpaperList->clear();

    if (m_mode == WallpaperMode) {
        QDBusPendingCall call = m_dbusAppearance->List("background");
        QDBusPendingCallWatcher *watcher = new QDBusPendingCallWatcher(call, this);
        connect(watcher, &QDBusPendingCallWatcher::finished, this, [this, call] {
            if (call.isError()) {
                qWarning() << "failed to get all backgrounds: " << call.error().message();
            } else {
                QDBusReply<QString> reply = call.reply();
                QString value = reply.value();
                QStringList strings = processListReply(value);

                foreach (QString path, strings) {
                    WallpaperItem * item = m_wallpaperList->addWallpaper(path);
                    item->setData(item->getPath());
                    item->setDeletable(m_deletableInfo.value(path));
                    item->addButton(DESKTOP_BUTTON_ID, tr("Only desktop"));
                    item->addButton(LOCK_SCREEN_BUTTON_ID, tr("Only lock screen"));
                    item->show();

                    connect(item, &WallpaperItem::buttonClicked, this, &Frame::onItemButtonClicked);
                }

                m_wallpaperList->setFixedWidth(width());
                m_wallpaperList->updateItemThumb();
                m_wallpaperList->show();
            }
        });
    }
#ifndef DISABLE_SCREENSAVER
    else if (m_mode == ScreenSaverMode) {
        if (!m_dbusScreenSaver) {
            m_dbusScreenSaver = new ComDeepinScreenSaverInterface("com.deepin.ScreenSaver", "/com/deepin/ScreenSaver",
                                                                  QDBusConnection::sessionBus(), this);
        }

        const QStringList &saver_name_list = m_dbusScreenSaver->allScreenSaver();

        for (const QString &name : saver_name_list) {
            const QString &cover_path = m_dbusScreenSaver->GetScreenSaverCover(name);

            WallpaperItem *item = m_wallpaperList->addWallpaper(cover_path);
            item->setData(name);
            item->setUseThumbnailManager(false);
            item->setDeletable(false);
            item->addButton(SCREENSAVER_BUTTON_ID, tr("Apply"));
            item->show();

            connect(item, &WallpaperItem::buttonClicked, this, &Frame::onItemButtonClicked);
        }

        m_wallpaperList->setFixedWidth(width());
        m_wallpaperList->updateItemThumb();
        m_wallpaperList->show();
    }
#endif
}

void Frame::onItemPressed(const QString &data)
{
    if (m_mode == WallpaperMode) {
        if (m_dbusDeepinWM)
            m_dbusDeepinWM->SetTransientBackground(data);

        if (m_backgroundHelper)
            m_backgroundHelper->setBackground(data);

        m_desktopWallpaper = data;
        m_lockWallpaper = data;
    }
#ifndef DISABLE_SCREENSAVER
    else if (m_mode == ScreenSaverMode) {
        // 防止壁纸背景盖住屏保预览窗口
        if (m_backgroundHelper) {
            m_backgroundHelper->setVisible(false);
        }

        m_dbusScreenSaver->Preview(data, 1);
    }
#endif
}

void Frame::onItemButtonClicked(const QString &buttonID)
{
    WallpaperItem *item = qobject_cast<WallpaperItem*>(sender());

    if (!item)
        return;

    if (buttonID == DESKTOP_BUTTON_ID) {
        m_lockWallpaper.clear();
    } else if (buttonID == LOCK_SCREEN_BUTTON_ID) {
        m_desktopWallpaper.clear();
    }
#ifndef DISABLE_SCREENSAVER
    else if (buttonID == SCREENSAVER_BUTTON_ID) {
        m_dbusScreenSaver->setCurrentScreenSaver(item->data());
    }
#endif

    hide();
}

QStringList Frame::processListReply(const QString &reply)
{
    QStringList result;

    QJsonDocument doc = QJsonDocument::fromJson(reply.toUtf8());
    if (doc.isArray()) {
        QJsonArray arr = doc.array();
        foreach (QJsonValue val, arr) {
            QJsonObject obj = val.toObject();
            QString id = obj["Id"].toString();
            result.append(id);
            m_deletableInfo[id] = obj["Deletable"].toBool();
        }
    }

    return result;
}
