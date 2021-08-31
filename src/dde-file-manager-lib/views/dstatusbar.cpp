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

//fix: 动态获取刻录选中文件的字节大小
#include "dfmopticalmediawidget.h"

#include "dstatusbar.h"
#include "windowmanager.h"
#include "dfmapplication.h"

#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "app/define.h"
#include "controllers/vaultcontroller.h"
#include "shutil/fileutils.h"

#include "dfileservices.h"
#include "dfilestatisticsjob.h"
#include <sys/stat.h>
#include "accessibility/ac-lib-file-manager.h"

#include "singleton.h"
#include <QComboBox>
#include <QPushButton>
#include <DLineEdit>
#include <DAnchors>

#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent>

DWIDGET_USE_NAMESPACE

#define STATUSBAR_WIDGET_DEFAULT_HEIGHT 24

DStatusBar::DStatusBar(QWidget *parent)
    : QFrame(parent)
{
    setObjectName("DStatusBar");
    AC_SET_ACCESSIBLE_NAME(this, AC_COMPUTER_STATUS_BAR);

    initUI();
    initConnect();
    setMode(Normal);
}

void DStatusBar::initUI()
{
    m_OnlyOneItemCounted = tr("%1 item");
    m_counted = tr("%1 items");
    m_OnlyOneItemSelected = tr("%1 item selected");
    m_selected = tr("%1 items selected");
    m_selectOnlyOneFolder = tr("%1 folder selected (contains %2)");
    m_selectFolders = tr("%1 folders selected (contains %2)");
    m_selectOnlyOneFile = tr("%1 file selected (%2)");
    m_selectFiles = tr("%1 files selected (%2)");
    m_selectedNetworkOnlyOneFolder = tr("%1 folder selected");
    m_layout = new QHBoxLayout(this);

    m_loadingIndicator = new DPictureSequenceView(this);
    m_loadingIndicator->setFixedSize(18, 18);
    AC_SET_OBJECT_NAME(m_loadingIndicator, AC_COMPUTER_STATUS_BAR_LOADING_INDICATOR);
    AC_SET_ACCESSIBLE_NAME(m_loadingIndicator, AC_COMPUTER_STATUS_BAR_LOADING_INDICATOR);
#if 0 //性能优化，放到 setLoadingIncatorVisible函数处理
    {
        QStringList seq;
        for (int i(1); i != 91; ++i)
            seq.append(QString(":/images/images/Spinner/Spinner%1.png").arg(i, 2, 10, QChar('0')));
        m_loadingIndicator->setPictureSequence(seq, true);
    }
#endif
    m_loadingIndicator->setSpeed(20);
    m_loadingIndicator->hide();

    m_scaleSlider = new QSlider(this);
    AC_SET_OBJECT_NAME(m_scaleSlider, AC_COMPUTER_STATUS_BAR_SCALE_SLIDER);
    AC_SET_ACCESSIBLE_NAME(m_scaleSlider, AC_COMPUTER_STATUS_BAR_SCALE_SLIDER);
    m_scaleSlider->setOrientation(Qt::Horizontal);
    m_scaleSlider->adjustSize();
    m_scaleSlider->setFixedWidth(120);

    setBackgroundRole(QPalette::Window);

    setFocusPolicy(Qt::NoFocus);
    setLayout(m_layout);
}

void DStatusBar::initConnect()
{
//    connect(fileSignalManager, &FileSignalManager::statusBarItemsSelected, this, &DStatusBar::itemSelected);
//    connect(fileSignalManager, &FileSignalManager::statusBarItemsCounted, this, &DStatusBar::itemCounted);
    //    connect(fileSignalManager, &FileSignalManager::loadingIndicatorShowed, this, &DStatusBar::setLoadingIncatorVisible);
}

DStatusBar::Mode DStatusBar::mode() const
{
    return m_mode;
}

