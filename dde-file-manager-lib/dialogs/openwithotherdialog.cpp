#include "openwithotherdialog.h"

#include <QListWidget>
#include <QListWidgetItem>
#include <QButtonGroup>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QCheckBox>
#include <QPushButton>
#include <QLabel>
#include <QFuture>
#include <dthememanager.h>
#include <QtConcurrent>
#include <QMutex>
#include <QScrollBar>
#include <QtMath>

#include "app/define.h"

#include "dfileservices.h"

#include "shutil/mimesappsmanager.h"
#include "shutil/desktopfile.h"
#include "shutil/fileutils.h"
#include "interfaces/dfileiconprovider.h"

#include "utils.h"

#include "singleton.h"
#include <QVBoxLayout>
#include "../chinese2pinyin/chinese2pinyin.h"

OpenWithOtherDialog::OpenWithOtherDialog(const DUrl &url, QWidget *parent) :
    DDialog(parent),
    m_appMatchWorker(new AppMatchWorker),
    m_searchEdit(new DSearchLineEdit(this)),
    m_appListWidget(new QListWidget(this)),
    m_verticalScrollBar(m_appListWidget->verticalScrollBar()),
    m_emptyPage(new QWidget(this)),
    m_mainFrame(new QWidget(this))
{
    qRegisterMetaType<DesktopAppList>();
    m_searchTimer = new QTimer(this);
    m_url = url;
    setModal(true);
    m_searchTimer->setInterval(400);
    m_searchTimer->setSingleShot(true);

    initUI();
    initConnect();

    //init datas asynchronously
    initDataAsync();
}

OpenWithOtherDialog::~OpenWithOtherDialog()
{

}

void OpenWithOtherDialog::initData()
{
    const DAbstractFileInfoPointer &info = fileService->createFileInfo(this, m_url);

    QStringList recommendApps = mimeAppsManager->getRecommendedApps(info->fileUrl());

    bool hasItems = false;
    foreach (const QString& f, mimeAppsManager->DesktopObjs.keys()) {
        //filter recommend apps , no show apps and no mime support apps
        const DesktopFile& app = mimeAppsManager->DesktopObjs.value(f);
        if(recommendApps.contains(f))
            continue;

        if(mimeAppsManager->DesktopObjs.value(f).getNoShow())
            continue;

        if(mimeAppsManager->DesktopObjs.value(f).getMimeType().first() == "")
            continue;

        bool isSameDesktop = false;
        foreach (const DesktopFile& otherApp, m_otherAppList) {
            if(otherApp.getExec() == app.getExec() && otherApp.getLocalName() == app.getLocalName())
                isSameDesktop = true;
        }

        if(!isSameDesktop){
            m_otherAppList << mimeAppsManager->DesktopObjs.value(f);
            m_appQueue << mimeAppsManager->DesktopObjs.value(f);
            if(!hasItems && m_appQueue.count() == 25){
                emit requestAppendPageItems();
                hasItems = true;
            }
        }
    }

    if(!hasItems)
        emit requestAppendPageItems();
}

void OpenWithOtherDialog::initDataAsync()
{
    QtConcurrent::run([=]{
        initData();
    });
}

void OpenWithOtherDialog::initUI()
{
    setFixedSize(300, 435);
    m_OpenWithButtonGroup = new QButtonGroup(m_appListWidget);
    m_appListWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_appListWidget->setVerticalScrollBarPolicy(Qt::ScrollBarAsNeeded);
    m_appListWidget->setStyleSheet(getQssFromFile(":/light/OpenWithOtherDialog.theme"));

    QLabel* titleLabel = new QLabel(tr("All Programs"), this);
    titleLabel->setFixedSize(200, 40);
    titleLabel->move(50, 0);
    titleLabel->setAlignment(Qt::AlignCenter);

    QVBoxLayout* frameLayout = new QVBoxLayout;
    frameLayout->setContentsMargins(0, 0, 0, 0);
    m_mainFrame->setContentsMargins(0, 0, 0, 0);

    m_searchEdit->setPlaceHolder(tr("Search"));
    m_searchEdit->setFixedHeight(24);
    m_searchEdit->setFixedWidth(260);
    frameLayout->addWidget(m_searchEdit, 0 , Qt::AlignHCenter);
    frameLayout->addWidget(m_appListWidget);

    m_emptyPage->setFixedHeight(36*9);
    frameLayout->addWidget(m_emptyPage, Qt::AlignHCenter);
    QLabel* emptyTipLabel = new QLabel(tr("No results"), this);
    emptyTipLabel->setObjectName("contentLabel");
    emptyTipLabel->setStyleSheet(getQssFromFile(":/light/OpenWithOtherDialog.theme"));
    QVBoxLayout* emptyPageLayout = new QVBoxLayout;
    emptyPageLayout->addStretch(1);
    emptyPageLayout->addWidget(emptyTipLabel, 0, Qt::AlignHCenter);
    emptyPageLayout->addStretch(1);
    m_emptyPage->setLayout(emptyPageLayout);
    m_emptyPage->hide();

    m_mainFrame->setLayout(frameLayout);

    addButton(tr("Cancel"));
    addButton(tr("Choose"), true, ButtonRecommend);

    setSpacing(0);
    setDefaultButton(1);
    m_searchEdit->setFocus();
    m_appListWidget->setMouseTracking(true);
    m_appListWidget->setFixedHeight(36*9);

    m_emptyPage->setFocusPolicy(Qt::NoFocus);
    m_appListWidget->setFocusPolicy(Qt::NoFocus);
    setFocusPolicy(Qt::NoFocus);

    m_mainFrame->setFixedWidth(width());
    addContent(m_mainFrame, Qt::AlignHCenter);
    m_verticalScrollBar->setParent(m_mainFrame);
    m_verticalScrollBar->setFixedSize(10, m_appListWidget->height());
    setContentLayoutContentsMargins(QMargins(0, 0, 0, 0));

    //ddialog still has bug for ibheriting qss from parent
//    setStyleSheet(getQssFromFile(":/light/OpenWithOtherDialog.theme"));
}

