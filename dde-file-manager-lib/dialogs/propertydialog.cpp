#include "propertydialog.h"
#include "dabstractfilewatcher.h"
#include "dfileinfo.h"

#include "app/define.h"

#include "dfileservices.h"

#include "shutil/fileutils.h"
#include "shutil/mimesappsmanager.h"
#include "shutil/iconprovider.h"
#include "shutil/filessizeworker.h"
#include "shutil/fileutils.h"

#include "views/deditorwidgetmenu.h"

#include "dialogs/dialogmanager.h"

#include "deviceinfo/udisklistener.h"

#include "utils.h"

#include "widgets/singleton.h"

#include "shareinfoframe.h"
#include "views/dfilemanagerwindow.h"
#include "views/windowmanager.h"
#include "views/dfileview.h"
#include "interfaces/dfilesystemmodel.h"

#include <dscrollbar.h>
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


NameTextEdit::NameTextEdit(const QString &text, QWidget *parent):
    QTextEdit(text, parent)
{
    setObjectName("NameTextEdit");
    setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    setFrameShape(QFrame::NoFrame);
    setFixedSize(200, 60);
    Q_UNUSED(new DEditorWidgetMenu(this))

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
    QString shareManager = tr("Share manager");
    initUI();
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
        m_expandGroup->expand(0)->setExpandedSeparatorVisible(false);
        m_expandGroup->expand(0)->setContent(m_deviceInfoFrame);
        m_expandGroup->expand(0)->setExpand(true);

    }else if (m_url == DUrl::fromLocalFile("/")){
        m_icon->setPixmap(svgToPixmap(":/devices/images/device/drive-harddisk-deepin.svg", 128, 128));
        m_edit->setPlainText(tr("Disk"));
        m_editDisbaled = true;

        m_localDeviceInfoFrame = createLocalDeviceInfoWidget(m_url);
        QStringList titleList;
        titleList << basicInfo;
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup->expand(0)->setExpandedSeparatorVisible(false);
        m_expandGroup->expand(0)->setContent(m_localDeviceInfoFrame);
        m_expandGroup->expand(0)->setExpand(true);
    }else{
        const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(m_url);
        if(!fileInfo){
            close();
            return;
        }
        m_icon->setPixmap(fileInfo->fileIcon().pixmap(160, 160));
        m_edit->setPlainText(fileInfo->fileDisplayName());
        m_edit->setAlignment(Qt::AlignHCenter);

        if (!fileInfo->isCanRename()){
            m_editDisbaled = true;
        }

        m_basicInfoFrame = createBasicInfoWidget(fileInfo);

        QStringList titleList;
        if (fileInfo->isFile()){
            titleList << basicInfo;
        }else{
            titleList << basicInfo;
            if (fileInfo->isCanShare()){
                titleList << shareManager;
            }
        }
        m_expandGroup = addExpandWidget(titleList);
        m_expandGroup->expand(0)->setExpandedSeparatorVisible(false);
        m_expandGroup->expand(0)->setContent(m_basicInfoFrame);
        m_expandGroup->expand(0)->setExpand(true);

        if (fileInfo->isFile()){
    //        m_OpenWithListWidget = createOpenWithListWidget(fileInfo);
    //        expandGroup->expand(1)->setContent(m_OpenWithListWidget);
            m_fileCount = 1;
            m_size = fileInfo->size();
        }else if (fileInfo->isDir()){
            if (fileInfo->isCanShare()){
                setFixedSize(QSize(320, 500));
                m_shareinfoFrame = createShareInfoFrame(fileInfo);
                m_expandGroup->expand(0)->setExpandedSeparatorVisible(true);
                m_expandGroup->expand(1)->setExpandedSeparatorVisible(false);
                m_expandGroup->expand(1)->setContent(m_shareinfoFrame);
                m_expandGroup->expand(1)->setExpand(false);
            }
            startComputerFolderSize(m_url);
            m_fileCount = fileInfo->size();
        }
    }
    initTextShowFrame(m_edit->toPlainText());
    if (m_editDisbaled){
        m_editButton->hide();
    }

    initConnect();
}

