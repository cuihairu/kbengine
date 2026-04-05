import { defaultTheme } from '@vuepress/theme-default'
import { defineUserConfig } from 'vuepress/cli'
import { viteBundler } from '@vuepress/bundler-vite'
import { searchPlugin } from '@vuepress/plugin-search'

export default defineUserConfig({
  lang: 'zh-CN',
  title: 'KBEngine 文档',
  description: '基于 CHM 与 PDF 重建的 KBEngine 中文文档站点',
  head: [
    ['link', { rel: 'icon', href: '/logo.png' }],
  ],
  bundler: viteBundler({
    viteOptions: {
      css: {
        preprocessorOptions: {
          scss: {
            quietDeps: true,
            silenceDeprecations: ['import'],
          },
        },
      },
    },
  }),
  plugins: [
    searchPlugin({
      locales: {
        '/': {
          placeholder: '搜索文档',
        },
      },
      maxSuggestions: 10,
      isSearchable: (page) => !page.path.startsWith('/404'),
    }),
  ],
  theme: defaultTheme({
    logo: '/logo.png',
    repo: 'cuihairu/kbengine',
    editLink: false,
    contributors: false,
    lastUpdated: false,
    navbar: [
      { text: '首页', link: '/' },
      { text: '源码学习', link: '/book/' },
      { text: '指南', link: '/guide/' },
      { text: '架构', link: '/architecture/' },
      { text: 'API', link: '/api/' },
      { text: '资料', link: '/resources/' },
    ],
    sidebar: {
      '/book/': [
        {
          text: 'Part I 为什么长这样',
          children: [
            '/book/README.md',
            '/book/01-introduction-and-method.md',
            '/book/02-bigworld-problems-model-concepts.md',
            '/book/03-kbengine-system-panorama.md',
          ],
        },
        {
          text: 'Part II 运行骨架',
          children: [
            '/book/04-startup-and-process-model.md',
            '/book/05-entitydef-and-entity-definition.md',
            '/book/06-python-runtime-and-script-bridge.md',
          ],
        },
        {
          text: 'Part III 基础设施层',
          children: [
            '/book/07-concurrency-threads-and-memory.md',
            '/book/08-network-infrastructure.md',
            '/book/09-distributed-foundation.md',
          ],
        },
        {
          text: 'Part IV 通信与协作',
          children: [
            '/book/10-serialization-bundle-and-messages.md',
            '/book/11-rpc-entitycall-and-communication-patterns.md',
            '/book/12-property-sync-and-broadcast.md',
            '/book/13-database-dbmgr-and-persistence.md',
          ],
        },
        {
          text: 'Part V 空间、运动与拓扑',
          children: [
            '/book/14-space-aoi-and-vision-system.md',
            '/book/15-space-topology-and-dynamic-scaling.md',
            '/book/16-movement-pathfinding-and-navigation.md',
            '/book/17-ghost-system.md',
          ],
        },
        {
          text: 'Part VI 脚本层行为',
          children: [
            '/book/18-hooks-callbacks-timers-and-events.md',
          ],
        },
        {
          text: 'Part VII 前后端交互',
          children: [
            '/book/19-client-protocol-and-interaction.md',
          ],
        },
        {
          text: 'Part VIII 运维、调试与稳定性',
          children: [
            '/book/20-observability-monitoring-profiling-and-debugging.md',
            '/book/21-hotupdate-fault-tolerance-and-ops.md',
          ],
        },
        {
          text: 'Part IX 串联与实战',
          children: [
            '/book/22-player-complete-lifecycle.md',
            '/book/23-bigworld-and-kbengine-comparison.md',
            '/book/24-practical-source-walkthroughs.md',
          ],
        },
        {
          text: 'Appendix',
          children: [
            '/book/appendix-reading-map-and-next-steps.md',
            '/book/appendix-key-algorithms.md',
            '/book/appendix-external-reference-systems.md',
          ],
        },
      ],
      '/architecture/': [
        {
          text: '总览',
          children: [
            '/architecture/README.md',
          ],
        },
        {
          text: 'BigWorld',
          children: [
            '/architecture/bigworld/README.md',
            '/architecture/bigworld/concepts.md',
            '/architecture/bigworld/process-model.md',
            '/architecture/bigworld/entity-space.md',
          ],
        },
        {
          text: 'KBEngine 源码分析',
          children: [
            '/architecture/source-analysis/README.md',
            '/architecture/source-analysis/entry-and-bootstrap.md',
            '/architecture/source-analysis/process-model.md',
            '/architecture/source-analysis/entity-system.md',
            '/architecture/source-analysis/space-aoi.md',
            '/architecture/source-analysis/networking.md',
            '/architecture/source-analysis/persistence.md',
            '/architecture/source-analysis/scripting.md',
          ],
        },
        {
          text: '对照分析',
          children: [
            '/architecture/comparison/README.md',
            '/architecture/comparison/terminology.md',
            '/architecture/comparison/architecture.md',
            '/architecture/comparison/implementation-differences.md',
          ],
        },
        {
          text: '附录',
          children: [
            '/architecture/redesign.md',
          ],
        },
      ],
      '/guide/': [
        {
          text: '指南',
          children: ['/guide/README.md', '/guide/webconsole.md'],
        },
      ],
      '/api/': [
        {
          text: '核心概念',
          children: ['/api/README.md', '/api/basetypes.md', '/api/keywords.md'],
        },
        {
          text: '客户端',
          children: ['/api/client/README.md', '/api/client/KBEngine.md', '/api/client/Entity.md', '/api/bots/README.md', '/api/bots/KBEngine.md', '/api/bots/Entity.md', '/api/bots/PyClientApp.md'],
        },
        {
          text: '服务端组件',
          children: ['/api/cellapp/README.md', '/api/cellapp/KBEngine.md', '/api/cellapp/Entity.md', '/api/baseapp/README.md', '/api/baseapp/KBEngine.md', '/api/baseapp/Entity.md', '/api/baseapp/Proxy.md', '/api/loginapp/README.md', '/api/loginapp/KBEngine.md', '/api/dbmgr/README.md', '/api/dbmgr/KBEngine.md', '/api/interfaces/README.md', '/api/interfaces/KBEngine.md', '/api/logger/README.md', '/api/logger/KBEngine.md'],
        },
      ],
      '/resources/': [
        {
          text: '资料',
          children: ['/resources/README.md', '/resources/provenance.md', '/resources/verification-status.md'],
        },
      ],
      '/': [],
    },
  }),
})
