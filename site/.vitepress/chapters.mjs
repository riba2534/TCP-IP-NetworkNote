// 19 章数据 — config.mts 与 scripts/build.mjs 共享的单一数据源
// 新增/调整章节只改这里一处
// 用 .mjs（纯 JS）以便 Node 脚本和 VitePress 配置都能直接 import

export const CHAPTERS = [
  { dir: 'ch01', title: '第 1 章 理解网络编程和套接字' },
  { dir: 'ch02', title: '第 2 章 套接字类型与协议设置' },
  { dir: 'ch03', title: '第 3 章 地址族与数据序列' },
  { dir: 'ch04', title: '第 4 章 基于 TCP 的服务端/客户端（1）' },
  { dir: 'ch05', title: '第 5 章 基于 TCP 的服务端/客户端（2）' },
  { dir: 'ch06', title: '第 6 章 基于 UDP 的服务端/客户端' },
  { dir: 'ch07', title: '第 7 章 优雅地断开套接字的连接' },
  { dir: 'ch08', title: '第 8 章 域名及网络地址' },
  { dir: 'ch09', title: '第 9 章 套接字的多种可选项' },
  { dir: 'ch10', title: '第 10 章 多进程服务器端' },
  { dir: 'ch11', title: '第 11 章 进程间通信' },
  { dir: 'ch12', title: '第 12 章 I/O 复用' },
  { dir: 'ch13', title: '第 13 章 多种 I/O 函数' },
  { dir: 'ch14', title: '第 14 章 多播与广播' },
  { dir: 'ch15', title: '第 15 章 套接字和标准 I/O' },
  { dir: 'ch16', title: '第 16 章 关于 I/O 流分离的其他内容' },
  { dir: 'ch17', title: '第 17 章 优于 select 的 epoll' },
  { dir: 'ch18', title: '第 18 章 多线程服务器端的实现' },
  { dir: 'ch24', title: '第 24 章 制作 HTTP 服务器端' },
]
