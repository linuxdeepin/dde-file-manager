// SPDX-FileCopyrightText: 2022 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "burnjobmanager.h"
#include "utils/burnjob.h"
#include "utils/auditlogjob.h"
#include "utils/packetwritingjob.h"

#include <dfm-base/file/local/localfilehandler.h>
#include <dfm-base/utils/dialogmanager.h>
#include <dfm-base/base/schemefactory.h>
#include <dfm-base/base/device/devicemanager.h>
#include <dfm-base/base/device/deviceproxymanager.h>

#include <dfm-framework/event/event.h>

#include <dfm-io/dfileinfo.h>

#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>

DFMBASE_USE_NAMESPACE

using namespace dfmplugin_burn;
using namespace GlobalServerDefines;

BurnJobManager *BurnJobManager::instance()
{
    static BurnJobManager manager;
    return &manager;
}

void BurnJobManager::startEraseDisc(const QString &dev)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    DialogManagerInstance->addTask(jobHandler);

    AbstractBurnJob *job = new EraseJob(dev, jobHandler);
    initBurnJobConnect(job);
    connect(qobject_cast<EraseJob *>(job), &EraseJob::eraseFinished, this, [job, this](bool result) {
        startAuditLogForEraseDisc(job->currentDeviceInfo(), result);
    });
    job->start();
}

void BurnJobManager::startBurnISOFiles(const QString &dev, const QUrl &stagingUrl, const BurnJobManager::Config &conf)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    DialogManagerInstance->addTask(jobHandler);

    AbstractBurnJob *job = new BurnISOFilesJob(dev, jobHandler);
    initBurnJobConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::KStagingUrl, stagingUrl);
    job->setProperty(AbstractBurnJob::PropertyType::kSpeeds, conf.speeds);
    job->setProperty(AbstractBurnJob::PropertyType::kVolumeName, conf.volName);
    job->setProperty(AbstractBurnJob::PropertyType::kBurnOpts, QVariant::fromValue(conf.opts));
    job->start();
}

void BurnJobManager::startBurnISOImage(const QString &dev, const QUrl &imageUrl, const BurnJobManager::Config &conf)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    DialogManagerInstance->addTask(jobHandler);

    AbstractBurnJob *job = new BurnISOImageJob(dev, jobHandler);
    initBurnJobConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::kImageUrl, imageUrl);
    job->setProperty(AbstractBurnJob::PropertyType::kSpeeds, conf.speeds);
    job->setProperty(AbstractBurnJob::PropertyType::kBurnOpts, QVariant::fromValue(conf.opts));
    job->start();
}

void BurnJobManager::startBurnUDFFiles(const QString &dev, const QUrl &stagingUrl, const BurnJobManager::Config &conf)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    DialogManagerInstance->addTask(jobHandler);

    AbstractBurnJob *job = new BurnUDFFilesJob(dev, jobHandler);
    initBurnJobConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::KStagingUrl, stagingUrl);
    job->setProperty(AbstractBurnJob::PropertyType::kSpeeds, conf.speeds);
    job->setProperty(AbstractBurnJob::PropertyType::kVolumeName, conf.volName);
    job->setProperty(AbstractBurnJob::PropertyType::kBurnOpts, QVariant::fromValue(conf.opts));
    job->start();
}

void BurnJobManager::startDumpISOImage(const QString &dev, const QUrl &imageUrl)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    DialogManagerInstance->addTask(jobHandler);

    DumpISOImageJob *job = new DumpISOImageJob(dev, jobHandler);
    initDumpJobConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::kImageUrl, imageUrl);
    job->start();
}

void BurnJobManager::startAuditLogForCopyFromDisc(const QList<QUrl> &srcList, const QList<QUrl> &destList)
{
    Q_ASSERT(srcList.size() == destList.size());
    AbstractAuditLogJob *job = new CopyFromDiscAuditLog(srcList, destList);
    connect(job, &AbstractAuditLogJob::finished, job, &QObject::deleteLater);
    job->start();
}

void BurnJobManager::startAuditLogForBurnFiles(const QVariantMap &info, const QUrl &stagingUrl, bool result)
{
    AbstractAuditLogJob *job = new BurnFilesAuditLogJob(stagingUrl, result);
    job->setProperty(DeviceProperty::kDevice, info.value(DeviceProperty::kDevice));
    job->setProperty(DeviceProperty::kDrive, info.value(DeviceProperty::kDrive));
    job->setProperty(DeviceProperty::kMedia, info.value(DeviceProperty::kMedia));
    connect(job, &AbstractAuditLogJob::finished, this, [this, job, stagingUrl, result]() {
        if (result)
            this->deleteStagingDir(stagingUrl);
        job->deleteLater();
    });
    job->start();
}

void BurnJobManager::startAuditLogForEraseDisc(const QVariantMap &info, bool result)
{
    EraseDiscAuditLogJob *job = new EraseDiscAuditLogJob(result);
    job->setProperty(DeviceProperty::kDrive, info.value(DeviceProperty::kDrive));
    job->setProperty(DeviceProperty::kMedia, info.value(DeviceProperty::kMedia));
    connect(job, &AbstractAuditLogJob::finished, job, &QObject::deleteLater);
    job->start();
}

