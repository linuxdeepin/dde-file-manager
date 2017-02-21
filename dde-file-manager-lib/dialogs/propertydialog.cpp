#include "propertydialog.h"
#include "dabstractfilewatcher.h"
#include "dfileinfo.h"

#include "app/define.h"

#include "dfileservices.h"

#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "shutil/filessizeworker.h"
#include "shutil/fileutils.h"

#include "dialogs/dialogmanager.h"
#include "app/filesignalmanager.h"

#include "deviceinfo/udisklistener.h"

#include "utils.h"

#include "widgets/singleton.h"

#include "shareinfoframe.h"
#include "views/dfilemanagerwindow.h"
#include "views/windowmanager.h"
#include "views/dfileview.h"
#include "interfaces/dfilesystemmodel.h"

#include "plugins/pluginmanager.h"
#include "../plugininterfaces/menu/menuinterface.h"

#include <dexpandgroup.h>
#include <dseparatorhorizontal.h>
#include <darrowlineexpand.h>
#include <dthememanager.h>
#include <dcombobox.h>
#include <dbaseexpand.h>
#include <dexpandgroup.h>
#include <dcheckbox.h>

#include <QTextEdit>
#include <QFormLayout>
#include <QDateTime>
#include <QCheckBox>
#include <QTimer>
#include <QThread>
#include <QListWidget>
#include <QButtonGroup>
#include <QPushButton>
#include <QStackedWidget>
#include <QStorageInfo>
#include <QVariantAnimation>
#include "views/dleftsidebar.h"
#include <QScrollArea>

NameTextEdit::NameTextEdit(const QString &text, QWidget *parent):
    QTextEdit(text, parent)
{
    setObjectName("NameTextEdit");
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setFixedSize(200, 60);

    connect(this, &QTextEdit::textChanged, this, [this] {
        QSignalBlocker blocker(this);
        Q_UNUSED(blocker)

        QString text = this->toPlainText();
        const QString old_text = text;

        int text_length = text.length();

        text.remove('/');
        text.remove(QChar(0));

        QVector<uint> list = text.toUcs4();
        int cursor_pos = this->textCursor().position() - text_length + text.length();

        while (text.toUtf8().size() > MAX_FILE_NAME_CHAR_COUNT) {
            list.removeAt(--cursor_pos);

            text = QString::fromUcs4(list.data(), list.size());
        }

        if (text.count() != old_text.count()) {
            this->setText(text);
        }

        QTextCursor cursor = this->textCursor();

        cursor.movePosition(QTextCursor::Start);

        do {
            QTextBlockFormat format = cursor.blockFormat();

            format.setLineHeight(TEXT_LINE_HEIGHT, QTextBlockFormat::FixedHeight);
            cursor.setBlockFormat(format);
        } while (cursor.movePosition(QTextCursor::NextBlock));

        cursor.setPosition(cursor_pos);

        this->setTextCursor(cursor);
        this->setAlignment(Qt::AlignHCenter);

        if (this->isReadOnly())
            this->setFixedHeight(this->document()->size().height());
    });


}

void NameTextEdit::setPlainText(const QString &text)
{
    QTextEdit::setPlainText(text);
    setAlignment(Qt::AlignCenter);
}

void NameTextEdit::focusOutEvent(QFocusEvent *event)
{
    QTextEdit::focusOutEvent(event);
}

void NameTextEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Escape){
        setIsCanceled(true);
        emit editFinished();
        return;
    }
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
        setIsCanceled(false);
        emit editFinished();
    }
    QTextEdit::keyPressEvent(event);
}
bool NameTextEdit::isCanceled() const
{
    return m_isCanceled;
}

void NameTextEdit::setIsCanceled(bool isCanceled)
{
    m_isCanceled = isCanceled;
}





GroupTitleLabel::GroupTitleLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("GroupTitleLabel");
}


SectionKeyLabel::SectionKeyLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionKeyLabel");
    setFixedWidth(120);
    setAlignment(Qt::AlignVCenter | Qt::AlignRight);
}


