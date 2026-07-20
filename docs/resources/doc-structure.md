# 文档结构设计

> 这页定义文档站的信息架构，不讨论具体源码内容。目标只有一个：让“学习、查阅、核对、维护”四类动作各有明确入口，避免同一主题在不同目录下反复写一遍。

## 当前问题

### 1. 入口职责重复

- `study/**` 已经是主学习路径，但 `guide/**`、`architecture/**`、`resources/**` 也在承担“从哪里开始看”的入口职责。
- 结果是首页、学习首页、架构首页、指南页、资料页都在解释站点结构，用户需要先理解目录，再开始读内容。

### 2. 主题边界重复

- `study/**` 和 `architecture/source-analysis/**` 同时覆盖进程模型、实体系统、网络、持久化、空间、脚本等主题。
- 前者是章节主线，后者是源码深挖；角色本来不同，但现在目录名字和首页文案仍然容易被理解成“两套平行正文”。

### 3. “资料”与“引用”语义混杂

- `resources/**` 里既有原始资料列表，也有来源边界、核对状态。
- 它更接近“文档治理与校验”区域，而不是读者理解中的“参考资料库”。

### 4. 工程说明散落在根目录

- `docs/CODECOV_SETUP.md`
- `docs/CODE_COVERAGE_SETUP.md`
- `docs/COVERAGE_IMPROVEMENTS.md`
- `docs/NETWORK_IO_RESEARCH_README.md`
- `docs/python-3.12-upgrade-notes.md`
- `docs/vcpkg-migration-status.md`

这些页面属于工程维护记录，不应长期和面向读者的正文入口并列。

### 5. 背景材料与正文有交叉

- BigWorld 背景既出现在 `study/**` 主线中，也出现在 `architecture/bigworld/**` 中。
- 对照分析既出现在 `study/23-*`，也出现在 `architecture/comparison/**`。

这类内容可以同时存在，但必须明确：一个负责主线中的摘要，一个负责独立专题中的系统展开。

## 设计原则

### 单一主线

- 只有一个“从头读到尾”的学习入口，即 `study/**`。
- 其他目录都不能再伪装成第二套正文。

### 一处详写

- 同一个主题只保留一个“详细展开点”。
- 其他位置只做摘要、边界说明和跳转链接。

### 角色先于目录名

- 先定义页面职责，再决定物理目录。
- 即使暂时不改路径，也要先冻结逻辑分层。

### 原始资料和人工整理分离

- CHM/PDF 转写页、源码抽取页、人工分析页、设计草案页必须长期显式区分。
- “能否对原始资料逐页核对”要成为一级元信息。

### 面向任务导航

- 导航不按作者整理过程组织，而按读者任务组织：
  - 学习系统
  - 深挖机制
  - 查接口
  - 看背景对照
  - 查来源与校验
  - 看工程维护说明

## 目标结构

下面是推荐的目标信息架构。这里先定义逻辑层，不要求一次性完成物理迁移。

```text
docs/
├── index.md                      # 站点首页
├── learn/                        # 唯一主学习路径
│   └── ...                       # 当前 study/**
├── deep-dive/                    # 按问题继续深挖源码
│   └── ...                       # 当前 architecture/source-analysis/**
├── background/                   # 背景知识与对照分析
│   ├── bigworld/                 # 当前 architecture/bigworld/**
│   └── comparison/               # 当前 architecture/comparison/**
├── reference/                    # 工具书入口
│   └── api/                      # 当前 api/**
├── guides/                       # 操作型指南与工程说明
│   ├── webconsole.md
│   └── engineering/              # 构建、迁移、覆盖率等维护文档
├── meta/                         # 文档治理信息
│   ├── provenance.md
│   ├── verification-status.md
│   └── doc-structure.md
├── proposals/                    # 设计草案与重构提案
│   └── redesign.md
└── archive/                      # 历史材料
    └── book-legacy/**
```

## 当前目录到目标结构的映射

