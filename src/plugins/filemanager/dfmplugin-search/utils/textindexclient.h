#ifndef TEXTINDEXCLIENT_H
#define TEXTINDEXCLIENT_H

#include "dfmplugin_search_global.h"

#include <QObject>
#include <memory>
#include <optional>

class OrgDeepinFilemanagerTextIndexInterface;

DPSEARCH_BEGIN_NAMESPACE

class TextIndexClient : public QObject
{
    Q_OBJECT
public:
    enum class TaskType {
        Create,
        Update
    };
    Q_ENUM(TaskType)

    enum class ServiceStatus {
        Available,   // 服务正常可用
        Unavailable,   // 服务不可用
        Error   // 服务出错
    };
    Q_ENUM(ServiceStatus)

    static TextIndexClient *instance();

    // 异步方法，通过信号返回结果
    void startTask(TaskType type, const QString &path);

    // 返回值：
    // - std::nullopt: 服务不可用或出错
    // - true/false: 索引是否存在
    std::optional<bool> indexExists();

    // 检查服务状态
    ServiceStatus checkService();

Q_SIGNALS:
    void taskStarted(TaskType type, const QString &path);
    void taskFinished(TaskType type, const QString &path, bool success);
    void taskFailed(TaskType type, const QString &path, const QString &error);

private:
    explicit TextIndexClient(QObject *parent = nullptr);
    ~TextIndexClient();
    bool ensureInterface();

private:
    std::unique_ptr<OrgDeepinFilemanagerTextIndexInterface> interface;

private Q_SLOTS:
    void onDBusTaskFinished(const QString &type, const QString &path, bool success);
};

DPSEARCH_END_NAMESPACE

Q_DECLARE_METATYPE(dfmplugin_search::TextIndexClient::TaskType)

#endif   // TEXTINDEXCLIENT_H
