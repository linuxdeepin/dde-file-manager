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

#include "mainwindow.h"
#include <QPainter>
#include <QImage>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QLabel>
#include <QPixmap>
#include <QComboBox>
#include <QPushButton>
#include <QGridLayout>
#include <QImage>
#include <QProgressBar>
#include <QLineEdit>
#include <QCheckBox>
#include <QTextBrowser>
#include <QFile>
#include <QDebug>
#include <QMouseEvent>
#include <QApplication>
#include <QtConcurrent>
#include <DWindowManagerHelper>
#include "dialogs/messagedialog.h"
#include "utils/udisksutils.h"

#include <dblockdevice.h>
#include <ddiskmanager.h>
#include <dudisksjob.h>

MainWindow::MainWindow(const QString &path, QWidget *parent):
    DDialog(parent),
    m_diskm(new DDiskManager)
{
    DPlatformWindowHandle handle(this);
    Q_UNUSED(handle)

    setObjectName("UsbDeviceFormatter");
    m_diskm->setWatchChanges(true);
    m_formatPath = path;
    m_formatType = UDisksBlock(path).fsType();
    if(m_formatType == "vfat")
        m_formatType = "fat32";
    initUI();
//    initStyleSheet();
    initConnect();
}

MainWindow::~MainWindow()
{

}

void MainWindow::initUI()
{
    setWindowFlags(Qt::WindowCloseButtonHint |
                   Qt::Dialog);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->setContentsMargins(0, 2, 0, 0);

    QPixmap pixmap(24, 24);
    pixmap.fill(QColor(Qt::transparent));
    QIcon transparentIcon(pixmap);

    QString title = tr("Format");
    setTitle(title);

    m_pageStack = new QStackedWidget(this);
    m_pageStack->setFixedSize(width(), 340);
    m_mainPage = new MainPage(m_formatType, this);
    m_mainPage->setTargetPath(m_formatPath);
    m_warnPage = new WarnPage(this);
    m_formatingPage = new FormatingPage(this);
    m_finishPage = new FinishPage(this);
    m_errorPage = new ErrorPage("", this);

    m_pageStack->addWidget(m_mainPage);
    m_pageStack->addWidget(m_warnPage);
    m_pageStack->addWidget(m_formatingPage);
    m_pageStack->addWidget(m_finishPage);
    m_pageStack->addWidget(m_errorPage);

    m_comfirmButton = new QPushButton(tr("Format"), this);
    m_comfirmButton->setFixedSize(160, 36);
    m_comfirmButton->setObjectName("ComfirmButton");

    mainLayout->addWidget(m_pageStack);
    mainLayout->addSpacing(10);
    mainLayout->addStretch(1);
    mainLayout->addWidget(m_comfirmButton, 0, Qt::AlignHCenter);
    mainLayout->addSpacing(34);
    QWidget *centralWidget = new QWidget();
    centralWidget->setLayout(mainLayout);
    addContent(centralWidget);
}

void MainWindow::initConnect()
{
    connect(m_comfirmButton, &QPushButton::clicked, this, &MainWindow::nextStep);
    connect(m_diskm.data(), &DDiskManager::jobAdded, [this](const QString &jobs) {
        QScopedPointer<DUDisksJob> job(DDiskManager::createJob(jobs));
        if (job->operation().contains("format") && job->objects().contains(UDisksBlock(m_formatPath)->path())) {
            m_job.reset(DDiskManager::createJob(jobs));
            connect(m_job.data(), &DUDisksJob::progressChanged, [this](double p){m_formatingPage->setProgress(p);});
            connect(m_job.data(), &DUDisksJob::completed, [this](bool r, QString){this->onFormatingFinished(r);});
        }
    });
}

void MainWindow::formatDevice()
{
    DWindowManagerHelper::setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, false);
    setCloseButtonVisible(false);

    QtConcurrent::run([=]{
        UDisksBlock blk(m_formatPath);
        blk->unmount({});
        if (blk->lastError().isValid()) {
            QMetaObject::invokeMethod(this, std::bind(&MainWindow::onFormatingFinished, this, false), Qt::ConnectionType::QueuedConnection);
            return;
        }
        QVariantMap opt = {{"label", m_mainPage->getLabel()}};
        if (m_mainPage->shouldErase()) opt["erase"] = "zero";
        blk->format(m_mainPage->getSelectedFs(), opt);
    });
}

bool MainWindow::checkBackup()
{
    return !UDisksBlock(m_formatPath).fsType().isEmpty();
}

void MainWindow::nextStep()
{
    switch (m_currentStep) {
    case Normal:
        m_pageStack->setCurrentWidget(m_warnPage);
        m_currentStep = Warn;
        m_comfirmButton->setText(tr("Continue"));
        break;
    case Warn:
        m_pageStack->setCurrentWidget(m_formatingPage);
        m_currentStep = Formating;
        m_comfirmButton->setText(tr("Formatting..."));
        m_comfirmButton->setEnabled(false);
        formatDevice();
        break;
    case Finished:
        qApp->quit();
        break;
    case FormattError:
        m_pageStack->setCurrentWidget(m_mainPage);
        m_currentStep = Normal;
        break;
    case RemovedWhenFormattingError:
        qApp->quit();
        break;
    default:
        break;
    }
}

void MainWindow::onFormatingFinished(const bool &successful)
{
    DWindowManagerHelper::setMotifFunctions(windowHandle(), DWindowManagerHelper::FUNC_CLOSE, true);
    setCloseButtonVisible(true);

    if (successful) {
        m_currentStep = Finished;
        m_comfirmButton->setText(tr("Done"));
        m_comfirmButton->setEnabled(true);
        m_pageStack->setCurrentWidget(m_finishPage);
    } else {
        if(!QFile::exists(m_formatPath)){
            m_currentStep = RemovedWhenFormattingError;
            m_comfirmButton->setText(tr("Quit"));
            m_errorPage->setErrorMsg(tr("Your disk is removed when formatting"));
        } else{
            m_currentStep = FormattError;
            m_errorPage->setErrorMsg(tr("Failed to format the device"));
            m_comfirmButton->setText(tr("Reformat"));
        }
        m_comfirmButton->setEnabled(true);
        m_pageStack->setCurrentWidget(m_errorPage);
    }
}
