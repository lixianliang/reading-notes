# scheduler

    pod为k8s最小的调度单位

## 扩容方式
    CA HPA VPA
    VPA: 调整pod的cpu、内存大小，重启pod
    
## 资源模型
    cpu为可压缩资源，可压缩资源不足时pod会饥饿但不会退出
    内存为不可压缩资源，不足时则pod会oom

## 资源设置
    没有配置request cpu，cpu share默认是1024相当于是1000mcpu
    当limits cpu设置为500m，则cpu cfs_quota_us只为100ms，则只能使用cpu的50%

    k8s建议使用较小的requests值，使用较大的limits值

## pod qos的三种分类
    guaranteed:(保证) requests和limits都设置且相等 (只设置了limit，默认两者相等)
    burstable(易爆的): 设置了requests 或者 request limit不一致
    besteffort(最大努力)：没有设置request

    当资源紧张，首先会删除besteffort类别pod；然后是burstable且发生发生饥饿的资源使用量远远超过requests的pod

## cpuset
    cpuset就是将容器绑定到某个cpu核上，而不是cpushart那样共享cpu的计算能力
    减少cpu的上线文切换，个人感觉cpu>=2才做相对应的绑定
    要求：为guaranteed的类型pod cpu的requests和limits为相同的整数

## 理解
    当cpu小时1000m，并不会先pod的cpu计算限制在一个cpu上的
## 默认调度器
    默认调度器就是为一个新创建的出来的pod，寻找一个最合适的节点
1. 从集群所有的节点中，根据调度算法挑出所有可以运行该pod的节点
2. 从第一步结果中，再根据调度算法挑一个符合条件的节点最为最终结果
3. 调度器先调用一组叫做predicate调度算法来检查每个node；然后再调用一组priority调度算法给上一步得到结果里的每个node打分，选择得分最高的那个node
4. 调度器对一个pod调度成功，实际就是将它的spec.nodeName字段填上调度结果的节点名字

理解：从上面看调度器对应两个控制循环，informer path和scheduling path
### informer path
    启动informer wathch etcd中的pod、node、svc等与调度相关的API对象的变化，比如当一个待调度的pod被创建出来之后，调度器会通过pod informer的handler，将这个待调度的pod添加到调度队列
    放到一个优先级队列priority queue；另外负责对调度器缓存进行更新

### scheduling path
    负责pod调度的主循环，从调度队列获取一个pod，调用predicate算法进行过滤；这一步过得到一组node，在调用priority算法进行打分，分数从0到10，最高得分node作为调度的结果
    调度器将pod的对象nodeName字段修改为调度的node，这个称为bind;调度器更新scheduler cache里的pod、node信息叫做assume；后面才会异步向apiserver发起pod请求，完成真正的bind
    优化：并行计算法node得分和过滤计算？

## 调度
在 Scheduling Path 上，调度器会启动多个 Goroutine 以节点为粒度并发执行 Predicates 算法，从而提高这一阶段的执行效率。而与之类似的，Priorities 算法也会以 MapReduce 的方式并行计算然后再进行汇总

Kubernetes 调度器只有对调度队列和 Scheduler Cache 进行操作时，才需要加锁

pod调度过程都是串行执行

节点taint污点，节点toleration容忍，在daemonset用的比较多

Predicates计算的信息可以缓存

## 优先级和抢占式调度
### activeQ
    在activeQ中的pod都是下一个周期需要调度的pod
### unschedulableQ
    专门调度存放调度失败的pod，给一个重生的机会

