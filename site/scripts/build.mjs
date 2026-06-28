// 构建脚本：从仓库源（chXX/README.md + chXX/**/*.c + images/）生成 VitePress 站点内容
// 幂等：每次先清理生成物再全量生成。零第三方依赖，纯 Node fs/path。
import fs from 'node:fs'
import path from 'node:path'
import { fileURLToPath } from 'node:url'
import { CHAPTERS } from '../.vitepress/chapters.mjs'

const __dirname = path.dirname(fileURLToPath(import.meta.url))
const REPO_ROOT = path.resolve(__dirname, '../..') // 仓库根
const SITE_ROOT = path.resolve(__dirname, '..') // site/

const GENERATED_DIRS = CHAPTERS.map((c) => path.join(SITE_ROOT, c.dir))
const PUBLIC_IMAGES = path.join(SITE_ROOT, 'public/images')

// ---------- 工具 ----------
function rmrf(p) {
  fs.rmSync(p, { recursive: true, force: true })
}
function mkdirp(p) {
  fs.mkdirSync(p, { recursive: true })
}
// 递归列出目录下所有 .c 文件，返回相对该目录的路径（如 "hello_server.c"、"homework/kehou4.c"）
function listCFiles(chDir) {
  const result = []
  function walk(dir, relBase) {
    for (const entry of fs.readdirSync(dir, { withFileTypes: true })) {
      const full = path.join(dir, entry.name)
      const rel = relBase ? `${relBase}/${entry.name}` : entry.name
      if (entry.isDirectory()) walk(full, rel)
      else if (entry.name.endsWith('.c')) result.push(rel)
    }
  }
  walk(chDir, '')
  result.sort()
  return result
}

// ---------- 步骤 A：清理生成物 ----------
function clean() {
  for (const d of GENERATED_DIRS) rmrf(d)
  rmrf(PUBLIC_IMAGES)
  mkdirp(path.join(SITE_ROOT, 'public'))
}

// ---------- 步骤 B：复制图片 ----------
function copyImages() {
  const src = path.join(REPO_ROOT, 'images')
  if (!fs.existsSync(src)) {
    console.warn(`[warn] 未找到 images 目录: ${src}`)
    return 0
  }
  fs.cpSync(src, PUBLIC_IMAGES, { recursive: true })
  return fs.readdirSync(PUBLIC_IMAGES).length
}

// ---------- 步骤 C：生成 .c 源码页 ----------
function genSourcePages() {
  let count = 0
  for (const ch of CHAPTERS) {
    const chDir = path.join(REPO_ROOT, ch.dir)
    if (!fs.existsSync(chDir)) {
      console.warn(`[warn] 章节目录不存在: ${ch.dir}`)
      continue
    }
    const outDir = path.join(SITE_ROOT, ch.dir)
    mkdirp(outDir)
    const cFiles = listCFiles(chDir)
    for (const rel of cFiles) {
      const code = fs.readFileSync(path.join(chDir, rel), 'utf8')
      const baseName = path.basename(rel) // hello_server.c
      const outMd = path.join(outDir, `${rel}.md`) // .../hello_server.c.md 或 .../homework/kehou4.c.md
      mkdirp(path.dirname(outMd))
      // 相对返回链接：源码页可能在 homework/ 子目录，需回到章节根
      const depth = rel.split('/').length - 1
      const backHref = depth > 0 ? `${'../'.repeat(depth)}` : './'
      const md = `---
title: ${baseName}
sidebar: false
---

<a class="back-to-chapter" href="${backHref}">← 返回${ch.title}</a>

# ${baseName}

\`\`\`c
${code}
\`\`\`
`
      fs.writeFileSync(outMd, md)
      count++
    }
  }
  return count
}

