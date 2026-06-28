import { defineConfig } from 'vitepress'
import { CHAPTERS } from './chapters.mjs'

export default defineConfig({
  title: 'TCP/IP 网络编程学习笔记',
  description: '《TCP/IP 网络编程》学习笔记电子书',
  lang: 'zh-CN',
  cleanUrls: true,
  lastUpdated: true,

  markdown: {
    theme: { light: 'github-light', dark: 'github-dark' },
    lineNumbers: true,
  },

  head: [
    ['meta', { name: 'viewport', content: 'width=device-width,initial-scale=1' }],
    ['link', { rel: 'icon', type: 'image/png', sizes: '32x32', href: '/favicon-32.png' }],
    ['link', { rel: 'icon', type: 'image/png', sizes: '180x180', href: '/favicon-180.png' }],
    ['link', { rel: 'apple-touch-icon', sizes: '180x180', href: '/favicon-180.png' }],
    ['link', { rel: 'mask-icon', href: '/favicon-512.png', color: '#06b6d4' }],
    ['meta', { name: 'theme-color', content: '#0a0e27' }],
    ['meta', { property: 'og:image', content: '/cover.png' }],
  ],

  themeConfig: {
    siteTitle: 'TCP/IP 网络编程笔记',
    outline: { level: [2, 3], label: '本页目录' },
    docFooter: { prev: '上一章', next: '下一章' },
    lastUpdatedText: '最后更新',
    returnToTopLabel: '回到顶部',
    sidebarMenuLabel: '目录',

    search: {
      provider: 'local',
      options: {
        translations: {
          button: { buttonText: '搜索', buttonAriaLabel: '搜索' },
          modal: {
            displayDetails: '显示详情',
            resetButtonTitle: '清除查询',
            backButtonTitle: '返回',
            noResultsText: '无结果',
            footer: {
              selectText: '选择',
              navigateText: '切换',
              closeText: '关闭',
            },
          },
        },
      },
    },

    nav: [
      { text: '首页', link: '/' },
      { text: 'GitHub 仓库', link: 'https://github.com/riba2534/TCP-IP-NetworkNote' },
    ],

    sidebar: [
      {
        text: '章节',
        collapsed: false,
        items: CHAPTERS.map((c) => ({ text: c.title, link: `/${c.dir}/` })),
      },
    ],

    socialLinks: [
      { icon: 'github', link: 'https://github.com/riba2534/TCP-IP-NetworkNote' },
    ],

    footer: {
      message: '基于 VitePress 构建，部署于 Cloudflare Pages',
      copyright: 'Copyright © riba2534',
    },
  },
})