void BurnJobManager::startPutFilesToDisc(const QString &dev, const QList<QUrl> &urls)
{
    AbstractPacketWritingJob *job { new PutPacketWritingJob(dev) };
    job->setProperty("pendingUrls", QVariant::fromValue(urls));

    fmDebug() << "Add new put packet writing job: " << job;
    PacketWritingScheduler::instance().addJob(job);
}

void BurnJobManager::startRemoveFilesFromDisc(const QString &dev, const QList<QUrl> &urls)
{
    AbstractPacketWritingJob *job { new RemovePacketWritingJob(dev) };
    job->setProperty("pendingUrls", QVariant::fromValue(urls));

    fmDebug() << "Add new remove packet writing job: " << job;
    PacketWritingScheduler::instance().addJob(job);
}

void BurnJobManager::startRenameFileFromDisc(const QString &dev, const QUrl &src, const QUrl &dest)
{
    AbstractPacketWritingJob *job { new RenamePacketWritingJob(dev) };
    job->setProperty("srcUrl", QVariant::fromValue(src));
    job->setProperty("destUrl", QVariant::fromValue(dest));

    fmDebug() << "Add new rename packet writing job: " << job;
    PacketWritingScheduler::instance().addJob(job);
}

void BurnJobManager::initBurnJobConnect(AbstractBurnJob *job)
{
    Q_ASSERT(job);
    connect(job, &AbstractBurnJob::finished, job, &QObject::deleteLater);
    connect(job, &AbstractBurnJob::requestCompletionDialog, this, &BurnJobManager::showOpticalJobCompletionDialog);
    connect(job, &AbstractBurnJob::requestFailureDialog, this, &BurnJobManager::showOpticalJobFailureDialog);
    connect(job, &AbstractBurnJob::requestErrorMessageDialog, DialogManagerInstance, &DialogManager::showErrorDialog);
    connect(job, &AbstractBurnJob::requestCloseTab, this, [](const QUrl &url) {
        dpfSlotChannel->push("dfmplugin_titlebar", "slot_Tab_Close", url);
    });
    connect(job, &AbstractBurnJob::requestReloadDisc, this, [](const QString &devId) {
        DevMngIns->mountBlockDevAsync(devId, {}, [devId](bool, const DFMMOUNT::OperationErrorInfo &, const QString &) {
            DevProxyMng->reloadOpticalInfo(devId);
            // WORKAROUND: 由于内核原因，reload总是失败，暂擦除后弹出光盘
            DeviceManager::instance()->ejectBlockDevAsync(devId);
        });
    });
    connect(job, &AbstractBurnJob::burnFinished, this, [this, job](int type, bool result) {
        startAuditLogForBurnFiles(job->currentDeviceInfo(),
                                  (type == AbstractBurnJob::JobType::kOpticalImageBurn) ? job->property(AbstractBurnJob::PropertyType::kImageUrl).toUrl()
                                                                                        : job->property(AbstractBurnJob::PropertyType::KStagingUrl).toUrl(),
                                  result);
    });
}

void BurnJobManager::initDumpJobConnect(DumpISOImageJob *job)
{
    Q_ASSERT(job);
    connect(job, &AbstractBurnJob::finished, job, &QObject::deleteLater);
    connect(job, &DumpISOImageJob::requestOpticalDumpISOSuccessDialog, this, &BurnJobManager::showOpticalDumpISOSuccessDialog);
    connect(job, &DumpISOImageJob::requestOpticalDumpISOFailedDialog, this, &BurnJobManager::showOpticalDumpISOFailedDialog);
}

bool BurnJobManager::deleteStagingDir(const QUrl &url)
{
    // we cannot delete image file
    bool isDir { DFMIO::DFileInfo(url).attribute(DFMIO::DFileInfo::AttributeID::kStandardIsDir).toBool() };
    if (!isDir) {
        fmInfo() << "Don't delelete img url: " << url;
        return false;
    }

    QString path { url.toLocalFile() };
    static QRegularExpression reg("_dev_sr[0-9]*");
    QRegularExpressionMatch match;
    if (!path.contains(reg, &match)) {
        fmWarning() << "Cannot delete dir (not staging dir)" << path;
        return false;
    }

    if (!LocalFileHandler().deleteFileRecursive(url)) {
        fmWarning() << "Delete " << url << "failed!";
        return false;
    }

    fmInfo() << "Delete cache folder: " << url << "success";
    return true;
}

void BurnJobManager::showOpticalJobCompletionDialog(const QString &msg, const QString &icon)
{
    DDialog d(qApp->activeWindow());
    d.setIcon(QIcon::fromTheme(icon));
    d.setTitle(msg);
    d.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.getButton(0)->setFocus();
    d.exec();
}