| 当前路径 | 目标角色 | 处理原则 |
| --- | --- | --- |
| `docs/study/**` | `learn/` | 保持唯一主线地位，不再与其他目录竞争入口。 |
| `docs/architecture/source-analysis/**` | `deep-dive/` | 保留专题深挖定位，不再写第二套总览。 |
| `docs/architecture/bigworld/**` | `background/bigworld/` | 负责背景模型，不重复 KBEngine 细节。 |
| `docs/architecture/comparison/**` | `background/comparison/` | 负责系统对照，不替代主线第 23 章。 |
| `docs/api/**` | `reference/api/` | 保持工具书定位。 |
| `docs/guide/webconsole.md` | `guides/` | 保留为操作型指南。 |
| `docs/resources/**` | `meta/` | 明确改成文档治理区，而不是泛化资料区。 |
| `docs/architecture/redesign.md` | `proposals/` | 明确是工程草案，不属于架构事实文档。 |
| `docs/book-legacy/**` | `archive/` | 只归档，不参与主导航。 |
| `docs/*.md` 中工程说明页 | `guides/engineering/` | 后续集中收拢。 |

## 顶层导航建议

推荐的顶层导航语义应当是：

1. 首页
2. 学习主线
3. 专题深挖
4. 背景与对照
5. API 参考
6. 使用与工程指南
7. 文档说明

对应到当前路径，可以先这样映射：

- 学习主线 → `/study/`
- 专题深挖 → `/architecture/source-analysis/`
- 背景与对照 → `/architecture/`
- API 参考 → `/api/`
- 使用与工程指南 → `/guide/`
- 文档说明 → `/resources/`

这样即使暂时不改目录名，站点对外表达也已经按职责收敛。

## 各区域的写作边界

### `study/**`

- 负责“按顺序建立全局理解”。
- 每章可以给出必要摘要，但不重复铺开专题细节。
- 出现深入问题时，跳到 `source-analysis/**`。

### `architecture/source-analysis/**`

- 负责“带着问题看源码”。
- 直接落到文件、类、函数、调用链。
- 不再承担整站导航，不再尝试重写 `study/**` 的导读内容。

### `architecture/bigworld/**` 与 `comparison/**`

- 负责背景模型、术语映射、设计来源、差异分析。
- 不重复解释 KBEngine 当前源码已经在 `study/**` 或 `source-analysis/**` 写过的实现细节。

### `api/**`

- 负责接口契约和工具书式回查。
- 不在 API 页里写长篇架构解释。

### `guide/**`

- 只保留可操作的使用指南和工程指南。
- 不再维护“全站阅读入口”这类职责。

### `resources/**`

- 只保留来源边界、校验状态、文档结构、维护规则。
- 不再充当读者意义上的“资料库”。

## 重复内容的收敛规则

### 架构类重复

- `study/**` 保留“为什么需要这套机制”与“主调用链摘要”。
- `source-analysis/**` 保留“代码怎么实现”与“边界在哪里”。
- `architecture/**` 首页只做地图，不再按主题写展开版正文。

### 学习类重复

- 所有“推荐阅读顺序”“建议先看哪里”的文案，主入口统一收敛到首页和 `study/**`。
- 其他目录最多保留一句角色说明和一个跳转链接。

### 资料 / 引用类重复

- “资料来源”放 `resources/provenance.md`。
- “校验结果”放 `resources/verification-status.md`。
- “为什么目录这样设计”放本页。
- 如果后续需要“参考书目 / 外部链接”，应单独建 `references.md`，不要继续塞进 `resources/index.md`。

## 推荐迁移顺序

### Phase 1：先收敛职责，不动大量路径

- 固定 `study/**` 是唯一主线。
- 固定 `guide/**` 只做指南。
- 固定 `resources/**` 只做文档治理。
- 固定 `architecture/**` 只做背景、地图和专题入口。

### Phase 2：把重复页改成“摘要 + 跳转”

- 对 `study/**` 与 `source-analysis/**` 的重叠主题，保留一处详写。
- 对 `architecture/index.md`、`guide/index.md`、`resources/index.md` 的导航文案做减法。

### Phase 3：做物理迁移

- 把根目录工程说明页迁到 `guides/engineering/`。
- 把 `redesign.md` 移到 `proposals/`。
- 把 `book-legacy/**` 明确降级为 `archive/`。

### Phase 4：最后再考虑目录改名

- 如果需要再把 `study / resources / guide / architecture/source-analysis` 物理重命名成 `learn / meta / guides / deep-dive`。
- 在这之前，优先保持链接稳定，避免一次性大规模重定向。

## 这次整理后的落地结论

- 现在不建议立刻做大规模目录移动。
- 更合理的做法是先冻结“逻辑结构”和“页面职责”，再逐批清理重复内容。
- 只要职责先清楚，后续不管是否改路径，站点都会明显更稳定，也更容易继续整理。
