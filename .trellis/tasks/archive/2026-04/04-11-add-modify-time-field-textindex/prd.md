# Add Modify Time Field For TextIndex Builders

## Goal
Add modify-time numeric index fields to the textindex document builders and align the update check logic with the new Lucene field names.

## Requirements
- Add `kModifyTime` numeric fields to both `ContentDocumentBuilder` and `OcrDocumentBuilder`.
- Replace the legacy `L"modified"` string field in `ContentDocumentBuilder` with the new modify-time timestamp field.
- Update `taskhandler.cpp` `checkNeedUpdate()` to read the new modify-time field.
- Keep the change scoped to existing textindex indexing behavior.

## Acceptance Criteria
- [ ] Content index documents store modify time using `Content::kModifyTime`.
- [ ] OCR index documents store modify time using `OcrText::kModifyTime`.
- [ ] `checkNeedUpdate()` compares file timestamps against the new stored modify-time field.
- [ ] The code builds cleanly for the touched files.

## Technical Notes
- `dfm-search/field_names.h` already defines the new modify-time field constants.
- Existing birth-time numeric fields should remain unchanged.