void OpenWithOtherDialog::  initConnect()
{
    connect(m_searchEdit, &DSearchLineEdit::textChanged, this, &OpenWithOtherDialog::onSearchTextChanged);
    connect(m_searchTimer, &QTimer::timeout, this, &OpenWithOtherDialog::searchApps);
    connect(this, &OpenWithOtherDialog::requestAppendPageItems, this, &OpenWithOtherDialog::appendPageItems);
    connect(this, &OpenWithOtherDialog::requestMatchApps, m_appMatchWorker, &AppMatchWorker::matchApps);
    connect(m_appMatchWorker, &AppMatchWorker::finishedMatchTask, this, &OpenWithOtherDialog::updateAppList);
    connect(m_verticalScrollBar, &QScrollBar::valueChanged, this, &OpenWithOtherDialog::onScrollBarValueChanged);
    connect(m_appListWidget, &QListWidget::currentItemChanged, this, &OpenWithOtherDialog::selectByItem);

    connect(m_searchEdit, &DSearchLineEdit::keyUpPressed, this, &OpenWithOtherDialog::selectUp);
    connect(m_searchEdit, &DSearchLineEdit::keyDownPressed, this, &OpenWithOtherDialog::selectDown);

    connect(m_appListWidget, &QListWidget::itemEntered, this, &OpenWithOtherDialog::onItemEntered);
}

void OpenWithOtherDialog::updateAppList(DesktopAppList desktopAppList)
{
    //clear ui items and appQueue that view haden't load.
    m_appQueue.clear();
    m_appListWidget->clear();

    if(desktopAppList.count() == 0){
        if(m_isPageEmpty)
            return;
        showEmptyPage();
    } else{
        if(m_isPageEmpty){
            hideEmptyPage();
        }
    }

    QtConcurrent::run([=]{
        bool hasItems =false;
        foreach (DesktopFile desktopApp, desktopAppList) {
            m_appQueue << desktopApp;
            if(m_appQueue.count() > 25 && !hasItems){
                hasItems = true;
                emit requestAppendPageItems();
            }
        }

        //when result list is less then 25, append page with all items
        if(!hasItems){
            emit requestAppendPageItems();
        }
    });
}

void OpenWithOtherDialog::onSearchTextChanged()
{
    m_searchKeyword = m_searchEdit->text();
    m_searchTimer->stop();
    m_searchTimer->start();
}

