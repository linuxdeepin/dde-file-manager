// SPDX-FileCopyrightText: 2026 UnionTech Software Technology Co., Ltd.
//
// SPDX-License-Identifier: GPL-3.0-or-later

#include "ocrextractor.h"

#include <dfm-base/base/configs/dconfig/dconfigmanager.h>

#include <DOcr>

#include <QFileInfo>
#include <QImage>
#include <QStringList>

EXTRACTOR_PLUGIN_BEGIN_NAMESPACE

namespace {

using DFMBASE_NAMESPACE::DConfigManager;

constexpr char kTextIndexSchema[] = "org.deepin.dde.file-manager.textindex";
constexpr char kSupportedOcrImageExtensions[] = "supportedOcrImageExtensions";
constexpr char kMaxOcrImageSizeMB[] = "maxOcrImageSizeMB";
constexpr int kDefaultMaxOcrImageSizeMB = 30;
constexpr int kMaxAllowedOcrImageSizeMB = 1024;
constexpr int kImageMaxWidth = 640;
constexpr int kImageMaxHeight = 480;
constexpr char kPreferredOcrPlugin[] = "PPOCR_V5";

QStringList defaultSupportedImageExtensions()
{
    return {
        "ani", "bmp", "jpe", "jpeg", "jpg", "pcx", "png", "psd",
        "tga", "tif", "tiff", "webp", "wmf", "heic", "heif", "raw"
    };
}

void ensureTextIndexConfigLoaded()
{
    static const bool kConfigLoaded = [] {
        QString err;
        if (!DConfigManager::instance()->addConfig(QString::fromLatin1(kTextIndexSchema), &err))
            fmWarning() << "OcrExtractor::ensureTextIndexConfigLoaded: failed to load DConfig schema:" << err;
        return true;
    }();

    Q_UNUSED(kConfigLoaded)
}

QStringList supportedImageExtensions()
{
    ensureTextIndexConfigLoaded();

    return DConfigManager::instance()->value(QString::fromLatin1(kTextIndexSchema),
                                             QString::fromLatin1(kSupportedOcrImageExtensions),
                                             QVariant::fromValue(defaultSupportedImageExtensions()))
            .toStringList();
}

size_t configuredMaxImageBytes()
{
    ensureTextIndexConfigLoaded();

    const int maxSizeMB = DConfigManager::instance()->value(QString::fromLatin1(kTextIndexSchema),
                                                            QString::fromLatin1(kMaxOcrImageSizeMB),
                                                            kDefaultMaxOcrImageSizeMB)
                                  .toInt();

    if (maxSizeMB <= 0 || maxSizeMB > kMaxAllowedOcrImageSizeMB) {
        fmWarning() << "OcrExtractor::configuredMaxImageBytes: invalid maxOcrImageSizeMB value:" << maxSizeMB
                    << ", using default:" << kDefaultMaxOcrImageSizeMB;
        return OcrExtractor::kDefaultMaxImageBytes;
    }

    return static_cast<size_t>(maxSizeMB) * 1024 * 1024;
}

bool hasSupportedExtension(const QString &filePath)
{
    const QString suffix = QFileInfo(filePath).suffix().toLower();
    return !suffix.isEmpty() && supportedImageExtensions().contains(suffix);
}

bool isWithinSizeLimit(const QString &filePath)
{
    const QFileInfo fileInfo(filePath);
    if (!fileInfo.exists() || !fileInfo.isFile())
        return false;

    const qint64 fileSize = fileInfo.size();
    if (fileSize < 0)
        return false;

    const size_t maxBytes = configuredMaxImageBytes();
    if (static_cast<quint64>(fileSize) > maxBytes) {
        fmInfo() << "OcrExtractor::isWithinSizeLimit: skip oversized image:" << filePath
                 << ", size:" << fileSize << ", max:" << maxBytes;
        return false;
    }

    return true;
}

QImage scaledImageForOcr(const QString &filePath)
{
    QImage image(filePath);
    if (image.isNull()) {
        fmWarning() << "OcrExtractor::scaledImageForOcr: failed to load image:" << filePath;
        return {};
    }

    const int width = image.width();
    const int height = image.height();

    if (width >= height) {
        if (width > kImageMaxWidth)
            image = image.scaledToWidth(kImageMaxWidth, Qt::SmoothTransformation);
    } else if (height > kImageMaxHeight) {
        image = image.scaledToHeight(kImageMaxHeight, Qt::SmoothTransformation);
    }

    return image;
}

bool initializeOcrEngine(Dtk::Ocr::DOcr &ocr)
{
    const QString preferredPlugin = QString::fromLatin1(kPreferredOcrPlugin);
    bool loaded = false;
    const QStringList plugins = ocr.installedPluginNames();
    if (plugins.contains(preferredPlugin, Qt::CaseInsensitive)) {
        loaded = ocr.loadPlugin(preferredPlugin);
        if (loaded)
            fmInfo() << "OcrExtractor::initializeOcrEngine: loaded OCR plugin:" << preferredPlugin;
        else
            fmWarning() << "OcrExtractor::initializeOcrEngine: failed to load OCR plugin:" << preferredPlugin;
    }

    if (!loaded) {
        loaded = ocr.loadDefaultPlugin();
        if (loaded) {
            fmInfo() << "OcrExtractor::initializeOcrEngine: loaded default OCR plugin";
        } else {
            fmWarning() << "OcrExtractor::initializeOcrEngine: failed to load default OCR plugin";
        }
    }

    if (loaded)
        ocr.setUseMaxThreadsCount(2);

    return loaded;
}

bool isSymbolOrPunctuation(const QChar &ch)
{
    switch (ch.category()) {
    case QChar::Punctuation_Connector:
    case QChar::Punctuation_Dash:
    case QChar::Punctuation_Open:
    case QChar::Punctuation_Close:
    case QChar::Punctuation_InitialQuote:
    case QChar::Punctuation_FinalQuote:
    case QChar::Punctuation_Other:
    case QChar::Symbol_Math:
    case QChar::Symbol_Currency:
    case QChar::Symbol_Modifier:
    case QChar::Symbol_Other:
        return true;
    default:
        return false;
    }
}

Dtk::Ocr::DOcr *sharedOcrEngine()
{
    static Dtk::Ocr::DOcr *ocr = []() -> Dtk::Ocr::DOcr * {
        auto *engine = new Dtk::Ocr::DOcr;
        if (!initializeOcrEngine(*engine)) {
            delete engine;
            return nullptr;
        }

        return engine;
    }();

    return ocr;
}

bool isNoiseLine(const QString &line)
{
    bool hasVisibleChar = false;
    bool hasLetterOrNumber = false;
    bool allVisibleCharsAreSymbols = true;
    int visibleCharCount = 0;

    for (const QChar &ch : line) {
        if (ch.isSpace())
            continue;

        hasVisibleChar = true;
        ++visibleCharCount;

        if (ch.isLetterOrNumber()) {
            hasLetterOrNumber = true;
            allVisibleCharsAreSymbols = false;
            break;
        }

        if (!isSymbolOrPunctuation(ch))
            allVisibleCharsAreSymbols = false;
    }

    if (!hasVisibleChar || hasLetterOrNumber)
        return false;

    // Filter common OCR noise like "□", "_" or a short run of punctuation.
    return allVisibleCharsAreSymbols && visibleCharCount <= 3;
}

QString cleanRecognizedText(const QString &text)
{
    QStringList cleanedLines;
    const QStringList lines = text.split('\n');

    for (const QString &line : lines) {
        const QString trimmed = line.trimmed();
        if (trimmed.isEmpty())
            continue;

        if (isNoiseLine(trimmed))
            continue;

        cleanedLines.append(trimmed);
    }

    return cleanedLines.join('\n');
}

}   // namespace

