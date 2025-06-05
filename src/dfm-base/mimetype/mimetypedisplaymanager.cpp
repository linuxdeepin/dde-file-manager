// SPDX-FileCopyrightText: 2021 - 2023 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "mimetypedisplaymanager.h"

#include <dfm-base/base/standardpaths.h>

#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm> // Required for std::sort

using namespace dfmbase;

MimeTypeDisplayManager::MimeTypeDisplayManager(QObject *parent)
    : QObject(parent)
{
    initData();
}

void MimeTypeDisplayManager::initData()
{
    displayNamesMap[FileInfo::FileType::kDirectory] = tr("Directory");
    displayNamesMap[FileInfo::FileType::kDesktopApplication] = tr("Application");
    displayNamesMap[FileInfo::FileType::kVideos] = tr("Video");
    displayNamesMap[FileInfo::FileType::kAudios] = tr("Audio");
    displayNamesMap[FileInfo::FileType::kImages] = tr("Image");
    displayNamesMap[FileInfo::FileType::kArchives] = tr("Archive");
    displayNamesMap[FileInfo::FileType::kDocuments] = tr("Text");
    displayNamesMap[FileInfo::FileType::kExecutable] = tr("Executable");
    displayNamesMap[FileInfo::FileType::kBackups] = tr("Backup file");
    displayNamesMap[FileInfo::FileType::kUnknown] = tr("Unknown");

    defaultIconNames[FileInfo::FileType::kDirectory] = "folder";
    defaultIconNames[FileInfo::FileType::kDesktopApplication] = "application-default-icon";
    defaultIconNames[FileInfo::FileType::kVideos] = "video";
    defaultIconNames[FileInfo::FileType::kAudios] = "music";
    defaultIconNames[FileInfo::FileType::kImages] = "image";
    defaultIconNames[FileInfo::FileType::kArchives] = "application-x-archive";
    defaultIconNames[FileInfo::FileType::kDocuments] = "text-plain";
    defaultIconNames[FileInfo::FileType::kExecutable] = "application-x-executable";
    defaultIconNames[FileInfo::FileType::kBackups] = "application-x-archive";   // generic backup file icon?
    defaultIconNames[FileInfo::FileType::kUnknown] = "application-default-icon";

    // Initialize extensionMap (all keys should be lowercase)
    // Desktop Applications
    extensionMap["desktop"] = FileInfo::FileType::kDesktopApplication;
    extensionMap["app"] = FileInfo::FileType::kDesktopApplication;

    // Videos
    QStringList videoExts = {
        "mp4", "avi", "mkv", "mov", "wmv", "flv", "webm", "mpeg", "mpg", "m4v", "3gp", 
        "ts", "mts", "m2ts", "vob", "ogv", "rm", "rmvb", "asf", "divx", "f4v", "mk3d", 
        "mxf", "nsv", "ogm", "qt", "tp", "vid", "yuv", "h264", "h265", "hevc", "264", "265",
        "swf", "dv", "mod", "tod", "dvr", "mjpeg", "mjpg"
    };
    for (const QString &ext : videoExts) extensionMap[ext.toLower()] = FileInfo::FileType::kVideos;

    // Audios
    QStringList audioExts = {
        "mp3", "wav", "ogg", "flac", "aac", "m4a", "wma", "ac3", "amr", "ape", "au", 
        "cda", "dts", "mka", "mid", "midi", "mpc", "oga", "opus", "ra", "snd", "spx",
        "aiff", "aifc", "alac", "pcm", "dsf", "dff", "wv", "it", "s3m", "xm", "mod", 
        "mpa", "mp2", "voc", "3ga", "m3u", "m3u8", "pls", "cue", "shn", "tak", "tta", "webm"
    };
    for (const QString &ext : audioExts) extensionMap[ext.toLower()] = FileInfo::FileType::kAudios;

    // Images
    QStringList imageExts = {
        "jpg", "jpeg", "png", "gif", "bmp", "svg", "webp", "tiff", "tif", "ico", "heic", "heif",
        "raw", "arw", "cr2", "nef", "orf", "sr2", "dng", "pef", "raf", "rw2", "rwl", "kdc", "dcr",
        "psd", "xcf", "ai", "cdr", "eps", "ps", "svgz", "jpe", "jfif", "jif", "jp2", "j2k", "jpx",
        "jpm", "mj2", "ppm", "pgm", "pbm", "pnm", "avif", "jxl", "pcx", "crw", "exr", "fpx", "tga",
        "dds", "hdr", "img", "mng", "pict", "sgi", "xbm", "xpm", "wmf", "emf", "djvu", "ani", "apng", "flif"
    };
    for (const QString &ext : imageExts) extensionMap[ext.toLower()] = FileInfo::FileType::kImages;

    // Archives (including compound extensions)
    QStringList archiveExts = {
        "zip", "gz", "bz2", "rar", "7z", "xz", "iso", "deb", "rpm", "jar", "war", "ear", "tar", "tar.gz", "tar.bz2", "tar.xz",
        "tgz", "tbz2", "txz", "ace", "apk", "arc", "arj", "cab", "lzh", "lha", "lz", "lz4", "lzma", "lzo", "rz", "z", "zst",
        "dmg", "img", "pk3", "pk4", "udf", "wim", "zap", "zpaq", "cpio", "pea", "sit", "sitx", "zoo", "zipx", "tlz", "uue",
        "xar", "alz", "egg", "ar", "par", "par2", "xpi", "crx", "taz", "tar.lzma", "tar.zst", "tar.lz", "tar.lz4", "shar", "vhd", "vmdk",
        "pkg", "hfs", "ipsw", "dar", "bz", "tbz", "qcow", "qcow2", "pacman", "ova", "gem", "whl", "nupkg", "cab"
    };
    for (const QString &ext : archiveExts) extensionMap[ext.toLower()] = FileInfo::FileType::kArchives;
    
    // Documents (including text, source code, markup, etc.)
    QStringList docExts = {
        // Common document formats
        "txt", "doc", "docx", "pdf", "odt", "rtf", "csv", "xls", "xlsx", "ppt", "pptx", "xlsm", "pptm", "docm",
        "md", "html", "htm", "xml", "json", "log", "tex", "ods", "odp", "odg", "epub", "mobi", "tsv", "sql",
        "djvu", "dot", "dotx", "xps", "ps", "wps", "wpt", "pages", "chm", "fb2", "azw", "azw3", "cbz", "cbr",
        "numbers", "keynote", "ttf", "otf", "woff", "woff2", "eot", "markdown", "rst", "org", "adoc", "fodt", "vsd", "vsdx",
        "fodp", "fods", "sxw", "stw", "wpd", "abw", "hwp", "zabw", "qxp", "indd", "pm6", "sdw", "mdb", "accdb", "key",
        // Programming and development files
        "c", "cpp", "h", "hpp", "java", "js", "ts", "cs", "go", "php", "rb", "swift", "kt", "kts", "py", "pl", "dart", "lua", "r", "scala", "groovy", "hs", "erl", "ex", "exs",
        "jsx", "tsx", "clj", "cljs", "cljc", "coffee", "elm", "f", "f90", "fs", "fsx", "lisp", "ml", "mli", "nim", "pas", "pp", "prolog", "rs", "sass", "scss", "vb", "asm", "s",
        "d", "jl", "rkt", "scm", "zig", "m", "mm", "v", "ada", "adb", "bb", "bmx", "cgis", "class", "cls", "cob", "cobol", "gd", "bat", "ps1", "psm1", "vbs", "wsf", "ahk", "au3",
        "apex", "cfc", "cfm", "e", "elm", "gml", "hx", "ino", "jade", "litcoffee", "mak", "monkey", "mq4", "mq5", "nsi", "nsh", "pde", "sol", "tcl", "xqy", "zsh", "vue", "svelte",
        // Shell scripts and related
        "pyw", "ksh", "csh", "tcsh",
        // Config and system files
        "ini", "conf", "cfg", "yaml", "yml", "toml", "sh", "bash", "zsh", "fish", "plist", "po", "pot",
        "bashrc", "profile", "zshrc", "gitconfig", "editorconfig", "vimrc", "gitignore", "env", "properties", "config",
        "service", "socket", "desktop", "rules", "nix", "dockerfile", "vagrantfile", "makefile", "cmake", "mk", "license", "xsd", "wsdl",
        "vcxproj", "csproj", "sln", "gradle", "pom", "maven", "ant", "iml", "project", "targets", "props", "policy", "manifest",
        "gemfile", "rakefile", "procfile", "sbt", "flake", "bzl", "bazel", "spec", "thrift", "eslintrc", "prettierrc", "stylelintrc",
        "babelrc", "npmrc", "yarnrc", "classpath", "htaccess", "htpasswd", "lock"
    };
    for (const QString &ext : docExts) extensionMap[ext.toLower()] = FileInfo::FileType::kDocuments;
    
    // Executables (specific binary executable formats only, not scripts)
    QStringList execExts = {
        "appimage", "run", "exe", "com", "msi", "pyc", "pyo", "flatpakref", "snap",
        "app", "bin", "out", "elf", "so", "o", "a", "dll", "sys", "scr", "ocx", "pyd", "dylib", "bundle",
        "msc", "cpl", "gadget", "msp", "action", "workflow", "xbe", "nexe", "wasm", "pex"
    };
    for (const QString &ext : execExts) extensionMap[ext.toLower()] = FileInfo::FileType::kExecutable;

    // Backups
    QStringList backupExts = {
        "bak", "old", "backup", "~", "swp", "swo", "tmp", "temp", "orig", 
        "save", "back", "copy", "sav", "autosave", "gho", "gbk", "bkp", "bk", "wbk", "vbk", "bck",
        "bkf", "abk", "dbk", ".git", "crdownload", "part", "partial", "cache", "log.1", "log.old", "timemachine",
        "000", "001", "002", "003", "dtapart", "dta", "timeshift", "sbk"
    };
    for (const QString &ext : backupExts) extensionMap[ext.toLower()] = FileInfo::FileType::kBackups;

    // Populate sortedExtensions (keys of extensionMap, sorted by length descending)
    sortedExtensions = extensionMap.keys();
    std::sort(sortedExtensions.begin(), sortedExtensions.end(),
              [](const QString &a, const QString &b) {
                  return a.length() > b.length();
              });

    loadSupportMimeTypes();
}

