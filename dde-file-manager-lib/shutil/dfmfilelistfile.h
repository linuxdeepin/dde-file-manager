#pragma once

#include <QIODevice>
#include <QObject>

class DFMFileListFilePrivate;
class DFMFileListFile : public QObject
{
    Q_OBJECT
public:
    enum Status {
        NoError = 0, //!< No error occurred.
        NotExisted,  //!< Not existed when try to read/parse the file.
        AccessError, //!< An access error occurred (e.g. trying to write to a read-only file).
        FormatError  //!< A format error occurred (e.g. loading a malformed desktop entry file).
    };
    Q_ENUM(Status)

public:
    explicit DFMFileListFile(const QString & filePath, QObject * parent = nullptr);
    ~DFMFileListFile();

    bool save() const;

    bool contains(const QString & fileName) const;
    void insert(const QString & fileName);
    bool remove(const QString & fileName);

public slots:
    bool reload();

private:
    QScopedPointer<DFMFileListFilePrivate> d_ptr;

    Q_DECLARE_PRIVATE(DFMFileListFile)
};
