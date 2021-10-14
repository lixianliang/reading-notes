# control

## 控制器实现模式
1. dep控制器从etcd获取所有app=nginx的pod，然后统计状态
1. dep对象的replicas字段的值就是期望状态
1. dep控制器将两个状态比较，根据结果是否创建、删除pod

这个过程就称为同步循环，使用对象控制对象


## 控制器的实现
Initializer： 动态admission，动态插件；在yaml文件添加一些字段

Envoy相比haproxy nginx的有点：可以通过api的方式动态流量管理

CRD：自定义API资源
自定义networks资源

``` yaml
apiVersion: apiextensions.k8s.io/v1beta1
kind: CustomResourceDefinition
metadata:
  name: networks.samplecrd.k8s.io
spec:
  group: samplecrd.k8s.io
  version: v1
  names:
    kind: Network
    plural: networks
  scope: Namespace
```

Informer就是一个自带缓存和索引机制，可触发handler的客户端库，这个本地缓存在k8s中称为store，索引一般被称为index
informaer使用了reflector包，通过ListAndWathc机制获取并监视API对象变化的客户端封装
Reflector和informer之间用到了队列进行协同，而informer与你要编写的控制循环之间则使用了一个工作队列来协同


学习k8s
https://github.com/caicloud/kube-ladder

## RBAC
    角色权限控制

Role就是一组权限规则列表，通过RoleBinding对象，将被作用者和权限列表进行绑定
ClusterRoleBinding和ClusterRole则是k8s集群级别的Role和RoleBinding，它们的作用范围不受namespace限制