QString MimeTypeDisplayManager::displayName(const QString &mimeType)
{
#ifdef QT_DEBUG
    return displayNamesMap.value(displayNameToEnum(mimeType)) + " (" + mimeType + ")";
#endif   // Q_DEBUG
    return displayNamesMap.value(displayNameToEnum(mimeType));
}

FileInfo::FileType MimeTypeDisplayManager::displayNameToEnum(const QString &mimeType)
{
    if (mimeType == "application/x-desktop") {
        return FileInfo::FileType::kDesktopApplication;
    } else if (mimeType == "inode/directory") {
        return FileInfo::FileType::kDirectory;
    } else if (mimeType == "application/x-executable" || executableMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kExecutable;
    } else if (mimeType.startsWith("video/") || videoMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kVideos;
    } else if (mimeType.startsWith("audio/") || audioMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kAudios;
    } else if (mimeType.startsWith("image/") || imageMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kImages;
    } else if (mimeType.startsWith("text/") || textMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kDocuments;
    } else if (archiveMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kArchives;
    } else if (backupMimeTypes.contains(mimeType)) {
        return FileInfo::FileType::kBackups;
    } else {
        return FileInfo::FileType::kUnknown;
    }
}

// New method to determine FileType by extension (revised)
FileInfo::FileType MimeTypeDisplayManager::fileTypeByExtension(const QString &filePath)
{
    if (filePath.isEmpty()) {
        return FileInfo::FileType::kUnknown;
    }

    // Extract filename from path
    int lastSeparatorSlash = filePath.lastIndexOf('/');
    int lastSeparatorBackslash = filePath.lastIndexOf('\\');
    int lastSeparator = qMax(lastSeparatorSlash, lastSeparatorBackslash);
    QString filename = (lastSeparator == -1) ? filePath : filePath.mid(lastSeparator + 1);

    if (filename.isEmpty() || filename == QLatin1String(".") || filename == QLatin1String("..")) {
        return FileInfo::FileType::kUnknown;
    }

    QString filenameLc = filename.toLower();

    // Handle hidden files like ".bashrc" (single dot at the beginning, no other dots)
    if (filenameLc.startsWith('.') && filenameLc.indexOf('.', 1) == -1) {
        QString hiddenId = filenameLc.mid(1); // Get "bashrc" from ".bashrc"
        if (extensionMap.contains(hiddenId)) {
            return extensionMap.value(hiddenId);
        } else {
            // If the specific hidden file id (e.g. "bashrc") is not known,
            // treat it as unknown to prevent partial matches later.
            return FileInfo::FileType::kUnknown;
        }
    }

    // General extension matching (handles compound like "tar.gz" and single like "gz")
    // Iterates sortedExtensions (longest first, e.g., "tar.gz" before "gz")
    for (const QString &knownExt : sortedExtensions) {
        if (filenameLc.endsWith(knownExt)) {
            int extStartIndex = filenameLc.length() - knownExt.length();
            // Check if the character before the extension is a dot
            if (extStartIndex > 0 && filenameLc.at(extStartIndex - 1) == QChar('.')) {
                return extensionMap.value(knownExt);
            }
            // This part is removed: (extStartIndex == 0) implies filename IS knownExt.
            // An extension usually means it follows a dot.
            // If a file is named "gz", it's not "file.gz".
            // If "Makefile" needs to be typed, it shouldn't be in extensionMap
            // unless handled by hidden file logic (e.g. if it was ".makefile")
        }
    }

    return FileInfo::FileType::kUnknown;
}