void DStatusBar::setMode(DStatusBar::Mode mode)
{
    m_mode = mode;

    if (mode == Normal) {
        if (m_label)
            return;

        if (m_acceptButton) {
            m_acceptButton->hide();
            m_acceptButton->deleteLater();
            m_acceptButton = Q_NULLPTR;
        }
        if (m_rejectButton) {
            m_rejectButton->hide();
            m_rejectButton->deleteLater();
            m_rejectButton = Q_NULLPTR;
        }
        if (m_lineEdit) {
            m_lineEdit->hide();
            m_lineEdit->deleteLater();
            m_lineEdit = Q_NULLPTR;
        }
        if (m_comboBox) {
            m_comboBox->hide();
            m_comboBox->deleteLater();
            m_comboBox = Q_NULLPTR;
        }

        m_label = new DFMElidLabel(m_counted.arg("0"), this);
        m_label->setAlignment(Qt::AlignCenter);
        m_label->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);

        clearLayoutAndAnchors();
        m_layout->addWidget(m_loadingIndicator);
        m_layout->addWidget(m_label);
        m_layout->addWidget(m_scaleSlider, 0, Qt::AlignRight);
        m_layout->setSpacing(14);
        m_layout->setContentsMargins(0, 0, 4, 0);

        return;
    }

    if (m_comboBox || m_lineEdit) {
        m_lineEdit->setVisible(mode == DialogSave);
        m_lineEditLabel->setVisible(m_lineEdit->isVisible());
        return;
    }

    m_comboBox = new QComboBox(this);
    m_comboBox->setMaximumWidth(200);
    m_comboBox->setFixedHeight(STATUSBAR_WIDGET_DEFAULT_HEIGHT);
    m_comboBox->hide();
    m_comboBoxLabel = new QLabel(this);
    m_comboBoxLabel->setObjectName("comboBoxLabel");
    m_comboBoxLabel->setText(tr("Filter"));
    m_comboBoxLabel->hide();

    m_lineEdit = new QLineEdit(this);
    AC_SET_OBJECT_NAME(m_lineEdit, AC_COMPUTER_STATUS_BAR_LINE_EDIT);
    AC_SET_ACCESSIBLE_NAME(m_lineEdit, AC_COMPUTER_STATUS_BAR_LINE_EDIT);
    m_lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_lineEdit->setFixedHeight(STATUSBAR_WIDGET_DEFAULT_HEIGHT);
    m_lineEdit->setVisible(mode == DialogSave);
    m_lineEdit->installEventFilter(this);
    m_lineEditLabel = new QLabel(this);
    m_lineEditLabel->setObjectName("lineEditLabel");
    AC_SET_OBJECT_NAME(m_lineEditLabel, AC_COMPUTER_STATUS_BAR_LINE_EDIT);
    AC_SET_ACCESSIBLE_NAME(m_lineEditLabel, AC_COMPUTER_STATUS_BAR_LINE_EDIT);
    m_lineEditLabel->setText(tr("Save as:"));
    m_lineEditLabel->hide();

    if (m_label) {
        m_label->hide();
        m_label->deleteLater();
        m_label = Q_NULLPTR;
    }
    if (!m_acceptButton) {
        m_acceptButton = new QPushButton(QString(), this);
        AC_SET_OBJECT_NAME(m_acceptButton, AC_COMPUTER_STATUS_BAR_ACCEPT_BUTTON);
        AC_SET_ACCESSIBLE_NAME(m_acceptButton, AC_COMPUTER_STATUS_BAR_ACCEPT_BUTTON);
        m_acceptButton->setFixedHeight(STATUSBAR_WIDGET_DEFAULT_HEIGHT);
        m_acceptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    if (!m_rejectButton) {
        m_rejectButton = new QPushButton(QString(), this);
        AC_SET_OBJECT_NAME(m_rejectButton, AC_COMPUTER_STATUS_BAR_REJECT_BUTTON);
        AC_SET_ACCESSIBLE_NAME(m_rejectButton, AC_COMPUTER_STATUS_BAR_REJECT_BUTTON);
        m_rejectButton->setFixedHeight(STATUSBAR_WIDGET_DEFAULT_HEIGHT);
        m_rejectButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }

    clearLayoutAndAnchors();
    m_scaleSlider->move(0, 0);
    m_layout->addWidget(m_scaleSlider);
    m_layout->addWidget(m_comboBoxLabel);
    m_layout->addWidget(m_comboBox);
    m_layout->addWidget(m_lineEditLabel);
    m_layout->addWidget(m_lineEdit, 1);
    m_layout->addStretch();
    m_layout->addWidget(m_loadingIndicator, 0, Qt::AlignRight);
    m_layout->addWidget(m_rejectButton, 0, Qt::AlignRight);
    m_layout->addWidget(m_acceptButton, 0, Qt::AlignRight);
    m_layout->setSpacing(10);
    m_layout->setContentsMargins(10, 10, 10, 10);

    emit modeChanged();
}

void DStatusBar::setComBoxItems(const QStringList &filters)
{
    if (!m_comboBox)
        return;

    m_comboBox->clear();
    m_comboBox->addItems(filters);
    m_comboBox->setVisible(!filters.isEmpty());
    m_comboBoxLabel->setVisible(m_comboBox->isVisible());
}

