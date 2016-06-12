#include "propertydialog.h"

#include "fileinfo.h"
#include "../app/global.h"
#include <../controllers/fileservices.h>
#include "../shutil/fileutils.h"
#include "../shutil/mimesappsmanager.h"
#include "../views/dscrollbar.h"
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


GroupTitleLabel::GroupTitleLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("GroupTitleLabel");
}


SectionKeyLabel::SectionKeyLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionKeyLabel");
}


SectionValueLabel::SectionValueLabel(const QString &text, QWidget *parent, Qt::WindowFlags f):
    QLabel(text, parent, f)
{
    setObjectName("SectionValueLabel");
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
    , m_edit(new QTextEdit)
{
    D_THEME_INIT_WIDGET(PropertyDialog)

    const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(url);

    m_icon->setPixmap(fileInfo->fileIcon().pixmap(100, 150));
    m_icon->setFixedHeight(150);

    m_edit->setPlainText(fileInfo->displayName());
    m_edit->setReadOnly(true);
    m_edit->setWordWrapMode(QTextOption::WrapAtWordBoundaryOrAnywhere);
    m_edit->setAlignment(Qt::AlignHCenter);
    m_edit->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_edit->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    m_edit->setFrameShape(QFrame::NoFrame);
    m_edit->setMaximumHeight(60);

    QVBoxLayout *layout = new QVBoxLayout(this);

    layout->setMargin(5);
    layout->setSpacing(0);
    layout->addWidget(m_icon, 0, Qt::AlignHCenter);
    layout->addWidget(m_edit, 0, Qt::AlignHCenter);
    layout->addWidget(new DSeparatorHorizontal);

    setLayout(layout);

    QStringList titleList;
    if (fileInfo->isFile()){
        titleList = QStringList() << tr("Basic Info") /*<< tr("Open with")*/;
    }else{
        titleList = QStringList() << tr("Basic Info");
    }
    DExpandGroup *expandGroup = addExpandWidget(titleList);

    layout->addStretch();

    m_basicInfoFrame = createBasicInfoWidget(fileInfo);

    expandGroup->expand(0)->setContent(m_basicInfoFrame);
    expandGroup->expand(0)->setExpand(true);


    if (fileInfo->isFile()){
//        m_OpenWithListWidget = createOpenWithListWidget(fileInfo);
//        expandGroup->expand(1)->setContent(m_OpenWithListWidget);
    }
    else if (fileInfo->isDir()){
        startComputerFolderSize(fileInfo->absoluteFilePath());
    }
}


void PropertyDialog::updateFolderSize(qint64 size)
{
    m_folderSizeLabel->setText(FileUtils::formatSize(size));
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

    return group;
}


QFrame *PropertyDialog::createBasicInfoWidget(const AbstractFileInfoPointer &info)
{
    QFrame *widget = new QFrame;
    SectionKeyLabel* sizeSectionLabel = new SectionKeyLabel(QObject::tr("Size"));
    SectionKeyLabel* fileAmountSectionLabel = new SectionKeyLabel(QObject::tr("File amount"));
    SectionKeyLabel* typeSectionLabel = new SectionKeyLabel(QObject::tr("Type"));
    SectionKeyLabel* TimeCreatedSectionLabel = new SectionKeyLabel(QObject::tr("Time created"));
    SectionKeyLabel* TimeModifiedSectionLabel = new SectionKeyLabel(QObject::tr("Time modified"));

    SectionValueLabel* sizeLabel = new SectionValueLabel(info->sizeDisplayName());
    m_folderSizeLabel = new SectionValueLabel;
    SectionValueLabel* typeLabel = new SectionValueLabel(info->mimeTypeDisplayName());
    SectionValueLabel* timeCreatedLabel = new SectionValueLabel(info->createdDisplayName());
    SectionValueLabel* timeModifiedLabel = new SectionValueLabel(info->lastModifiedDisplayName());
    SectionValueLabel* executableLabel = new SectionValueLabel(tr("Allow to execute as program"));


    QFormLayout *layout = new QFormLayout;
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

QListWidget *PropertyDialog::createOpenWithListWidget(const AbstractFileInfoPointer &info)
{
    QListWidget* listWidget = new QListWidget;
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

        QCheckBox* itemBox = new QCheckBox(mimeAppsManager->DesktopObjs.value(f).getName());
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


