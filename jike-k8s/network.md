# network


## 概念
网桥：docker0 cni0
虚拟网卡：vethxxx
pod内部的：eth0

## BGP
    边界网关协议
    自动添加路由协议，不需要管理员主动添加

## 网卡
    一个接收信息 转换信息 暂储信息的一个硬件。它是把接受到信息递交给上层，如（CUP）的一个接口

## 网桥
   工作在数据链路层，在不同或相同类型的lan之间存储并转发数据帧，必要时进行链路层上的协议转换 
    是把两个不同物理层，不同MAC子层，不同速率的局域网连接在一起
    
## 路由器
    连接因特网中各局域网、广域网的设备；用于连接多个逻辑上分开的网络
    用于lan-wan节点数据交互
    
## 交换机
    作用在数据链路层（2层），路由发生在第三层网络层
    工作在数据链路层，原理等同于多端口网桥
    用于lan内部节点数据交互

## 浅谈容器
    在linux中能够起到虚拟交换机作用的网络设置是网桥bridge，他是一个工作在数据链路层的设备，主要功能是根据mac地址学习来讲数据包转发到网桥不同的端口上（这里的端口就是网络接口，对应到不同的局域网内）

### docker网络
    docker会在宿主机器创建一个docker0网桥
    如何将容器连接到docker0网桥，使用veth pair虚拟设备
    veth pair设备：被创建出来总是以两张虚拟网卡形式出现，从其中一个“网卡”发出的数据包，可以直接出现在与它对应的另一张“网卡”上，哪怕这两个“网卡”在不同的 Network Namespace 里。
    这使得veth pair常常用作连接不同的network namespace的网线

    一台主机多个容器的在docker0上会有对应的虚拟网卡，多个容器间直接通过二层网络发往目的容器

## 深入容器网络

flannel三种网络实现方式：vxlan host-gw udp

### tun设备
    在操作系统内核和用户应用程序之间传递ip包，tun设备是是一种工作在三层的虚拟网络设备

### udp
    flannel udp模式是一个三层的overlay网络，flannel在不同的宿主机器上两个容器之间打通了一条隧道
    ip包发出需要经过三次内核态切换操作 flannel udp封包在用户态实现；多了两次上下文的切换
    在三层网络上覆盖一层三层网络，通过tun设备转发对应的ip包

### vxlan
    在现有的三层网络上，实现由内核vxlan模块负责维护的二层网络，使得连接这个vxlan二层网络上的主机之间可以像在局域网lan那样通信
    为了打通二层网络，vxlan会在宿主机器上设置一个特殊的网络设备岁尾隧道的两端 vtep
    vtep和flannel进程很相似，不过封装和解封是在二层数据帧，而这个工作执行流程全部在内核里完成，vxlan是linux内核中一个模块

1. 当数据从node1的容器1发送到node2的容器2
    容器1的数据通过容器2的ip地址路由到docker0网桥上过程
    首先通过容器网络进行docker0网桥，docker0网桥根据路由规则匹配子网，匹配到flannel的设备（所有flannel设备是在一个大的网段里面，每个pod就是大的虚拟网络下的一个子网）
    docker0网桥根据路由规则路由到flnanel1设备上，这个时候协带的是容器的原始ip包
        当node2启动加入flannel网络只有，在node1（其它所有节点）上，flanneld会天机一条路由规则10.1.16.0 10.1.16.0 255.255.255.0 UG 0 0 0 flannel.1
        docker0网桥根据路由规则发往10.1.16.0/23网段的ip包都需要经过flannel.1设备发出，并且发往的网关地址是10.1.16.0 10.1.16.0正式node2上的vtep（也就是flannel.1设备）的ip地址
        将原始ip包加上目的flannel mac地址，封装成一个二层数据帧，然后发送给目的vtep设置；此包称为内部数据帧
        目的flannel mac地址通过arp协议获得

1. flannel设备将内部数据帧+vxlan头封装成一个udp包(四层),此时不知道对应宿主机地址(node2拆包发现有vxlan头 vni=1，自动将包转发给flannel.1设备处理)
    flannel.1设备扮演网桥的角色，在二层网络进行udp包的转发
    bridge fdb show flannel.1 | grep 5e:f8:4f:00:e3:375e:f8:4f:00:e3:37 dev flannel.1 dst 10.168.0.3 self permanent 
    flannel设备维护一条路由规则，发现目的vtep设备的二层数据帧，应该通过flannel.1设备发往ip地址为10.168.0.3主机，这个就是node2
1. 接下来的流程就是一个正常的宿主机网络的封包过程
    UDP 包是一个四层数据包，所以 Linux 内核会在它前面加上一个 IP 头，即原理图中的 Outer IP Header，组成一个 IP 包
    然后再加上mac地址形成一个二层数据帧，从eth0网卡发出；这个帧数据经过宿主机器来到node2的eth0网卡

1. node2的内核网络栈发现这个数据帧有vxlan header，并且为1，所以内核会对他进行拆包拿到里面的内部数据帧根据vni值交给node2的flannel.1设备
    而flannel.1设备则进行进一步拆包，取出原始ip包将包转发到node2的容器2的network namespace里面
    可看单机容器网络的处理流程(匹配对应的ip的路由规则)

