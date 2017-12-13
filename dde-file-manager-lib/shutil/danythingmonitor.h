#ifndef FILEMONITOR_H
#define FILEMONITOR_H

#include <deque>
#include <regex>
#include <mutex>
#include <tuple>
#include <string>
#include <future>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <iterator>

#include <QObject>


class DAnythingMonitor : public QObject
{
    Q_OBJECT
public:
    explicit DAnythingMonitor(const QString& hookPath, QObject* const parent = nullptr);
    DAnythingMonitor(const DAnythingMonitor& other)=delete;
    DAnythingMonitor& operator=(const DAnythingMonitor& other)=delete;
    virtual ~DAnythingMonitor();

    const QString& getVfsChangePath()const noexcept;
    void setVfsChangePath(const QString& path)const=delete;
    void setVfsChangePath(const QString& path);
    void readVfsChange();

private:
    void doWork();

    std::list<QString> m_filesForDelete{};
    std::list<std::pair<QString, QString>> m_filesForRename{};


    std::basic_regex<char> m_regexDate{"([0-9]{4,4}-[0-9]{2,2}-[0-9]{2,2})"
                                       "(\\s)"
                                       "([0-9]{2,2}:[0-9]{2,2}:[0-9]{2,2}\\.[0-9]{3,3})"};

    std::mutex m_mutex{};
    std::condition_variable m_conditionVar{};
    std::basic_ifstream<char> m_iStream{};
    std::basic_string<char> m_currentEnd{};
    QString m_hookFilePath{""};

    std::atomic<bool> m_readyFlag{ true };
    std::thread m_workThread{};
};

#endif // FILEMONITOR_H
