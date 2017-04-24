#include "trashpropertydialog.h"
#include "dseparatorhorizontal.h"
#include "shutil/fileutils.h"
#include "dfileservices.h"
#include "shutil/filessizeworker.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QThread>
#include <QSvgRenderer>
#include <QPainter>
DWIDGET_USE_NAMESPACE

TrashPropertyDialog::TrashPropertyDialog(const DUrl& url, QWidget *parent) : BaseDialog(parent)
{
    m_url = url;
    setAttribute(Qt::WA_DeleteOnClose);
    setWindowFlags(windowFlags()
                           &~ Qt::WindowMaximizeButtonHint
                           &~ Qt::WindowMinimizeButtonHint
                           &~ Qt::WindowSystemMenuHint);
    initUI();
}

TrashPropertyDialog::~TrashPropertyDialog()
{

}

void TrashPropertyDialog::initUI()
{
    setFixedSize(320, 300);
    setTitle("");

    const DAbstractFileInfoPointer &fileInfo = DFileService::instance()->createFileInfo(this, m_url);
    QString iconPath;
    if(fileInfo->filesCount()>0)
        iconPath = ":/images/images/trash_full.png";
    else
        iconPath = ":/images/images/trash.png";

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(160, 160);
    m_iconLabel->setPixmap(QPixmap(iconPath));
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_nameLable = new QLabel(tr("Trash"), this);
    m_nameLable->setAlignment(Qt::AlignCenter);

    const int fCount = fileInfo->filesCount();
    QString itemStr = tr("item");
    if(fCount > 1)
        itemStr = tr("items");

    DSeparatorHorizontal* hLine = new DSeparatorHorizontal(this);
    m_countLabel = new QLabel(tr("Contains %1 %2").arg(QString::number(fCount),itemStr), this);
    m_sizeLabel = new QLabel(this);

    QHBoxLayout* infoLayout = new QHBoxLayout;
    infoLayout->addWidget(m_countLabel);
    infoLayout->addStretch();
    infoLayout->addWidget(m_sizeLabel);
    infoLayout->setContentsMargins(10, 0, 0, 0);

    QVBoxLayout* mainLayout = new QVBoxLayout;
    mainLayout->addWidget(m_iconLabel, 0, Qt::AlignHCenter);
    mainLayout->addWidget(m_nameLable, 0, Qt::AlignHCenter);
    mainLayout->addWidget(hLine);
    mainLayout->addLayout(infoLayout);
    mainLayout->setContentsMargins(10, 10, 10, 10);
    setLayout(mainLayout);

    startComputerFolderSize(m_url);
}

void TrashPropertyDialog::startComputerFolderSize(const DUrl &url)
{
    DUrlList urls;
    urls << url;
    FilesSizeWorker* worker = new FilesSizeWorker(urls);
    QThread*  workerThread = new QThread;
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, worker, &FilesSizeWorker::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

    connect(this, &TrashPropertyDialog::requestStartComputerFolderSize, worker, &FilesSizeWorker::coumpueteSize);
    connect(worker, &FilesSizeWorker::sizeUpdated, this, &TrashPropertyDialog::updateFolderSize);

    workerThread->start();

    emit requestStartComputerFolderSize();
}

void TrashPropertyDialog::updateFolderSize(qint64 size)
{
    m_sizeLabel->setText(FileUtils::formatSize(size));
}


