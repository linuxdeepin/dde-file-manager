#ifndef DESKTOPFILEINFO_H
#define DESKTOPFILEINFO_H

#include "fileinfo.h"

#include <QObject>

#define DESKTOP_SURRIX "desktop"

class DesktopFileInfo : public FileInfo
{
public:
    DesktopFileInfo();
    DesktopFileInfo(const QString & fileUrl);
    DesktopFileInfo(const DUrl & fileUrl);
    DesktopFileInfo(const QFileInfo &fileInfo);

    ~DesktopFileInfo();

    void setFile(const DUrl &fileUrl) Q_DECL_OVERRIDE;

    QString getName() const;
    QString getExec() const;
    QString getIconName() const;
    QString getType() const;
    QStringList getCategories() const;

    QIcon fileIcon() const Q_DECL_OVERRIDE;
    QString displayName() const Q_DECL_OVERRIDE;

    static QMap<QString, QVariant> getDesktopFileInfo(const DUrl &fileUrl);

private:
    QString name;
    QString exec;
    QString iconName;
    QString type;
    QStringList categories;
    QStringList mimeType;

    void init(const DUrl &fileUrl);
};

#endif // DESKTOPFILEINFO_H