void OpenWithOtherDialog::appendPageItems()
{
    for(int i = 0; i < 25; i ++){
        if(m_appQueue.count() > 0){
            DAbstractFileInfoPointer info = DFileService::instance()->createFileInfo(this, m_url);
            QString defaultApp = mimeAppsManager->getDefaultAppByMimeType(info->mimeTypeName());
            const DesktopFile& desktopApp = m_appQueue.dequeue();
            QString iconName = desktopApp.getIcon();

            //some dekstop file's icon is set as file path not from icon theme
            QIcon icon = QIcon::fromTheme(iconName);
            if(icon.isNull())
                icon = QIcon(iconName);
            if(icon.pixmap(16,16).isNull()){
                const DAbstractFileInfoPointer& info = fileService->createFileInfo(this, DUrl::fromLocalFile(desktopApp.getFileName()));
                icon = info->fileIcon();
            }

            QListWidgetItem* item = new QListWidgetItem;

            QCheckBox* itemBox = new QCheckBox(desktopApp.getLocalName());
            itemBox->setFocusPolicy(Qt::NoFocus);

            connect(itemBox, &QCheckBox::clicked, [=](const bool& checked){
                if(checked){
                    m_appListWidget->setCurrentItem(item);
                }
            });
            itemBox->setIcon(icon);
            itemBox->setIconSize(QSize(16, 16));
            m_OpenWithButtonGroup->addButton(itemBox);
            itemBox->setProperty("app", desktopApp.getFileName());
            m_appListWidget->addItem(item);
            m_appListWidget->setItemWidget(item, itemBox);

            if (desktopApp.getFileName() == defaultApp){
                itemBox->setChecked(true);
            }
        } else{
            break;
        }
    }
//    if(m_appListWidget->currentRow() < 0 || m_appListWidget->currentRow() >= m_appListWidget->count())
//        m_appListWidget->setCurrentRow(0);
}

void OpenWithOtherDialog::onScrollBarValueChanged(const int &value)
{
    if(m_appQueue.count() == 0)
        return;

    if(m_verticalScrollBar->maximum() - value < 3){
        appendPageItems();
    }
}

void OpenWithOtherDialog::showEmptyPage()
{
    m_isPageEmpty = true;
    m_appListWidget->hide();
    m_emptyPage->show();
}

void OpenWithOtherDialog::hideEmptyPage()
{
    m_isPageEmpty = false;
    m_appListWidget->show();
    m_emptyPage->hide();
}

void OpenWithOtherDialog::selectUp()
{
    int currentRow = m_appListWidget->currentRow();
    int upRow = currentRow -1;

    if(upRow < 0)
        upRow = m_appListWidget->count() -1;

    m_appListWidget->setCurrentRow(upRow);
}

void OpenWithOtherDialog::selectDown()
{
    int currentRow = m_appListWidget->currentRow();
    int downRow = currentRow + 1;

    if(downRow >= m_appListWidget->count())
        downRow = 0;

    m_appListWidget->setCurrentRow(downRow);
}

void OpenWithOtherDialog::selectByItem(QListWidgetItem *item)
{
    if(!item)
        return;
    QCheckBox* itemBox = qobject_cast<QCheckBox*> (m_appListWidget->itemWidget(item));
    itemBox->setChecked(true);
}

bool OpenWithOtherDialog::confirmSelection()
{
    QString app;
    QString file = m_url.toString();
    QListWidgetItem* item = NULL;
    QWidget* bnt = NULL;
    QWidget* checkedBnt = NULL;

    checkedBnt = m_OpenWithButtonGroup->checkedButton();
    if(checkedBnt){
        app = checkedBnt->property("app").toString();
    }

    if(app.isEmpty()){
        item = m_appListWidget->currentItem();
        if(item)
            bnt = m_appListWidget->itemWidget(item);

        if(bnt)
            app = bnt->property("app").toString();
    }

    return FileUtils::openFileByApp(file, app);
}

void OpenWithOtherDialog::onItemEntered(QListWidgetItem *item)
{
    if(!item)
        return;
    item->setSelected(true);
}

void OpenWithOtherDialog::done(int r)
{
    if(r == 1){
        bool ret = confirmSelection();
        if(!ret)
            return;
    }
    DDialog::done(r);
}

void OpenWithOtherDialog::resizeEvent(QResizeEvent *event)
{
    DDialog::resizeEvent(event);
    m_verticalScrollBar->move(m_mainFrame->width() - m_verticalScrollBar->width(),
                              m_mainFrame->height() - m_verticalScrollBar->height());
}

void OpenWithOtherDialog::searchApps()
{
    if(m_appMatchWorker->workerState() == AppMatchWorker::Ready){
        if(m_searchKeyword.isEmpty()){
            updateAppList(m_otherAppList);
        } else{
            QThread* workerThread = new QThread;

            m_appMatchWorker->moveToThread(workerThread);
            connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);
            connect(workerThread, &QThread::finished, [=]{
                m_appMatchWorker->moveToThread(this->thread());
                m_appMatchWorker->setWorkerState(AppMatchWorker::Ready);
            });

            workerThread->start();
            emit requestMatchApps(m_searchKeyword, m_otherAppList);
        }

    } else if(m_appMatchWorker->workerState() == AppMatchWorker::Running){
        m_appMatchWorker->setWorkerState(AppMatchWorker::Finished);
    }
}

