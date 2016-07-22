#include "propertydialog.h"

#include "fileinfo.h"
#include "../app/global.h"
#include <../controllers/fileservices.h>
#include "../shutil/fileutils.h"
#include "../shutil/mimesappsmanager.h"
#include "deviceinfo/deviceinfo.h"
#include "../shutil/fileutils.h"
#include "utils/utils.h"


#include <dscrollbar.h>
#include <dexpandgroup.h>
#include <dseparatorhorizontal.h>
#include <darrowlineexpand.h>
#include <dthememanager.h>
#include <dcombobox.h>
#include <dbaseexpand.h>
#include <dexpandgroup.h>

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
#include "../views/deditorwidgetmenu.h"



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
    emit editFinished();
    QTextEdit::focusOutEvent(event);
}

void NameTextEdit::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_Return || event->key() == Qt::Key_Enter){
        emit editFinished();
    }
    QTextEdit::keyPressEvent(event);
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
    setFixedWidth(180);
    setAlignment(Qt::AlignVCenter | Qt::AlignLeft);
}


ComupterFolderSizeWorker::ComupterFolderSizeWorker(const QString &dir):
    QObject(),
    m_dir(dir)

{
    m_size = 0;
}


void ComupterFolderSizeWorker::coumpueteSize()
{
    QFileInfo targetInfo(m_dir);
    if (targetInfo.exists()){
        if (targetInfo.isDir()){
            QDir d(m_dir);
            QFileInfoList entryInfoList = d.entryInfoList(QDir::AllEntries | QDir::System
                        | QDir::NoDotAndDotDot | QDir::NoSymLinks
                        | QDir::Hidden);
            foreach (QFileInfo file, entryInfoList) {
                if (file.isFile()){
                    m_size += file.size();
                    updateSize();
                }
                else {
                    QDirIterator it(file.absoluteFilePath(), QDir::AllEntries | QDir::System
                                  | QDir::NoDotAndDotDot | QDir::NoSymLinks
                                  | QDir::Hidden, QDirIterator::Subdirectories);
                    while (it.hasNext()) {
                        it.next();
                        m_size += it.fileInfo().size();
                        updateSize();
                    }
                }
            }
        }else{
            m_size += targetInfo.size();
        }
    }
    emit sizeUpdated(m_size);
}

QString ComupterFolderSizeWorker::dir() const
{
    return m_dir;
}

void ComupterFolderSizeWorker::setDir(const QString &dir)
{
    m_dir = dir;
}

void ComupterFolderSizeWorker::updateSize()
{
    emit sizeUpdated(m_size);
}



PropertyDialog::PropertyDialog(const DUrl &url, QWidget* parent)
    : BaseDialog(parent)
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
    startTimer(1000);
    m_absolutePath = url.toLocalFile();
    qDebug() << url;

    QString basicInfo = tr("Basic info");
    initUI();
    UDiskDeviceInfo* diskInfo = deviceListener->getDevice(url.query());
    if (diskInfo == NULL){
        diskInfo = deviceListener->getDeviceByPath(url.path());
    }
    if (diskInfo){
        qDebug() << diskInfo->getDiskInfo();
        m_absolutePath = diskInfo->getMountPoint();
        QString name = diskInfo->getName();
        m_icon->setPixmap(diskInfo->fileIcon().pixmap(128, 128));
        m_edit->setPlainText(name);
        m_editDisbaled = true;
        m_deviceInfoFrame = createDeviceInfoWidget(diskInfo);

        QStringList titleList;
        titleList << basicInfo;
        DExpandGroup *expandGroup = addExpandWidget(titleList);
        expandGroup->expand(0)->setExpandedSeparatorVisible(false);
        expandGroup->expand(0)->setContent(m_deviceInfoFrame);
        expandGroup->expand(0)->setExpand(true);

    }else if (url == DUrl::fromLocalFile("/")){
        m_icon->setPixmap(svgToPixmap(":/devices/images/device/drive-harddisk-deepin.svg", 128, 128));
        m_edit->setPlainText(tr("Disk"));
        m_editDisbaled = true;

        m_localDeviceInfoFrame = createLocalDeviceInfoWidget(url);
        QStringList titleList;
        titleList << basicInfo;
        DExpandGroup *expandGroup = addExpandWidget(titleList);
        expandGroup->expand(0)->setExpandedSeparatorVisible(false);
        expandGroup->expand(0)->setContent(m_localDeviceInfoFrame);
        expandGroup->expand(0)->setExpand(true);
    }else{
        const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(url);
        m_icon->setPixmap(fileInfo->fileIcon().pixmap(160, 160));
        m_edit->setPlainText(fileInfo->displayName());
        m_edit->setAlignment(Qt::AlignHCenter);

        m_basicInfoFrame = createBasicInfoWidget(fileInfo);

        QStringList titleList;
        if (fileInfo->isFile()){
            titleList = QStringList() << basicInfo;
        }else{
            titleList = QStringList() << basicInfo;
        }
        DExpandGroup *expandGroup = addExpandWidget(titleList);
        expandGroup->expand(0)->setExpandedSeparatorVisible(false);
        expandGroup->expand(0)->setContent(m_basicInfoFrame);
        expandGroup->expand(0)->setExpand(true);

        if (fileInfo->isFile()){
    //        m_OpenWithListWidget = createOpenWithListWidget(fileInfo);
    //        expandGroup->expand(1)->setContent(m_OpenWithListWidget);
            m_fileCount = 1;
            m_size = fileInfo->size();
        }else if (fileInfo->isDir()){
            startComputerFolderSize(fileInfo->absoluteFilePath());
            m_fileCount = fileInfo->size();
        }
    }
    initTextShowFrame(m_edit->toPlainText());
    if (m_editDisbaled){
        m_editButton->hide();
    }
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

    connect(m_edit, &NameTextEdit::editFinished, this, &PropertyDialog::showTextShowFrame);
}


