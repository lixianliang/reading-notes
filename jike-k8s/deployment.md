# deployment

## deployment
    deployment多个版本的replicaset，做滚动升级
    replicaset控制pod的个数，做扩容缩容

## replicaset
    replicaset的命名就是pod template hash加上deployment名字的组合
    在做deployment变更时，会重新生成一个新的rs

## deployment的回滚操作
    kubectl rollout histroy deployment xxx
    kubectl rollout undo deployment xxx 回滚到最近一个版本
    kubectl rollout undo deployment xxx --to-revision=xxx 回滚到指定版本 貌似默认保留5个版本
    kubectl apply -f xxx --record xxx 变更记录
    比如上次遇到新加坡变更出问题，可以通过这回滚操作快速恢复到之间的状态
