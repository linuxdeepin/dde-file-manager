
#include "tag/tagmanager.h"
#include "danythingmonitor.h"
#include "dfileinfo.h"

#include <sys/xattr.h>

#include <QString>



#define RENAME "file-renamed"
#define DELETE "file-deleted"

static QString StartPoint{};

DAnythingMonitor::DAnythingMonitor(const QString& hookPath, QObject* const parent)
             :QObject{ parent },
              m_hookFilePath{ hookPath }
{
    std::function<void(DAnythingMonitor* const monitor)> worker{ &DAnythingMonitor::doWork };
    m_workThread = std::thread{std::move(worker), this};
}

DAnythingMonitor::~DAnythingMonitor()
{
    if(m_iStream.is_open()){
        m_iStream.close();
    }

    if(m_workThread.joinable()){
        m_workThread.join();
    }
}

const QString& DAnythingMonitor::getVfsChangePath()const noexcept
{
    return m_hookFilePath;
}

void DAnythingMonitor::setVfsChangePath(const QString& path)
{
    m_hookFilePath = path;
}

void DAnythingMonitor::readVfsChange()
{
    if(!m_hookFilePath.isEmpty() && DFileInfo::exists(DUrl::fromLocalFile(m_hookFilePath))){
        m_iStream.open(m_hookFilePath.toStdString());
        std::list<std::basic_string<char>> lineContents{};
        std::basic_string<char> lineContent{};
        std::list<std::match_results<std::basic_string<char>::const_iterator>> results{};
        std::basic_string<char> oldEnd{ m_currentEnd }; //###: get the point of last reading.

        ///###: start reading file.
        while(std::getline(m_iStream, lineContent)){

            if(!lineContent.empty()){
                lineContents.emplace_back(std::move(lineContent));
            }

        }

        for(const std::basic_string<char>& content : lineContents){
            std::match_results<std::basic_string<char>::const_iterator> result{};

            if(std::regex_search(content, result, m_regexDate)){
                results.emplace_back(std::move(result));
            }

        }


        std::list<std::match_results<
                std::basic_string<char>::const_iterator>>::const_iterator cbeg{ results.cbegin() };
        std::list<std::match_results<
                std::basic_string<char>::const_iterator>>::const_iterator cend{ results.cend() };
        m_currentEnd = (--cend)->str(0);
        cend = results.cend();

        std::list<std::match_results<
                std::basic_string<char>::const_iterator>>::const_iterator oldPos{};
        if(!oldEnd.empty()){

            oldPos = std::find_if(cbeg, cend, [&](const std::match_results<std::basic_string<char>::const_iterator>& rlt)->bool
            {
                if(oldEnd == rlt.str(0)){
                    return true;
                }
                return false;
            }
            );
        }else{
            oldPos = results.cend();
        }


        if(oldPos == cend){
            std::lock_guard<std::mutex> raiiLock{ m_mutex };

            for(const std::match_results<std::basic_string<char>::const_iterator>& rlt : results){
                QString suffixStr{ QString::fromStdString( rlt.suffix().str() ) };
                suffixStr = suffixStr.trimmed();
                QList<QString> pieces{ suffixStr.split(" ") };

                if(pieces[0] == DELETE){
                    m_filesForDelete.emplace_back(pieces[1]);
                }else if(pieces[0] == RENAME){
                    m_filesForRename.emplace_back(pieces[1], pieces[2]);
                }
            }
            m_readyFlag.store(true, std::memory_order_release);

        }else{ //###: oldPos != cend
            std::lock_guard<std::mutex> raiiLock{ m_mutex };

            for(; oldPos != cend; ++oldPos){

                QString suffixStr{ QString::fromStdString(oldPos->suffix().str()) };

                suffixStr = suffixStr.trimmed();
                QList<QString> pieces{ suffixStr.split(" ") };

                if(pieces[0] == DELETE){
                    m_filesForDelete.emplace_back(pieces[1]);
                }else if(pieces[0] == RENAME){
                    m_filesForRename.emplace_back(pieces[1], pieces[2]);
                }

            }

            m_readyFlag.store(true, std::memory_order_release);
        }

        m_conditionVar.notify_one();
    }
}

void DAnythingMonitor::doWork()
{
    std::unique_lock<std::mutex> uniqueLock{ m_mutex };
    m_conditionVar.wait(uniqueLock, [&]{return  m_readyFlag.load(std::memory_order_consume);});
    m_readyFlag.store(false, std::memory_order_release);
    QSharedPointer<TagManager> tagManager{ TagManager::instance() };

    std::future<bool> resultOfDeleting{ std::async(std::launch::async,
                                                   &TagManager::deleteTags, tagManager, QList<QString>::fromStdList(m_filesForDelete)) };

    if(!m_filesForRename.empty()){

        for(const std::pair<QString, QString>& oldAndNewName : m_filesForRename){
            TagManager::instance()->changeTagName(QPair<QString, QString>{ oldAndNewName.first, oldAndNewName.second });
        }
    }

    resultOfDeleting.get();
    std::this_thread::sleep_for(std::chrono::duration<std::size_t, std::ratio<1, 1000>>{100});
}