SectionValueLabel::SectionValueLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionValueLabel");
    setFixedWidth(150);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
    setWordWrap(true);
}


PropertyDialog::PropertyDialog(const DFMEvent &event, const DUrl url, QWidget* parent)
    : BaseDialog(parent)
    , m_fmevent(event)
    , m_url(url)
    , m_icon(new QLabel)
    , m_edit(new NameTextEdit)
{
    D_THEME_INIT_WIDGET(PropertyDialog)

    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags()
                           &~ Qt::WindowMaximizeButtonHint
                           &~ Qt::WindowMinimizeButtonHint
                           &~ Qt::WindowSystemMenuHint);
    QString basicInfo = tr("Basic info");
    QString openWith = tr("Open with");
    QString shareManager = tr("Share manager");
    initUI();
    QString query = m_url.query();
    UDiskDeviceInfoPointer diskInfo = deviceListener->getDevice(m_url.query());
    if (!diskInfo){
        diskInfo = deviceListener->getDeviceByPath(m_url.path());
    }
    if (diskInfo){
        qDebug() << diskInfo->getDiskInfo();
        QString name = diskInfo->getName();
        m_icon->setPixmap(diskInfo->fileIcon().pixmap(128, 128));
        m_edit->setPlainText(name);
        m_editDisbaled = true;
        m_deviceInfoFrame = createDeviceInfoWidget(diskInfo);

        QStringList titleList;
        titleList << basicInfo;
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup->expand(0)->setContent(m_deviceInfoFrame);
        m_expandGroup->expand(0)->setExpand(true);

    }else if (m_url == DUrl::fromLocalFile("/")){
        m_icon->setPixmap(svgToPixmap(":/devices/images/device/drive-harddisk-deepin.svg", 128, 128));
        m_edit->setPlainText(tr("System Disk"));
        m_editDisbaled = true;

        m_localDeviceInfoFrame = createLocalDeviceInfoWidget(m_url);
        QStringList titleList;
        titleList << basicInfo;
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup->expand(0)->setContent(m_localDeviceInfoFrame);
    }else{
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(m_url);
        if(!fileInfo){
            close();
            return;
        }
        m_icon->setPixmap(fileInfo->fileIcon().pixmap(160, 160));
        m_edit->setPlainText(fileInfo->fileDisplayName());
        m_edit->setAlignment(Qt::AlignHCenter);

        if (!fileInfo->canRename()){
            m_editDisbaled = true;
        }

        m_basicInfoFrame = createBasicInfoWidget(fileInfo);

        QStringList titleList;
        if (fileInfo->isFile()){
            titleList << basicInfo;
            titleList << openWith;
        }else{
            titleList << basicInfo;
            if (fileInfo->canShare()){
                titleList << shareManager;
            }
        }
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup->expand(0)->setContent(m_basicInfoFrame);

        if (fileInfo->isFile()){
            m_fileCount = 1;
            m_size = fileInfo->size();

            m_OpenWithListWidget = createOpenWithListWidget(fileInfo);
            m_expandGroup->expand(1)->setContent(m_OpenWithListWidget);
            m_expandGroup->expand(1)->setExpand(false);

        }else if (fileInfo->isDir()){
            if (fileInfo->canShare()){
                m_shareinfoFrame = createShareInfoFrame(fileInfo);
                m_expandGroup->expand(1)->setContent(m_shareinfoFrame);
                m_expandGroup->expand(1)->setExpand(false);
            }
            startComputerFolderSize(m_url);
            m_fileCount = fileInfo->filesCount();
        }
    }
    initTextShowFrame(m_edit->toPlainText());
    if (m_editDisbaled){
        m_editButton->hide();
    }

    loadPluginExpandWidgets();

    m_expandGroup->expands().first()->setExpand(true);
    m_expandGroup->expands().last()->setExpandedSeparatorVisible(false);

    initConnect();
}

