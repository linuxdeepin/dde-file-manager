// SPDX-FileCopyrightText: 2022 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "framemanager.h"
#include "private/framemanager_p.h"
#include "config/configpresenter.h"
#include "interface/canvasmodelshell.h"
#include "interface/canvasviewshell.h"
#include "desktoputils/ddplugin_eventinterface_helper.h"
#include "menus/extendcanvasscene.h"

#include "plugins/common/dfmplugin-menu/menu_eventinterface_helper.h"
#include <dfm-base/dfm_desktop_defines.h>

#include <DDBusSender>

#include <QAbstractItemView>
#include <QDBusInterface>

DFMBASE_USE_NAMESPACE
using namespace ddplugin_organizer;

#define CanvasCoreSubscribe(topic, func) \
    dpfSignalDispatcher->subscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

#define CanvasCoreUnsubscribe(topic, func) \
    dpfSignalDispatcher->unsubscribe("ddplugin_core", QT_STRINGIFY2(topic), this, func);

FrameManagerPrivate::FrameManagerPrivate(FrameManager *qq)
    : QObject(qq), q(qq)
{
    layoutTimer = new QTimer(this);
    layoutTimer->setInterval(1000);
    layoutTimer->setSingleShot(true);
    connect(layoutTimer, &QTimer::timeout, this, [this]{
        if (organizer) {
            organizer->layout();
        }
    });
}

FrameManagerPrivate::~FrameManagerPrivate()
{
    delete options;
    options = nullptr;
}

void FrameManagerPrivate::buildSurface()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    QMap<QString, QWidget *> rootMap;
    // create surface
    for (QWidget *win : root) {
        const QString screenName = win->property(DesktopFrameProperty::kPropScreenName).toString();
        if (screenName.isEmpty()) {
            fmWarning() << "can not get screen name from root window";
            continue;
        }

        SurfacePointer surface = surfaceWidgets.value(screenName);
        if (surface.isNull()) {
            // add new widget
            fmInfo() << "screen:" << screenName << "  added, create it.";
            surface = createSurface(win);
            surfaceWidgets.insert(screenName, surface);
        }

        layoutSurface(win, surface);
        rootMap.insert(screenName, win);
    }

    // clean up invalid widget
    {
        for (const QString &sp : surfaceWidgets.keys()) {
            if (!rootMap.contains(sp)) {
                surfaceWidgets.take(sp);
            }
        }
    }
}

void FrameManagerPrivate::clearSurface()
{
    for (const SurfacePointer &sur : surfaceWidgets.values())
        sur->setParent(nullptr);

    surfaceWidgets.clear();
}

SurfacePointer FrameManagerPrivate::createSurface(QWidget *root)
{
    SurfacePointer surface = nullptr;
    if (Q_UNLIKELY(!root)) {
        fmWarning() << "Cannot create surface for null root widget";
        return surface;
    }

    surface.reset(new Surface());
    surface->setProperty(DesktopFrameProperty::kPropScreenName, root->property(DesktopFrameProperty::kPropScreenName).toString());
    surface->setProperty(DesktopFrameProperty::kPropWidgetName, "organizersurface");
    surface->setProperty(DesktopFrameProperty::kPropWidgetLevel, 11.0);
    return surface;
}

void FrameManagerPrivate::layoutSurface(QWidget *root, SurfacePointer surface, bool hidden)
{
    Q_ASSERT(surface);
    Q_ASSERT(root);

    auto view = dynamic_cast<QAbstractItemView *>(findView(root));
    // check hidden flags
    if (view && !hidden) {
        surface->setParent(view->viewport());
        surface->setGeometry(QRect(QPoint(0, 0), view->geometry().size()));
    } else {
        surface->setParent(root);
        surface->setGeometry(QRect(QPoint(0, 0), root->geometry().size()));
    }
}

void FrameManagerPrivate::buildOrganizer()
{
    q->switchMode(CfgPresenter->mode());
}

QList<SurfacePointer> FrameManagerPrivate::surfaces() const
{
    QList<SurfacePointer> ret;
    for (QWidget *root : ddplugin_desktop_util::desktopFrameRootWindows()) {
        const QString screenName = root->property(DesktopFrameProperty::kPropScreenName).toString();
        auto sur = surfaceWidgets.value(screenName);
        if (!sur.isNull())
            ret.append(sur);
    }

    return ret;
}

