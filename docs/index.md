---
layout: home
hero:
  image:
    src: /logo.png
    alt: KBEngine logo
  text: KBEngine 中文文档
  tagline: 单一学习主线，辅以源码专题、背景对照、API 回查与文档说明
  actions:
    - theme: brand
      text: 开始阅读
      link: /study/
    - theme: alt
      text: 专题深挖
      link: /architecture/source-analysis/
    - theme: alt
      text: API 参考
      link: /api/
features:
  - title: 学习主线
    details: “study/** 是唯一按顺序阅读的主入口。第一次阅读只沿 24 章推进，先建立完整系统地图。”
    link: /study/
  - title: 专题深挖
    details: “architecture/source-analysis/** 负责把具体问题继续拆到文件、类、函数和调用链，不再单独充当第二套正文。”
    link: /architecture/source-analysis/
  - title: 背景与对照
    details: “architecture/** 负责 BigWorld 背景、系统地图和两套引擎的差异对照，用来补'为什么这样设计'。”
    link: /architecture/
  - title: API 参考
    details: “api/** 只做接口契约与快速回查。源码实现和架构解释分别回到专题页和学习主线。”
    link: /api/
  - title: 文档说明
    details: “resources/** 记录来源边界、校验状态和结构设计，回答'这页是否来自原始资料、应该如何维护'。”
    link: /resources/
footer: 文档来源为仓库内已有资料，持续更新中
---

<script setup lang="ts">
// 首次访问按浏览器语言选择文档语言（vitepress 无此能力，GitHub Pages 不支持 _redirects）
// 优先级：localStorage 手动选择 > navigator.language > 保持当前 locale
import { onBeforeMount } from 'vue'
const KEY = 'kbengine-lang-auto-picked'
const BASE = '/kbengine/'
const ROOT_PATH = BASE
const ZH_PATH = BASE + 'zh/'
onBeforeMount(() => {{
  if (typeof window === 'undefined') return
  const p = window.location.pathname
  if (p !== ROOT_PATH && p !== ROOT_PATH.slice(0, -1)) return
  const picked = localStorage.getItem(KEY)
  if (picked) {{ if (picked !== p) window.location.replace(picked); return }}
  const langs = (navigator.languages?.length ? navigator.languages : [navigator.language]).map(l => l.toLowerCase())
  if (langs.some(l => l.startsWith('zh'))) window.location.replace(ZH_PATH)
}})
if (typeof window !== 'undefined') {{
  window.addEventListener('click', (e) => {{
    const a = (e.target as HTMLElement)?.closest?.('a') as HTMLAnchorElement | null
    const href = a?.getAttribute('href') || ''
    if (href.startsWith(ZH_PATH) || href.startsWith('zh/')) localStorage.setItem(KEY, ZH_PATH)
    else if (href === ROOT_PATH || href === ROOT_PATH.slice(0, -1)) localStorage.setItem(KEY, ROOT_PATH)
  }}, true)
}}
</script>


> [English](/en/) | **中文**
