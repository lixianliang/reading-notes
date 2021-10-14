# csi

## 安全容器
1. 宿主机器内核为3.6，但应用需要linux4.0，这时候katacontainer可以解决，gvisor不能（模拟出来的）

## kubelet
    kubelet 调用下层容器运行时的执行过程，并不会直接调用 Docker 的 API，而是通过一组叫作 CRI（Container Runtime Interface，容器运行时接口）的 gRPC 接口来间接执行的
    kubelet就是一个控制循环，syncloop
1. pod更新事件
1. pod生命周期变化
1. kubelet本身设置的执行周期
1. 定时的清理事件
