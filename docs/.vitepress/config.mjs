import { defineConfig } from 'vitepress'
import { withMermaid } from 'vitepress-plugin-mermaid'
import wikilinks from '@binyamin/markdown-it-wikilinks'

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
  title: 'KBEngine Docs',
  description: 'KBEngine documentation rebuilt from in-repository sources',

  // 使用自定义主题
  theme: './theme',

  markdown: {
    config: (md) => {
      md.use(wikilinks, {
        // 配置选项
        baseURL: '/kbengine/',  // 基础路径
        makeAllLinksAbsolute: true,
        uriSuffix: '',  // 不添加 .html 后缀
      })
    }
  },
  // 死链零容忍：build 即断（官方 locales 结构，root=中文 /en=英文）
  ignoreDeadLinks: false,
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
    socialLinks: [
      { icon: 'github', link: 'https://github.com/cuihairu/kbengine' }
    ],
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
                resetButtonTitle: '重置',
                backButtonTitle: '关闭',
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
      { text: 'Home', link: '/' },
      { text: 'Learning Track', link: '/study/' },
      { text: 'Architecture', link: '/architecture/' },
      { text: 'API', link: '/api/' },
      { text: 'Doc Notes', link: '/resources/' },
      { text: 'Guide', link: '/guide/' },
    ],
    sidebar: {
      '/study/': [
        {
          text: 'Overview',
          items: [
            { text: 'Learning track home', link: '/study/' },
            { text: 'Table of contents', link: '/study/table-of-contents' }
          ]
        },
        {
          text: 'Part I - Why It Looks Like This',
          items: [
            { text: '01 - Introduction and method', link: '/study/01-introduction-and-method' },
            { text: '02 - BigWorld problems and model', link: '/study/02-bigworld-problems-model-concepts' },
            { text: '03 - KBEngine system panorama', link: '/study/03-kbengine-system-panorama' }
          ]
        },
        {
          text: 'Part II - Runtime Skeleton',
          items: [
            { text: '04 - Startup and process model', link: '/study/04-startup-and-process-model' },
            { text: '05 - EntityDef and entity definition', link: '/study/05-entitydef-and-entity-definition' },
            { text: '06 - Python runtime and script bridge', link: '/study/06-python-runtime-and-script-bridge' }
          ]
        },
        {
          text: 'Part III - Infrastructure',
          items: [
            { text: '07 - Concurrency, threads, and memory', link: '/study/07-concurrency-threads-and-memory' },
            { text: '08 - Network infrastructure', link: '/study/08-network-infrastructure' },
            { text: '09 - Distributed foundation', link: '/study/09-distributed-foundation' }
          ]
        },
        {
          text: 'Part IV - Communication and Coordination',
          items: [
            { text: '10 - Serialization, bundles, and messages', link: '/study/10-serialization-bundle-and-messages' },
            { text: '11 - RPC, EntityCall, and communication patterns', link: '/study/11-rpc-entitycall-and-communication-patterns' },
            { text: '12 - Property sync and broadcast', link: '/study/12-property-sync-and-broadcast' },
            { text: '12.5 - cellData and Base-Cell sync', link: '/study/12-celldata-and-base-cell-sync' },
            { text: '13 - Database, DBMgr, and persistence', link: '/study/13-database-dbmgr-and-persistence' }
          ]
        },
        {
          text: 'Part V - Spaces, Motion, and Topology',
          items: [
            { text: '14 - Spaces, AOI, and vision systems', link: '/study/14-space-aoi-and-vision-system' },
            { text: '15 - Space topology and dynamic scaling', link: '/study/15-space-topology-and-dynamic-scaling' },
            { text: '16 - Movement, pathfinding, and navigation', link: '/study/16-movement-pathfinding-and-navigation' },
            { text: '17 - Ghost system', link: '/study/17-ghost-system' }
          ]
        },
        {
          text: 'Part VI - Lifecycle and Operations',
          items: [
            { text: '18 - Hooks, callbacks, timers, and events', link: '/study/18-hooks-callbacks-timers-and-events' },
            { text: '19 - Client protocol and interaction', link: '/study/19-client-protocol-and-interaction' },
            { text: '20 - Observability, monitoring, and debugging', link: '/study/20-observability-monitoring-profiling-and-debugging' },
            { text: '21 - Hot updates and fault-tolerant operations', link: '/study/21-hotupdate-fault-tolerance-and-ops' },
            { text: '22 - Complete player lifecycle', link: '/study/22-player-complete-lifecycle' }
          ]
        },
        {
          text: 'Part VII - Comparison and Reading Map',
          items: [
            { text: '23 - BigWorld and KBEngine comparison', link: '/study/23-bigworld-and-kbengine-comparison' },
            { text: '24 - Practical source walkthroughs', link: '/study/24-practical-source-walkthroughs' },
            { text: 'Appendix - Coordinate system conventions', link: '/study/appendix-coordinate-system-conventions' },
            { text: 'Appendix - Engine suitability and game types', link: '/study/appendix-engine-suitability-and-game-types' },
            { text: 'Appendix - External reference systems', link: '/study/appendix-external-reference-systems' },
            { text: 'Appendix - Key algorithms', link: '/study/appendix-key-algorithms' },
            { text: 'Appendix - Reading map and next steps', link: '/study/appendix-reading-map-and-next-steps' },
            { text: 'Appendix - Server time and world clock', link: '/study/appendix-server-time-management-and-world-clock' },
            { text: 'Appendix - Terminology quick reference', link: '/study/appendix-terminology-quick-reference' }
          ]
        }
      ],
      '/architecture/': [
        {
          text: 'Architecture',
          items: [
            { text: 'Overview', link: '/architecture/' },
            { text: 'Redesign', link: '/architecture/redesign' },
            { text: 'BigWorld', link: '/architecture/bigworld/' },
            { text: 'Comparison', link: '/architecture/comparison/' },
            { text: 'Source deep dives', link: '/architecture/source-analysis/' }
          ]
        },
        {
          text: 'BigWorld',
          items: [
            { text: 'Core concepts', link: '/architecture/bigworld/concepts' },
            { text: 'Process model', link: '/architecture/bigworld/process-model' },
            { text: 'Entities and spaces', link: '/architecture/bigworld/entity-space' }
          ]
        },
        {
          text: 'Comparison',
          items: [
            { text: 'Terminology', link: '/architecture/comparison/terminology' },
            { text: 'Architecture', link: '/architecture/comparison/architecture' },
            { text: 'Implementation differences', link: '/architecture/comparison/implementation-differences' }
          ]
        },
        {
          text: 'Source deep dives',
          items: [
            { text: 'Overview', link: '/architecture/source-analysis/' },
            { text: 'Entry and bootstrap', link: '/architecture/source-analysis/entry-and-bootstrap' },
            { text: 'Process model', link: '/architecture/source-analysis/process-model' },
            { text: 'Entity system', link: '/architecture/source-analysis/entity-system' },
            { text: 'EntityDef type system', link: '/architecture/source-analysis/entitydef-type-system' },
            { text: 'Networking', link: '/architecture/source-analysis/networking' },
            { text: 'Events', link: '/architecture/source-analysis/events' },
            { text: 'Scripting', link: '/architecture/source-analysis/scripting' },
            { text: 'Persistence', link: '/architecture/source-analysis/persistence' },
            { text: 'Non-entity timers', link: '/architecture/source-analysis/non-entity-timers' },
            { text: 'Spaces and AOI', link: '/architecture/source-analysis/space-aoi' },
            { text: 'API coverage', link: '/architecture/source-analysis/api-coverage' },
            { text: 'Runtime configuration and types', link: '/architecture/source-analysis/runtime-config-and-types' },
            { text: 'Runtime utility API', link: '/architecture/source-analysis/runtime-utility-api' },
            { text: 'Component script API', link: '/architecture/source-analysis/component-script-api' },
            { text: 'Base entity lifecycle', link: '/architecture/source-analysis/base-entity-lifecycle' },
            { text: 'BaseApp runtime API', link: '/architecture/source-analysis/baseapp-kbengine-runtime-api' },
            { text: 'CellApp and space runtime API', link: '/architecture/source-analysis/cellapp-kbengine-space-runtime-api' },
            { text: 'Client login and space data API', link: '/architecture/source-analysis/client-login-and-space-data-api' },
            { text: 'Proxy client session API', link: '/architecture/source-analysis/proxy-client-session-api' },
            { text: 'Optimization plan', link: '/architecture/source-analysis/doc-optimization-plan' },
            { text: 'Optimization summary', link: '/architecture/source-analysis/doc-optimization-summary' },
            { text: 'TODO', link: '/architecture/source-analysis/todo' }
          ]
        }
      ],
      '/api/': [
        {
          text: 'API Overview',
          items: [
            { text: 'API home', link: '/api/' },
            { text: 'KBEngine API', link: '/api/kbengine/' },
            { text: 'BigWorld API', link: '/api/bigworld/' }
          ]
        },
        {
          text: 'KBEngine Core Concepts',
          items: [
            { text: 'Audit checklist', link: '/api/kbengine/audit-checklist' },
            { text: 'Basic types', link: '/api/kbengine/basetypes' },
            { text: 'Keywords', link: '/api/kbengine/keywords' }
          ]
        },
        {
          text: 'KBEngine Client',
          items: [
            { text: 'Client home', link: '/api/kbengine/client/' },
            { text: 'KBEngine', link: '/api/kbengine/client/KBEngine' },
            { text: 'Entity', link: '/api/kbengine/client/Entity' },
            { text: 'Bots home', link: '/api/kbengine/bots/' },
            { text: 'Bots KBEngine', link: '/api/kbengine/bots/KBEngine' },
            { text: 'Bots Entity', link: '/api/kbengine/bots/Entity' },
            { text: 'PyClientApp', link: '/api/kbengine/bots/PyClientApp' }
          ]
        },
        {
          text: 'KBEngine Server Components',
          items: [
            { text: 'CellApp home', link: '/api/kbengine/cellapp/' },
            { text: 'CellApp KBEngine', link: '/api/kbengine/cellapp/KBEngine' },
            { text: 'CellApp Entity', link: '/api/kbengine/cellapp/Entity' },
            { text: 'BaseApp home', link: '/api/kbengine/baseapp/' },
            { text: 'BaseApp KBEngine', link: '/api/kbengine/baseapp/KBEngine' },
            { text: 'BaseApp Entity', link: '/api/kbengine/baseapp/Entity' },
            { text: 'BaseApp Proxy', link: '/api/kbengine/baseapp/Proxy' },
            { text: 'LoginApp home', link: '/api/kbengine/loginapp/' },
            { text: 'LoginApp KBEngine', link: '/api/kbengine/loginapp/KBEngine' },
            { text: 'DBMgr home', link: '/api/kbengine/dbmgr/' },
            { text: 'DBMgr KBEngine', link: '/api/kbengine/dbmgr/KBEngine' },
            { text: 'Interfaces home', link: '/api/kbengine/interfaces/' },
            { text: 'Interfaces KBEngine', link: '/api/kbengine/interfaces/KBEngine' },
            { text: 'Logger home', link: '/api/kbengine/logger/' },
            { text: 'Logger KBEngine', link: '/api/kbengine/logger/KBEngine' }
          ]
        },
        {
          text: 'BigWorld API',
          items: [
            { text: 'BigWorld API home', link: '/api/bigworld/' },
            { text: 'BigWorld module', link: '/api/bigworld/bigworld-module' },
            { text: 'Base entities', link: '/api/bigworld/base' },
            { text: 'Cell entities', link: '/api/bigworld/entity' },
            { text: 'BWPersonality', link: '/api/bigworld/bwpersonality' }
          ]
        }
      ],
      '/guide/': [
        {
          text: 'Guide',
          items: [
            { text: 'Guide home', link: '/guide/' },
            { text: 'WebConsole', link: '/guide/webconsole' },
            { text: 'Engineering guide', link: '/guide/engineering/' }
          ]
        },
        {
          text: 'Engineering',
          items: [
            { text: 'Coverage and Codecov', link: '/guide/engineering/coverage' },
            { text: 'Python 3.12 upgrade', link: '/guide/engineering/python-3.12-upgrade' },
            { text: 'vcpkg migration status', link: '/guide/engineering/vcpkg-migration' },
            { text: 'Network I/O research', link: '/guide/engineering/network-io-research' }
          ]
        }
      ],
      '/resources/': [
        {
          text: 'Doc Notes',
          items: [
            { text: 'Notes home', link: '/resources/' },
            { text: 'Source boundaries', link: '/resources/provenance' },
            { text: 'Verification status', link: '/resources/verification-status' },
            { text: 'Structure design', link: '/resources/doc-structure' }
          ]
        }
      ],
      '/': []
    },
  },
})