void PropertyDialog::updateFolderSize(qint64 size)
{
    m_size = size;
    m_folderSizeLabel->setText(FileUtils::formatSize(size));
}

void PropertyDialog::renameFile()
{
    const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(m_url);
    m_edit->setPlainText(fileInfo->displayName());
    m_editStackWidget->setCurrentIndex(0);

    const AbstractFileInfoPointer pfile = fileService->createFileInfo(m_url);
    int endPos = -1;
    if(pfile->isFile()){
        endPos = m_edit->toPlainText().length() - pfile->suffix().length() - 1;
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
    DUrl oldUrl = m_url;
    DUrl newUrl = DUrl(QString("%1/%2").arg(DUrl::parentUrl(m_url).toString(), m_edit->toPlainText()));

    QFile file(oldUrl.toLocalFile());
    const QString &newFilePath = newUrl.toLocalFile();
    bool result = file.rename(newFilePath);
    if (!result) {
        result = QProcess::execute("mv \"" + file.fileName().toUtf8() + "\" \"" + newFilePath.toUtf8() + "\"") == 0;
    }

    qDebug() << result;

    if (result){
        m_url = newUrl;
        m_absolutePath = m_url.toLocalFile();
        const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(m_url);
        initTextShowFrame(fileInfo->displayName());
        dialogManager->refreshPropertyDialogs(oldUrl, newUrl);
    }else{
        m_editStackWidget->setCurrentIndex(1);
    }
}

void PropertyDialog::startComputerFolderSize(const QString &dir)
{
    ComupterFolderSizeWorker* worker = new ComupterFolderSizeWorker(dir);
    QThread*  workerThread = new QThread;
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, worker, &ComupterFolderSizeWorker::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

    connect(this, &PropertyDialog::requestStartComputerFolderSize, worker, &ComupterFolderSizeWorker::coumpueteSize);
    connect(worker, &ComupterFolderSizeWorker::sizeUpdated, this, &PropertyDialog::updateFolderSize);

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
    qDebug() << event;
    emit aboutToClosed(m_url);
    BaseDialog::closeEvent(event);
    emit closed(m_url);
}

void PropertyDialog::timerEvent(QTimerEvent *event)
{
    Q_UNUSED(event)
    if (!QFile(m_absolutePath).exists()){
        close();
    }
}

void PropertyDialog::resizeEvent(QResizeEvent *event)
{
    BaseDialog::resizeEvent(event);
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
    QString t = Global::elideText(text, m_edit->size(), font, QTextOption::WrapAtWordBoundaryOrAnywhere, Qt::ElideMiddle, 0);
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


QFrame *PropertyDialog::createBasicInfoWidget(const AbstractFileInfoPointer &info)
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
        m_executableCheckBox = new QCheckBox;
        connect(m_executableCheckBox, &QCheckBox::toggled, this, &PropertyDialog::toggleFileExecutable);
        if(info->permission(QFile::ExeUser) || info->permission(QFile::ExeGroup) || info->permission(QFile::ExeOther)){
            m_executableCheckBox->setChecked(true);
        }
        layout->addRow(m_executableCheckBox, executableLabel);
    }
    widget->setLayout(layout);

    return widget;
}

QFrame *PropertyDialog::createLocalDeviceInfoWidget(const DUrl &url)
{
    QStorageInfo info(url.path());
    const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(url);
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

QFrame *PropertyDialog::createDeviceInfoWidget(UDiskDeviceInfo *info)
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

QListWidget *PropertyDialog::createOpenWithListWidget(const AbstractFileInfoPointer &info)
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

QFrame *PropertyDialog::createAuthorityManagermentWidget(const AbstractFileInfoPointer &info)
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