void FrameManagerPrivate::refeshCanvas()
{
    // refresh immediately to prevent show the files which is move to collection view.
    if (canvas)
        canvas->canvasModel()->refresh(0, false);
}

void FrameManagerPrivate::onHideAllKeyPressed()
{
    const auto &surfaces { organizer->getSurfaces() };
    if (surfaces.count() <= 0)
        return;

    fmDebug() << "Hide/Show all collections!";

    bool aboutToHide = surfaces.at(0)->isVisible();
    std::for_each(surfaces.begin(), surfaces.end(), [](SurfacePointer surface) {
        surface->setVisible(!surface->isVisible());
    });

    if (!CfgPresenter->isRepeatNoMore() && aboutToHide) {
        uint notifyId = QDate::currentDate().daysInYear();
        QString keySequence = CfgPresenter->hideAllKeySequence().toString();
        QString tips = tr("To disable the One-Click Hide feature, invoke the \"Desktop Settings\" window "
                          "in the desktop context menu and turn off the \"One-Click Hide Collection\".");
        QString cmdNoRepeation = "dde-dconfig,--set,-a,org.deepin.dde.file-manager,-r,org.deepin.dde.file-manager.desktop.organizer,-k,hideAllDialogRepeatNoMore,-v,true";
        QString cmdCloseNotify = QString("dbus-send,--type=method_call,--dest=org.freedesktop.Notifications,/org/freedesktop/Notifications,com.deepin.dde.Notification.CloseNotification,uint32:%1")
                                         .arg(notifyId);
        DDBusSender()
                .service("org.freedesktop.Notifications")
                .path("/org/freedesktop/Notifications")
                .interface("org.freedesktop.Notifications")
                .method(QString("Notify"))
                .arg(tr("Desktop organizer"))
                .arg(notifyId)
                .arg(QString("deepin-toggle-desktop"))
                .arg(tr("Shortcut \"%1\" to show collections").arg(keySequence))
                .arg(tips)
                .arg(QStringList { "close-notify", tr("Close"), "no-repeat", tr("No more prompts") })
                .arg(QVariantMap { { "x-deepin-action-no-repeat", cmdNoRepeation },
                                   { "x-deepin-action-close-notify", cmdCloseNotify } })
                .arg(3000)
                .call();
    }
}

void FrameManagerPrivate::enableChanged(bool e)
{
    if (e == CfgPresenter->isEnable())
        return;

    fmInfo() << "Organizer enable state changed to:" << e;
    CfgPresenter->setEnable(e);
    if (e) {
        q->turnOn();
    } else {
        q->turnOff();
        if (CfgPresenter->organizeOnTriggered())
            CfgPresenter->saveNormalProfile({});   // feature
    }
}

void FrameManagerPrivate::enableVisibility(bool e)
{
    fmDebug() << "Visibility state changed to:" << e;
    CfgPresenter->setEnableVisibility(e);
}

void FrameManagerPrivate::saveHideAllSequence(const QKeySequence &seq)
{
    fmDebug() << "Hide all key sequence changed to:" << seq.toString();
    CfgPresenter->setHideAllKeySequence(seq);
}

void FrameManagerPrivate::switchToCustom()
{
    Q_ASSERT(organizer);

    if (organizer->mode() == OrganizerMode::kCustom) {
        fmDebug() << "Reject switch to custom: already in custom mode";
        return;
    }

    fmInfo() << "Switching organizer to custom mode";
    CfgPresenter->setMode(kCustom);
    buildOrganizer();
}

void FrameManagerPrivate::switchToNormalized(int cf)
{
    Q_ASSERT(organizer);

    if (organizer->mode() == OrganizerMode::kNormalized) {
        fmInfo() << "Already in normalized mode, changing classification to:" << cf;
        CfgPresenter->setClassification(static_cast<Classifier>(cf));
        organizer->reset();
    } else {
        fmInfo() << "Switching organizer to normalized mode with classification:" << cf;
        CfgPresenter->setMode(kNormalized);
        CfgPresenter->setClassification(static_cast<Classifier>(cf));
        buildOrganizer();
    }
}

