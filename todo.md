# todo — kbengine

## 已完成

- [x] 双语收口：README.md 中文正文移出、英文版补对应内容、首屏顶部居中语言切换器；新建 README.zh.md 收中文原文（2026-10-16；README.zh.md 入库）
- [x] 文档双语收口 `docs/` 根级：根目录 7 篇全部配对，两向语言切换器已加（2026-10-16；`CODECOV_SETUP.md` 等 7 篇配对）
- [x] 中英双版媒体文件互索引：`docs/resources/index.md`、`docs/resources/provenance.md` 登记 4 个原始资料文件，`docs/guide/index.md`、`docs/api/index.md` 补目录内索引（2026-10-16 R1016F；4 个二进制登记，用仓库路径内联代码免死链）
- [x] 文档双语收口 `docs/` 子目录：入口层、`api/bigworld/**`、`api/kbengine/**`（26 对）完成（2026-10-22；commit `3b08b08`、`daaf492`；`docs:build` 通过，`test:docs` 4/4，权威核对 24/24）
- [x] 文档双语收口 `docs/` 子目录：`architecture/**`、`study/**`、`guide/**`、`resources/**`、`book-legacy/**`、`tutorial/**` 完成（b9e91b4 布局翻转后英文根 + `docs/zh/` 镜像，131 篇对 131 篇、0 缺失，2026-11-01 复核）
- [x] `api/kbengine/**` 双语化核对口径：24 页拆 13 个 CHM 直通页与 11 个增强页，新增 `tools/tests/test_verify_authoritative_docs.py` 三类回归（2026-10-16 R1016PATROL；中文权威页首行不改，英文走 `.en.md` 与目录级索引）

## 待办

（空）