// ---------- 链接重写 ----------
// 形态3：绝对 GitHub URL → 相对 clean URL（已验证均同章，但写稳健分支）
function rewriteGitHubLinks(md, curCh) {
  const re =
    /\]\(https:\/\/github\.com\/riba2534\/TCP-IP-NetworkNote\/blob\/master\/([^)]+)\)/g
  return md.replace(re, (m, p) => {
    // p 形如 "ch18/thread4.c" 或 "ch18/homework/x.c"
    const parts = p.split('/')
    const targetCh = parts[0]
    const file = parts[parts.length - 1]
    if (targetCh === curCh) {
      // 同章：直接文件名（章节页在 chXX/index.md，同目录）
      return `](${file})`
    }
    // 跨章：相对路径
    return `](../${targetCh}/${file})`
  })
}

// 形态1/2：相对链接保持不变（镜像目录结构下天然正确）
// 但为避免 VitePress 对无扩展名链接报死链，给同章裸 .c 链接加 ./ 前缀
function normalizeRelativeLinks(md) {
  // 匹配 ](xxx.c) 但不匹配已带 ./ 或 ../ 或 / 或 http 的
  return md.replace(/\]\((?!\.\/|\.\.\/|\/|https?:|mailto:|#)([^)]+\.c)\)/g, (m, p) => `](./${p})`)
}

// 图片路径：](images/ → ](/images/
function rewriteImages(md) {
  return md.replaceAll('](images/', '](/images/')
}

// ---------- 步骤 D：转换章节 README → index.md ----------
function genChapterPages() {
  let count = 0
  for (let i = 0; i < CHAPTERS.length; i++) {
    const ch = CHAPTERS[i]
    const srcReadme = path.join(REPO_ROOT, ch.dir, 'README.md')
    if (!fs.existsSync(srcReadme)) {
      console.warn(`[warn] 章节笔记不存在: ${srcReadme}`)
      continue
    }
    let raw = fs.readFileSync(srcReadme, 'utf8')

    // D1. 去掉首行 H1（## 第X章 …），由 frontmatter title 接管
    raw = raw.replace(/^##\s+[^\n]*\n+/, '')

    // D2. 图片路径转换
    raw = rewriteImages(raw)

    // D3. 代码链接重写
    raw = rewriteGitHubLinks(raw, ch.dir)
    raw = normalizeRelativeLinks(raw)

    // D4. 追加"本章源码"附录
    const chDir = path.join(REPO_ROOT, ch.dir)
    if (fs.existsSync(chDir)) {
      const cFiles = listCFiles(chDir)
      if (cFiles.length > 0) {
        raw += `\n\n## 本章源码\n\n`
        for (const rel of cFiles) {
          const baseName = path.basename(rel)
          // 章节页在 chXX/index.md，源码页在 chXX/<rel>.md，相对链接加 ./ 前缀
          raw += `- [${baseName}](./${rel})\n`
        }
      }
    }

    // D5. 注入 frontmatter（title + prev/next）
    const prev =
      i > 0
        ? { text: CHAPTERS[i - 1].title, link: `/${CHAPTERS[i - 1].dir}/` }
        : { text: '首页', link: '/' }
    const next =
      i < CHAPTERS.length - 1
        ? { text: CHAPTERS[i + 1].title, link: `/${CHAPTERS[i + 1].dir}/` }
        : null

    const fm = [`---`, `title: ${ch.title}`, `prev:`, `  text: ${prev.text}`, `  link: ${prev.link}`]
    if (next) {
      fm.push(`next:`, `  text: ${next.text}`, `  link: ${next.link}`)
    }
    fm.push('---', '', `# ${ch.title}`, '')

    const outDir = path.join(SITE_ROOT, ch.dir)
    mkdirp(outDir)
    fs.writeFileSync(path.join(outDir, 'index.md'), fm.join('\n') + raw)
    count++
  }
  return count
}

// ---------- 主流程 ----------
console.log('▶ 清理生成物...')
clean()
console.log('▶ 复制图片...')
const imgCount = copyImages()
console.log(`  图片: ${imgCount}`)
console.log('▶ 生成 .c 源码页...')
const srcCount = genSourcePages()
console.log(`  源码页: ${srcCount}`)
console.log('▶ 转换章节 README...')
const chCount = genChapterPages()
console.log(`  章节页: ${chCount}`)
console.log(`✓ 完成：${chCount} 章 + ${srcCount} 源码页 + ${imgCount} 图片`)