void FrameManagerPrivate::showOptionWindow()
{
    if (options) {
        fmDebug() << "Options window already exists, activating it";
        options->activateWindow();
        return;
    }

    options = new OptionsWindow();
    options->setAttribute(Qt::WA_DeleteOnClose);
    options->initialize();
    connect(
            options, &OptionsWindow::destroyed, this, [this]() {
                options = nullptr;
            },
            Qt::DirectConnection);

    options->moveToCenter(QCursor::pos());
    options->show();
}

QWidget *FrameManagerPrivate::findView(QWidget *root) const
{
    if (Q_UNLIKELY(!root))
        return nullptr;

    for (QObject *obj : root->children()) {
        if (QWidget *wid = dynamic_cast<QWidget *>(obj)) {
            QString type = wid->property(DesktopFrameProperty::kPropWidgetName).toString();
            if (type == "canvas") {
                return wid;
            }
        }
    }
    return nullptr;
}

FrameManager::FrameManager(QObject *parent)
    : QObject(parent), d(new FrameManagerPrivate(this))
{
}

FrameManager::~FrameManager()
{
    turnOff();

    // unregister menu
    dfmplugin_menu_util::menuSceneUnbind(ExtendCanvasCreator::name());
    auto creator = dfmplugin_menu_util::menuSceneUnregisterScene(ExtendCanvasCreator::name());
    if (creator)
        delete creator;
}

bool FrameManager::initialize()
{
    // initialize config
    CfgPresenter->initialize();

    // register menu for canvas
    dfmplugin_menu_util::menuSceneRegisterScene(ExtendCanvasCreator::name(), new ExtendCanvasCreator());
    dfmplugin_menu_util::menuSceneBind(ExtendCanvasCreator::name(), "CanvasMenu");

    bool enable = CfgPresenter->isEnable();
    fmInfo() << "Organizer enable:" << enable;
    if (enable)
        turnOn(false);   // builded by signal.

    connect(CfgPresenter, &ConfigPresenter::changeEnableState, d, &FrameManagerPrivate::enableChanged, Qt::QueuedConnection);
    connect(CfgPresenter, &ConfigPresenter::changeEnableVisibilityState, d, &FrameManagerPrivate::enableVisibility, Qt::QueuedConnection);
    connect(CfgPresenter, &ConfigPresenter::changeHideAllKeySequence, d, &FrameManagerPrivate::saveHideAllSequence, Qt::QueuedConnection);
    connect(CfgPresenter, &ConfigPresenter::switchToNormalized, d, &FrameManagerPrivate::switchToNormalized, Qt::QueuedConnection);
    connect(CfgPresenter, &ConfigPresenter::switchToCustom, d, &FrameManagerPrivate::switchToCustom, Qt::QueuedConnection);
    connect(CfgPresenter, &ConfigPresenter::showOptionWindow, d, &FrameManagerPrivate::showOptionWindow, Qt::QueuedConnection);

    return true;
}

void FrameManager::layout()
{
    if (d->organizer)
        d->organizer->layout();
}

void FrameManager::switchMode(OrganizerMode mode)
{
    if (d->organizer) {
        fmDebug() << "Deleting existing organizer before mode switch";
        delete d->organizer;
    }

    fmInfo() << "Switching organizer to mode:" << static_cast<int>(mode);

    d->organizer = OrganizerCreator::createOrganizer(mode);
    Q_ASSERT(d->organizer);

    connect(d->organizer, &CanvasOrganizer::collectionChanged, d, &FrameManagerPrivate::refeshCanvas);
    connect(d->organizer, &CanvasOrganizer::hideAllKeyPressed, d, &FrameManagerPrivate::onHideAllKeyPressed);

    // initialize to create collection widgets
    if (!d->surfaceWidgets.isEmpty()) {
        fmDebug() << "Setting" << d->surfaceWidgets.size() << "surfaces to organizer";
        d->organizer->setSurfaces(d->surfaces());
    }

    d->organizer->setCanvasModelShell(d->canvas->canvasModel());
    d->organizer->setCanvasViewShell(d->canvas->canvasView());
    d->organizer->setCanvasGridShell(d->canvas->canvasGrid());
    d->organizer->setCanvasManagerShell(d->canvas->canvasManager());
    d->organizer->setCanvasSelectionShell(d->canvas->canvasSelectionShell());
    d->organizer->initialize(d->model);
}