## k8s网络模型和cni网络插件
    网络插件需要做的事情通过某种方法，把不同宿主机器上的特殊特备联通，从而达到容器跨主机通信目的
    k8s通过cni接口，维护了一个单独网桥替代docker0这个网桥就是cni，默认名称为cni0

## 纯三层网络方案host-gw
   host-gw的性能损失在10%左右，而vxlan性能损失在20-30%左右；最高效的直接使用host主机网络 
    Flannel host-gw 模式必须要求集群宿主机之间是二层连通的

    === host充当网关
    host-gw的模式工作原理，其实就是将每个flannel子网flannel subset的下一跳设置成子网对应的宿主机的ip地址，也就是这台host会充当这条容器通信路径的网关；这就是host-gw的含义
   ip包从网络层进入链路层封装成帧的时候，eth0设备就会使用下一跳地址对应的mac地址，作为该数据帧的目的mac地址，显然这个mac地址正是node2的mac地址
    而 Node 2 的内核网络栈从二层数据帧里拿到 IP 包后，会“看到”这个 IP 包的目的 IP 地址是 10.244.1.3，即 Infra-container-2 的 IP 地址。这时候，根据 Node 2 上的路由表，该目的地址会匹配到第二条路由规则（也就是 10.244.1.0 对应的路由规则），从而进入 cni0 网桥，进而进入到 Infra-container-2 当中
    直接将三层的数据包通过2层封装的做了一下转换
    节点加入，需要在每台机器上加一条路由规则，通过etcd watch可以实现

1. calico
    没有使用etcd，通过bgp自动在整个集群中分发路由信息 不会在主机上创建任何网桥设备
    所谓 BGP，就是在大规模网络中实现节点路由信息共享的一种协议。
    bgp：路由协议是自动添加，自治路由协议，不需要人工添加

    阿里云的terway貌似是基于calico，里面包含Felix

    问题：calico默认配置为node-to-node mesh模式，随着节点数量指数增加会带来网络压力；需要配置router relector模式
    在这种模式下，Calico 会指定一个或者几个专门的节点，来负责跟所有节点建立 BGP 连接从而学习到全局的路由规则。而其他节点，只需要跟这几个专门的节点交换路由信息，就可以获得整个集群的路由规则信息了 

    我在前面提到过，Flannel host-gw 模式最主要的限制，就是要求集群宿主机之间是二层连通的。而这个限制对于 Calico 来说，也同样存在
    不难看到，当 Calico 使用 IPIP 模式的时候，集群的网络性能会因为额外的封包和解包工作而下降。在实际测试中，Calico IPIP 模式与 Flannel VXLAN 模式的性能大致相当。所以，在实际使用时，如非硬性需求，我建议你将所有宿主机节点放在一个子网里，避免使用 IPIP。

## dns svc发现

### svc的iptable vs ipvs模式
1. iptable实现的svc，对应的服务需要大量的iptable规则，pod频繁变动会导致规则不停的变更和刷新
    通过vip的方式代理后端的pod
1. ipvs原理
    kube-proxy在宿主机上创建虚拟网卡kube-ipvs0并分配svc vip作为地址
    为这个vip设置三个ipvs虚拟主机，通过rr来做负载均衡，可通过ipvsadm来设置
    ipvs在内核中也是基于netfilter的nat模式，在转发这个ipvs有显著的性能提升，将规则处理放到内核态
``` shell
    设置对应的ipvs规则替代之前的iptable规则
# ipvsadm -ln
 IP Virtual Server version 1.2.1 (size=4096)
  Prot LocalAddress:Port Scheduler Flags
    ->  RemoteAddress:Port           Forward  Weight ActiveConn InActConn     
  TCP  10.102.128.4:80 rr
    ->  10.244.3.6:9376    Masq    1       0          0         
    ->  10.244.1.7:9376    Masq    1       0          0
    ->  10.244.2.3:9376    Masq    1       0          0
```

## 外部访问svc三种方式
### nodeport
    直接在svc里面设置nodeport端口做代理 端口的范围默认是 30000-32767
    svc需要对ip进行snat，返回只有再进行dnat，保证ip连接的正确性
    先通过iptalbe进行路由选择负责（使用随机）；在进行snat；将这个ip包的源地址替换为这台宿主机器的cni网桥地址或者宿主机本身的ip地址
    snat操作只会对svc转发出来的ip包进行操作，后面根据这个包的0x4000标记在进行dnat

1. 使用snat的原因
    保证ip包链路是原路返回
    所以pod里面的服务是不知道client的源地址的；目前tx aliyun推出了类似terway这种网络；pod的ip和ecs处于不同的网络中且方便lb直接代理pod ip和混合云的打通

### lb
    就是lb类型的svc

### externalName

### 区分是是svc配置问题还是集群dns问题，可检查k8s自己的master节点的svc dns记录是否正常
``` shell
[ root@curl:/ ]$ nslookup kubernetes.default
Server:    192.168.160.10
Address 1: 192.168.160.10 kube-dns.kube-system.svc.cluster.local

Name:      kubernetes.default
Address 1: 192.168.160.1 kubernetes.default.svc.cluster.local
```
    
### snat的理解
    k8s pod里面的服务默认会开启snat，不然没办法访问外网

## ingress
    为svc的svc；代理多个不同的svc；如果是svc直接挂到slb则需要多个外网服务；不过ingress只能代理7层服务
    ingress为k8s里面的反向代理，里面可以定义多个ingress rule
    ingress controller可以有不同的实现