void PropertyDialog::initUI()
{
    setTitle("");
    setFixedSize(QSize(320, 480));
    m_icon->setFixedHeight(150);


    QFrame* m_editFrame = new QFrame;

    QHBoxLayout* editLayout = new QHBoxLayout;
    editLayout->addStretch();
    editLayout->addWidget(m_edit);
    editLayout->addStretch();
    editLayout->setSpacing(0);
    editLayout->setContentsMargins(0, 0, 0, 0);
    m_editFrame->setLayout(editLayout);

    m_editStackWidget = new QStackedWidget(this);
    m_editStackWidget->setFixedHeight(60);
    m_editStackWidget->addWidget(m_editFrame);

    QVBoxLayout *layout = new QVBoxLayout(this);

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

        if (fileService->renameFile(oldUrl, newUrl)) {
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
    FilesSizeWorker* worker = new FilesSizeWorker(urls);
    QThread*  workerThread = new QThread;
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, worker, &FilesSizeWorker::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

    connect(this, &PropertyDialog::requestStartComputerFolderSize, worker, &FilesSizeWorker::coumpueteSize);
    connect(worker, &FilesSizeWorker::sizeUpdated, this, &PropertyDialog::updateFolderSize);

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

void PropertyDialog::closeEvent(QCloseEvent *event)
{
    emit aboutToClosed(m_url);
    BaseDialog::closeEvent(event);
    emit closed(m_url);
}

void PropertyDialog::resizeEvent(QResizeEvent *event)
{
    BaseDialog::resizeEvent(event);
}
DExpandGroup *PropertyDialog::expandGroup() const
{
    return m_expandGroup;
}

DExpandGroup *PropertyDialog::addExpandWidget(const QStringList &titleList)
{
    QBoxLayout *layout = qobject_cast<QBoxLayout*>(this->layout());
    DExpandGroup *group = new DExpandGroup;

    for(const QString &title : titleList) {
        DArrowLineExpand *expand = new DArrowLineExpand;

        expand->setTitle(title);
        expand->setFixedHeight(30);

        layout->addWidget(expand, 0, Qt::AlignTop);

        group->addExpand(expand);
    }
    layout->addStretch();
    return group;
}

void PropertyDialog::initTextShowFrame(const QString &text)
{
    m_textShowFrame = new QFrame(this);
    m_textShowFrame->setFixedHeight(60);

    m_editButton = new QPushButton(m_textShowFrame);
    m_editButton->setObjectName("EditButton");
    m_editButton->setFixedSize(16, 16);
    connect(m_editButton, &QPushButton::clicked, this, &PropertyDialog::renameFile);

    QFontMetrics font = m_edit->fontMetrics();
    QString t = DFMGlobal::elideText(text, m_edit->size(), font, QTextOption::WrapAtWordBoundaryOrAnywhere, Qt::ElideMiddle, 0);
    QStringList labelTexts = t.split("\n");


    qDebug() << text << labelTexts;
    QVBoxLayout* textShowLayout = new QVBoxLayout;

    textShowLayout->addStretch();
    for(int i=0; i< labelTexts.length(); i++){
        QString labelText = labelTexts.at(i);
        QLabel* label = new QLabel(labelText, m_textShowFrame);
        label->setFixedHeight(20);
        label->setAlignment(Qt::AlignCenter);
        QHBoxLayout* hLayout = new QHBoxLayout;
        hLayout->addStretch();

        if (i < (labelTexts.length() - 1)){
            if (label->fontMetrics().width(labelText) > (m_edit->width() - 10) ){
                label->setFixedWidth(m_edit->width());
            }
            hLayout->addWidget(label);
        }else{
            hLayout->addWidget(label);
            if (label->fontMetrics().width(labelText) > (m_edit->width() - 2*m_editButton->width()) && labelTexts.length() >=3){
                labelText = label->fontMetrics().elidedText(labelText, Qt::ElideMiddle, m_edit->width() - 2*m_editButton->width());
            }
            label->setText(labelText);
            hLayout->addSpacing(2);
            hLayout->addWidget(m_editButton);
        }
        hLayout->addStretch();

        textShowLayout->addLayout(hLayout, Qt::AlignCenter);
        m_textShowLastLabel = label;

    }
    textShowLayout->addStretch();
    textShowLayout->setSpacing(0);
    textShowLayout->setContentsMargins(0, 0, 0, 0);
    m_textShowFrame->setLayout(textShowLayout);

    if (m_editStackWidget->count() == 1){
        m_editStackWidget->addWidget(m_textShowFrame);
    }else{
        m_editStackWidget->insertWidget(1, m_textShowFrame);
    }
    m_editStackWidget->setCurrentIndex(1);
}


QFrame *PropertyDialog::createBasicInfoWidget(const DAbstractFileInfoPointer &info)
{
    QFrame *widget = new QFrame;
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
        m_executableCheckBox = new DCheckBox;
        m_executableCheckBox->setFixedHeight(20);
        connect(m_executableCheckBox, &DCheckBox::toggled, this, &PropertyDialog::toggleFileExecutable);
        if (!info->isWritable()){
            m_executableCheckBox->setDisabled(true);
        }
        if(info->permission(QFile::ExeUser) || info->permission(QFile::ExeGroup) || info->permission(QFile::ExeOther)){
            m_executableCheckBox->setChecked(true);
        }
        layout->addRow(m_executableCheckBox, executableLabel);
    }
    widget->setLayout(layout);

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
    QFrame *widget = new QFrame;
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

    return widget;
}

QFrame *PropertyDialog::createDeviceInfoWidget(UDiskDeviceInfoPointer info)
{
    QFrame *widget = new QFrame;
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

    return widget;
}

QListWidget *PropertyDialog::createOpenWithListWidget(const DAbstractFileInfoPointer &info)
{
    QListWidget* listWidget = new QListWidget(this);
    m_OpenWithButtonGroup = new QButtonGroup(listWidget);
    listWidget->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    listWidget->setVerticalScrollBar(new DScrollBar);

    QString path = info->absoluteFilePath();
    QMimeType mimeType = mimeAppsManager->getMimeType(path);

    QStringList recommendApps = mimeAppsManager->MimeApps.value(MimesAppsManager::getMimeTypeByFileName(path));;
    foreach (QString name, mimeType.aliases()) {
        QStringList apps = mimeAppsManager->MimeApps.value(name);
        foreach (QString app, apps) {
            if (!recommendApps.contains(app)){
                recommendApps.append(app);
            }
        }
    }
    QString defaultApp = mimeAppsManager->getDefaultAppByMimeType(mimeType);

    foreach (QString f, recommendApps){
        QString iconName = mimeAppsManager->DesktopObjs.value(f).getIcon();
        QIcon icon(fileIconProvider->getDesktopIcon(iconName, 48));
        QListWidgetItem* item = new QListWidgetItem;

        QCheckBox* itemBox = new QCheckBox(mimeAppsManager->DesktopObjs.value(f).getLocalName());
        itemBox->setObjectName("OpenWithItem");
        itemBox->setIcon(icon);
        itemBox->setFixedHeight(36);
        itemBox->setIconSize(QSize(16, 16));
        m_OpenWithButtonGroup->addButton(itemBox);
        item->setData(Qt::UserRole, f);
        listWidget->addItem(item);
        listWidget->setItemWidget(item, itemBox);

        if (QFileInfo(f).fileName() == defaultApp){
            itemBox->setChecked(true);
        }

    }

    QListWidgetItem* item = new QListWidgetItem;
    QPushButton* otherButton = new QPushButton(tr("Others"));
    otherButton->setObjectName("OtherButton");
    otherButton->setFixedHeight(36);
    otherButton->setFixedWidth(100);
    listWidget->addItem(item);
    listWidget->setItemWidget(item, otherButton);


    int listHeight = 2;
    for(int i=0; i < listWidget->count(); i++){
        QListWidgetItem* item = listWidget->item(i);
        item->setFlags(Qt::NoItemFlags);
        int h = listWidget->itemWidget(item)->height();
        item->setSizeHint(QSize(item->sizeHint().width(), h));
        listHeight += h;
    }

    if (listHeight >= 36 * 4){
        listWidget->setFixedHeight(36 * 4);
    }else{
        listWidget->setFixedHeight(listHeight);
    }


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
