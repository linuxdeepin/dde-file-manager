#ifndef FILEMONITOR_H
#define FILEMONITOR_H


#include <deque>
#include <regex>
#include <mutex>
#include <tuple>
#include <memory>
#include <string>
#include <future>
#include <chrono>
#include <thread>
#include <fstream>
#include <sstream>
#include <iterator>
#include <unordered_map>

#include <QString>



#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

#include <deepin-anything/fs_buf.h>

#ifdef __cplusplus
}
#endif //__cplusplus



class DAnythingMonitor : public std::enable_shared_from_this<DAnythingMonitor>
{
public:
    explicit DAnythingMonitor();
    DAnythingMonitor(const DAnythingMonitor& other)=delete;
    DAnythingMonitor& operator=(const DAnythingMonitor& other)=delete;
    virtual ~DAnythingMonitor()=default;

    const QString& getVfsChangePath()const noexcept;
    void setVfsChangePath(const QString& path)const=delete;
    void setVfsChangePath(const QString& path);
    void workSignal();
    void doWork();

private:

    ///###: why delete and rename operation use the same data structure.
    ///###: if use different data structure. Maybe the deleting was previous of renaming.
    ///###  but we synchronize with sqlite through these data tructure. Maybe the renamed files were deleted.
    ///###  Or, reverse.
    std::deque<std::pair<QString, QString>> m_changedFiles{};

    std::mutex m_mutex{};
    std::condition_variable m_conditionVar{};

    std::atomic<bool> m_readyFlag{ false };
    std::thread m_workThread{};
};

#endif // FILEMONITOR_H