void FrameManager::turnOn(bool build)
{
    fmInfo() << "Turning on organizer framework";

#ifdef QT_DEBUG
    QDBusInterface ifs("com.deepin.dde.desktop",
                       "/org/deepin/dde/desktop/canvas",
                       "org.deepin.dde.desktop.canvas");
    ifs.call("EnableUIDebug", QVariant::fromValue(false));
#endif
    Q_ASSERT(!d->canvas);
    Q_ASSERT(!d->model);
    Q_ASSERT(!d->organizer);

    CanvasCoreSubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &FrameManager::onDetachWindows);
    CanvasCoreSubscribe(signal_DesktopFrame_WindowBuilded, &FrameManager::onBuild);
    CanvasCoreSubscribe(signal_DesktopFrame_WindowShowed, &FrameManager::onWindowShowed);
    CanvasCoreSubscribe(signal_DesktopFrame_GeometryChanged, &FrameManager::onGeometryChanged);
    CanvasCoreSubscribe(signal_DesktopFrame_AvailableGeometryChanged, &FrameManager::onGeometryChanged);

    d->canvas = new CanvasInterface(this);
    d->canvas->initialize();

    d->model = new CollectionModel(this);
    d->model->setModelShell(d->canvas->fileInfoModel());

    if (build) {
        onBuild();

        // show surface
        for (const SurfacePointer &sur : d->surfaceWidgets.values())
            sur->setVisible(true);
    }
}

void FrameManager::turnOff()
{
    fmInfo() << "Turning off organizer framework";

#ifdef QT_DEBUG
    QDBusInterface ifs("com.deepin.dde.desktop",
                       "/org/deepin/dde/desktop/canvas",
                       "org.deepin.dde.desktop.canvas");
    ifs.call("EnableUIDebug", QVariant::fromValue(true));
#endif
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowAboutToBeBuilded, &FrameManager::onDetachWindows);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowBuilded, &FrameManager::onBuild);
    CanvasCoreUnsubscribe(signal_DesktopFrame_WindowShowed, &FrameManager::onWindowShowed);
    CanvasCoreUnsubscribe(signal_DesktopFrame_GeometryChanged, &FrameManager::onGeometryChanged);
    CanvasCoreUnsubscribe(signal_DesktopFrame_AvailableGeometryChanged, &FrameManager::onGeometryChanged);

    delete d->organizer;
    d->organizer = nullptr;

    delete d->model;
    d->model = nullptr;

    d->clearSurface();

    // restore canvas
    d->refeshCanvas();

    delete d->canvas;
    d->canvas = nullptr;
}

bool FrameManager::organizerEnabled()
{
    return CfgPresenter->isEnable();
}

void FrameManager::onBuild()
{
    // 1071 added, tag config file changed
    if (CfgPresenter->version() != "2.0.0") {
        fmInfo() << "Updating config version to 2.0.0";
        CfgPresenter->setVersion("2.0.0");
    }

    d->buildSurface();

    if (d->organizer) {
        d->organizer->setSurfaces(d->surfaces());
        d->layoutTimer->start();
    } else {
        d->buildOrganizer();
    }
}

void FrameManager::onWindowShowed()
{
    // todo hide the canvas if needed.
}

void FrameManager::onDetachWindows()
{
    for (const SurfacePointer &sur : d->surfaceWidgets.values())
        sur->setParent(nullptr);

    // 解绑集合窗口
    if (d->organizer) {
        fmDebug() << "Detaching organizer layout";
        d->organizer->detachLayout();
    }
}

void FrameManager::onGeometryChanged()
{
    QList<QWidget *> root = ddplugin_desktop_util::desktopFrameRootWindows();
    for (auto win : root) {
        QString screen = win->property(DesktopFrameProperty::kPropScreenName).toString();
        auto surface = d->surfaceWidgets.value(screen);
        if (surface)
            d->layoutSurface(win, surface);
    }

    // layout collection widgets
    if (d->organizer)
        d->organizer->layout();
}
