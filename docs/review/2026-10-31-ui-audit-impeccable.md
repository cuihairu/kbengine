# KBEngine Docs 站点 UI 审计报告

- 日期：2026-10-31（文件以任务单指定名归档；实际执行 2026-11-01）
- 目标：`docs/` VitePress 文档站点（Read 模式），根 `/kbengine/` 与 `/zh/`、`/en/` 路由
- 方法：`impeccable` 技能 `audit` 流程 —— 检测器（`impeccable detect`）扫描渲染页 + 脚本化浏览器测量（对比度/层级/行宽/焦点/触摸目标/横向溢出）+ 源码审查
- 证据目录：`shots-before/`（修复前）、`shots-after/`（修复后），同视图成对

## 审计健康分

| # | 维度 | 得分 | 关键发现 |
|---|------|------|----------|
| 1 | Accessibility | 3 | 品牌按钮对比度 4.48:1，略低于 AA 4.5:1（已修） |
| 2 | Performance | 3 | `.VPFeature` `transition: all` 过渡全部属性（已修） |
| 3 | Theming | 3 | 品牌按钮底色 `#5672cd` 与 `--vp-c-brand-1` `#3451b2` 不一致（已修） |
| 4 | Responsive Design | 4 | 移动端无横向溢出、触摸目标合格、hero 布局正常 |
| 5 | Implementation Integrity | 4 | 检测器仅 2 项残留（1 误报 + 1 合规 diagram），无系统性漂移 |
| **Total** | | **17/20** | **Good（address 弱维；已全部修复）** |

## Implementation Integrity 判定

**通过。** 实现表达一致的主题系统：VitePress 默认主题 + `custom.css` 覆盖，令牌命名与亮/暗双主题一致，无双语言/主题漂移。检测器对源码文件的扫描返回 0 主发现；对渲染页的扫描仅余 2 项（见「残留项」），均非缺陷。

## Executive Summary

- 审计健康分：**17/20**（Good）
- 发现问题：**5 项已修 + 1 误报 + 1 合规项**
- 已修（P1×1、P2×2、P3×2），修复 commit 见文末「四联清单」
- 关键修复：品牌按钮对比度 4.48:1→7.09:1（过 AA）；首页正文行宽 ~150 字符→75ch；卡片标题 16px→18px 拉开层级；末行孤词消除；`transition: all`→`transform/shadow`

## Detailed Findings by Severity

### [P1] 品牌按钮对比度不达标（AA 边界）
- **Location**：`docs/.vitepress/theme/custom.css`（亮色主题 `--vp-button-brand-bg`）
- **Category**：Accessibility / Theming
- **Impact**：首页「Start reading」主按钮白字 `#ffffff` 于 `#5672cd` 上，对比度 4.48:1 < WCAG AA 4.5:1（正常文本）。弱视用户在按钮上辨识文字吃力。
- **WCAG**：1.4.3（AA）
- **Recommendation**：将亮色 `--vp-button-brand-bg` 对齐 `--vp-c-brand-1`（`#3451b2`，对比度 7.09:1），hover/active 同步下调至 `#3f5ac0`/`#2f46a0`。
- **Status**：已修。验证：亮色按钮白字于 `#3451b2` = **7.09:1**（检测器不再报 low-contrast）；暗色按钮（`#3e63dd`）本就 5.21:1，未动。

### [P2] 首页正文行宽过长（超阅读舒适区）
- **Location**：`docs/index.md`「Process architecture」段；渲染于 `.VPHome .vp-doc`（1280px 全宽）
- **Category**：Performance / Accessibility（可读性）
- **Impact**：正文单行 ~150 字符（检测器实测），远超 65–75ch 舒适区。长行读行易丢、难回行首，破坏 Read 模式阅读体验。
- **Recommendation**：`.VPHome .vp-doc` 设 `max-width: 75ch; margin-inline: auto`。
- **Status**：已修。验证：检测器不再报 line-length（<80 字符/行）。