QSlider *DStatusBar::scalingSlider() const
{
    return m_scaleSlider;
}

QPushButton *DStatusBar::acceptButton() const
{
    return m_acceptButton;
}

QPushButton *DStatusBar::rejectButton() const
{
    return m_rejectButton;
}

QLineEdit *DStatusBar::lineEdit() const
{
    return m_lineEdit;
}

QComboBox *DStatusBar::comboBox() const
{
    return m_comboBox;
}

QSize DStatusBar::sizeHint() const
{
    QSize size = QFrame::sizeHint();

    size.setHeight(qMax(25, size.height()));

    return size;
}

qint64 DStatusBar::computerSize(const DUrlList &urllist)
{
    qint64 fileSize = 0;
    foreach (DUrl url, urllist) {
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, url);
        if (fileInfo->isFile()) {
            fileSize += fileInfo->size();
        }
    }
    return fileSize;
}

int DStatusBar::computerFolderContains(const DUrlList &urllist)
{
    int folderContains = 0;
    foreach (DUrl url, urllist) {
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, url);
        if (fileInfo->isDir()) {
            folderContains += fileInfo->filesCount();
        }
    }
    return folderContains;
}

QVariantList DStatusBar::calcFolderAndFile(const DUrlList &urllist)
{
    int folderCount(0);
    int folderContains(0);
    int fileCount(0);
    qint64 fileSize(0);
    foreach (const DUrl &url, urllist) {
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, url);
        if (fileInfo->isDir()) {
            // 统计目录个数及目录内的文件个数
            folderCount++;
            folderContains += fileInfo->filesCount();
        } else {
            // 统计文件个数及文件的大小
            fileCount++;
            fileSize += fileInfo->size();
        }
    }
    return QVariantList{folderCount, folderContains, fileCount, fileSize};
}

void DStatusBar::initJobConnection()
{
    if (!m_fileStatisticsJob) {
        return;
    }

    auto onFoundFile = [this] {
        if (!sender())
            return;

        ++m_folderContains;
        updateStatusMessage();
    };

    connect(m_fileStatisticsJob, &DFileStatisticsJob::finished, this,
    [this] {
        m_folderContains = m_fileStatisticsJob->filesCount() + m_fileStatisticsJob->directorysCount();
        updateStatusMessage();
    }
           );
    connect(m_fileStatisticsJob, &DFileStatisticsJob::fileFound, this, onFoundFile);
    connect(m_fileStatisticsJob, &DFileStatisticsJob::directoryFound, this, onFoundFile);
}

void DStatusBar::showMtpStatus(const DFMEvent &event, int number)
{
    //  重置统计状态
    m_fileCount = 0;
    m_fileSize = 0;
    m_folderCount = 0;
    m_folderContains = 0;

    // number 大于 1 时, 显示不一样, 需要拼接文本
    if (number > 1) {
        DUrl fileUrl;
        if (event.fileUrlList().count() > 0) {
            fileUrl = event.fileUrlList().first();
        } else {
            fileUrl = event.fileUrl();
        }
        foreach (const DUrl &url, event.fileUrlList()) {
            struct stat statInfo;
            int fileStat = stat(url.path().toStdString().c_str(), &statInfo);
            if (0 != fileStat) {
                continue;
            }
            if (S_ISDIR(statInfo.st_mode)) {
                m_folderCount += 1; // 文件夹数量
            } else {
                m_fileCount += 1; // 文件数量
            }
        }
        QString selectItems;

        if (m_folderCount > 0 && m_fileCount > 0) // 同时有文件夹和文件时, 统一显示成项
            selectItems = m_OnlyOneItemSelected.arg(QString::number(m_folderCount+m_fileCount));
        else if (m_folderCount > 0)
            selectItems = m_selectedNetworkOnlyOneFolder.arg(QString::number(m_folderCount));
        else if (m_fileCount > 0)
            selectItems = m_OnlyOneItemSelected.arg(QString::number(m_fileCount));
        else
            selectItems = "";

        m_label->setText(QString("%1").arg(selectItems));
    } else {
        if (number == 1) { // 选中数量为1, 要么是文件夹, 要么是文件
            if (event.fileUrlList().count() == 1) {
                DUrl durl = event.fileUrlList().first();
                const DAbstractFileInfoPointer &info = fileService->createFileInfo(this, durl);
                //check network folder at first
                QStringList networkSchemeList = {SMB_SCHEME, FTP_SCHEME, SFTP_SCHEME, MTP_SCHEME, DAV_SCHEME};
                if (networkSchemeList.contains(event.fileUrlList().first().scheme())) {
                    m_label->setText(m_selectedNetworkOnlyOneFolder.arg(QString::number(number)));
                } else if (info) {
                    if (info->isDir()) {
                        m_folderCount = 1;
                        m_label->setText(m_selectedNetworkOnlyOneFolder.arg(QString::number(number)));
                    } else {
                        m_fileCount = 1;
                        m_label->setText(m_OnlyOneItemSelected.arg(QString::number(1)));
                    }
                }
            } else {
                m_label->setText(m_OnlyOneItemSelected.arg(QString::number(1)));
            }
        }
    }
}