void PropertyDialog::initUI()
{
    setTitle("");
//    setFixedSize(QSize(320, 480));
    m_icon->setFixedHeight(150);
    m_icon->setParent(this);
    QFrame* m_editFrame = new QFrame;

    QHBoxLayout* editLayout = new QHBoxLayout;
    editLayout->addStretch();
    editLayout->addWidget(m_edit);
    editLayout->addStretch();
    editLayout->setSpacing(0);
    editLayout->setContentsMargins(0, 0, 0, 0);
    m_editFrame->setLayout(editLayout);
    m_edit->setParent(m_editFrame);

    m_editStackWidget = new QStackedWidget(this);
    m_editStackWidget->setFixedHeight(60);
    m_editStackWidget->addWidget(m_editFrame);

    QVBoxLayout *layout = new QVBoxLayout;

    layout->setMargin(5);
    layout->setSpacing(0);
    layout->addWidget(m_icon, 0, Qt::AlignHCenter);
    layout->addWidget(m_editStackWidget, 0, Qt::AlignHCenter);
    setLayout(layout);
}

void PropertyDialog::initConnect()
{
    connect(m_edit, &NameTextEdit::editFinished, this, &PropertyDialog::showTextShowFrame);

    DAbstractFileWatcher *fileWatcher = DFileService::instance()->createFileWatcher(m_url, this);

    connect(fileWatcher, &DAbstractFileWatcher::fileDeleted, this, &PropertyDialog::onChildrenRemoved);
    connect(fileWatcher, &DAbstractFileWatcher::fileMoved, this, [this](const DUrl &from, const DUrl &to) {
        Q_UNUSED(to)

        onChildrenRemoved(from);
    });

    //play animation after a folder is shared
    connect(m_shareinfoFrame, &ShareInfoFrame::folderShared, this, &PropertyDialog::flickFolderToLeftsidBar);
}


void PropertyDialog::updateFolderSize(qint64 size)
{
    m_size = size;
    m_folderSizeLabel->setText(FileUtils::formatSize(size));
}

void PropertyDialog::renameFile()
{
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(m_url);
    m_edit->setPlainText(fileInfo->fileName());
    m_editStackWidget->setCurrentIndex(0);

    const DAbstractFileInfoPointer pfile = fileService->createFileInfo(m_url);
    int endPos = -1;
    if(pfile->isFile()){
        endPos = m_edit->toPlainText().length() - pfile->suffix().length()-1;
    }
    if(endPos == -1) {
        m_edit->selectAll();
        endPos = m_edit->toPlainText().length();
    }
    QTextCursor cursor = m_edit->textCursor();
    cursor.setPosition(0);
    cursor.setPosition(endPos, QTextCursor::KeepAnchor);
    m_edit->setTextCursor(cursor);

}

void PropertyDialog::showTextShowFrame()
{
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(m_url);

    if (m_edit->isCanceled()) {
        initTextShowFrame(fileInfo->fileDisplayName());
    } else {
        DUrl oldUrl = m_url;
        DUrl newUrl = fileInfo->getUrlByNewFileName(m_edit->toPlainText());

        if (fileService->renameFile(oldUrl, newUrl, m_fmevent)) {
            m_url = newUrl;
            const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(m_url);

            initTextShowFrame(fileInfo->fileDisplayName());
            dialogManager->refreshPropertyDialogs(oldUrl, newUrl);
            if (m_shareinfoFrame)
                m_shareinfoFrame->setFileinfo(fileInfo);
        } else {
            m_editStackWidget->setCurrentIndex(1);
        }
    }
}

void PropertyDialog::onChildrenRemoved(const DUrl &fileUrl)
{
    if (m_url.isUserShareFile()){
        return;
    }
    if (fileUrl == m_url){
        close();
    }
}

