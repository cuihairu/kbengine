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
      { text: '指南', link: '/guide/' },
      { text: 'API', link: '/api/' },
    ],
    sidebar: {
      '/': [],
      '/guide/': [
        {
          text: '指南',
          children: ['/guide/README.md', '/guide/webconsole.md'],
        },
      ],
      '/api/': [
        {
          text: '通用内容',
          children: ['/api/README.md', '/api/basetypes.md', '/api/keywords.md'],
        },
        {
          text: 'client',
          children: ['/api/client/README.md', '/api/client/KBEngine.md', '/api/client/Entity.md'],
        },
        {
          text: 'cellapp',
          children: ['/api/cellapp/README.md', '/api/cellapp/KBEngine.md', '/api/cellapp/Entity.md'],
        },
        {
          text: 'baseapp',
          children: ['/api/baseapp/README.md', '/api/baseapp/KBEngine.md', '/api/baseapp/Entity.md', '/api/baseapp/Proxy.md'],
        },
        {
          text: 'loginapp',
          children: ['/api/loginapp/README.md', '/api/loginapp/KBEngine.md'],
        },
        {
          text: 'dbmgr',
          children: ['/api/dbmgr/README.md', '/api/dbmgr/KBEngine.md'],
        },
        {
          text: 'interfaces',
          children: ['/api/interfaces/README.md', '/api/interfaces/KBEngine.md'],
        },
        {
          text: 'logger',
          children: ['/api/logger/README.md', '/api/logger/KBEngine.md'],
        },
        {
          text: 'bots',
          children: ['/api/bots/README.md', '/api/bots/KBEngine.md', '/api/bots/Entity.md', '/api/bots/PyClientApp.md'],
        },
      ],
    },
  }),
})
