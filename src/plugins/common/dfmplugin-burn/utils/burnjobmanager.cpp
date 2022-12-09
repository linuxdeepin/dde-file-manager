/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
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
#include "burnjobmanager.h"
#include "utils/burnjob.h"
#include "utils/auditlogjob.h"

#include "dfm-base/utils/dialogmanager.h"
#include "dfm-base/utils/decorator/decoratorfileoperator.h"
#include "dfm-base/utils/decorator/decoratorfileinfo.h"
#include "dfm-base/base/schemefactory.h"

#include <QVBoxLayout>
#include <QTextEdit>
#include <QLabel>

using namespace dfmplugin_burn;
DFMBASE_USE_NAMESPACE

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
    AbstractAuditLogJob *job = new BurnFilesAuditLogJob(info, stagingUrl, result);
    connect(job, &AbstractAuditLogJob::finished, this, [this, job, stagingUrl, result]() {
        if (result)
            this->deleteStagingDir(stagingUrl);
        job->deleteLater();
    });
    job->start();
}

void BurnJobManager::initBurnJobConnect(AbstractBurnJob *job)
{
    Q_ASSERT(job);
    connect(job, &AbstractBurnJob::finished, job, &QObject::deleteLater);
    connect(job, &AbstractBurnJob::requestCompletionDialog, this, &BurnJobManager::showOpticalJobCompletionDialog);
    connect(job, &AbstractBurnJob::requestFailureDialog, this, &BurnJobManager::showOpticalJobFailureDialog);
    connect(job, &AbstractBurnJob::requestErrorMessageDialog, DialogManagerInstance, &DialogManager::showErrorDialog);
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

void BurnJobManager::deleteStagingDir(const QUrl &url)
{
    // we cannot delete image file
    if (!DecoratorFileInfo(url).isDir()) {
        qInfo() << "Don't delelete img url: " << url;
        return;
    }

    QString path { url.toLocalFile() };
    static QRegularExpression reg("_dev_sr[0-9]*");
    QRegularExpressionMatch match;
    if (!path.contains(reg, &match)) {
        qWarning() << "Cannot delete dir (not staging dir)" << path;
        return;
    }

    if (!DecoratorFileOperator(url).deleteFile())
        qWarning() << "Delete " << url << "failed!";
    else
        qInfo() << "Delete cache folder: " << url << "success";
}

void BurnJobManager::showOpticalJobCompletionDialog(const QString &msg, const QString &icon)
{
    DDialog d;
    d.setIcon(QIcon::fromTheme(icon));
    d.setTitle(msg);
    d.addButton(tr("OK", "button"), true, DDialog::ButtonRecommend);
    d.setDefaultButton(0);
    d.getButton(0)->setFocus();
    d.exec();
}

void BurnJobManager::showOpticalJobFailureDialog(int type, const QString &err, const QStringList &details)
{
    DDialog d;
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
    DDialog d;
    d.setFixedSize(400, 242);
    d.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    d.setIcon(QIcon::fromTheme("media-optical").pixmap(32, 32));
    d.addButton(QObject::tr("Close", "button"));
    d.addButton(tr("View Image File", "button"), true, DDialog::ButtonType::ButtonRecommend);
    connect(&d, &DDialog::buttonClicked, this, [imageUrl](int index, const QString &text) {
        qInfo() << "button clicked" << text;
        if (index == 1) {
            const auto &fileInfo { InfoFactory::create<AbstractFileInfo>(imageUrl) };
            QUrl parentUrl { fileInfo->urlOf(UrlInfoType::kParentUrl) };
            parentUrl.setQuery("selectUrl=" + imageUrl.toString());
            dpfSignalDispatcher->publish(GlobalEventType::kOpenNewWindow, parentUrl);
        }
    });

    QFrame *contentFrame { new QFrame };
    QVBoxLayout *mainLayout { new QVBoxLayout };
    mainLayout->setMargin(0);
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
    DDialog d;
    d.setFixedSize(400, 242);
    d.setSizePolicy(QSizePolicy::Minimum, QSizePolicy::Expanding);
    d.setIcon(QIcon::fromTheme("media-optical").pixmap(32, 32));
    d.addButton(QObject::tr("Close", "button"));

    QFrame *contentFrame { new QFrame };
    QVBoxLayout *mainLayout { new QVBoxLayout };
    mainLayout->setMargin(0);
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
