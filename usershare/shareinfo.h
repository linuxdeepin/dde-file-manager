#ifndef SHAREINFO_H
#define SHAREINFO_H

#include <QString>

class ShareInfo
{
public:
    ShareInfo();
    ~ShareInfo();

    QString path;
    QString shareName;
    QString comment;
    bool isWritable = false;
    bool isGuestOk = false;
};

#endif // SHAREINFO_H
