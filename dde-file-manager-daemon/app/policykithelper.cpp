#include "policykithelper.h"

bool PolicyKitHelper::checkAuthorization(const QString& actionId, qint64 applicationPid)
{
    Authority::Result result;
    result = Authority::instance()->checkAuthorizationSync(actionId, UnixProcessSubject(applicationPid), /// 第一个参数是需要验证的action，和规则文件写的保持一致
             Authority::AllowUserInteraction);
    if (result == Authority::Yes) {
        return true;
    }else {
        return false;
    }
}

PolicyKitHelper::PolicyKitHelper()
{

}

PolicyKitHelper::~PolicyKitHelper()
{

}
