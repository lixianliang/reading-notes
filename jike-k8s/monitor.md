# monitor

## k8s内部监控的指标
1. metrics 是宿主机监控的数据
1. 来自k8s的api server、kubelet等组件
1. k8s相关，pod node 容器 svc等metics
## kube-aggregator
    kube-aggregator其实就是一个根据URL选择具体的API后端代理服务器，可以扩展出custmon external的指标

## 业界通用指标监控规划
1. USE原则,规则资源指标
    利用率（Utilization），资源被有效利用起来提供服务的平均时间占比
    饱和度（Saturation），资源拥挤的程度，比如工作队列的长度
    错误率（Errors），错误的数量

1. RED原则，规划服务指标
    每秒请求数量（Rate）
    每秒错误数量（Errors）
    服务响应时间（Duration）