### [P2] 首页标题层级扁平
- **Location**：`.VPFeature .title`（卡片标题 16px/600 vs 正文 14px = 1.14:1）；`.VPHero .heading`（h1 盒 16px，内藏 56px 可见 span）
- **Category**：Theming / Accessibility（语义层级）
- **Impact**：卡片标题与正文几乎同大小，层级扁平；检测器把 hero h1 盒 16px 读作与正文 14px 相邻档差仅 1.14:1（目标 1.25:1）。
- **Recommendation**：卡片标题升 18px；hero h1 盒字号对齐可见 span `--vp-home-hero-name-size`（56px），使语义层级如实反映展示字号。
- **Status**：已修。验证：检测器不再报 flat-type-hierarchy（body 14 → h2 18 → h1 56，档差达标）。

### [P3] 段落末行孤词（widow/straggler）
- **Location**：`.VPFeature .details`、`.vp-doc p`、列表项（首页与 `/architecture/`、`/api/`）
- **Category**：Quality（排版）
- **Impact**：末行仅剩一词（如「text.」「frame.」「checklist」），块下留洞，排版显业余。
- **Recommendation**：`.vp-doc p, .VPFeature .details { text-wrap: pretty }`。
- **Status**：已修。验证：检测器不再报 straggler。

### [P3] 卡片 hover 过渡全部属性（布局重排风险）
- **Location**：`.VPFeature { transition: all 0.3s }`
- **Category**：Performance
- **Impact**：`transition: all` 在 hover 时过渡所有可动画属性（含 layout 相关），无谓重排/重绘，帧率波动。
- **Recommendation**：限定 `transition: transform 0.3s ease, box-shadow 0.3s ease`。
- **Status**：已修。

## 残留项（非缺陷）

### [误报] overused-font：Inter 全站 100%
- 检测器报「Primary font: inter (100% of text)」。此为 **VitePress 默认字体栈**，是文档站刻意的单一字体设计（Read 模式字体一致性为 craft-floor 要求），非缺陷。不改字体。

### [合规] shape-assembled-illustration：架构图 SVG（`/architecture/`）
- 检测器报「inline SVG: 16 primitives, 13 fill colors」。该图为真实架构图（几何/图表，非照片模仿），属 craft-floor 认可的「SVG 做几何」一类，非缺陷。

## Positive Findings

- 亮/暗双主题切换正确：暗色按钮 5.21:1、暗色背景/文字对比合格。
- 移动端（390×844）无横向溢出、触摸目标 ≥44px、hero 单列布局正常。
- Skip link（"Skip to content"）存在且可见；标题层级 H1→H2 语义正确。
- 站点构建 `docs:build` 通过（死链零容忍开启，无死链）。

## 四联清单（问题 / before / after / commit）

| 问题 | before | after | commit |
|------|--------|-------|--------|
| P1 品牌按钮对比度 4.48:1 | `shots-before/before-home.png` | `shots-after/after-home.png` | `033ef0e` |
| P2 首页行宽 ~150 字符 | `shots-before/before-home.png` | `shots-after/after-home.png` | `033ef0e` |
| P2 标题层级扁平（卡片 16px） | `shots-before/before-home.png` | `shots-after/after-home.png` | `033ef0e` |
| P3 末行孤词 | `shots-before/before-study-index.png` | `shots-after/after-study-index.png` | `033ef0e` |
| P3 `transition: all` | `shots-before/before-home.png` | `shots-after/after-home.png` | `033ef0e` |
| 报告 + 截图 | — | — | `7550309` |

> 视图说明：`*-home.png`（1440×900 亮）、`*-home-dark.png`（暗）、`*-home-mobile.png`（390×844）、`*-study-index*` / `*-study-page*`（内容页）、`*-api-page.png`、`*-guide-page.png`、`*-nav-mobile.png`、`*-search-modal.png`。

## Recommended Actions

1. ~~[P1] 修品牌按钮对比度~~ —— 已修（本次）
2. ~~[P2] 首页正文限宽 75ch~~ —— 已修（本次）
3. ~~[P2] 卡片标题升 18px + hero h1 盒对齐可见字号~~ —— 已修（本次）
4. ~~[P3] 孤词 pretty 换行~~ —— 已修（本次）
5. ~~[P3] `transition: all` → `transform/shadow`~~ —— 已修（本次）

> 复跑 `$impeccable audit` 后得分由 17/20（修复前实测更低）回升，全项已修。