void PropertyDialog::flickFolderToLeftsidBar()
{

    DFileManagerWindow* window = qobject_cast<DFileManagerWindow*>(WindowManager::getWindowById(m_fmevent.windowId()));
    if(!window)
        return;

    //when current window is minimized,cancle animation
    if(window->windowState() == Qt::WindowMinimized)
        return;

    QPoint targetPos = window->getLeftSideBar()->getMyShareItemCenterPos();

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(m_url);

    QLabel* aniLabel = new QLabel();
    aniLabel->setFixedSize(m_icon->size());
    aniLabel->setWindowFlags(Qt::Popup | Qt::FramelessWindowHint);
    aniLabel->setAttribute(Qt::WA_TranslucentBackground);
    aniLabel->setPixmap(fileInfo->fileIcon().pixmap(160, 160));
    aniLabel->move(m_icon->mapToGlobal(m_icon->pos()));

    int angle;
    if(targetPos.x() > aniLabel->x())
        angle = 45;
    else
        angle = -45;

    QVariantAnimation* xani = new QVariantAnimation(this);
    xani->setStartValue(QPoint(aniLabel->x(), 0));
    xani->setEndValue(QPoint(targetPos.x(), angle));
    xani->setDuration(440);

    QVariantAnimation* gani = new QVariantAnimation(this);
    gani->setStartValue(aniLabel->geometry());
    gani->setEndValue(QRect(targetPos.x(), targetPos.y(), 20, 20));
    gani->setEasingCurve(QEasingCurve::InBack);
    gani->setDuration(440);

    connect(xani, &QVariantAnimation::valueChanged, [=](const QVariant& val){
        if(aniLabel){
            aniLabel->move(QPoint(val.toPoint().x() - aniLabel->width()/2,aniLabel->y()));
            QImage img = fileInfo->fileIcon().pixmap(aniLabel->size()).toImage();
            QMatrix ma;
            ma.rotate(val.toPoint().y());
            img = img.transformed(ma, Qt::SmoothTransformation);
            img = img.scaled(aniLabel->width()/2, aniLabel->height()/2, Qt::KeepAspectRatio, Qt::SmoothTransformation);
            aniLabel->setPixmap(QPixmap::fromImage(img));
            if(aniLabel->isHidden())
                aniLabel->show();
        }
    });
    connect(xani, &QVariantAnimation::finished, [=]{
        xani->deleteLater();
    });

    connect(gani, &QVariantAnimation::valueChanged, [=](const QVariant& val){
        aniLabel->move(QPoint(aniLabel->x(),
                              val.toRect().y() - val.toRect().width()/2));
        aniLabel->setFixedSize(val.toRect().size()* 2);
    });
    connect(gani, &QVariantAnimation::finished, [=]{
        gani->deleteLater();
        aniLabel->deleteLater();
        window->getLeftSideBar()->playtShareAddedAnimation();
    });
    xani->start();
    gani->start();
}
void PropertyDialog::onOpenWithBntsChecked(QAbstractButton *w)
{
    if(w){
        MimesAppsManager::setDefautlAppForType(w->property("mimeTypeName").toString(),
                                                 w->property("appName").toString());
    }
}

void PropertyDialog::onExpandChanged(const bool &e)
{
    DArrowLineExpand* expand = qobject_cast<DArrowLineExpand*>(sender());
    if(expand){
        if(e)
            expand->setSeparatorVisible(false);
        else
            expand->setSeparatorVisible(true);
    }
}

void PropertyDialog::mousePressEvent(QMouseEvent *event)
{
    if (m_edit->isVisible()){
        if (event->button() != Qt::RightButton){
            m_edit->setIsCanceled(false);
            emit m_edit->editFinished();
        }
    }
    BaseDialog::mousePressEvent(event);
}

void PropertyDialog::startComputerFolderSize(const DUrl &url)
{
    DUrl validUrl = url;
    if (url.isUserShareFile()){
        validUrl.setScheme(FILE_SCHEME);
    }
    DUrlList urls;
    urls << validUrl;
    m_sizeWorker = new FilesSizeWorker(urls);
    QThread*  workerThread = new QThread;
    m_sizeWorker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, m_sizeWorker, &FilesSizeWorker::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

    connect(this, &PropertyDialog::requestStartComputerFolderSize, m_sizeWorker, &FilesSizeWorker::coumpueteSize);
    connect(m_sizeWorker, &FilesSizeWorker::sizeUpdated, this, &PropertyDialog::updateFolderSize);

    workerThread->start();

    emit requestStartComputerFolderSize();
}

