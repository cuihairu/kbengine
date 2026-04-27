import { defineConfig } from 'vitepress'
import { withMermaid } from 'vitepress-plugin-mermaid'

const LARGE_CODE_BLOCK_RE = /```[\s\S]*?```/g
const MERMAID_BLOCK_RE = /```mermaid[\s\S]*?```/g
const HTML_PRE_BLOCK_RE = /<pre[\s\S]*?<\/pre>/gi

function stripSearchNoise(markdown) {
  return markdown
    .replace(MERMAID_BLOCK_RE, '\n')
    .replace(LARGE_CODE_BLOCK_RE, (block) => {
      const lineCount = block.split('\n').length
      return lineCount > 20 ? '\n' : block
    })
}

const config = defineConfig({
  srcDir: '.',
  base: '/kbengine/',
  lang: 'zh-CN',
  title: 'KBEngine 文档',
  description: '基于 CHM 与 PDF 重建的 KBEngine 中文文档站点',
  vite: {
    build: {
      chunkSizeWarningLimit: 2500,
    },
  },
  head: [
    ['link', { rel: 'icon', href: '/kbengine/logo.png' }],
  ],
  themeConfig: {
    logo: '/logo.png',
    repo: 'cuihairu/kbengine',
    editLink: false,
    contributors: false,
    lastUpdated: false,
    search: {
      provider: 'local',
      options: {
        _render(src, env, md) {
          const rendered = md.render(stripSearchNoise(src), env)
          return env.frontmatter?.search === false
            ? ''
            : rendered.replace(HTML_PRE_BLOCK_RE, '\n')
        },
        locales: {
          '/': {
            translations: {
              button: {
                buttonText: '搜索文档',
                buttonAriaLabel: '搜索文档'
              },
              modal: {
                noResultsText: '没有结果',
                resetButtonTitle: '重置搜索',
                backButtonTitle: '关闭搜索',
                footer: {
                  selectText: '选择',
                  navigateText: '导航',
                  closeText: '关闭'
                }
              }
            }
          }
        }
      }
    },
    nav: [
      { text: '首页', link: '/' },
      { text: '源码学习', link: '/study/' },
      { text: '架构', link: '/architecture/' },
      { text: 'API', link: '/api/' },
      { text: '资料', link: '/resources/' },
      { text: '指南', link: '/guide/' },
    ],
    sidebar: {
      '/study/': [
        {
          text: 'Part I 为什么长这样',
          items: [
            { text: '源码学习首页', link: '/study/' },
            { text: '01 - 介绍与方法', link: '/study/01-introduction-and-method' },
            { text: '02 - BigWorld 问题与模型', link: '/study/02-bigworld-problems-model-concepts' },
            { text: '03 - KBEngine 系统全景', link: '/study/03-kbengine-system-panorama' },
          ],
        },
        {
          text: 'Part II 运行骨架',
          items: [
            { text: '04 - 启动与进程模型', link: '/study/04-startup-and-process-model' },
            { text: '05 - EntityDef 与实体定义', link: '/study/05-entitydef-and-entity-definition' },
            { text: '06 - Python 运行时与脚本桥', link: '/study/06-python-runtime-and-script-bridge' },
          ],
        },
        {
          text: 'Part III 基础设施层',
          items: [
            { text: '07 - 并发、线程与内存', link: '/study/07-concurrency-threads-and-memory' },
            { text: '08 - 网络基础设施', link: '/study/08-network-infrastructure' },
            { text: '09 - 分布式基础', link: '/study/09-distributed-foundation' },
          ],
        },
        {
          text: 'Part IV 通信与协作',
          items: [
            { text: '10 - 序列化、Bundle 与消息', link: '/study/10-serialization-bundle-and-messages' },
            { text: '11 - RPC、EntityCall 与通信模式', link: '/study/11-rpc-entitycall-and-communication-patterns' },
            { text: '12 - 属性同步与广播', link: '/study/12-property-sync-and-broadcast' },
            { text: '13 - 数据库、DBMgr 与持久化', link: '/study/13-database-dbmgr-and-persistence' },
          ],
        },
        {
          text: 'Part V 空间、运动与拓扑',
          items: [
            { text: '14 - 空间、AOI 与视野系统', link: '/study/14-space-aoi-and-vision-system' },
            { text: '15 - 空间拓扑与动态扩展', link: '/study/15-space-topology-and-dynamic-scaling' },
            { text: '16 - 移动、寻路与导航', link: '/study/16-movement-pathfinding-and-navigation' },
            { text: '17 - Ghost 系统', link: '/study/17-ghost-system' },
          ],
        },
        {
          text: 'Part VI 脚本层行为',
          items: [
            { text: '18 - Hooks、回调、定时器与事件', link: '/study/18-hooks-callbacks-timers-and-events' },
          ],
        },
        {
          text: 'Part VII 前后端交互',
          items: [
            { text: '19 - 客户端协议与交互', link: '/study/19-client-protocol-and-interaction' },
          ],
        },
        {
          text: 'Part VIII 运维、调试与稳定性',
          items: [
            { text: '20 - 可观测性、监控、分析与调试', link: '/study/20-observability-monitoring-profiling-and-debugging' },
            { text: '21 - 热更新、容错与运维', link: '/study/21-hotupdate-fault-tolerance-and-ops' },
          ],
        },
        {
          text: 'Part IX 串联与实战',
          items: [
            { text: '22 - 玩家完整生命周期', link: '/study/22-player-complete-lifecycle' },
            { text: '23 - BigWorld 与 KBEngine 对比', link: '/study/23-bigworld-and-kbengine-comparison' },
            { text: '24 - 实战源码导读', link: '/study/24-practical-source-walkthroughs' },
          ],
        },
        {
          text: '阅读辅助',
          items: [
            { text: '目录', link: '/study/table-of-contents' },
          ],
        },
        {
          text: 'Appendix',
          items: [
            { text: '阅读地图与后续步骤', link: '/study/appendix-reading-map-and-next-steps' },
            { text: '关键算法', link: '/study/appendix-key-algorithms' },
            { text: '外部参考系统', link: '/study/appendix-external-reference-systems' },
            { text: '术语速查', link: '/study/appendix-terminology-quick-reference' },
            { text: '引擎适用性', link: '/study/appendix-engine-suitability-and-game-types' },
            { text: '坐标系约定', link: '/study/appendix-coordinate-system-conventions' },
            { text: '服务器时间管理', link: '/study/appendix-server-time-management-and-world-clock' },
          ],
        },
      ],
      '/architecture/': [
        {
          text: '总览',
          items: [
            { text: '架构首页', link: '/architecture/' },
          ],
        },
        {
          text: 'BigWorld',
          items: [
            { text: 'BigWorld 首页', link: '/architecture/bigworld/' },
            { text: '概念', link: '/architecture/bigworld/concepts' },
            { text: '进程模型', link: '/architecture/bigworld/process-model' },
            { text: '实体与空间', link: '/architecture/bigworld/entity-space' },
          ],
        },
        {
          text: 'KBEngine 源码学习',
          items: [
            { text: '源码学习首页', link: '/architecture/source-analysis/' },
            { text: '入口与引导', link: '/architecture/source-analysis/entry-and-bootstrap' },
            { text: '进程模型', link: '/architecture/source-analysis/process-model' },
            { text: '实体系统', link: '/architecture/source-analysis/entity-system' },
            { text: '类型系统与实体定义文件', link: '/architecture/source-analysis/entitydef-type-system' },
            { text: '空间与 AOI', link: '/architecture/source-analysis/space-aoi' },
            { text: '网络层', link: '/architecture/source-analysis/networking' },
            { text: '持久化', link: '/architecture/source-analysis/persistence' },
            { text: '脚本层', link: '/architecture/source-analysis/scripting' },
            { text: '事件系统', link: '/architecture/source-analysis/events' },
            { text: 'Base 实体生命周期', link: '/architecture/source-analysis/base-entity-lifecycle' },
            { text: 'BaseApp 运行时 API', link: '/architecture/source-analysis/baseapp-kbengine-runtime-api' },
            { text: 'Proxy 会话与流式传输 API', link: '/architecture/source-analysis/proxy-client-session-api' },
            { text: 'CellApp 空间运行时 API', link: '/architecture/source-analysis/cellapp-kbengine-space-runtime-api' },
            { text: '客户端登录、重登录与 SpaceData API', link: '/architecture/source-analysis/client-login-and-space-data-api' },
            { text: '组件型脚本 API', link: '/architecture/source-analysis/component-script-api' },
            { text: '通用运行时工具 API', link: '/architecture/source-analysis/runtime-utility-api' },
            { text: '运行时配置与基础类型', link: '/architecture/source-analysis/runtime-config-and-types' },
            { text: 'API 覆盖矩阵', link: '/architecture/source-analysis/api-coverage' },
            { text: '文档重构 TODO', link: '/architecture/source-analysis/todo' },
          ],
        },
        {
          text: '对照分析',
          items: [
            { text: '对照分析首页', link: '/architecture/comparison/' },
            { text: '术语', link: '/architecture/comparison/terminology' },
            { text: '架构', link: '/architecture/comparison/architecture' },
            { text: '实现差异', link: '/architecture/comparison/implementation-differences' },
          ],
        },
        {
          text: '附录',
          items: [
            { text: '重新设计', link: '/architecture/redesign' },
          ],
        },
      ],
      '/guide/': [
        {
          text: '指南',
          items: [{ text: '指南首页', link: '/guide/' }, { text: 'WebConsole', link: '/guide/webconsole' }],
        },
      ],
      '/api/': [
        {
          text: '核心概念',
          items: [
            { text: 'API 首页', link: '/api/' },
            { text: '审计清单', link: '/api/audit-checklist' },
            { text: '基础类型', link: '/api/basetypes' },
            { text: '关键字', link: '/api/keywords' },
          ],
        },
        {
          text: '客户端',
          items: [
            { text: '客户端首页', link: '/api/client/' },
            { text: 'KBEngine', link: '/api/client/KBEngine' },
            { text: 'Entity', link: '/api/client/Entity' },
            { text: 'Bots 首页', link: '/api/bots/' },
            { text: 'Bots KBEngine', link: '/api/bots/KBEngine' },
            { text: 'Bots Entity', link: '/api/bots/Entity' },
            { text: 'PyClientApp', link: '/api/bots/PyClientApp' },
          ],
        },
        {
          text: '服务端组件',
          items: [
            { text: 'CellApp 首页', link: '/api/cellapp/' },
            { text: 'CellApp KBEngine', link: '/api/cellapp/KBEngine' },
            { text: 'CellApp Entity', link: '/api/cellapp/Entity' },
            { text: 'BaseApp 首页', link: '/api/baseapp/' },
            { text: 'BaseApp KBEngine', link: '/api/baseapp/KBEngine' },
            { text: 'BaseApp Entity', link: '/api/baseapp/Entity' },
            { text: 'BaseApp Proxy', link: '/api/baseapp/Proxy' },
            { text: 'LoginApp 首页', link: '/api/loginapp/' },
            { text: 'LoginApp KBEngine', link: '/api/loginapp/KBEngine' },
            { text: 'DBMgr 首页', link: '/api/dbmgr/' },
            { text: 'DBMgr KBEngine', link: '/api/dbmgr/KBEngine' },
            { text: 'Interfaces 首页', link: '/api/interfaces/' },
            { text: 'Interfaces KBEngine', link: '/api/interfaces/KBEngine' },
            { text: 'Logger 首页', link: '/api/logger/' },
            { text: 'Logger KBEngine', link: '/api/logger/KBEngine' },
          ],
        },
      ],
      '/resources/': [
        {
          text: '资料',
          items: [
            { text: '资料首页', link: '/resources/' },
            { text: '来源', link: '/resources/provenance' },
            { text: '验证状态', link: '/resources/verification-status' },
          ],
        },
      ],
      '/': [],
    },
  },
})

export default withMermaid(config)
