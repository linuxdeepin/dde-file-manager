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

#include "dstatusbar.h"
#include "windowmanager.h"

#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "app/define.h"

#include "shutil/fileutils.h"

#include "dfileservices.h"
#include "dfilestatisticsjob.h"

#include "singleton.h"
#include <QComboBox>
#include <dtextbutton.h>
#include <dlineedit.h>
#include <danchors.h>
#include <DThemeManager>

#include <QFutureWatcher>
#include <QFuture>
#include <QtConcurrent>

DWIDGET_USE_NAMESPACE

#define STATUSBAR_WIDGET_DEFAULT_HEIGHT 24

DStatusBar::DStatusBar(QWidget *parent)
    : QFrame(parent)
{
    D_THEME_INIT_WIDGET(DStatusBar, mode)

    setObjectName("DStatusBar");

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

    QStringList seq;

    for (int i(1); i != 91; ++i)
        seq.append(QString(":/images/images/Spinner/Spinner%1.png").arg(i, 2, 10, QChar('0')));

    m_loadingIndicator = new DPictureSequenceView(this);
    m_loadingIndicator->setFixedSize(18, 18);
    m_loadingIndicator->setPictureSequence(seq, true);
    m_loadingIndicator->setSpeed(20);
    m_loadingIndicator->hide();

    m_scaleSlider = new QSlider(this);
    m_scaleSlider->setOrientation(Qt::Horizontal);
    m_scaleSlider->adjustSize();
    m_scaleSlider->setFixedWidth(120);

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

        m_label = new QLabel(m_counted.arg("0"), this);
        m_label->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Expanding);

        clearLayoutAndAnchors();
        m_layout->addStretch();
        m_layout->addWidget(m_loadingIndicator);
        m_layout->addWidget(m_label);
        m_layout->addStretch();
        m_layout->setSpacing(14);
        m_layout->setContentsMargins(0, 0, 4, 0);

        DAnchors<QSlider> sliderAnchor(m_scaleSlider);

        sliderAnchor.setAnchor(Qt::AnchorRight, this, Qt::AnchorRight);
        sliderAnchor.setAnchor(Qt::AnchorVerticalCenter, this, Qt::AnchorVerticalCenter);
        sliderAnchor.setRightMargin(20);

//        setStyleSheet("QFrame{"
//                      "background-color: white;"
//                      "color: #797979;}");

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
    m_lineEdit->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Fixed);
    m_lineEdit->setFixedHeight(STATUSBAR_WIDGET_DEFAULT_HEIGHT);
    m_lineEdit->setVisible(mode == DialogSave);
    m_lineEdit->installEventFilter(this);
    m_lineEditLabel = new QLabel(this);
    m_lineEditLabel->setObjectName("lineEditLabel");
    m_lineEditLabel->setText(tr("Save as:"));
    m_lineEditLabel->hide();

    if (m_label) {
        m_label->hide();
        m_label->deleteLater();
        m_label = Q_NULLPTR;
    }
    if (!m_acceptButton) {
        m_acceptButton = new QPushButton(QString(), this);
        m_acceptButton->setFixedHeight(STATUSBAR_WIDGET_DEFAULT_HEIGHT);
        m_acceptButton->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
    }
    if (!m_rejectButton) {
        m_rejectButton = new QPushButton(QString(), this);
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

    size.setHeight(qMax(22, size.height()));

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

void DStatusBar::itemSelected(const DFMEvent &event, int number)
{
    if (!m_label || event.windowId() != WindowManager::getWindowId(this))
        return;

    /* remark 190412:
     * This function is triggered by multiple different events when
     * using keyboard navigation, causing DFileStatisticsJob to be
     * started more than once.
     * A fix better than the current one should eventually be applied.
     */

    if (!m_fileStatisticsJob) {
        m_fileStatisticsJob = new DFileStatisticsJob(this);
        m_fileStatisticsJob->setFileHints(DFileStatisticsJob::ExcludeSourceFile | DFileStatisticsJob::SingleDepth);

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
    } else if (m_fileStatisticsJob->isRunning()) {
        m_fileStatisticsJob->stop();
        m_fileStatisticsJob->wait();
    }

    m_fileCount = 0;
    m_fileSize = 0;
    m_folderCount = 0;
    m_folderContains = 0;

    if (number > 1) {
        DUrl fileUrl;
        if (event.fileUrlList().count() > 0){
            fileUrl = event.fileUrlList().first();
        }else{
            fileUrl = event.fileUrl();
        }

        bool isInGVFs = FileUtils::isGvfsMountFile(fileUrl.toLocalFile());

        foreach (DUrl url, event.fileUrlList()) {
            const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, url);
            if (fileInfo->isFile()) {
                if (!isInGVFs){
                    m_fileSize += fileInfo->size();
                }
                m_fileCount += 1;
            } else {
                m_folderCount += 1;
                if (!isInGVFs){
//                    m_folderContains += fileInfo->filesCount();
                }
            }
        }

        if (isInGVFs){
             QFutureWatcher<qint64>* fileWatcher = new QFutureWatcher<qint64>();
             connect(fileWatcher, SIGNAL(finished()), this, SLOT(handdleComputerFileSizeFinished()));
             // Start the computation.
             QFuture<qint64> fileFuture = QtConcurrent::run(this, &DStatusBar::computerSize, event.fileUrlList());
             fileWatcher->setFuture(fileFuture);

             QFutureWatcher<int>* folderWatcher = new QFutureWatcher<int>();
             connect(folderWatcher, SIGNAL(finished()), this, SLOT(handdleComputerFolderContainsFinished()));
             // Start the computation.
             QFuture<int> folderFuture = QtConcurrent::run(this, &DStatusBar::computerFolderContains, event.fileUrlList());
             folderWatcher->setFuture(folderFuture);
        } else {
            m_fileStatisticsJob->start(event.fileUrlList());
        }

        updateStatusMessage();
    } else {
        if (number == 1) {
            if (event.fileUrlList().count() == 1) {
                DUrl url = event.fileUrlList().first();
                const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(this, url);

                //check network folder at first
                QStringList networkSchemeList = {SMB_SCHEME, FTP_SCHEME, SFTP_SCHEME, MTP_SCHEME, DAV_SCHEME};
                if (networkSchemeList.contains(event.fileUrlList().first().scheme())){
                    m_label->setText(m_selectedNetworkOnlyOneFolder.arg(QString::number(number)));
                } else{
                    if (!fileInfo)
                        return;
                    if (fileInfo->isFile()) {
                        m_fileCount = 1;
                        m_label->setText(m_selectOnlyOneFile.arg(QString::number(number), FileUtils::formatSize(fileInfo->size())));
                    }else if (fileInfo->isDir()) {
                        m_folderCount = 1;
//                        if (fileInfo->filesCount() <= 1) {
//                            m_label->setText(m_selectOnlyOneFolder.arg(QString::number(number),
//                                                                       m_OnlyOneItemCounted.arg(QString::number(fileInfo->filesCount()))));
//                        } else {
//                            m_label->setText(m_selectOnlyOneFolder.arg(QString::number(number),
//                                                                       m_counted.arg(QString::number(fileInfo->filesCount()))));
//                        }
                        m_label->setText(m_selectOnlyOneFolder.arg(number).arg(m_OnlyOneItemCounted.arg(0)));
                        m_fileStatisticsJob->start(event.fileUrlList());
                    }
                }
            } else{
                m_label->setText(m_OnlyOneItemSelected.arg(QString::number(1)));
            }
        }
    }
}

