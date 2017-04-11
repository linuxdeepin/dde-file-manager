#include "popupcontrolwidget.h"

#include <QVBoxLayout>
#include <QProcess>
#include <QDebug>
#include <QDir>

#include <ddialog.h>
#include "dfmglobal.h"


DWIDGET_USE_NAMESPACE

const QString TrashDir = QDir::homePath() + "/.local/share/Trash";


PopupControlWidget::PopupControlWidget(QWidget *parent)
    : QWidget(parent),

      m_empty(false),

      m_fsWatcher(new QFileSystemWatcher(this))
{
    connect(m_fsWatcher, &QFileSystemWatcher::directoryChanged, this, &PopupControlWidget::trashStatusChanged, Qt::QueuedConnection);

    setObjectName("trash");
    setFixedWidth(80);

    trashStatusChanged();
}

bool PopupControlWidget::empty() const
{
    return m_empty;
}

int PopupControlWidget::trashItems() const
{
    return m_trashItemsCount;
}

QSize PopupControlWidget::sizeHint() const
{
    return QSize(width(), m_empty ? 30 : 60);
}

const QString PopupControlWidget::trashDir()
{
    return TrashDir;
}

void PopupControlWidget::openTrashFloder()
{
    QProcess *proc = new QProcess;

    connect(proc, static_cast<void (QProcess::*)(int)>(&QProcess::finished), proc, &QProcess::deleteLater);

    proc->startDetached("gvfs-open trash:///");
}

void PopupControlWidget::clearTrashFloder()
{
    // show confrim dialog
    qDebug() << "clear trash empty";
    DFMGlobal::instance()->clearTrash();
}

int PopupControlWidget::trashItemCount() const
{
    return QDir(TrashDir + "/info").entryInfoList().count() - 2;
}

void PopupControlWidget::trashStatusChanged()
{
    const bool files = QDir(TrashDir + "/files").exists();
//    const bool info = QDir(TrashDir + "/info").exists();

    // add monitor paths
    m_fsWatcher->addPath(TrashDir);
    if (files)
        m_fsWatcher->addPath(TrashDir + "/files");
//    if (info)
//        m_fsWatcher->addPath(TrashDir + "/info");

    // check empty
    if (!files)
        m_trashItemsCount = 0;
    else
        m_trashItemsCount = QDir(TrashDir + "/files").entryList().count() - 2;

    const bool empty = m_trashItemsCount == 0;
    if (m_empty == empty)
        return;

//    m_clearBtn->setVisible(!empty);
    m_empty = empty;

    setFixedHeight(sizeHint().height());

    emit emptyChanged(m_empty);
}
