# pod

## infra容器

### pod支持的4中容器
    标准容器 sidecar init ephemeral容器

1. init
    最先执行

1. sidecar
    从v1.18开始，内置的sidecar将确保sidecar容器在正常之前开始行运行；只是运行起来，但并不保证所有数据有load好（比如 istio proxy需要从控制面板获取路由信息）
    导致jimu启动貌似有问题

1. 应用例子
    容器日志
    istio网络控制
    共享镜像（init）

### pod共享
1. istio-proxy利用network的共享，进行网络协议上的拦截
1. 修改pod的etc/hosts，同一个pod中的其它容器也是可见的
1. pod shareProcessNamespace开启，同一pod中其它容器互相可见
1. istio proxy的服务发现没有修改etc/hosts k8s pod的hosts会有pod的name，应该是给健康检查用的

## lifecycle
    postStart: 在容器ENTERPOINT之后执行，但有可能ENTERPOINT还没执行完成，postStart开始启动了；并不验证保证顺序
    preStop：容器kill之前，严格保证顺序

## pod状态
    pending
    running
    successed
    failed
    unknown

    细分条件：podschedule ready initalized unschedule
    ready： 已经对外提供服务，健康检查成功

## secret
    Secret 对象要求这些数据必须是经过 Base64 转码的，以免出现明文密码的安全隐患
    通过volumn挂载到本地文件

    kubelet会维护这些volumn，自动更新configmap secret，但会有一定的延迟；另外需要程序支持config的reload或者利用相对应的组件动态读取配置
    configmap和secret类似，但不需要base64的要求


## Downward
    接下来是 Downward API，它的作用是：让 Pod 里的容器能够直接获取到这个 Pod API 对象本身的信息

## service account
    像这样的 Service Account 的授权信息和文件，实际上保存在它所绑定的一个特殊的 Secret 对象里的。这个特殊的 Secret 对象，就叫作 ServiceAccountToken

## liveness
    容器的健康检测，当容器不健康会做容器的重启，会影响到pod的生命周期
    
## pod preset模板
    可以认为是pod模板通用字段
``` shell
apiVersion: settings.k8s.io/v1alpha1
kind: PodPreset
metadata:
  name: allow-database
spec:
  selector:
    matchLabels:
      role: frontend
  env:
    - name: DB_PORT
      value: "6379"
  volumeMounts:
    - mountPath: /cache
      name: cache-volume
  volumes:
    - name: cache-volume
      emptyDir: {}
```

## readinessProbe
    readinessProbe 检查结果的成功与否，决定的这个 Pod 是不是能被通过 Service 的方式访问到，而并不影响 Pod 的生命周期

## 自己线上可以将transgender
    修改只有含有readinessProbe；后续注意readinessProbe要晚于liveness，这样保证svc的访问是可靠的pod
    transgender目前是这样配置的
