#ifndef SHAREINFO_H
#define SHAREINFO_H

#include <QString>
#include <QFile>
#include <QDebug>

class ShareInfo
{
public:
    ShareInfo(QString shareName = "",
              QString path = "",
              QString comment = "",
              bool isWritable = false,
              bool isGuestOk = false);
    ~ShareInfo();

    QString shareName() const;
    void setShareName(const QString &shareName);

    QString path() const;
    void setPath(const QString &path);

    QString comment() const;
    void setComment(const QString &comment);

    QString usershare_acl() const;
    void setUsershare_acl(const QString &usershare_acl);

    QString guest_ok() const;
    void setGuest_ok(const QString &guest_ok);

    bool isWritable() const;
    void setIsWritable(bool isWritable);

    bool isGuestOk() const;
    void setIsGuestOk(bool isGuestOk);

    bool isValid();

private:
    QString m_shareName;
    QString m_path;
    QString m_comment;
    QString m_usershare_acl;
    QString m_guest_ok;

    bool m_isWritable = false;
    bool m_isGuestOk = false;
};

typedef QList<ShareInfo> ShareInfoList;

QDebug operator<<(QDebug dbg, const ShareInfo &obj);

#endif // SHAREINFO_H