std::optional<QByteArray> OcrExtractor::extract(const QString &filePath)
{
    if (!isSupportedImage(filePath)) {
        fmDebug() << "OcrExtractor::extract: unsupported OCR image:" << filePath;
        return std::nullopt;
    }

    QImage image = scaledImageForOcr(filePath);
    if (image.isNull())
        return std::nullopt;

    Dtk::Ocr::DOcr *ocr = sharedOcrEngine();
    if (!ocr) {
        fmWarning() << "OcrExtractor::extract: shared OCR engine is unavailable";
        return std::nullopt;
    }

    if (!ocr->setImage(image)) {
        fmWarning() << "OcrExtractor::extract: failed to set OCR image:" << filePath;
        return std::nullopt;
    }

    if (!ocr->analyze()) {
        fmWarning() << "OcrExtractor::extract: OCR analyze failed for:" << filePath;
        return std::nullopt;
    }

    const QString cleanedText = cleanRecognizedText(ocr->simpleResult());
    if (cleanedText.size() < 2) {
        fmDebug() << "OcrExtractor::extract: cleaned OCR result has less than 2 characters for:" << filePath;
        return std::nullopt;
    }

    return cleanedText.toUtf8();
}

bool OcrExtractor::isSupportedImage(const QString &filePath)
{
    if (!hasSupportedExtension(filePath))
        return false;

    return isWithinSizeLimit(filePath);
}

EXTRACTOR_PLUGIN_END_NAMESPACE