AppMatchWorker::AppMatchWorker(QObject *parent):
    QObject(parent),
    m_workerState(Ready)
{

}

AppMatchWorker::~AppMatchWorker()
{

}

void AppMatchWorker::matchApps(QString keyword, DesktopAppList otherDesktopAppList)
{
    DesktopAppList appList;

    DesktopAppList appsMatchedByLocalName;
    DesktopAppList appsMatchedByLocalNameUnsupportMime;
    DesktopAppList appsMatchedByFileName;
    DesktopAppList appsMatchedByFileNameUnsupportMime;

    m_workerState = Running;

    foreach (const DesktopFile& df, otherDesktopAppList) {
        if(m_workerState == Finished){
            thread()->quit();
            return;
        }

        //Filter out unsupport mime apps and recommended apps
        const bool unSupportMime = df.getMimeType().first() == "";

        //if keyword is empty,use all apps expect for unsupport mime apps and recommended apps
        if(keyword == ""){
            if(unSupportMime)
                appsMatchedByLocalNameUnsupportMime << df;
            else
                appList << df;
            continue;
        }

        keyword = keyword.toLower();
        QString targetText = df.getLocalName();
        targetText = targetText.toLower();

        //for app local name
        if(matchByFullName(keyword, targetText)){
            if(unSupportMime)
                appsMatchedByLocalNameUnsupportMime << df;
            else
                appsMatchedByLocalName << df;
            continue;
        }
        if(matchByCharactorQueue(keyword, targetText)){
            if(unSupportMime)
                appsMatchedByLocalNameUnsupportMime << df;
            else
                appsMatchedByLocalName << df;
            continue;
        }

        //for local name to pinyin
        QString pinyinTargetText = Pinyin::Chinese2Pinyin(targetText);
        for(int i = 1; i <= 4; i++){
            pinyinTargetText = pinyinTargetText.replace(QString::number(i), "");
        }

        if(matchByFullName(keyword, pinyinTargetText)){
            if(unSupportMime)
                appsMatchedByLocalNameUnsupportMime << df;
            else
                appsMatchedByLocalName << df;
            continue;
        }
        if(matchByCharactorQueue(keyword, pinyinTargetText)){
            if(unSupportMime)
                appsMatchedByLocalNameUnsupportMime << df;
            else
                appsMatchedByLocalName << df;
            continue;
        }

        //for app file name
        targetText = df.getPureFileName();
        targetText = targetText.toLower();

        if(matchByFullName(keyword, targetText)){
            if(unSupportMime)
                appsMatchedByFileNameUnsupportMime << df;
            else
                appsMatchedByFileName << df;
            continue;
        }
        if(matchByCharactorQueue(keyword, targetText)){
            if(unSupportMime)
                appsMatchedByFileNameUnsupportMime << df;
            else
                appsMatchedByFileName << df;
        }
    }

    //handle empty keyword
    if(keyword == "")
        return;

    //merge match result apps
    foreach (const DesktopFile& df, appsMatchedByLocalName) {
        appList << df;
    }
    foreach (const DesktopFile& df, appsMatchedByLocalNameUnsupportMime) {
        appList << df;
    }

    foreach (const DesktopFile& df, appsMatchedByFileName) {
        appList << df;
    }
    foreach (const DesktopFile& df, appsMatchedByFileNameUnsupportMime) {
        appList << df;
    }
    emit finishedMatchTask(appList);
    thread()->quit();
}

bool AppMatchWorker::matchByFullName(const QString &pattern, const QString &source)
{
    return source.contains(pattern);
}

bool AppMatchWorker::matchByCharactorQueue(const QString &pattern, const QString &source)
{
    int iter = 0;
    int i = 0;
    while (i < source.length()) {
        if(pattern.at(iter) == source.at(i))
            iter ++;
        if(iter == pattern.length()){
            return true;
        }
        i++;
    }
    return false;
}

AppMatchWorker::WorkerState AppMatchWorker::workerState() const
{
    return m_workerState;
}

void AppMatchWorker::setWorkerState(const WorkerState &workerState)
{
    m_workerState = workerState;
}

DSearchLineEdit::DSearchLineEdit(QWidget *parent):
    DSearchEdit(parent)
{

}

void DSearchLineEdit::focusOutEvent(QFocusEvent *event)
{
    Q_UNUSED(event);
    return;
}

void DSearchLineEdit::keyPressEvent(QKeyEvent *event)
{
    switch (event->key()) {
    case Qt::Key_Up:
        emit keyUpPressed();
        break;
    case Qt::Key_Down:
        emit keyDownPressed();
    default:
        break;
    }

    DSearchEdit::keyPressEvent(event);
}
