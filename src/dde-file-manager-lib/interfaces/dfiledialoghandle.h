// SPDX-FileCopyrightText: 2022 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#ifndef DFILEDIALOGHANDLE_H
#define DFILEDIALOGHANDLE_H

#include <QFileDialog>
#include <QScopedPointer>
#include <QDBusVariant>

class DFileDialogHandlePrivate;
class DFileDialogHandle : public QObject
{
    Q_OBJECT

public:
    explicit DFileDialogHandle(QWidget *parent = 0);
    ~DFileDialogHandle();

    void setParent(QWidget *parent);
    QWidget *widget() const;

    void setDirectory(const QString & directory);
    void setDirectory(const QDir & directory);
    QDir directory() const;

    void setDirectoryUrl(const QUrl & directory);
    QUrl directoryUrl() const;

    void selectFile(const QString &filename);
    QStringList selectedFiles() const;

    void selectUrl(const QUrl &url);
    QList<QUrl> selectedUrls() const;

    void addDisableUrlScheme(const QString &scheme);

    void setNameFilters(const QStringList &filters);
    QStringList nameFilters() const;
    void selectNameFilter(const QString &filter);
    QString selectedNameFilter() const;
    void selectNameFilterByIndex(int index);
    int selectedNameFilterIndex() const;

    QDir::Filters filter() const;
    void setFilter(QDir::Filters filters);

    void setViewMode(QFileDialog::ViewMode mode);
    QFileDialog::ViewMode viewMode() const;

    void setFileMode(QFileDialog::FileMode mode);

    void setAcceptMode(QFileDialog::AcceptMode mode);
    QFileDialog::AcceptMode acceptMode() const;

    void setLabelText(QFileDialog::DialogLabel label, const QString &text);
    QString labelText(QFileDialog::DialogLabel label) const;

    void setOptions(QFileDialog::Options options);
    void setOption(QFileDialog::Option option, bool on = true);
    QFileDialog::Options options() const;
    bool testOption(QFileDialog::Option option) const;

    void setCurrentInputName(const QString &name);
    void addCustomWidget(int type, const QString &data);
    QDBusVariant getCustomWidgetValue(int type, const QString &text) const;
    QVariantMap allCustomWidgetsValue(int type) const;
    void beginAddCustomWidget();
    void endAddCustomWidget();

    void setAllowMixedSelection(bool on);

    void setHideOnAccept(bool enable);
    bool hideOnAccept() const;

Q_SIGNALS:
    void finished(int result);
    void accepted();
    void rejected();
    void selectionFilesChanged();
    void currentUrlChanged();
    void selectedNameFilterChanged();

public Q_SLOTS:
    void show();
    void hide();
    void accept();
    void done(int r);
    int exec();
    void open();
    void reject();

private:
    QScopedPointer<DFileDialogHandlePrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), DFileDialogHandle)
    Q_DISABLE_COPY(DFileDialogHandle)
};

#endif // DFILEDIALOGHANDLE_H