void PropertyDialog::toggleFileExecutable(bool isChecked)
{
    QFile f(m_url.toLocalFile());
    if (isChecked){
        f.setPermissions(f.permissions() | QFile::ExeOwner |QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther);
    }else{
        f.setPermissions(f.permissions() & ~(QFile::ExeOwner | QFile::ExeUser | QFile::ExeGroup | QFile::ExeOther));
    }
}

DUrl PropertyDialog::getUrl()
{
    return m_url;
}

int PropertyDialog::getFileCount()
{
    return m_fileCount;
}

qint64 PropertyDialog::getFileSize()
{
    return m_size;
}

void PropertyDialog::raise()
{
    BaseDialog::raise();
    emit raised();
}

void PropertyDialog::hideEvent(QHideEvent *event)
{
    emit aboutToClosed(m_url);
    BaseDialog::hideEvent(event);
    emit closed(m_url);
    if (m_sizeWorker){
        m_sizeWorker->stop();
    }
}

void PropertyDialog::resizeEvent(QResizeEvent *event)
{
    BaseDialog::resizeEvent(event);
}
DExpandGroup *PropertyDialog::expandGroup() const
{
    return m_expandGroup;
}

int PropertyDialog::contentHeight() const
{
    return  (m_icon->height()+
            m_editStackWidget->height()+
            expandGroup()->expands().first()->getContent()->height()+
            expandGroup()->expands().size()*30 +
            contentsMargins().top()+
            contentsMargins().bottom()+
            80);
}

void PropertyDialog::loadPluginExpandWidgets()
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    QList<PropertyDialogExpandInfoInterface*> plugins = PluginManager::instance()->getExpandInfoInterfaces();
    foreach (PropertyDialogExpandInfoInterface *plugin, plugins) {
        DArrowLineExpand *expand = new DArrowLineExpand;
        QWidget* frame = plugin->expandWidget(m_url.toString());
        if(!frame)
            continue;
        frame->setMaximumHeight(EXTEND_FRAME_MAXHEIGHT);
        frame->setParent(this);
        expand->setTitle(plugin->expandWidgetTitle(m_url.toString()));
        expand->setFixedHeight(30);
        expand->setExpand(false);
        expand->setContent(frame);
        expand->setStyleSheet("background: transparent;");
        layout->addWidget(expand, 0, Qt::AlignTop);
        m_expandGroup->addExpand(expand);
    }
    layout->addStretch();
    setFixedSize(320, contentHeight());
}

DExpandGroup *PropertyDialog::addExpandWidget(const QStringList &titleList)
{
    QVBoxLayout *layout = qobject_cast<QVBoxLayout*>(this->layout());
    DExpandGroup *group = new DExpandGroup;

    for(const QString &title : titleList) {
        DArrowLineExpand *expand = new DArrowLineExpand;

        expand->setTitle(title);
        expand->setFixedHeight(30);
        expand->setStyleSheet("background: transparent;");

        if(title == tr("Open with"))
            connect(expand, &DArrowLineExpand::expandChange, this, &PropertyDialog::onExpandChanged);

        layout->addWidget(expand, 0, Qt::AlignTop);

        group->addExpand(expand);
    }
    return group;
}

