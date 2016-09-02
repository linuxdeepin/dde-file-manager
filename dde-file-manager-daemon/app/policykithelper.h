#ifndef POLICYKITHELPER_H
#define POLICYKITHELPER_H

#include <QObject>
#include <QCoreApplication>
#include <polkit-qt5-1/PolkitQt1/Authority>

using namespace PolkitQt1;

class PolicyKitHelper
{

public:
    inline static PolicyKitHelper* instance(){
        static PolicyKitHelper instance;
        return &instance;
    }

    bool checkAuthorization(const QString& actionId, qint64 applicationPid);


private:
    PolicyKitHelper();
    ~PolicyKitHelper();

    Q_DISABLE_COPY(PolicyKitHelper)
};

#endif // POLICYKITHELPER_H
