#include "dstatusbar.h"
#include "windowmanager.h"

#include "dfmevent.h"
#include "app/filesignalmanager.h"
#include "app/define.h"

#include "shutil/fileutils.h"

#include "dfileservices.h"

#include "widgets/singleton.h"

#include <dtextbutton.h>
#include <dcombobox.h>
#include <dlineedit.h>
#include <anchors.h>

DStatusBar::DStatusBar(QWidget *parent)
    : QFrame(parent)
{
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
    m_selectOnlyOneFolder = tr("%1 folder selected(contains %2)");
    m_selectFolders = tr("%1 folders selected(contains %2)");
    m_selectOnlyOneFile = tr("%1 file selected(%2)");
    m_selectFiles = tr("%1 files selected(%2)");
    m_layout = new QHBoxLayout(this);

    QStringList seq;

    for (int i(1); i != 91; ++i)
        seq.append(QString(":/images/images/Spinner/Spinner%1.png").arg(i, 2, 10, QChar('0')));

    m_loadingIndicator = new DPictureSequenceView(this);
    m_loadingIndicator->setFixedSize(18, 18);
    m_loadingIndicator->setPictureSequence(seq, DPictureSequenceView::AutoScaleMode);
    m_loadingIndicator->setSpeed(20);
    m_loadingIndicator->hide();

    m_scaleSlider = new DSlider(this);
    m_scaleSlider->setOrientation(Qt::Horizontal);
    m_scaleSlider->setFixedSize(120,20);

    setFocusPolicy(Qt::NoFocus);
    setLayout(m_layout);
}

void DStatusBar::initConnect()
{
//    connect(fileSignalManager, &FileSignalManager::statusBarItemsSelected, this, &DStatusBar::itemSelected);
//    connect(fileSignalManager, &FileSignalManager::statusBarItemsCounted, this, &DStatusBar::itemCounted);
//    connect(fileSignalManager, &FileSignalManager::loadingIndicatorShowed, this, &DStatusBar::setLoadingIncatorVisible);
}

void DStatusBar::setMode(DStatusBar::Mode mode)
{
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

        AnchorsBase::setAnchor(m_scaleSlider, Qt::AnchorRight, this, Qt::AnchorRight);

        setStyleSheet("QFrame{"
                      "background-color: white;"
                      "color: #797979;}");

        return;
    }

    if (m_comboBox || m_lineEdit) {
        m_lineEdit->setVisible(mode == DialogSave);
        return;
    }

    m_comboBox = new DComboBox(this);
    m_comboBox->setMaximumWidth(200);
    m_comboBox->hide();

    m_lineEdit = new DLineEdit(this);
    m_lineEdit->setMaximumWidth(200);
    m_lineEdit->setVisible(mode == DialogSave);

    if (m_label) {
        m_label->hide();
        m_label->deleteLater();
        m_label = Q_NULLPTR;
    }
    if (!m_acceptButton) {
        m_acceptButton = new DTextButton(QString(), this);
        m_acceptButton->setFixedHeight(28);
    }
    if (!m_rejectButton) {
        m_rejectButton = new DTextButton(QString(), this);
        m_rejectButton->setFixedHeight(28);
    }

    clearLayoutAndAnchors();
    m_scaleSlider->move(0, 0);
    m_layout->addWidget(m_scaleSlider);
    m_layout->addWidget(m_comboBox);
    m_layout->addWidget(m_lineEdit);
    m_layout->addStretch();
    m_layout->addWidget(m_loadingIndicator);
    m_layout->addWidget(m_rejectButton);
    m_layout->addWidget(m_acceptButton);
    m_layout->setSpacing(10);
    m_layout->setContentsMargins(10, 10, 10, 10);

    setStyleSheet("QFrame{"
                  "background-color: white;"
                  "color: #797979;"
                  "border-top: 1px solid rgba(0, 0, 0, 0.1);}");
}

void DStatusBar::setComBoxItems(const QStringList &filters)
{
    if (!m_comboBox)
        return;

    m_comboBox->clear();
    m_comboBox->addItems(filters);
    m_comboBox->setVisible(!filters.isEmpty());
}

DSlider *DStatusBar::scalingSlider() const
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