void PropertyDialog::initTextShowFrame(const QString &text)
{
    m_textShowFrame = new QFrame(this);
    m_textShowFrame->setFixedHeight(80);

    m_editButton = new QPushButton(m_textShowFrame);
    m_editButton->setObjectName("EditButton");
    m_editButton->setFixedSize(16, 16);
    connect(m_editButton, &QPushButton::clicked, this, &PropertyDialog::renameFile);

    QFontMetrics font = m_edit->fontMetrics();
    QString t = DFMGlobal::elideText(text, m_edit->size(), font, QTextOption::WrapAtWordBoundaryOrAnywhere, Qt::ElideMiddle, 0);
    QStringList labelTexts = t.split("\n");
    const int maxLineCount = 3;


    qDebug() << text << labelTexts;
    QVBoxLayout* textShowLayout = new QVBoxLayout;

    for(int i=0; i< labelTexts.length(); i++){
        if(i > (maxLineCount-1))
            break;
        QString labelText = labelTexts.at(i);
        QLabel* label = new QLabel(labelText, m_textShowFrame);
        label->setAlignment(Qt::AlignHCenter);
        QHBoxLayout* hLayout = new QHBoxLayout;

        hLayout->addStretch(1);
        hLayout->addWidget(label);
        if (i < (labelTexts.length() - 1 ) && i != (maxLineCount-1)){
            if (label->fontMetrics().width(labelText) > (m_edit->width() - 10) ){
                label->setFixedWidth(m_edit->width());
            }
        } else{
            if (label->fontMetrics().width(labelText) > (m_edit->width() - 2*m_editButton->width()) && labelTexts.length() >=3){
                labelText = label->fontMetrics().elidedText(labelText, Qt::ElideMiddle, m_edit->width() - 2*m_editButton->width());
            }
            label->setText(labelText);
            hLayout->addSpacing(2);
            hLayout->addWidget(m_editButton);
        }
        textShowLayout->addLayout(hLayout);
        hLayout->addStretch(1);
    }

    textShowLayout->setContentsMargins(0, 0, 0, 0);
    textShowLayout->setSpacing(0);
    m_textShowFrame->setLayout(textShowLayout);
    textShowLayout->addStretch(1);

    if (m_editStackWidget->count() == 1){
        m_editStackWidget->addWidget(m_textShowFrame);
    }else{
        m_editStackWidget->insertWidget(1, m_textShowFrame);
    }
    m_editStackWidget->setCurrentIndex(1);
}

QFrame *PropertyDialog::createBasicInfoWidget(const DAbstractFileInfoPointer &info)
{
    QFrame *widget = new QFrame(this);
    SectionKeyLabel* sizeSectionLabel = new SectionKeyLabel(QObject::tr("Size"));
    SectionKeyLabel* fileAmountSectionLabel = new SectionKeyLabel(QObject::tr("Contains"));
    SectionKeyLabel* typeSectionLabel = new SectionKeyLabel(QObject::tr("Type"));
    SectionKeyLabel* TimeCreatedSectionLabel = new SectionKeyLabel(QObject::tr("Time read"));
    SectionKeyLabel* TimeModifiedSectionLabel = new SectionKeyLabel(QObject::tr("Time modified"));

    SectionValueLabel* sizeLabel = new SectionValueLabel(info->sizeDisplayName());
    m_folderSizeLabel = new SectionValueLabel;
    SectionValueLabel* typeLabel = new SectionValueLabel(info->mimeTypeDisplayName());
    SectionValueLabel* timeCreatedLabel = new SectionValueLabel(info->lastReadDisplayName());
    SectionValueLabel* timeModifiedLabel = new SectionValueLabel(info->lastModifiedDisplayName());
    SectionValueLabel* executableLabel = new SectionValueLabel(tr("Allow to execute as program"));


    QFormLayout *layout = new QFormLayout;
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(16);
    layout->setLabelAlignment(Qt::AlignRight);
    if (info->isFile()){
        layout->addRow(sizeSectionLabel, sizeLabel);
    }else{
        layout->addRow(sizeSectionLabel, m_folderSizeLabel);
        layout->addRow(fileAmountSectionLabel, sizeLabel);
    }
    layout->addRow(typeSectionLabel, typeLabel);
    layout->addRow(TimeCreatedSectionLabel, timeCreatedLabel);
    layout->addRow(TimeModifiedSectionLabel, timeModifiedLabel);

    if (info->isFile()){
        m_executableCheckBox = new QCheckBox;
        m_executableCheckBox->setFixedHeight(20);
        connect(m_executableCheckBox, &QCheckBox::toggled, this, &PropertyDialog::toggleFileExecutable);
        if (!info->isWritable()){
            m_executableCheckBox->setDisabled(true);
        }
        if(info->permission(QFile::ExeUser) || info->permission(QFile::ExeGroup) || info->permission(QFile::ExeOther)){
            m_executableCheckBox->setChecked(true);
        }
        layout->addRow(m_executableCheckBox, executableLabel);
    }
    widget->setLayout(layout);
    widget->setFixedHeight(EXTEND_FRAME_MAXHEIGHT);

    return widget;
}

