#ifndef DESKTOPFILEINFO_H
#define DESKTOPFILEINFO_H

#include "fileinfo.h"

#include <QObject>

class DesktopFileInfo : public FileInfo
{
public:
    DesktopFileInfo();
    DesktopFileInfo(const QString & file);
    DesktopFileInfo(const QFileInfo &fileInfo);

    ~DesktopFileInfo();

    void setFile(const QString &file) Q_DECL_OVERRIDE;

    QString getName() const;
    QString getExec() const;
    QString getIcon() const;
    QString getType() const;
    QStringList getCategories() const;

private:
    QString name;
    QString exec;
    QString icon;
    QString type;
    QStringList categories;
    QStringList mimeType;

    void init(const QString &fileName);
};

#endif // DESKTOPFILEINFO_H
