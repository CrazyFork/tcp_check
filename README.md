## About

这个Repo fork 自 https://github.com/YanagiEiichi/tcp-check, 

这个Repo的目的就是用来扫描目标端口是否开放状态。用setsocketopt这个system api来设置socket选项，以一个比较低的成本（不会）
对目标服务器造成过多干扰。具体的解释看最下边的文档链接。

这个库的逻辑就是, 
    * 用libuv init 一个socket
    * 将socket转成fd(file descriptor)
    * 用setsockopt方法设置socket选项
    * 调用libuv的 `uv_tcp_connect` 方法尝试链接
    * 收集结果


用法:
```js
    return tcpCheck('127.0.0.1', 1).then(() => {
      throw new Error('must throw');
    }, error => {
      error.name.should.equal('ECONNREFUSED');
    })
```

对于我来说这个库可参考的点：
* nodejs native addon 的一个 demo
* libuv， v8，宏的使用


todo:
* 能否用rust写一个类似的addon，skip掉libuv这一层
* 还有libuv这一层有必要要么，貌似是解决了client端的并发。如果裸写的的话，可能并发的问题是需要解决的。
* 解决掉 libuv 的教程，有必要看么？
    * http://luohaha.github.io/Chinese-uvbook/source/introduction.html









docs <- https://yanagieiichi.github.io/tcp-check/
