// SPDX-FileCopyrightText: 2021 - 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "taskdialog.h"
#include "taskwidget.h"
#include <dfm-base/utils/universalutils.h>

#include <QListWidgetItem>
#include <QListWidget>
#include <QLayout>
#include <QApplication>
#include <QMutex>
#include <QKeyEvent>
#include <QScreen>
#include <QTimer>

using namespace dfmbase;

static const int kDefaultWidth { 700 };
int TaskDialog::kMaxHeight { 0 };

TaskDialog::TaskDialog(QObject *parent)
    : DAbstractDialog(parent)
{
    moveToThread(qApp->thread());
    initUI();
}
/*!
 * \brief TaskDialog::addTask 添加一个任务显示，立即显示，绑定所有taskHandler的信号，在listview中添加一个taskwidget
 * \param taskHandler 任务的控制处理器
 */
void TaskDialog::addTask(const JobHandlePointer taskHandler)
{
    TaskWidget *wid = nullptr;
    if (!taskHandler) {
        qCWarning(logDFMBase) << "task handler is null";
        return;
    }

    if (taskItems.contains(taskHandler)) {
        show();
        raise();
        activateWindow();
        return;
    }

    wid = new TaskWidget(this);

    connect(wid, &TaskWidget::heightChanged, this, &TaskDialog::scheduleResize, Qt::QueuedConnection);
    connect(this, &TaskDialog::closed, wid, &TaskWidget::parentClose, Qt::QueuedConnection);
    taskHandler->connect(taskHandler.get(), &AbstractJobHandler::requestRemoveTaskWidget, this, &TaskDialog::removeTask);

    wid->setTaskHandle(taskHandler);

    taskHandler->setSignalConnectFinished();

    addTaskWidget(taskHandler, wid);

    return;
}
/*!
 * \brief TaskDialog::initUI 初始化界面UI
 */
void TaskDialog::initUI()
{
    QFont f = font();
    f.setPixelSize(14);
    setFont(f);

    setWindowFlags((windowFlags() & ~Qt::WindowSystemMenuHint & ~Qt::Dialog) | Qt::Window
                   | Qt::WindowMinMaxButtonsHint);
    setWindowIcon(QIcon::fromTheme("dde-file-manager"));
    setFixedWidth(kDefaultWidth);
    titlebar = new DTitlebar(this);
    titlebar->layout()->setContentsMargins(0, 0, 0, 0);
    titlebar->setMenuVisible(false);
    titlebar->setIcon(QIcon::fromTheme("dde-file-manager"));
    titlebar->setAutoFillBackground(false);

    taskListWidget = new QListWidget(this);
    taskListWidget->setSelectionMode(QListWidget::NoSelection);
    taskListWidget->viewport()->setAutoFillBackground(false);
    taskListWidget->setFrameShape(QFrame::NoFrame);

    QVBoxLayout *mainLayout = new QVBoxLayout(this);
    mainLayout->setContentsMargins(0, 0, 0, 0);
    mainLayout->setSpacing(0);
    mainLayout->addWidget(titlebar);
    mainLayout->addWidget(taskListWidget);
    mainLayout->addStretch(1);
    setLayout(mainLayout);

    moveToCenter();
}
/*!
 * \brief TaskDialog::blockShutdown 调用dbus处理任务中，阻止系统进入休眠或关机，避免文件损坏
 */
void TaskDialog::blockShutdown()
{
    UniversalUtils::blockShutdown(replyBlokShutDown);
    // 添加进入节能模式的锁定处理
    // 防止重复调用导致 cookie 被覆盖
    if (screenSaverCookie == kInvalidScreenSaverCookie) {
        screenSaverCookie = UniversalUtils::lockScreenSaver();
    }

    QObject::connect(this, &TaskDialog::closed, this, [this]() {
        QDBusReply<QDBusUnixFileDescriptor> tmp = replyBlokShutDown;   //::close(fd);
        replyBlokShutDown = QDBusReply<QDBusUnixFileDescriptor>();
        // 添加进入节能模式的解除锁定处理
        if (screenSaverCookie != kInvalidScreenSaverCookie) {
            UniversalUtils::unlockScreenSaver(screenSaverCookie);
            screenSaverCookie = kInvalidScreenSaverCookie;
        }
    });
}
/*!
 * \brief TaskDialog::addTaskWidget 在任务进度对话框中添加一个item，并调整高度
 * \param taskHandler 任务控制器
 * \param wid item的widget
 */
void TaskDialog::addTaskWidget(const JobHandlePointer taskHandler, TaskWidget *wid)
{
    if (!wid) {
        qCWarning(logDFMBase) << "TaskWidget is a null value!";
        return;
    }

    blockShutdown();

    QListWidgetItem *item = new QListWidgetItem();
    item->setSizeHint(QSize(wid->width(), wid->height()));
    item->setFlags(Qt::NoItemFlags);
    taskListWidget->addItem(item);
    taskListWidget->setItemWidget(item, wid);
    taskItems.insert(taskHandler, item);

    setWindowFlags(Qt::WindowMinimizeButtonHint | Qt::WindowCloseButtonHint);
    setTitle(taskListWidget->count());
    scheduleResize();

    setModal(false);
    show();
    activateWindow();
}
/*!
 * \brief TaskDialog::setTitle 设置任务进度对话框的title
 * \param taskCount 当前任务的个数
 */