// —— 官方国际化（vitepress locales，唯一方案；内容内不放语言头行）——
// 结构铁律（kbengine 例外）：root=中文，en=英文，路由 /en/...
const prefixEn = (link) =>
  typeof link === 'string' && link.startsWith('/') && !link.startsWith('/en/')
    ? (link === '/' ? '/en/' : '/en' + link)
    : link
const prefixItems = (items) =>
  (items || []).map((item) => ({
    ...item,
    link: prefixEn(item.link),
    ...(item.items ? { items: prefixItems(item.items) } : {})
  }))
const rootTheme = config.themeConfig || {}
config.locales = {
  root: {
    label: '简体中文',
    lang: 'zh-CN',
    title: 'KBEngine 文档',
    description: '基于 CHM 与 PDF 重建的 KBEngine 中文文档站点',
    themeConfig: rootTheme
  },
  en: {
    label: 'English',
    lang: 'en-US',
    title: 'KBEngine Docs',
    description: 'KBEngine documentation rebuilt from in-repository sources',
    themeConfig: {
      ...rootTheme,
      nav: prefixItems(rootTheme.nav),
      sidebar:
        rootTheme.sidebar && typeof rootTheme.sidebar === 'object' && !Array.isArray(rootTheme.sidebar)
          ? Object.fromEntries(
              Object.entries(rootTheme.sidebar).map(([key, value]) => [key, Array.isArray(value) ? prefixItems(value) : value])
            )
          : rootTheme.sidebar,
      search: {
        ...rootTheme.search,
        options: {
          ...rootTheme.search?.options,
          locales: {
            '/en/': {
              translations: {
                button: {
                  buttonText: 'Search docs',
                  buttonAriaLabel: 'Search docs'
                },
                modal: {
                  noResultsText: 'No results',
                  resetButtonTitle: 'Reset',
                  backButtonTitle: 'Close',
                  footer: {
                    selectText: 'to select',
                    navigateText: 'to navigate',
                    closeText: 'to close'
                  }
                }
              }
            }
          }
        }
      }
    }
  }
}

export default withMermaid(config)
