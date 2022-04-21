/*
 * Copyright (C) 2022 Uniontech Software Technology Co., Ltd.
 *
 * Author:     zhangsheng<zhangsheng@uniontech.com>
 *
 * Maintainer: max-lv<lvwujun@uniontech.com>
 *             lanxuesong<lanxuesong@uniontech.com>
 *             xushitong<xushitong@uniontech.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/
#ifndef FILEDIALOGHANDLE_H
#define FILEDIALOGHANDLE_H

#include <QFileDialog>
#include <QScopedPointer>
#include <QDBusVariant>

class FileDialogHandlePrivate;
class FileDialogHandle : public QObject
{
    Q_OBJECT

public:
    explicit FileDialogHandle(QWidget *parent = nullptr);
    ~FileDialogHandle();

public Q_SLOTS:
    void setParent(QWidget *parent);
    QWidget *widget() const;

    void setDirectory(const QString &directory);
    void setDirectory(const QDir &directory);
    QDir directory() const;

    void setDirectoryUrl(const QUrl &directory);
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

    void show();
    void hide();
    void accept();
    void done(int r);
    int exec();
    void open();
    void reject();

Q_SIGNALS:
    void finished(int result);
    void accepted();
    void rejected();
    void selectionFilesChanged();
    void currentUrlChanged();
    void selectedNameFilterChanged();

private:
    QScopedPointer<FileDialogHandlePrivate> d_ptr;

    Q_DECLARE_PRIVATE_D(qGetPtrHelper(d_ptr), FileDialogHandle)
    Q_DISABLE_COPY(FileDialogHandle)
};

#endif   // FILEDIALOGHANDLE_H
