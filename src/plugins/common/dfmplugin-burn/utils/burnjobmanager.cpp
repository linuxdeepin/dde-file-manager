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

#include "dfm-base/utils/dialogmanager.h"

#include <QVBoxLayout>
#include <QTextEdit>

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
    initConnect(job);
    job->start();
}

void BurnJobManager::startBurnISOFiles(const QString &dev, const QUrl &stagingUrl, const BurnJobManager::Config &conf)
{
    JobHandlePointer jobHandler { new AbstractJobHandler };
    DialogManagerInstance->addTask(jobHandler);

    AbstractBurnJob *job = new BurnISOFilesJob(dev, jobHandler);
    initConnect(job);
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
    initConnect(job);
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
    initConnect(job);
    job->setProperty(AbstractBurnJob::PropertyType::KStagingUrl, stagingUrl);
    job->setProperty(AbstractBurnJob::PropertyType::kSpeeds, conf.speeds);
    job->setProperty(AbstractBurnJob::PropertyType::kVolumeName, conf.volName);
    job->setProperty(AbstractBurnJob::PropertyType::kBurnOpts, QVariant::fromValue(conf.opts));
    job->start();
}

void BurnJobManager::initConnect(AbstractBurnJob *job)
{
    Q_ASSERT(job);
    connect(job, &AbstractBurnJob::finished, job, &QObject::deleteLater);
    connect(job, &AbstractBurnJob::requestCompletionDialog, this, &BurnJobManager::showOpticalJobCompletionDialog);
    connect(job, &AbstractBurnJob::requestFailureDialog, this, &BurnJobManager::showOpticalJobFailureDialog);
    connect(job, &AbstractBurnJob::requestErrorMessageDialog, DialogManagerInstance, &DialogManager::showErrorDialog);
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
    QString failure_str = QString(tr("%1: %2")).arg(failureType).arg(err);
    d.setTitle(failure_str);
    QWidget *detailsw = new QWidget(&d);
    detailsw->setLayout(new QVBoxLayout());
    QTextEdit *te = new QTextEdit();
    te->setPlainText(details.join('\n'));
    te->setReadOnly(true);
    te->hide();
    detailsw->layout()->addWidget(te);
    connect(&d, &DDialog::buttonClicked, this, [failure_str, te, &d](int idx, const QString &) {
        if (idx == 1) {
            d.done(idx);
            return;
        }
        if (te->isVisible()) {
            te->hide();
            d.getButton(0)->setText(tr("Show details"));
            d.setTitle(failure_str);
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

BurnJobManager::BurnJobManager(QObject *parent)
    : QObject(parent)
{
}
