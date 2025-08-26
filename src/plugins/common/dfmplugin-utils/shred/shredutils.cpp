// SPDX-FileCopyrightText: 2025 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "shredutils.h"
#include "progressdialog.h"
#include "fileshredworker.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>
#include <dfm-base/utils/fileutils.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/deviceproxymanager.h>
#include <dfm-base/widgets/filemanagerwindowsmanager.h>

#include <DSwitchButton>
#include <DSettingsOption>
#include <DTipLabel>
#include <DDialog>
#include <DFrame>

#include <QListWidget>
#include <QStandardPaths>
#include <QFileInfo>
#include <QStorageInfo>
#include <QRegularExpression>
#include <QApplication>
#include <QVBoxLayout>

#include <mutex>

inline constexpr char kShredDConfigName[] = "org.deepin.dde.file-manager.shred";

DWIDGET_USE_NAMESPACE
DFMBASE_USE_NAMESPACE
using namespace dfmplugin_utils;

ShredUtils::ShredUtils(QObject *parent)
    : QObject(parent)
{
}

ShredUtils::~ShredUtils()
{
}

ShredUtils *ShredUtils::instance()
{
    static ShredUtils ins;
    return &ins;
}

void ShredUtils::setShredEnabled(bool enable)
{
    DConfigManager::instance()->setValue(kShredDConfigName, "shred.enabled", enable);
}

bool ShredUtils::isShredEnabled()
{
    const QVariant vRe = DConfigManager::instance()->value(kShredDConfigName, "shred.enabled");
    return vRe.toBool();
}

void ShredUtils::initDconfig()
{
    static std::once_flag flag;
    std::call_once(flag, [] {
        QString err;
        if (!DConfigManager::instance()->addConfig(kShredDConfigName, &err))
            fmWarning() << "Shred: create dconfig failed: " << err;
    });
}

bool ShredUtils::isValidFile(const QUrl &file)
{
    // 获取真实路径(解析软链接)
    QString realPath = QFileInfo(file.toLocalFile()).canonicalFilePath();
    if (realPath.isEmpty())
        realPath = file.toLocalFile();

    if (DevProxyMng->isFileOfExternalBlockMounts(realPath))
        return true;

    // 如果是数据盘路径，移除前缀后再判断
    QString homePath = QStandardPaths::writableLocation(QStandardPaths::HomeLocation);
    realPath = FileUtils::bindPathTransform(realPath, false);
    if (!realPath.startsWith(homePath) || realPath == homePath)
        return false;

    // 检查是否为特殊目录
    static QStringList protectedDirs = {
        QStandardPaths::writableLocation(QStandardPaths::DesktopLocation),
        QStandardPaths::writableLocation(QStandardPaths::DownloadLocation),
        QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation),
        QStandardPaths::writableLocation(QStandardPaths::PicturesLocation),
        QStandardPaths::writableLocation(QStandardPaths::MusicLocation),
        QStandardPaths::writableLocation(QStandardPaths::MoviesLocation),
        homePath + "/Templates",
        homePath + "/Public"
    };

    // 检查路径是否为受保护目录（只检查目录本身，不包括子目录）
    if (protectedDirs.contains(realPath)) {
        fmWarning() << "Cannot shred protected directory: " << realPath;
        return false;
    }

    return true;
}

void ShredUtils::shredfile(const QList<QUrl> &fileList, quint64 winId)
{
    if (fileList.isEmpty())
        return;

    if (!confirmAndDisplayFiles(fileList))
        return;

    // Create and setup worker thread
    FileShredWorker *worker = new FileShredWorker;
    QSharedPointer<QThread> thread(new QThread);
    worker->moveToThread(thread.data());
    connect(thread.data(), &QThread::finished, worker, &QObject::deleteLater);
    connect(qApp, &QApplication::aboutToQuit, thread.data(), [thread, worker] {
        worker->stop();
        thread->quit();
        thread->wait();
    });

    // Create progress dialog
    auto window = FMWindowsIns.findWindowById(winId);
    ProgressDialog *progressDialog = new ProgressDialog(window);
    progressDialog->setAttribute(Qt::WA_DeleteOnClose);

    // Setup signal connections
    connect(worker, &FileShredWorker::progressUpdated, progressDialog, &ProgressDialog::updateProgressValue);
    connect(worker, &FileShredWorker::finished, this, [progressDialog](bool success, const QString &message) {
        if (success) {
            progressDialog->updateProgressValue(100, message);
            QTimer::singleShot(500, progressDialog, [progressDialog] {
                progressDialog->close();
            });
        } else {
            progressDialog->handleShredResult(false, message);
            progressDialog->exec();
        }
    });

    // Start the process
    progressDialog->show();
    thread->start();

    // Trigger worker to start processing
    QMetaObject::invokeMethod(worker, "shredFile", Qt::QueuedConnection, Q_ARG(QList<QUrl>, fileList));
}

