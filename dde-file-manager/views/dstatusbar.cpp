#include "dstatusbar.h"
#include "windowmanager.h"

#include "app/fmevent.h"
#include "app/filesignalmanager.h"
#include "app/global.h"

#include "shutil/fileutils.h"

#include "controllers/fileservices.h"

#include "widgets/singleton.h"

DStatusBar::DStatusBar(QWidget *parent)
    : QFrame(parent)
{
    initUI();
    initConnect();
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

    m_label = new QLabel(m_counted.arg("0"), this);
    m_scaleSlider = new DSlider(this);
    m_scaleSlider->setOrientation(Qt::Horizontal);
    m_scaleSlider->setFixedWidth(120);
    m_scaleSlider->setPageStep(1);
    m_scaleSlider->setTickInterval(1);
    m_scaleSlider->setMinimum(0);
    m_scaleSlider->setMaximum(4);

    m_layout->addStretch(3);
    m_layout->addWidget(m_loadingIndicator);
    m_layout->addWidget(m_label);
    m_layout->addStretch(2);
    m_layout->addWidget(m_scaleSlider);
    m_layout->setSpacing(14);
    m_layout->addSpacing(4);
    m_layout->setContentsMargins(0, 0, 0, 0);
    setFixedHeight(24);
    setFocusPolicy(Qt::NoFocus);
    setStyleSheet("QFrame{\
                  background-color: white;\
                  color: #797979;\
              }");
    setLayout(m_layout);
}

void DStatusBar::initConnect()
{
    connect(fileSignalManager, &FileSignalManager::statusBarItemsSelected, this, &DStatusBar::itemSelected);
    connect(fileSignalManager, &FileSignalManager::statusBarItemsCounted, this, &DStatusBar::itemCounted);
    connect(fileSignalManager, &FileSignalManager::loadingIndicatorShowed, this, &DStatusBar::setLoadingIncatorVisible);
}

DSlider *DStatusBar::scalingSlider()
{
    return m_scaleSlider;
}

void DStatusBar::itemSelected(const FMEvent &event, int number)
{
    if(event.windowId() != WindowManager::getWindowId(window()))
        return;

    if(number > 1)
    {
        int fileCount = 0;
        qint64 fileSize = 0;
        int folderCount = 0;
        int folderContains = 0;
        foreach (DUrl url, event.fileUrlList()) {
            const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);

            if (fileInfo->isSymLink()){
                QFileInfo targetInfo(fileInfo->symLinkTarget());
                if (targetInfo.exists() && targetInfo.isDir()){
                    folderCount += 1;
                    folderContains += fileInfo->filesCount();
                }else if (targetInfo.exists() && targetInfo.isFile()){
                    fileSize += fileInfo->size();
                    fileCount += 1;
                }else{
                    fileSize += fileInfo->size();
                    fileCount += 1;
                }
            }
            else if (fileInfo->isFile()){
                fileSize += fileInfo->size();
                fileCount += 1;
            }else{
                folderCount += 1;
                folderContains += fileInfo->filesCount();
            }
        }

        QString selectedFolders;
        if (folderCount == 1 && folderContains <= 1){
            selectedFolders = m_selectOnlyOneFolder.arg(QString::number(folderCount), m_OnlyOneItemCounted.arg(folderContains));
        }else if (folderCount == 1 && folderContains > 1){
            selectedFolders = m_selectOnlyOneFolder.arg(QString::number(folderCount), m_counted.arg(folderContains));
        }else if (folderCount > 1 && folderContains <= 1){
            selectedFolders = m_selectFolders.arg(QString::number(folderCount), m_OnlyOneItemCounted.arg(folderContains));
        }else if (folderCount > 1 && folderContains > 1){
            selectedFolders = m_selectFolders.arg(QString::number(folderCount), m_counted.arg(folderContains));
        }else{
            selectedFolders = "";
        }

        QString selectedFiles;
        if (fileCount == 1){
            selectedFiles = m_selectOnlyOneFile.arg(QString::number(fileCount), FileUtils::formatSize(fileSize));
        }else if (fileCount > 1 ){
            selectedFiles = m_selectFiles.arg(QString::number(fileCount), FileUtils::formatSize(fileSize));
        }else{
            selectedFiles = "";
        }

        if (selectedFolders.isEmpty()){
            m_label->setText(QString("%1").arg(selectedFiles));
        }else if (selectedFiles.isEmpty()){
            m_label->setText(QString("%1").arg(selectedFolders));
        }else{
            m_label->setText(QString("%1,%2").arg(selectedFolders, selectedFiles));
        }
    }
    else
    {
        if (number == 1){
            if (event.fileUrlList().count() == 1){
                DUrl url = event.fileUrlList().first();

                const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(url);
                if (fileInfo->isFile()){
                    m_label->setText(m_selectOnlyOneFile.arg(QString::number(number), FileUtils::formatSize(fileInfo->size())));
                }else if (fileInfo->isDir()){
                    if (fileInfo->filesCount() <= 1){
                        m_label->setText(m_selectOnlyOneFolder.arg(QString::number(number),
                                                                        m_OnlyOneItemCounted.arg(QString::number(fileInfo->filesCount()))));
                    }else{
                        m_label->setText(m_selectOnlyOneFolder.arg(QString::number(number),
                                                                        m_counted.arg(QString::number(fileInfo->filesCount()))));
                    }
                }
            }
        }
    }
}

void DStatusBar::itemCounted(const FMEvent &event, int number)
{
    if(event.windowId() != WindowManager::getWindowId(window()))
        return;

    if(number > 1)
    {
        m_label->setText(m_counted.arg(QString::number(number)));
    }
    else
    {
        m_label->setText(m_OnlyOneItemCounted.arg(QString::number(number)));
    }
}

void DStatusBar::setLoadingIncatorVisible(const FMEvent &event, bool visible)
{
    if (event.windowId() != WindowManager::getWindowId(window()))
        return;

    m_loadingIndicator->setVisible(visible);

    if (visible) {
        const AbstractFileInfoPointer &fileInfo = fileService->createFileInfo(event.fileUrl());

        if (fileInfo)
            m_label->setText(fileInfo->loadingTip());
        else
            m_label->setText(tr("Loading..."));
    } else {
        m_label->setText(QString());
    }
}

void DStatusBar::resizeEvent(QResizeEvent *event)
{
    m_loadingIndicator->move((event->size().width() - m_loadingIndicator->width()) / 2, 0);

    QFrame::resizeEvent(event);
}
