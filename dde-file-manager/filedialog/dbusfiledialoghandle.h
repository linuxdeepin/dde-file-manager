#ifndef DBUSFILEDIALOGHANDLE_H
#define DBUSFILEDIALOGHANDLE_H

#include "dfiledialoghandle.h"

class DBusFileDialogHandle : public DFileDialogHandle
{
    Q_OBJECT

    Q_PROPERTY(QString directory READ directory WRITE setDirectory)
    Q_PROPERTY(QString directoryUrl READ directoryUrl WRITE setDirectoryUrl)
    Q_PROPERTY(QStringList nameFilters READ nameFilters WRITE setNameFilters)
    Q_PROPERTY(int filter READ filter WRITE setFilter)
    Q_PROPERTY(int viewMode READ viewMode WRITE setViewMode)
    Q_PROPERTY(int acceptMode READ acceptMode WRITE setAcceptMode)

public:
    explicit DBusFileDialogHandle(QWidget *parent = 0);

    QString directory() const;

    void setDirectoryUrl(const QString & directory);
    QString directoryUrl() const;

    void selectUrl(const QString &url);
    QStringList selectedUrls() const;

    int filter() const;
    void setFilter(int filters);

    void setViewMode(int mode);
    int viewMode() const;

    void setFileMode(int mode);

    void setAcceptMode(int mode);
    int acceptMode() const;

    void setLabelText(int label, const QString &text);
    QString labelText(int label) const;

    void setOptions(int options);

    WId winId() const;
    void setWindowTitle(const QString &title);
};

#endif // DBUSFILEDIALOGHANDLE_H
