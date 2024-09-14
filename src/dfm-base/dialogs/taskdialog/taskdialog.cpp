// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
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

    connect(wid, &TaskWidget::heightChanged, this, &TaskDialog::adjustSize, Qt::QueuedConnection);
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
    int fd = -1;
    if (replyBlokShutDown.isValid()) {
        fd = replyBlokShutDown.value().fileDescriptor();
    }

    if (fd > 0) {
        QObject::connect(this, &TaskDialog::closed, this, [this]() {
            QDBusReply<QDBusUnixFileDescriptor> tmp = replyBlokShutDown;   //::close(fd);
            replyBlokShutDown = QDBusReply<QDBusUnixFileDescriptor>();
        });
    }
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
    adjustSize();

    if (taskItems.count() == 1)
        moveToCenter();

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
 * \brief TaskDialog::adjustSize 调整整个进度显示的高度，当每个item中的widget发生变化时
 */
void TaskDialog::adjustSize(int hight)
{
    auto widgit = sender();
    int listHeight = 2;
    for (int i = 0; i < taskListWidget->count(); i++) {
        QListWidgetItem *item = taskListWidget->item(i);
        auto wg = taskListWidget->itemWidget(item);
        int h = widgit == wg && hight > 0 ? hight : wg->height();
        item->setSizeHint(QSize(item->sizeHint().width(), h));
        listHeight += h;
    }

    if (listHeight < qApp->primaryScreen()->availableGeometry().height() - 60) {
        taskListWidget->setFixedHeight(listHeight);
        setFixedHeight(listHeight + 60);
        kMaxHeight = height();
    } else {
        setFixedHeight(kMaxHeight);
    }

    layout()->setSizeConstraint(QLayout::SetNoConstraint);
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
    taskItems.remove(jobHandler);
    setTitle(taskListWidget->count());
    if (taskListWidget->count() == 0) {
        close();
    } else {
        adjustSize();
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
        taskListWidget->takeItem(taskListWidget->row(it.value()));
        it = taskItems.erase(it);
    }
    emit closed();

    DAbstractDialog::closeEvent(event);
}

void TaskDialog::keyPressEvent(QKeyEvent *event)
{
    //handle escape key press event for emitting close event
    if (event->key() == Qt::Key_Escape) {
        emit close();
    }
    QDialog::keyPressEvent(event);
}

TaskDialog::~TaskDialog()
{
}