void DStatusBar::updateStatusMessage()
{
    QString selectedFolders;

    if (m_folderCount == 1 && m_folderContains <= 1) {
        selectedFolders = m_selectOnlyOneFolder.arg(QString::number(m_folderCount), m_OnlyOneItemCounted.arg(m_folderContains));
    } else if (m_folderCount == 1 && m_folderContains > 1) {
        selectedFolders = m_selectOnlyOneFolder.arg(QString::number(m_folderCount), m_counted.arg(m_folderContains));
    } else if (m_folderCount > 1 && m_folderContains <= 1) {
        selectedFolders = m_selectFolders.arg(QString::number(m_folderCount), m_OnlyOneItemCounted.arg(m_folderContains));
    } else if (m_folderCount > 1 && m_folderContains > 1) {
        selectedFolders = m_selectFolders.arg(QString::number(m_folderCount), m_counted.arg(m_folderContains));
    } else {
        selectedFolders = "";
    }

    QString selectedFiles;

    if (m_fileCount == 1) {
        selectedFiles = m_selectOnlyOneFile.arg(QString::number(m_fileCount), FileUtils::formatSize(m_fileSize));
    } else if (m_fileCount > 1 ) {
        selectedFiles = m_selectFiles.arg(QString::number(m_fileCount), FileUtils::formatSize(m_fileSize));
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
    QFutureWatcher<int>* watcher = static_cast<QFutureWatcher<int>*>(sender());
    int result = watcher->future().result();
    m_folderContains = result;
    updateStatusMessage();
}

void DStatusBar::handdleComputerFileSizeFinished()
{
    QFutureWatcher<qint64>* watcher = static_cast<QFutureWatcher<qint64>*>(sender());
    qint64 result = watcher->future().result();
    m_fileSize = result;
    updateStatusMessage();
}

void DStatusBar::itemCounted(const DFMEvent &event, int number)
{
    if (m_fileStatisticsJob) {
        if (m_fileStatisticsJob->isRunning()) {
            m_fileStatisticsJob->stop();
            m_fileStatisticsJob->wait();
            m_fileStatisticsJob->disconnect();
        }

        delete m_fileStatisticsJob;
        m_fileStatisticsJob = nullptr;
    }

    if (!m_label || event.windowId() != WindowManager::getWindowId(this))
        return;

    if (number > 1) {
        m_label->setText(m_counted.arg(QString::number(number)));
    } else {
        m_label->setText(m_OnlyOneItemCounted.arg(QString::number(number)));
    }
}

void DStatusBar::setLoadingIncatorVisible(bool visible, const QString &tipText)
{
    m_loadingIndicator->setVisible(visible);

    if (visible)
        m_loadingIndicator->play();

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