QString MimeTypeDisplayManager::defaultIcon(const QString &mimeType)
{
    return defaultIconNames.value(displayNameToEnum(mimeType));
}

QString MimeTypeDisplayManager::displayTypeFromPath(const QString &filePath)
{
    // Use the existing fileTypeByExtension to determine file type from the path
    FileInfo::FileType fileType = fileTypeByExtension(filePath);
    
    // If the type is unknown, check if it's a directory
    if (fileType == FileInfo::FileType::kUnknown) {
        QFileInfo fileInfo(filePath);
        if (fileInfo.isDir()) {
            fileType = FileInfo::FileType::kDirectory;
        }
    }
    
    // Return the display name for this file type
    return displayNamesMap.value(fileType);
}

QMap<FileInfo::FileType, QString> MimeTypeDisplayManager::displayNames()
{
    return displayNamesMap;
}

QStringList MimeTypeDisplayManager::readlines(const QString &path)
{
    QStringList result;
    QFile file(path);
    if (!file.open(QIODevice::ReadOnly)) {
        return result;
    }
    QTextStream in(&file);
    while (!in.atEnd()) {
        // Read new line
        QString line = in.readLine();
        // Skip empty line or line with invalid format
        if (line.trimmed().isEmpty()) {
            continue;
        }
        result.append(line.trimmed());
    }
    file.close();
    return result;
}

