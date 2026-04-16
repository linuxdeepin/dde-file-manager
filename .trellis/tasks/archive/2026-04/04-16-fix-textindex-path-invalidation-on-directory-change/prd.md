# brainstorm: fix textindex path invalidation on directory change

## Goal

修复 textindex 服务在目录重命名或目录移动过程中，因队列中的增量索引任务仍持有旧路径而导致的永久漏索引问题。目标是在尽量最小化修改、避免新增复杂抽象的前提下，让目录路径变化后的索引结果最终保持完整且一致。

## What I already know

* 当前 `FSEventCollector` 会把目录移动事件按 move 事件收集，最终通过 `requestProcessFileMoves` 下发到 `TaskManager::startFileMoveTask`。
* `TaskHandlers::MoveFileListHandler` 中，目录移动会走 `DirectoryMoveProcessor::processDirectoryMove`。
* `DirectoryMoveProcessor::processDirectoryMove` 只会搜索索引里当前已经存在的旧目录文档，并将这些文档路径改写到新目录。
* 如果目录下有大量文件正在等待 `CreateFileList` / `UpdateFileList` 之类的增量索引任务处理，而目录此时被重命名或移动，这些排队任务里记录的路径仍然是旧路径。
* 旧路径对应文件在任务真正执行时通常已不存在，导致这些文件更新失败；之后目录 move 任务只会迁移“已经入库”的文档，无法补回之前尚未入索引的文件。
* `TaskManager` 当前仅保存原始队列项，不存在“路径重写”或“move 感知”的机制。
* `CreateOrUpdateFileListHandler` 基于传入 `fileList` 直接遍历并更新文件，不会在执行前尝试纠正失效路径。

## Assumptions (temporary)

* 该问题的主战场是 `TaskManager` 队列与 `MoveFileListHandler` 的协作，而不是 Lucene 层索引迁移本身的算法错误。
* 目录路径变化后，最稳妥的修复应保证“未执行的旧路径任务”能在 move 后继续指向新路径，或者能被可靠补偿。
* 用户更看重最终索引完整性和低风险改动，而不是极致的即时性能最优。
* 本任务忽略单元测试，但仍应控制修改范围并保留现有职责边界。

## Requirements (evolving)

* 当目录重命名或移动发生在 textindex 增量索引期间时，目录内原本应被索引的文件最终不能因为旧路径失效而永久漏索引。
* 修复应优先复用现有 `TaskManager`、move processor、file-list handler 结构，避免引入新的大层级设计。
* 目录 move 后，已存在索引文档的路径迁移行为必须继续正确工作。
* 新逻辑需要尽量局限在 textindex task / fs event 相关模块，降低对其他索引流程的影响。
* MVP 需要同时覆盖：
  * 队列中尚未执行的旧路径增量任务自动跟随目录新路径。
  * 当前正在执行、因目录 move 导致旧路径失效而被中断/遗漏的目录内文件，后续能获得补偿索引。

## Acceptance Criteria (evolving)

* [ ] 当目录下有待索引文件排队时，目录被重命名后，这批文件最终仍能在新路径下完成索引。
* [ ] 当目录下有待索引文件排队时，目录被移动后，这批文件最终仍能在新路径下完成索引。
* [ ] 已经存在于索引中的旧目录文档仍会被正确迁移到新目录。
* [ ] 修复不要求引入新的公开接口层或大规模重构。
* [ ] 对正在执行中的增量任务，即使部分旧路径在目录 move 后立即失效，也会通过补偿任务在新路径下补齐遗漏文件。

## Definition of Done (team quality bar)

* 忽略单元测试
* 代码改动保持最小且职责清晰
* 构建/静态检查若能在本地执行则通过
* 行为变化在任务记录中清楚说明

## Out of Scope (explicit)

* 全量重构任务调度模型
* 为索引任务引入持久化队列系统
* 新增复杂事件溯源机制
* 本轮补齐单元测试

## Technical Approach

优先在 `TaskManager` 内完成修复，而不是扩散到更多 handler 或引入新的全局服务：

* 对队列中的 `TaskQueueItem` 增加目录 move 感知能力。
* 当收到目录 move 任务时，先把队列里仍引用旧目录前缀的 `fileList` / `pathList` / `movedFiles` 重写到新路径，保证尚未执行的任务不会继续使用失效路径。
* 对“当前正在执行”的增量任务，不尝试侵入式追踪每个已完成/未完成文件，而是在目录 move 事件进入任务系统时，为该目录追加一个基于新目录的补偿性增量扫描任务，用最小改动兜住遗漏文件。
* 保持 `DirectoryMoveProcessor` 现有职责不变，它继续只处理“已存在索引文档”的路径迁移；“未入索引文件补偿”由任务调度层保证。

## Decision (ADR-lite)

**Context**: 目录 move 时，Lucene 中已存在文档和任务队列中的待处理路径处于两个不同状态层。仅依靠 `DirectoryMoveProcessor` 迁移现有文档，无法覆盖排队任务和执行中任务的旧路径失效问题。

**Decision**: 采用“任务队列路径重写 + 目录级补偿增量任务”的组合方案，不新增公开接口，不重构 handler 层职责。

**Consequences**:

* 优点：
  * 修改面集中在 `TaskManager` 附近，符合最小修改原则。
  * 已有索引迁移和未入索引补偿分层清晰，职责不混乱。
  * 不需要引入复杂的运行时全局 move 映射或持久化恢复机制。
* 代价：
  * 补偿任务可能对被 move 的目录执行一次额外增量扫描。
  * 对“正在执行任务”的补偿是目录级兜底，而非精确恢复到中断点。
* 风险控制：
  * 只在目录 move 发生时追加补偿，避免扩大日常任务开销。
  * 维持现有 DBus / handler 接口签名不变，降低回归风险。

## Technical Notes

* 已检查文件：
  * `src/services/textindex/fsmonitor/fseventcollector.cpp`
  * `src/services/textindex/task/taskmanager.h`
  * `src/services/textindex/task/taskmanager.cpp`
  * `src/services/textindex/task/taskhandler.cpp`
  * `src/services/textindex/task/moveprocessor.cpp`
* 当前关键链路：
  * `FSEventCollector::filesMoved`
  * `TextIndexDBus::ProcessFileMoves` / `OcrIndexDBus::ProcessFileMoves`
  * `TaskManager::startFileMoveTask`
  * `TaskHandlers::MoveFileListHandler`
  * `DirectoryMoveProcessor::processDirectoryMove`
* 备选方案对比结论：
  * 队列路径重写适合解决“尚未执行”的过时路径。
  * 目录级补偿任务适合兜住“执行中被 move 打断”的遗漏。
  * 执行时懒修正方案需要跨层共享 move 映射，侵入性和隐式耦合更高，因此本轮不选。