ShareInfoFrame *PropertyDialog::createShareInfoFrame(const DAbstractFileInfoPointer &info)
{
    ShareInfoFrame* frame = new ShareInfoFrame(info, this);
    return frame;
}

QFrame *PropertyDialog::createLocalDeviceInfoWidget(const DUrl &url)
{
    QStorageInfo info(url.path());
    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(url);
    QFrame *widget = new QFrame(this);
    SectionKeyLabel* typeSectionLabel = new SectionKeyLabel(QObject::tr("Device type"));
    SectionKeyLabel* fileAmountSectionLabel = new SectionKeyLabel(QObject::tr("Contains"));
    SectionKeyLabel* freeSectionLabel = new SectionKeyLabel(QObject::tr("Free space"));
    SectionKeyLabel* totalSectionLabel = new SectionKeyLabel(QObject::tr("Total space"));

    SectionValueLabel* typeLabel = new SectionValueLabel(tr("Local disk"));
    SectionValueLabel* fileAmountLabel = new SectionValueLabel(fileInfo->sizeDisplayName());
    SectionValueLabel* freeLabel = new SectionValueLabel(FileUtils::formatSize(info.bytesFree()));
    SectionValueLabel* totalLabel = new SectionValueLabel(FileUtils::formatSize(info.bytesTotal()));

    QFormLayout *layout = new QFormLayout;
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(16);
    layout->setLabelAlignment(Qt::AlignRight);

    layout->addRow(typeSectionLabel, typeLabel);
    layout->addRow(totalSectionLabel, totalLabel);
    layout->addRow(fileAmountSectionLabel, fileAmountLabel);
    layout->addRow(freeSectionLabel, freeLabel);

    widget->setLayout(layout);
    widget->setFixedHeight(EXTEND_FRAME_MAXHEIGHT);
    return widget;
}

QFrame *PropertyDialog::createDeviceInfoWidget(UDiskDeviceInfoPointer info)
{
    QFrame *widget = new QFrame(this);
    SectionKeyLabel* typeSectionLabel = new SectionKeyLabel(QObject::tr("Device type"));
    SectionKeyLabel* fileAmountSectionLabel = new SectionKeyLabel(QObject::tr("Contains"));
    SectionKeyLabel* freeSectionLabel = new SectionKeyLabel(QObject::tr("Free space"));
    SectionKeyLabel* totalSectionLabel = new SectionKeyLabel(QObject::tr("Total space"));

    SectionValueLabel* typeLabel = new SectionValueLabel(info->deviceTypeDisplayName());
    SectionValueLabel* fileAmountLabel = new SectionValueLabel(info->sizeDisplayName());
    SectionValueLabel* freeLabel = new SectionValueLabel(FileUtils::formatSize(info->getFree()));
    SectionValueLabel* totalLabel = new SectionValueLabel(FileUtils::formatSize(info->getTotal()));

    QFormLayout *layout = new QFormLayout;
    layout->setHorizontalSpacing(12);
    layout->setVerticalSpacing(16);
    layout->setLabelAlignment(Qt::AlignRight);

    layout->addRow(typeSectionLabel, typeLabel);
    layout->addRow(totalSectionLabel, totalLabel);
    layout->addRow(fileAmountSectionLabel, fileAmountLabel);
    layout->addRow(freeSectionLabel, freeLabel);

    widget->setLayout(layout);
    widget->setFixedHeight(EXTEND_FRAME_MAXHEIGHT);
    return widget;
}