void MimeTypeDisplayManager::loadSupportMimeTypes()
{
    QString textPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "text.mimetype");
    QString archivePath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "archive.mimetype");
    QString videoPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "video.mimetype");
    QString audioPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "audio.mimetype");
    QString imagePath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "image.mimetype");
    QString executablePath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "executable.mimetype");
    QString backupPath = QString("%1/%2").arg(StandardPaths::location(StandardPaths::kMimeTypePath), "backup.mimetype");
    textMimeTypes = readlines(textPath);
    archiveMimeTypes = readlines(archivePath);
    videoMimeTypes = readlines(videoPath);
    audioMimeTypes = readlines(audioPath);
    imageMimeTypes = readlines(imagePath);
    executableMimeTypes = readlines(executablePath);
    backupMimeTypes = readlines(backupPath);
}

QStringList MimeTypeDisplayManager::supportArchiveMimetypes()
{
    return archiveMimeTypes;
}

QStringList MimeTypeDisplayManager::supportVideoMimeTypes()
{
    return videoMimeTypes;
}

MimeTypeDisplayManager *MimeTypeDisplayManager::instance()
{
    static MimeTypeDisplayManager ins;
    return &ins;
}

QStringList MimeTypeDisplayManager::supportAudioMimeTypes()
{
    return audioMimeTypes;
}
