# daemonset

## 原理
    daemonset只管理pod对象，然后nodeAffinity和Toleration调度功能保证每个节点只有一个pod

    demonset通过ControllerRevision来保存和管理对应的版本，类似statefulset也是使用ControllerRevision；但其原理和daemonset控制replicset还有点不一样，回滚操作还是生成一个新的版本号

    
$ kubectl get controllerrevision -n kube-system -l name=fluentd-elasticsearch
NAME                               CONTROLLER                             REVISION   AGE
fluentd-elasticsearch-64dc6799c9   daemonset.apps/fluentd-elasticsearch   2          1h

$ kubectl rollout undo daemonset fluentd-elasticsearch --to-revision=1 -n kube-system
daemonset.extensions/fluentd-elasticsearch rolled back
