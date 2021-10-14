# docker

## 白话容器基础（三）：深入理解容器镜像
    https://time.geekbang.org/column/article/17921

1. 容器的rootfs包含3层 操作系统文件 共用宿主机器的操作系统内核
    可读写层: 比如删除某些文件，可在without文件，把需要删除的文件遮挡起来
    init层: 包含/etc/host /etc/resolv.conf等文件信息
    只读层: 包含基础镜像