void BurnJobManager::showOpticalJobFailureDialog(int type, const QString &err, const QStringList &details)
{
    DDialog d(qApp->activeWindow());
    d.setIcon(QIcon::fromTheme("dialog-error"));
    QString failureType;
    switch (type) {
    case AbstractBurnJob::kOpticalBlank:
        failureType = tr("Disc erase failed");
        break;
    case AbstractBurnJob::kOpticalBurn:
    case AbstractBurnJob::kOpticalImageBurn:
        failureType = tr("Burn process failed");
        break;
    case AbstractBurnJob::kOpticalCheck:
        failureType = tr("Data verification failed");
        break;
    }
    QString failureStr = QString(tr("%1: %2")).arg(failureType).arg(err);
    d.setTitle(failureStr);
    QWidget *detailsw = new QWidget(&d);
    detailsw->setLayout(new QVBoxLayout());
    QTextEdit *te = new QTextEdit();
    te->setPlainText(details.join('\n'));
    te->setReadOnly(true);
    te->hide();
    detailsw->layout()->addWidget(te);
    connect(&d, &DDialog::buttonClicked, this, [failureStr, te, &d](int idx, const QString &) {
        if (idx == 1) {
            d.done(idx);
            return;
        }
        if (te->isVisible()) {
            te->hide();
            d.getButton(0)->setText(tr("Show details"));
            d.setTitle(failureStr);
        } else {
            te->show();
            d.getButton(0)->setText(tr("Hide details"));
            d.setTitle(tr("Error"));
        }
    });

    detailsw->setFixedWidth(360);
    d.layout()->setSizeConstraint(QLayout::SetFixedSize);   // make sure dialog can shrank after expanded for more info.

    d.addContent(detailsw);
    d.setOnButtonClickedClose(false);
    d.addButton(tr("Show details", "button"));
    d.addButton(tr("Confirm", "button"), true, DDialog::ButtonRecommend);
    d.setDefaultButton(1);
    d.getButton(1)->setFocus();
    d.exec();
}

void BurnJobManager::showOpticalDumpISOSuccessDialog(const QUrl &imageUrl)
{
    DDialog d(qApp->activeWindow());
    d.setFixedSize(400, 242);
    d.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    d.setIcon(QIcon::fromTheme("media-optical").pixmap(32, 32));
    d.addButton(QObject::tr("Close", "button"));
    d.addButton(tr("View Image File", "button"), true, DDialog::ButtonType::ButtonRecommend);
    connect(&d, &DDialog::buttonClicked, this, [imageUrl](int index, const QString &text) {
        fmInfo() << "button clicked" << text;
        if (index == 1) {
            const auto &fileInfo { InfoFactory::create<FileInfo>(imageUrl) };
            QUrl parentUrl { fileInfo->urlOf(UrlInfoType::kParentUrl) };
            parentUrl.setQuery("selectUrl=" + imageUrl.toString());
            dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, parentUrl);
        }
    });

    QFrame *contentFrame { new QFrame };
    QVBoxLayout *mainLayout { new QVBoxLayout };
    mainLayout->setContentsMargins(0, 0, 0, 0);
    contentFrame->setLayout(mainLayout);
    d.addContent(contentFrame);

    // add textlabel
    QLabel *textLabel { new QLabel };
    textLabel->setText(tr("Image successfully created"));
    textLabel->setAlignment(Qt::AlignHCenter);
    QFont font { textLabel->font() };
    font.setPixelSize(14);
    font.setWeight(QFont::Medium);
    font.setFamily("SourceHanSansSC");
    textLabel->setFont(font);
    mainLayout->addWidget(textLabel, 0, Qt::AlignTop | Qt::AlignCenter);

    // add icon label
    QLabel *iconLabel { new QLabel };
    iconLabel->setPixmap(QIcon::fromTheme("dialog-ok").pixmap(96, 96));
    mainLayout->addWidget(iconLabel, 0, Qt::AlignTop | Qt::AlignCenter);

    d.moveToCenter();
    d.exec();
}

void BurnJobManager::showOpticalDumpISOFailedDialog()
{
    DDialog d(qApp->activeWindow());
    d.setFixedSize(400, 242);
    d.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    d.setIcon(QIcon::fromTheme("media-optical").pixmap(32, 32));
    d.addButton(QObject::tr("Close", "button"));

    QFrame *contentFrame { new QFrame };
    QVBoxLayout *mainLayout { new QVBoxLayout };
    mainLayout->setContentsMargins(0, 0, 0, 0);
    contentFrame->setLayout(mainLayout);
    d.addContent(contentFrame);

    // add textlabel
    QLabel *textLabel { new QLabel };
    textLabel->setText(tr("Image creation failed"));
    textLabel->setAlignment(Qt::AlignHCenter);
    QFont font { textLabel->font() };
    font.setPixelSize(14);
    font.setWeight(QFont::Medium);
    font.setFamily("SourceHanSansSC");
    textLabel->setFont(font);
    mainLayout->addWidget(textLabel, 0, Qt::AlignTop | Qt::AlignCenter);

    // add icon label
    QLabel *iconLabel { new QLabel };
    iconLabel->setPixmap(QIcon::fromTheme("dialog-error").pixmap(96, 96));
    mainLayout->addWidget(iconLabel, 0, Qt::AlignTop | Qt::AlignCenter);

    d.moveToCenter();
    d.exec();
}

BurnJobManager::BurnJobManager(QObject *parent)
    : QObject(parent)
{
}