void DStatusBar::itemSelected(const DFMEvent &event, int number)
{
    if (!m_label || event.windowId() != WindowManager::getWindowId(this))
        return;

    { // mtp挂载时 暂时不显示下方状态栏数据 能大幅提升访问性能
        // 首先判断配置
        const bool showInfo = DFMApplication::genericAttribute(DFMApplication::GA_MTPShowBottomInfo).toBool();
        if (!showInfo) { // mtp 不显示底部状态的设置下 再进行 mtp 判断
            static const QString &mtpType = "gvfs/mtp:host";
            const QString &path = event.fileUrlList().count() > 0 ? event.fileUrlList().first().toLocalFile()
                                                                  : event.fileUrl().toLocalFile();
            if (path.contains(mtpType)) {
                showMtpStatus(event, number);
                return;
            }
        }
    }

    /* remark 190412:
     * This function is triggered by multiple different events when
     * using keyboard navigation, causing DFileStatisticsJob to be
     * started more than once.
     * A fix better than the current one should eventually be applied.
     */
    if (!m_fileStatisticsJob) {
        m_fileStatisticsJob = new DFileStatisticsJob(this);
        m_fileStatisticsJob->setFileHints(DFileStatisticsJob::ExcludeSourceFile | DFileStatisticsJob::SingleDepth);
    } else if (m_fileStatisticsJob->isRunning()) {
        m_fileStatisticsJob->stop();
        m_fileStatisticsJob->wait();
    }
    if (m_isjobDisconnect) {
        m_isjobDisconnect = false;
        initJobConnection();
    }
    m_fileCount = 0;
    m_fileSize = 0;
    m_folderCount = 0;
    m_folderContains = 0;

    //fix: 动态获取刻录选中文件的字节大小
    DFMOpticalMediaWidget::g_selectBurnFilesSize = 0;
    DFMOpticalMediaWidget::g_selectBurnDirFileCount = 0;
    if (number > 1) {
        DUrl fileUrl;
        if (event.fileUrlList().count() > 0) {
            fileUrl = event.fileUrlList().first();
        } else {
            fileUrl = event.fileUrl();
        }
        bool isInGVFs = FileUtils::isGvfsMountFile(fileUrl.toLocalFile());
        DUrlList folderList;
        if (isInGVFs) {
            foreach (const DUrl &url, event.fileUrlList()) {
                struct stat statInfo;
                int fileStat = stat(url.path().toStdString().c_str(), &statInfo);
                if (0 != fileStat) {
                    continue;
                }
                if (S_ISDIR(statInfo.st_mode)) {
                    m_folderCount += 1;
                    folderList << url;
                } else {
                    m_fileCount += 1;
                }
            }
        } else {
            // 保险箱内，全选文件达到2000个时，文管会很卡顿，以下修改优化该问题
            if (VaultController::isVaultFile(fileUrl.toLocalFile())) {
                // 将文件个数及大小的计算过程放入异步线程中
                QFutureWatcher<QVariantList> *fileWatcher = new QFutureWatcher<QVariantList>();
                connect(fileWatcher, &QFutureWatcher<QVariantList>::finished, this, &DStatusBar::handleCalcFolderAndFileFinished);
                QFuture<QVariantList> fileFuture = QtConcurrent::run(this, &DStatusBar::calcFolderAndFile, event.fileUrlList());
                fileWatcher->setFuture(fileFuture);
            } else {
                foreach (const DUrl &url, event.fileUrlList()) {
                    const DAbstractFileInfoPointer &info = fileService->createFileInfo(this, url);
                    if (info->isDir()) {
                        m_folderCount += 1;
                        folderList << url;
                    } else {
                        if (!isInGVFs) {
                            m_fileSize += info->size();
                        }
                        m_fileCount += 1;
                    }
                }
            }
        }

        if (isInGVFs) {
            QFutureWatcher<qint64> *fileWatcher = new QFutureWatcher<qint64>();
            connect(fileWatcher, SIGNAL(finished()), this, SLOT(handdleComputerFileSizeFinished()));
            // Start the computation.
            QFuture<qint64> fileFuture = QtConcurrent::run(this, &DStatusBar::computerSize, event.fileUrlList());
            fileWatcher->setFuture(fileFuture);

            QFutureWatcher<int> *folderWatcher = new QFutureWatcher<int>();
            connect(folderWatcher, SIGNAL(finished()), this, SLOT(handdleComputerFolderContainsFinished()));
            // Start the computation.
            QFuture<int> folderFuture = QtConcurrent::run(this, &DStatusBar::computerFolderContains, event.fileUrlList());
            folderWatcher->setFuture(folderFuture);
        } else {
            m_fileStatisticsJob->start(folderList);
        }
        updateStatusMessage();
    } else {
        if (number == 1) {
            if (event.fileUrlList().count() == 1) {
                DUrl durl = event.fileUrlList().first();
                const DAbstractFileInfoPointer &info = fileService->createFileInfo(this, durl);

                //check network folder at first
                QStringList networkSchemeList = {SMB_SCHEME, FTP_SCHEME, SFTP_SCHEME, MTP_SCHEME, DAV_SCHEME};
                if (networkSchemeList.contains(event.fileUrlList().first().scheme())) {
                    m_label->setText(m_selectedNetworkOnlyOneFolder.arg(QString::number(number)));
                } else if (info) {
                    if (info->isDir()) {
                        m_folderCount = 1;
                        m_label->setText(m_selectOnlyOneFolder.arg(number).arg(m_counted.arg(0)));
                        m_fileStatisticsJob->start(event.fileUrlList());


                    } else { /*if (fileInfo->isFile())*/
                        m_fileCount = 1;
                        m_label->setText(m_selectOnlyOneFile.arg(QString::number(number), FileUtils::formatSize(info->size())));

                        //fix: 动态获取刻录选中文件的字节大小
                        DFMOpticalMediaWidget::g_selectBurnFilesSize = info->size();
                    }
                }
            } else {
                m_label->setText(m_OnlyOneItemSelected.arg(QString::number(1)));
            }
        }
    }
    //fix: 动态获取刻录选中文件夹的个数
    DFMOpticalMediaWidget::g_selectBurnDirFileCount = m_folderCount;
}