void DStatusBar::itemSelected(const DFMEvent &event, int number)
{
    if (!m_label || event.windowId() != WindowManager::getWindowId(this))
        return;

    if (number > 1) {
        int fileCount = 0;
        qint64 fileSize = 0;
        int folderCount = 0;
        int folderContains = 0;

        foreach (DUrl url, event.fileUrlList()) {
            const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);

            if (fileInfo->isSymLink()) {
                QFileInfo targetInfo(fileInfo->symLinkTarget());

                if (targetInfo.exists() && targetInfo.isDir()) {
                    folderCount += 1;
                    folderContains += fileInfo->filesCount();
                } else if (targetInfo.exists() && targetInfo.isFile()) {
                    fileSize += fileInfo->size();
                    fileCount += 1;
                } else {
                    fileSize += fileInfo->size();
                    fileCount += 1;
                }
            } else if (fileInfo->isFile()) {
                fileSize += fileInfo->size();
                fileCount += 1;
            } else {
                folderCount += 1;
                folderContains += fileInfo->filesCount();
            }
        }

        QString selectedFolders;

        if (folderCount == 1 && folderContains <= 1) {
            selectedFolders = m_selectOnlyOneFolder.arg(QString::number(folderCount), m_OnlyOneItemCounted.arg(folderContains));
        } else if (folderCount == 1 && folderContains > 1) {
            selectedFolders = m_selectOnlyOneFolder.arg(QString::number(folderCount), m_counted.arg(folderContains));
        } else if (folderCount > 1 && folderContains <= 1) {
            selectedFolders = m_selectFolders.arg(QString::number(folderCount), m_OnlyOneItemCounted.arg(folderContains));
        } else if (folderCount > 1 && folderContains > 1) {
            selectedFolders = m_selectFolders.arg(QString::number(folderCount), m_counted.arg(folderContains));
        } else {
            selectedFolders = "";
        }

        QString selectedFiles;

        if (fileCount == 1) {
            selectedFiles = m_selectOnlyOneFile.arg(QString::number(fileCount), FileUtils::formatSize(fileSize));
        } else if (fileCount > 1 ) {
            selectedFiles = m_selectFiles.arg(QString::number(fileCount), FileUtils::formatSize(fileSize));
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
    } else {
        if (number == 1) {
            if (event.fileUrlList().count() == 1) {
                DUrl url = event.fileUrlList().first();
                const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);

                if (fileInfo->isFile()) {
                    m_label->setText(m_selectOnlyOneFile.arg(QString::number(number), FileUtils::formatSize(fileInfo->size())));
                } else if (fileInfo->isDir()) {
                    if (fileInfo->filesCount() <= 1) {
                        m_label->setText(m_selectOnlyOneFolder.arg(QString::number(number),
                                                                   m_OnlyOneItemCounted.arg(QString::number(fileInfo->filesCount()))));
                    } else {
                        m_label->setText(m_selectOnlyOneFolder.arg(QString::number(number),
                                                                   m_counted.arg(QString::number(fileInfo->filesCount()))));
                    }
                }
            }
        }
    }
}

void DStatusBar::itemCounted(const DFMEvent &event, int number)
{
    if (!m_label || event.windowId() != WindowManager::getWindowId(this))
        return;

    if (number > 1) {
        m_label->setText(m_counted.arg(QString::number(number)));
    } else {
        m_label->setText(m_OnlyOneItemCounted.arg(QString::number(number)));
    }
}

void DStatusBar::setLoadingIncatorVisible(const DFMEvent &event, bool visible)
{
    if (event.windowId() != WindowManager::getWindowId(this))
        return;

    m_loadingIndicator->setVisible(visible);

    if (visible)
        m_loadingIndicator->play();

    if (!m_label)
        return;

    if (visible) {
        const DAbstractFileInfoPointer &fileInfo = fileService->createFileInfo(event.fileUrl());

        if (fileInfo)
            m_label->setText(fileInfo->loadingTip());
        else
            m_label->setText(tr("Loading..."));
    } else {
        m_label->setText(QString());
    }
}

void DStatusBar::clearLayoutAndAnchors()
{
    while (m_layout->count() > 0)
        delete m_layout->takeAt(0);

    AnchorsBase::clearAnchors(this);
    AnchorsBase::clearAnchors(m_scaleSlider);
}