void ShredUtils::updateVaultMenuConfig()
{
    const QString kVaultDConfigName = "org.deepin.dde.file-manager.vault";

    // 获取现有的菜单配置
    const QVariant vRe = DConfigManager::instance()->value(kVaultDConfigName, "normalMenuActions");
    QStringList currentConfig = vRe.toStringList();
    fmDebug() << "Current vault menu config: " << currentConfig;

    if (!currentConfig.contains("shredfile")) {
        // 找到 "reverse-select" 的位置
        int reverseSelectIndex = currentConfig.indexOf("reverse-select");

        if (reverseSelectIndex != -1) {
            // 在 "reverse-select" 后面插入 "shredfile"
            currentConfig.insert(reverseSelectIndex + 1, "separator-line");
            currentConfig.insert(reverseSelectIndex + 2, "shredfile");
        } else {
            // 果找不到 "reverse-select"，就追加到末尾
            currentConfig.append("shredfile");
        }

        fmDebug() << "New config to be set: " << currentConfig;

        // 使用 DConfigManager 写入新配置
        DConfigManager::instance()->setValue(kVaultDConfigName, "normalMenuActions", currentConfig);
    }
}

QPair<QWidget *, QWidget *> ShredUtils::createShredSettingItem(QObject *opt)
{
    auto option = qobject_cast<Dtk::Core::DSettingsOption *>(opt);

    auto widget = new QWidget;
    widget->setContentsMargins(0, 0, 0, 0);
    QVBoxLayout *layout = new QVBoxLayout(widget);
    layout->setContentsMargins(0, 0, 0, 0);

    auto lab = new QLabel(option->data("text").toString());
    layout->addWidget(lab);

    QHBoxLayout *hLayout = new QHBoxLayout();
    hLayout->setContentsMargins(0, 0, 0, 0);
    layout->addLayout(hLayout);

    auto msgLabel = new DTipLabel(option->data("message").toString(), widget);
    msgLabel->setAlignment(Qt::AlignLeft);
    msgLabel->setWordWrap(true);

    hLayout->addWidget(msgLabel);

    auto btn = new DSwitchButton;
    // 设置初始状态
    bool status = ShredUtils::instance()->isShredEnabled();
    btn->setChecked(status);
    option->setValue(status);

    // 连接点击信号
    connect(btn, &DSwitchButton::clicked, option, [option](bool checked) {
        ShredUtils::instance()->setShredEnabled(checked);
        option->setValue(checked);

        fmDebug() << "Shred function" << checked;
    });

    connect(option, &Dtk::Core::DSettingsOption::valueChanged, btn, [btn](QVariant value) {
        bool checked = value.toBool();
        fmDebug() << "Shred function DSettingsOption" << checked;
        ShredUtils::instance()->setShredEnabled(checked);
        btn->setChecked(checked);
    });

    return qMakePair(widget, btn);
}

bool ShredUtils::confirmAndDisplayFiles(const QList<QUrl> &fileList)
{
    DDialog dialog(qApp->activeWindow());
    dialog.setIcon(QIcon::fromTheme("dialog-warning"));

    QString title = tr("Are you sure to shred these %1 items?").arg(fileList.count());
    QString message = tr("The file will be completely deleted and cannot be recovered.");
    dialog.setTitle(title);
    dialog.setMessage(message);

    DFrame *frame = new DFrame(&dialog);
    QVBoxLayout *layout = new QVBoxLayout(frame);
    layout->setContentsMargins(4, 4, 4, 4);
    QListWidget *listWidget = new QListWidget(frame);
    listWidget->setFrameShape(QFrame::NoFrame);
    layout->addWidget(listWidget);

    for (const auto &url : fileList) {
        auto info = InfoFactory::create<FileInfo>(url, Global::CreateFileInfoType::kCreateFileInfoAuto);
        if (!info->exists())
            continue;

        QListWidgetItem *item = new QListWidgetItem(info->fileIcon(), info->displayOf(DisPlayInfoType::kFileDisplayName));
        item->setSizeHint(QSize(item->sizeHint().width(), 35));   // 设置item高度
        listWidget->addItem(item);
    }

    dialog.addContent(frame);
    dialog.addButton(tr("Cancel"));
    dialog.addButton(tr("Shred"), true, DDialog::ButtonWarning);

    int ret = dialog.exec();
    return ret == 1;
}
