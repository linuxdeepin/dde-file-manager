# Add OCR Extractor Plugin

## Goal
Add a new extractor plugin under `dde-file-manager-extractor` that extracts text from images through OCR, following the existing plugin-based extractor architecture.

## Requirements
- Add a standalone `ocr-extractor` plugin similar to `text-extractor`.
- Use `assets/configs/org.deepin.dde.file-manager.textindex.json` for runtime limits.
- Add OCR-specific config keys for maximum image size and supported image extensions.
- Read the OCR-related config values in the plugin implementation and enforce them in runtime behavior.
- Implement OCR using `Dtk::Ocr::DOcr`, following the loading behavior used in `deepin-ocr`.
- Scale large images before OCR to avoid wasted work.
- Clean OCR output to remove empty lines and low-value symbol-only lines.
- Add required CMake and Debian dependencies for `dtk6ocr`.

## Acceptance Criteria
- [ ] A new OCR extractor plugin is built and installed with the extractor plugins.
- [ ] Supported image types are accepted only when they match configured extensions.
- [ ] Images larger than the configured limit are rejected by OCR extraction.
- [ ] OCR uses the default `deepin-ocr` style plugin loading flow.
- [ ] OCR text output is cleaned before returning UTF-8 bytes.
- [ ] Build configuration includes the required OCR library dependency.

## Technical Notes
- Reuse the existing extractor plugin interface without changing extractor IPC.
- Keep OCR language behavior aligned with `deepin-ocr` defaults.
- Keep text cleanup conservative to avoid deleting meaningful OCR output.
