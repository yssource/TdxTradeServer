1.8 增加批量操作相关接口支持
----
* 修改部分参数，变量名称，统一命名风格
* 增加批量操作接口QueryDatas, GetQuotes, CancelOrders, SendOrders支持

1.7 修复logoff的时候，指定错误的client_id时会出现服务器崩溃的问题 2018-03-22
----
* 修复logoff时程序崩溃bug issue #7
* 增加`active_clients`配置项和`get_active_clients`命令。

1.6 增加query_history_data接口 2018-02-05
----
* 增加query_history_data接口

1.5 多账户版本开发完成 2017-12-18
----
* 支持多账户交易，自动生成对应的trade.dll
* 对并发api调用加锁。
* 大幅修改并优化了conan依赖包设置。

0.1 基础版本完成
----
* 基于http的rest api
* aes128的加密传输模式