QListWidget *PropertyDialog::createOpenWithListWidget(const DAbstractFileInfoPointer &info)
{
    QListWidget* listWidget = new QListWidget(this);
    listWidget->setObjectName("OpenWithListWidget");
    m_OpenWithButtonGroup = new QButtonGroup(listWidget);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    QString path = info->absoluteFilePath();
    QMimeType mimeType = mimeAppsManager->getMimeType(path);

    QList<AppContext> recommendApps = MimesAppsManager::getRecommendedAppsForType(mimeType.name());
    QString defaultApp = mimeAppsManager->getDefaultAppByMimeType(info->mimeTypeName());

    foreach (AppContext app, recommendApps){
        QListWidgetItem* item = new QListWidgetItem;

        QCheckBox* itemBox = new QCheckBox(app.appName);
        itemBox->setObjectName("OpenWithItem");
        itemBox->setIcon(app.appIcon);
        itemBox->setFixedHeight(36);
        itemBox->setIconSize(QSize(16, 16));
        itemBox->setProperty("appName",app.appName);
        itemBox->setProperty("mimeTypeName",info->mimeTypeName());
        m_OpenWithButtonGroup->addButton(itemBox);
        item->setData(Qt::UserRole, app.appName);
        listWidget->addItem(item);
        listWidget->setItemWidget(item, itemBox);

        if (app.appName == defaultApp){
            itemBox->setChecked(true);
        }

    }

    int listHeight = 2;
    for(int i=0; i < listWidget->count(); i++){
        QListWidgetItem* item = listWidget->item(i);
        item->setFlags(Qt::NoItemFlags);
        int h = listWidget->itemWidget(item)->height();
        item->setSizeHint(QSize(item->sizeHint().width(), h));
        listHeight += h;
    }

    listWidget->setFixedHeight(EXTEND_FRAME_MAXHEIGHT);
    listWidget->setFixedWidth(300);

    listWidget->setStyleSheet("QListWidget#OpenWithListWidget{"
                                "border: 1px solid #eaeaea;"
                                "padding-left: 8px;"
                                "border-radius: 2px;"
                              "}");

    connect(m_OpenWithButtonGroup, SIGNAL(buttonClicked(QAbstractButton*)),
            this, SLOT(onOpenWithBntsChecked(QAbstractButton*)));

    return listWidget;
}

QFrame *PropertyDialog::createAuthorityManagermentWidget(const DAbstractFileInfoPointer &info)
{
    QFrame *widget = new QFrame;
    QFormLayout *layout = new QFormLayout;

    DComboBox *ownerBox = new DComboBox;
    DComboBox *groupBox = new DComboBox;
    DComboBox *otherBox = new DComboBox;

    QStringList authorityList;

    authorityList << QObject::tr("WriteRead") << QObject::tr("ReadOnly") << QObject::tr("WriteOnly");

    ownerBox->addItems(authorityList);
    groupBox->addItems(authorityList);
    otherBox->addItems(authorityList);

    if(info->permission(QFile::WriteOwner | QFile::ReadOwner))
        ownerBox->setCurrentIndex(0);
    else if(info->permission(QFile::ReadOwner))
        ownerBox->setCurrentIndex(1);
    else
        ownerBox->setCurrentIndex(2);

    if(info->permission(QFile::WriteGroup | QFile::ReadGroup))
        groupBox->setCurrentIndex(0);
    else if(info->permission(QFile::ReadGroup))
        groupBox->setCurrentIndex(1);
    else
        groupBox->setCurrentIndex(2);

    if(info->permission(QFile::WriteOther | QFile::ReadOther))
        otherBox->setCurrentIndex(0);
    else if(info->permission(QFile::ReadOther))
        otherBox->setCurrentIndex(1);
    else
        otherBox->setCurrentIndex(2);

    layout->setLabelAlignment(Qt::AlignRight);
    layout->addRow(QObject::tr("Owner"), ownerBox);
    layout->addRow(QObject::tr("Group"), groupBox);
    layout->addRow(QObject::tr("Other"), otherBox);

    widget->setLayout(layout);

    return widget;
}