void TaskDialog::setTitle(int taskCount)
{
    titlebar->setTitle(QObject::tr("%1 tasks in progress").arg(QString::number(taskCount)));
}

/*!
 * \brief TaskDialog::scheduleResize 合并同帧内的多次几何请求，事件循环末帧只执行一次几何更新
 * 新增/移除任务、widget 内部高度变化都会触发本方法，用布尔标志去重，
 * 保证单帧内无论触发多少次，都只做一次 resizeAndCenter，从而消除抖动。
 */
void TaskDialog::scheduleResize()
{
    if (m_resizeScheduled)
        return;
    m_resizeScheduled = true;
    QTimer::singleShot(0, this, [this]() {
        m_resizeScheduled = false;
        resizeAndCenter();
    });
}

/*!
 * \brief TaskDialog::resizeAndCenter 计算任务列表高度并一次性完成窗口高度调整与居中
 * 将「变高/变矮」与「居中移动」合并到同一次事件处理中，中间不产生错位帧，避免视觉抖动。
 */
void TaskDialog::resizeAndCenter()
{
    int listHeight = 2;
    for (int i = 0; i < taskListWidget->count(); i++) {
        QListWidgetItem *item = taskListWidget->item(i);
        auto wg = taskListWidget->itemWidget(item);
        if (!wg)
            continue;
        item->setSizeHint(QSize(item->sizeHint().width(), wg->height()));
        listHeight += wg->height();
    }

    int oldHeight = height();
    if (listHeight < qApp->primaryScreen()->availableGeometry().height() - 60) {
        taskListWidget->setFixedHeight(listHeight);
        setFixedHeight(listHeight + 60);
        kMaxHeight = height();
    } else {
        setFixedHeight(kMaxHeight);
    }

    layout()->setSizeConstraint(QLayout::SetNoConstraint);

    // 高度未变化时无需移动，避免无谓的窗口重定位
    if (oldHeight == height())
        return;

    // 未显示时整体居中；已显示时仅重算纵向位置，保持水平位置不变
    if (!isVisible()) {
        moveToCenter();
    } else {
        moveYCenter();
    }
}
/*!
 * \brief TaskDialog::moveYCenter 任务进度框自动调整到屏幕的中央
 */
void TaskDialog::moveYCenter()
{
    QRect qr = frameGeometry();
    QPoint cp;
    if (parent()) {
        cp = static_cast<QWidget *>(parent())->geometry().center();
    } else {
        cp = qApp->primaryScreen()->availableGeometry().center();
    }
    qr.moveCenter(cp);
    move(x(), qr.y());
}
/*!
 * \brief TaskDialog::removeTask 移除任务的item，当list中的item <= 0时，关闭整个进度显示框
 */
void TaskDialog::removeTask()
{
    auto send = sender();
    JobHandlePointer jobHandler { nullptr };
    for (const auto &handler : taskItems.keys()) {
        if (handler.data() == send) {
            jobHandler = handler;
            break;
        }
    }
    if (!jobHandler)
        return;

    if (!taskItems.contains(jobHandler)) {
        qCWarning(logDFMBase) << "taskItems not contains the task!";
        return;
    }

    QListWidgetItem *item = taskItems.value(jobHandler);
    taskListWidget->removeItemWidget(item);
    taskListWidget->takeItem(taskListWidget->row(item));
    delete item;   // 显式删除动态分配的对象
    taskItems.remove(jobHandler);
    setTitle(taskListWidget->count());
    if (taskListWidget->count() == 0) {
        close();
    } else {
        scheduleResize();
    }
}
/*!
 * \brief TaskDialog::closeEvent 关闭事件，这里是为了发送当前窗口关闭信号
 * \param event 关闭事件
 */
void TaskDialog::closeEvent(QCloseEvent *event)
{
    QMap<JobHandlePointer, QListWidgetItem *>::iterator it = taskItems.begin();
    while (it != taskItems.end()) {
        it.key()->operateTaskJob(AbstractJobHandler::SupportAction::kStopAction);
        taskListWidget->removeItemWidget(it.value());
        QListWidgetItem *item = taskListWidget->takeItem(taskListWidget->row(it.value()));
        delete item;   // 显式删除动态分配的对象
        it = taskItems.erase(it);
    }
    emit closed();

    DAbstractDialog::closeEvent(event);
}

void TaskDialog::keyPressEvent(QKeyEvent *event)
{
    // handle escape key press event for emitting close event
    if (event->key() == Qt::Key_Escape) {
        emit close();
    }
    QDialog::keyPressEvent(event);
}

TaskDialog::~TaskDialog()
{
    // 确保在析构时解锁屏幕保护程序
    if (screenSaverCookie != kInvalidScreenSaverCookie) {
        UniversalUtils::unlockScreenSaver(screenSaverCookie);
        screenSaverCookie = kInvalidScreenSaverCookie;
    }
}