void DStatusBar::updateStatusMessage()
{
    QString selectedFolders;

    if (m_folderCount == 1 && m_folderContains == 1) {
        selectedFolders = m_selectOnlyOneFolder.arg(QString::number(m_folderCount), m_OnlyOneItemCounted.arg(m_folderContains));
    } else if (m_folderCount == 1 && m_folderContains != 1) {
        selectedFolders = m_selectOnlyOneFolder.arg(QString::number(m_folderCount), m_counted.arg(m_folderContains));
    } else if (m_folderCount > 1 && m_folderContains == 1) {
        selectedFolders = m_selectFolders.arg(QString::number(m_folderCount), m_OnlyOneItemCounted.arg(m_folderContains));
    } else if (m_folderCount > 1 && m_folderContains != 1) {
        selectedFolders = m_selectFolders.arg(QString::number(m_folderCount), m_counted.arg(m_folderContains));
    } else {
        selectedFolders = "";
    }

    QString selectedFiles;

    if (m_fileCount == 1) {
        selectedFiles = m_selectOnlyOneFile.arg(QString::number(m_fileCount), FileUtils::formatSize(m_fileSize));
        //fix: 动态获取刻录选中文件的字节大小
        DFMOpticalMediaWidget::g_selectBurnFilesSize = m_fileSize;
    } else if (m_fileCount > 1) {
        selectedFiles = m_selectFiles.arg(QString::number(m_fileCount), FileUtils::formatSize(m_fileSize));
        //fix: 动态获取刻录选中文件的字节大小
        DFMOpticalMediaWidget::g_selectBurnFilesSize = m_fileSize;
    } else {
        selectedFiles = "";
    }

    if (selectedFolders.isEmpty()) {
        m_label->setText(QString("%1").arg(selectedFiles));
    } else if (selectedFiles.isEmpty()) {
        m_label->setText(QString("%1").arg(selectedFolders));
    } else {
        m_label->setText(QString("%1,%2").arg(selectedFolders, selectedFiles));
    }
}

