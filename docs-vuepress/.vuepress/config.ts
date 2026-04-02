import { defaultTheme } from '@vuepress/theme-default'
import { defineUserConfig } from 'vuepress/cli'
import { viteBundler } from '@vuepress/bundler-vite'

export default defineUserConfig({
  lang: 'zh-CN',
  title: 'KBEngine',
  description: 'KBEngine 服务端引擎文档',

  head: [
    ['link', { rel: 'icon', href: '/logo.png' }],
  ],

  locales: {
    '/': {
      lang: 'zh-CN',
      title: 'KBEngine',
      description: 'KBEngine 服务端引擎文档'
    },
    '/en/': {
      lang: 'en-US',
      title: 'KBEngine',
      description: 'KBEngine Server Engine Documentation'
    }
  },

  bundler: viteBundler({
    viteOptions: {},
    vuePluginOptions: {}
  }),

  theme: defaultTheme({
    logo: '/logo.png',

    locales: {
      '/': {
        selectLanguageName: '简体中文',
        selectLanguageText: '选择语言',
        editLinkText: '在 GitHub 上编辑此页',
        lastUpdatedText: '最后更新',
        contributorsText: '贡献者',
        tip: '提示',
        warning: '警告',
        danger: '危险',
        notFound: ['没有找到页面'],
        backToHome: '返回首页',
        openInNewWindow: '在新窗口打开',
        toggleDarkMode: '切换深色模式',
        toggleSidebar: '切换侧边栏',

        navbar: [
          {
            text: '指南',
            link: '/guide/',
          },
          {
            text: 'API',
            link: '/api/',
          },
          {
            text: '教程',
            link: '/tutorial/',
          },
          {
            text: '开发',
            link: '/guide/development',
          },
        ],

        sidebar: {
          '/guide/': [
            {
              text: '指南',
              children: [
                '/guide/README.md',
                '/guide/installation.md',
                '/guide/configuration.md',
                '/guide/webconsole.md',
              ],
            },
          ],
          '/api/': [
            {
              text: 'API 参考',
              children: [
                '/api/README.md',
                '/api/logger.md',
                '/api/entity.md',
              ],
            },
          ],
          '/tutorial/': [
            {
              text: '教程',
              children: [
                '/tutorial/README.md',
              ],
            },
          ],
        },
      },

      '/en/': {
        selectLanguageName: 'English',
        selectLanguageText: 'Languages',
        editLinkText: 'Edit this page on GitHub',
        lastUpdatedText: 'Last Updated',
        contributorsText: 'Contributors',
        tip: 'Tip',
        warning: 'Warning',
        danger: 'Danger',
        notFound: ['Not Found'],
        backToHome: 'Back to home',
        openInNewWindow: 'Open in new window',
        toggleDarkMode: 'Toggle dark mode',
        toggleSidebar: 'Toggle sidebar',

        navbar: [
          {
            text: 'Guide',
            link: '/en/guide/',
          },
          {
            text: 'API',
            link: '/en/api/',
          },
          {
            text: 'Tutorial',
            link: '/en/tutorial/',
          },
          {
            text: 'Development',
            link: '/en/guide/development',
          },
        ],

        sidebar: {
          '/en/guide/': [
            {
              text: 'Guide',
              children: [
                '/en/guide/README.md',
                '/en/guide/installation.md',
                '/en/guide/configuration.md',
                '/en/guide/webconsole.md',
              ],
            },
          ],
          '/en/api/': [
            {
              text: 'API Reference',
              children: [
                '/en/api/README.md',
                '/en/api/logger.md',
                '/en/api/entity.md',
              ],
            },
          ],
          '/en/tutorial/': [
            {
              text: 'Tutorial',
              children: [
                '/en/tutorial/README.md',
              ],
            },
          ],
        },
      },
    },

    // 编辑链接
    editLink: true,
    repo: 'cuihairu/kbengine',

    // 最后更新
    lastUpdated: true,
    contributors: true,
  }),
})
