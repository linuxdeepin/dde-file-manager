/*
 * Copyright (C) 2016 ~ 2018 Deepin Technology Co., Ltd.
 *               2016 ~ 2018 dragondjf
/*
 * Copyright (C) 2020 ~ 2021 Uniontech Software Technology Co., Ltd.
 *
 * Author:     gongheng<gongheng@uniontech.com>
 *
 * Maintainer: zhengyouge<zhengyouge@uniontech.com>
 *             gongheng<gongheng@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include "dmultifilepropertydialog.h"
#include "dfilestatisticsjob.h"
#include "dabstractfileinfo.h"
#include "shutil/fileutils.h"
#include "dfileservices.h"
#include "dfileinfo.h"

#include <map>
#include <array>
#include <random>
#include <thread>
#include <functional>

#include <QDebug>
#include <QLabel>
#include <QString>
#include <QProcess>
#include <QPainter>
#include <QVBoxLayout>
#include <QGridLayout>
#include <QHBoxLayout>
#include <QGraphicsView>
#include <QGraphicsScene>
#include <QGraphicsPixmapItem>
#include <QWindow>


using namespace Dtk::Widget;



class DMultiFilePropertyDialogPrivate
{
public:
    DMultiFilePropertyDialogPrivate(const QList<DUrl>& urlList, DMultiFilePropertyDialog* const qPtr);

    ~DMultiFilePropertyDialogPrivate()=default;
    DMultiFilePropertyDialogPrivate(const DMultiFilePropertyDialogPrivate& other)=delete;
    DMultiFilePropertyDialogPrivate& operator=(const DMultiFilePropertyDialogPrivate& other)=delete;


    void getRandomUrls()noexcept;

    void initUi();
    void initUiParameter()noexcept;
    void layoutItems()noexcept;


    QList<DUrl> m_urlList{}; //###: this list contains all the urls which are selected!
    std::array<DUrl, 3> m_selectedUrls{}; //###: I chose 3 urls from list.
    std::unique_ptr<QIcon> m_icon{ nullptr };

    QLabel* m_iconLabel{ nullptr };

    std::tuple<QLabel*, QLabel*> m_nameLabelAndLineLabel{};
    QLabel* m_basicInfoLabel{ nullptr };

    QHBoxLayout* m_basicInfoLayout{ nullptr };
    QLabel* m_folderSizeLabel{ nullptr };
    QGridLayout* m_gridLayout{ nullptr };
    QVBoxLayout* m_mainLayout{ nullptr };
    QFrame* m_mainFrame{ nullptr };


    std::shared_ptr<DFileStatisticsJob> m_filesSizeWorker{ nullptr };
    DMultiFilePropertyDialog* q_ptr{ nullptr };

};

DMultiFilePropertyDialogPrivate::DMultiFilePropertyDialogPrivate(const QList<DUrl> &urlList, DMultiFilePropertyDialog * const qPtr)
                                : m_urlList{urlList},
                                  q_ptr{ qPtr }
{
    this->getRandomUrls();

    this->initUi();
    this->initUiParameter();
    this->layoutItems();
}


void DMultiFilePropertyDialogPrivate::getRandomUrls()noexcept
{
    for(std::size_t index = 0; index != 3; ++index){
        std::size_t randomNumer{ DMultiFilePropertyDialog::getRandomNumber(std::make_pair(0, m_urlList.size() - 1)) };

        m_selectedUrls[index] = m_urlList[randomNumer];
    }

}


void DMultiFilePropertyDialogPrivate::initUi()
{

    m_icon = std::unique_ptr<QIcon>{ new QIcon{} };

    m_iconLabel = new QLabel{};

    m_nameLabelAndLineLabel = std::make_tuple(new QLabel{}, new QLabel{});

    m_basicInfoLabel = new QLabel;

    m_basicInfoLayout = new QHBoxLayout;

    m_folderSizeLabel = nullptr;

    m_gridLayout = new QGridLayout;
    m_mainLayout = new QVBoxLayout;
    m_mainFrame = new QFrame;
}



void DMultiFilePropertyDialogPrivate::initUiParameter()noexcept
{
    q_ptr->setFixedSize(300, 360);

    m_icon->addFile(QString{":/images/images/multiple_files.png"});
    m_icon->addFile(QString{":/images/images/multiple_files@2x.png"});

    m_iconLabel->setPixmap(m_icon->pixmap(128, 128));


    QLabel* nameLabel{ std::get<0>(m_nameLabelAndLineLabel) };
    nameLabel->setText(QObject::tr("Multiple Files"));
    nameLabel->setObjectName(QString{"NameLabel"});
    nameLabel->setStyleSheet(QString{"QLabel#NameLabel{ font-size: 12px;}"});

    QLabel* lineLabel{ std::get<1>(m_nameLabelAndLineLabel) };
    lineLabel->setFixedSize( QSize{260, 1} );
    lineLabel->setObjectName( QString{"LabelAsLine"} );
    lineLabel->setStyleSheet(QString{"QLabel#LabelAsLine{ border: none; background-color: #f0f0f0;}"});

    m_basicInfoLabel->setText(QObject::tr("Basic info"));
    m_basicInfoLabel->setObjectName(QString{"GridLayoutLabel"});
    m_basicInfoLabel->setObjectName(QString{"QLabel#GridLayoutLabel{ font-size: 12px;}"});

    std::pair<std::size_t, std::size_t> quantityOfFilesAndFolders{
                                         DMultiFilePropertyDialog::getTheQuantityOfFilesAndFolders(m_urlList)
                                                                 };
    std::list<std::pair<QString, QString>> coupleLabelContent;
    coupleLabelContent.emplace_back(QObject::tr("Total size"), QString{""});
    coupleLabelContent.emplace_back(QObject::tr("Number of files"), QObject::tr("%1 file(s), %2 folder(s)")
                                    .arg( QString::fromStdString(std::to_string(quantityOfFilesAndFolders.first)))
                                    .arg( QString::fromStdString(std::to_string(quantityOfFilesAndFolders.second))));


    coupleLabelContent.emplace_back(QObject::tr("Time accessed"), QString{"-"});
    coupleLabelContent.emplace_back(QObject::tr("Time modified"), QString{"-"});


    m_gridLayout->setColumnMinimumWidth(0, 100);
    m_gridLayout->setColumnMinimumWidth(1, 180);
    m_gridLayout->setSpacing(10);

    //add the couple of label to gridlayout.
    std::size_t row{ 0 };
    bool flag{ false };
    for(const auto& coupleContent : coupleLabelContent){
        QLabel* husbandLabel{ new QLabel{ coupleContent.first }  };
        QLabel* wifeLabel{ new QLabel{ coupleContent.second } };

        husbandLabel->setObjectName(QString{"HusbandLabel"});
        husbandLabel->setStyleSheet(QString{ "QLabel#HusbandLabel{ font-size: 11px;}"});

        if(flag == false){
            m_folderSizeLabel = wifeLabel; //###: this label for showing the quantity of current selected folders and files.
            flag = true;
        }

        wifeLabel->setObjectName(QString{"WifeLabel"});
        wifeLabel->setStyleSheet(QString{ "QLabel#WifeLabel{ font-size:11px;}"});
        wifeLabel->setTextFormat(Qt::PlainText);
        wifeLabel->setWordWrap(true);
        wifeLabel->setFixedWidth(180);



        m_gridLayout->addWidget(husbandLabel, row, 0, Qt::AlignRight | Qt::AlignTop);
        m_gridLayout->addWidget(wifeLabel, row, 1, Qt::AlignLeft | Qt::AlignTop);
        m_gridLayout->setRowMinimumHeight(row, 12);
        ++row;
    }

    m_filesSizeWorker =  std::shared_ptr<DFileStatisticsJob>{ new DFileStatisticsJob(q_ptr) };

    nameLabel = nullptr;
    lineLabel = nullptr;
}

void DMultiFilePropertyDialogPrivate::layoutItems()noexcept
{
    m_mainLayout->setSpacing(0);
    m_mainLayout->setContentsMargins(0, 0, 0, 0);
    m_mainLayout->setMargin(0);
    m_mainLayout->addWidget(m_iconLabel, 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addWidget(std::get<0>(m_nameLabelAndLineLabel), 0, Qt::AlignCenter);
    m_mainLayout->addSpacing(15);
    m_mainLayout->addWidget(std::get<1>(m_nameLabelAndLineLabel), 0, Qt::AlignCenter);

    m_basicInfoLayout->setSpacing(0);
    m_basicInfoLayout->addSpacing(10);
    m_basicInfoLayout->addWidget(m_basicInfoLabel);

    m_mainLayout->addLayout(m_basicInfoLayout);
    m_mainLayout->addSpacing(10);
    m_mainLayout->addLayout(m_gridLayout);
    m_mainLayout->addStretch(1);
    m_mainLayout->addSpacing(15);


    m_mainFrame->setLayout(m_mainLayout);
    q_ptr->addContent(m_mainFrame, Qt::AlignCenter);
}




DMultiFilePropertyDialog::DMultiFilePropertyDialog(const QList<DUrl> &urlList, QWidget * const parent)
                         :DDialog{ parent },
                          d_ptr{new DMultiFilePropertyDialogPrivate{ urlList, this} }
{
    if(DFMGlobal::isWayLand())
    {
        //设置对话框窗口最大最小化按钮隐藏
        this->setWindowFlags(this->windowFlags() & ~Qt::WindowMinMaxButtonsHint);
        this->setAttribute(Qt::WA_NativeWindow);
        //this->windowHandle()->setProperty("_d_dwayland_window-type", "wallpaper");
        this->windowHandle()->setProperty("_d_dwayland_minimizable", false);
        this->windowHandle()->setProperty("_d_dwayland_maximizable", false);
        this->windowHandle()->setProperty("_d_dwayland_resizable", false);
    }

    this->initConnect();
    this->startComputingFolderSize();
}

DMultiFilePropertyDialog::~DMultiFilePropertyDialog()
{
    DMultiFilePropertyDialogPrivate* const d{ d_func() };
    d->m_filesSizeWorker->stop();
}

std::size_t DMultiFilePropertyDialog::getRandomNumber(const std::pair<std::size_t, std::size_t> &scope)
{
    static std::random_device randomDevice; //###: here the device must be static.
    std::default_random_engine engine{ randomDevice() };
    std::uniform_int_distribution<std::size_t> distribution{scope.first, scope.second};

    return distribution(engine);
}


void DMultiFilePropertyDialog::updateFolderSizeLabel(const qint64 foldersSize)noexcept
{
    DMultiFilePropertyDialogPrivate* const d{ d_func() };
    d->m_folderSizeLabel->setText(FileUtils::formatSize(foldersSize));
}

void DMultiFilePropertyDialog::initConnect()
{
    DMultiFilePropertyDialogPrivate* const d{ d_func() };

    QObject::connect(d->m_filesSizeWorker.get(), &DFileStatisticsJob::dataNotify, this, &DMultiFilePropertyDialog::updateFolderSizeLabel);
}

void DMultiFilePropertyDialog::startComputingFolderSize()
{
    DMultiFilePropertyDialogPrivate* const d{ d_func() };

    d->m_filesSizeWorker->start(d->m_urlList);
}


///###: static function,compute the quantity of files and folders
std::pair<std::size_t, std::size_t> DMultiFilePropertyDialog::getTheQuantityOfFilesAndFolders(const QList<DUrl> &urlList)noexcept
{
    std::pair<std::size_t, std::size_t> quantityOfFilesAndFolders{0, 0};

    if (urlList.empty())
        return quantityOfFilesAndFolders;

    for (const auto& url : urlList) {

        DAbstractFileInfoPointer fileInfoPointer = DFileService::instance()->createFileInfo(nullptr, url);

        DUrl fileUrl = url;

        if (fileInfoPointer && fileInfoPointer->canRedirectionFileUrl())
            fileUrl = fileInfoPointer->redirectedFileUrl();

        QFileInfo info{ fileUrl.toLocalFile() };

        if (info.isFile()) {
            ++quantityOfFilesAndFolders.first;
        } else {
            ++quantityOfFilesAndFolders.second;
        }

    }

    return quantityOfFilesAndFolders;
}


