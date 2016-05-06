#include "propertydialog.h"

#include "fileinfo.h"

#include <../controllers/fileservices.h>

#include <dseparatorhorizontal.h>
#include <darrowlineexpand.h>
#include <dthememanager.h>
#include <dcombobox.h>
#include <dbaseexpand.h>
#include <dexpandgroup.h>

#include <QTextEdit>
#include <QFormLayout>
#include <QDateTime>

QWidget *createBasicInfoWidget(const AbstractFileInfoPointer &info)
{
    QWidget *widget = new QWidget;
    QFormLayout *layout = new QFormLayout;

    layout->setLabelAlignment(Qt::AlignRight);
    layout->addRow(QObject::tr("Size"), new QLabel(info->sizeDisplayName()));
    layout->addRow(QObject::tr("Type"), new QLabel(info->mimeTypeDisplayName()));
    layout->addRow(QObject::tr("Created Date"), new QLabel(info->createdDisplayName()));
    layout->addRow(QObject::tr("Modified Date"), new QLabel(info->lastModifiedDisplayName()));

    widget->setLayout(layout);

    return widget;
}

QWidget *createAuthorityManagermentWidget(const AbstractFileInfoPointer &info)
{
    QWidget *widget = new QWidget;
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

PropertyDialog::PropertyDialog(const DUrl &url, QWidget* parent)
    : BaseDialog(parent)
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

    QStringList titleList = QStringList() << tr("Basic Info") << tr("Authority Management") << tr("Opens");
    DExpandGroup *expandGroup = addExpandWidget(titleList);

    layout->addStretch();

    expandGroup->expand(0)->setContent(createBasicInfoWidget(fileInfo));
    expandGroup->expand(1)->setContent(createAuthorityManagermentWidget(fileInfo));
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
