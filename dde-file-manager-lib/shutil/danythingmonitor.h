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


#include <QObject>
#include <QString>

#ifdef __cplusplus
extern "C"
{
#endif //__cplusplus

#include <deepin-anything/fs_buf.h>

#ifdef __cplusplus
}
#endif //__cplusplus



QT_DEPRECATED class DAnythingMonitor final : public QObject
{
    Q_OBJECT
public:
    explicit DAnythingMonitor(QObject* const parent = nullptr);
    DAnythingMonitor(const DAnythingMonitor& other)=delete;
    DAnythingMonitor& operator=(const DAnythingMonitor& other)=delete;
    virtual ~DAnythingMonitor()=default;


    void workSignal();
    void doWork();

private:

    void notify() noexcept;

    ///###: why delete and rename operation use the same data structure.
    ///###: if use different data structure. Maybe the deleting was previous of renaming.
    ///###  but we synchronize with sqlite through these data tructure. Maybe the renamed files were deleted.
    ///###  Or, reverse.
    std::deque<std::pair<QString, QString>> m_changedFiles{};

    std::mutex m_mutex{};
    std::condition_variable m_conditionVar{};

    std::atomic<bool> m_readyFlag{ false };
};

#endif // FILEMONITOR_H
