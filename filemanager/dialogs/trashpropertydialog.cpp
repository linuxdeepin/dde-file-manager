#include "trashpropertydialog.h"
#include "dseparatorhorizontal.h"
#include "../shutil/fileutils.h"
#include "../controllers/fileservices.h"
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
    setFixedSize(360, 300);
    setTitle("");

    const AbstractFileInfoPointer &fileInfo = FileServices::instance()->createFileInfo(m_url);

    m_iconLabel = new QLabel(this);
    m_iconLabel->setFixedSize(160, 160);
    m_iconLabel->setPixmap(QPixmap(":/images/images/trash.png"));
    m_iconLabel->setAlignment(Qt::AlignCenter);
    m_nameLable = new QLabel(tr("Trash"), this);
    m_nameLable->setAlignment(Qt::AlignCenter);

    DSeparatorHorizontal* hLine = new DSeparatorHorizontal(this);
    m_countLabel = new QLabel(tr("Contains %1").arg(QString::number(fileInfo->filesCount())), this);
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

    startComputerFolderSize(fileInfo->absoluteFilePath());
}

void TrashPropertyDialog::startComputerFolderSize(const QString &dir)
{
    ComupterFolderSizeWorker* worker = new ComupterFolderSizeWorker(dir);
    QThread*  workerThread = new QThread;
    worker->moveToThread(workerThread);

    connect(workerThread, &QThread::finished, worker, &ComupterFolderSizeWorker::deleteLater);
    connect(workerThread, &QThread::finished, workerThread, &QThread::deleteLater);

    connect(this, &TrashPropertyDialog::requestStartComputerFolderSize, worker, &ComupterFolderSizeWorker::coumpueteSize);
    connect(worker, &ComupterFolderSizeWorker::sizeUpdated, this, &TrashPropertyDialog::updateFolderSize);

    workerThread->start();

    emit requestStartComputerFolderSize();
}

void TrashPropertyDialog::updateFolderSize(qint64 size)
{
    m_sizeLabel->setText(FileUtils::formatSize(size));
}