void DStatusBar::handdleComputerFolderContainsFinished()
{
    QFutureWatcher<int> *watcher = static_cast<QFutureWatcher<int>*>(sender());
    int result = watcher->future().result();
    m_folderContains = result;
    updateStatusMessage();
    watcher->deleteLater();
}

void DStatusBar::handleCalcFolderAndFileFinished()
{
    QFutureWatcher<QVariantList> *watcher = dynamic_cast<QFutureWatcher<QVariantList> *>(sender());
    if (watcher) {
        QVariantList result = watcher->future().result();
        if (result.size() == 4) {
            // 设置目录和文件的个数及大小
            m_folderCount = result[0].toInt();
            m_folderContains = result[1].toInt();
            m_fileCount = result[2].toInt();
            m_fileSize = result[3].toLongLong();
            // 状态栏显示目录和文件的个数及大小
            updateStatusMessage();
        }
        watcher->deleteLater();
    }
}

void DStatusBar::handdleComputerFileSizeFinished()
{
    QFutureWatcher<qint64> *watcher = static_cast<QFutureWatcher<qint64>*>(sender());
    qint64 result = watcher->future().result();
    m_fileSize = result;
    updateStatusMessage();
    watcher->deleteLater();
}

void DStatusBar::itemCounted(const DFMEvent &event, int number)
{
    if (m_fileStatisticsJob) {
        if (m_fileStatisticsJob->isRunning()) {
            m_fileStatisticsJob->stop();
            m_fileStatisticsJob->wait();
            if (m_fileStatisticsJob->disconnect()) {
                m_isjobDisconnect = true;
            }
        }
    }

    if (!m_label || event.windowId() != WindowManager::getWindowId(this))
        return;

    if (number != 1) {
        m_label->setText(m_counted.arg(QString::number(number)));
    } else {
        m_label->setText(m_OnlyOneItemCounted.arg(QString::number(number)));
    }

    //fix: 动态获取刻录选中文件的字节大小
    DFMOpticalMediaWidget::g_selectBurnFilesSize = 0;
    DFMOpticalMediaWidget::g_selectBurnDirFileCount = 0;
}

void DStatusBar::setLoadingIncatorVisible(bool visible, const QString &tipText)
{
    m_loadingIndicator->setVisible(visible);

    if (visible) {
        if (!m_loadingIndicatorInited) {
            QStringList seq;
            for (int i(1); i != 91; ++i)
                seq.append(QString(":/images/images/Spinner/Spinner%1.png").arg(i, 2, 10, QChar('0')));
            m_loadingIndicator->setPictureSequence(seq, true);
            m_loadingIndicatorInited = true;
        }

        m_loadingIndicator->play();
    }

    if (!m_label)
        return;

    if (visible) {
        m_label->setText(tipText.isEmpty() ? tr("Loading...") : tipText);
    } else {
        m_label->setText(QString());
    }
}

bool DStatusBar::eventFilter(QObject *watched, QEvent *event)
{
    if (watched != m_lineEdit)
        return false;

    if (event->type() == QEvent::FocusIn) {
        TIMER_SINGLESHOT_OBJECT(this, 10, {
            QMimeDatabase db;
            const QString &name = m_lineEdit->text();
            const QString &suffix = db.suffixForFileName(name);

            if (suffix.isEmpty())
                m_lineEdit->selectAll();
            else
                m_lineEdit->setSelection(0, name.length() - suffix.length() - 1);
        }, this)
    } else if (event->type() == QEvent::Show) {
        TIMER_SINGLESHOT_OBJECT(this, 10, m_lineEdit->setFocus(), this);
    }

    return false;
}

void DStatusBar::clearLayoutAndAnchors()
{
    while (m_layout->count() > 0)
        delete m_layout->takeAt(0);

    DAnchorsBase::clearAnchors(this);
    DAnchorsBase::clearAnchors(m_scaleSlider);
}

void DFMElidLabel::setText(const QString &text)
{
    m_text = text;
    setElidText(text);
}

void DFMElidLabel::setElidText(const QString &text)
{
    QFont font = this->font();
    QFontMetrics fm(font);
    QString str = fm.elidedText(text, Qt::ElideRight, width());
    setToolTip(m_text);
    QLabel::setText(str);
}

void DFMElidLabel::resizeEvent(QResizeEvent *event)
{
    QLabel::resizeEvent(event);
    setElidText(m_text);
}
