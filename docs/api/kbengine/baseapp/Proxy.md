# Proxy类

[KBEngine模块]

Proxy是KBEngine模块的一部分。[更多...](#detailed_description)

```python
import KBEngine
```

## 父类

[Entity](./Entity.md)

## 成员函数

- def [disconnect](#disconnect)( self ):
- def [getClientType](#getClientType)( self ):
- def [getClientDatas](#getClientDatas)( self ):
- def [giveClientTo](#giveClientTo)( self, proxy ):
- def [streamFileToClient](#streamFileToClient)( self, resourceName, desc="", id=-1 ):
- def [streamStringToClient](#streamStringToClient)( self, data, desc="", id=-1 ):

## 回调函数

- def [onClientDeath](#onClientDeath)( self ):
- def [onClientGetCell](#onClientGetCell)( self ):
- def [onClientEnabled](#onClientEnabled)( self ):
- def [onGiveClientToFailure](#onGiveClientToFailure)( self ):
- def [onLogOnAttempt](#onLogOnAttempt)( self, ip, port, password ):
- def [onStreamComplete](#onStreamComplete)( self, id, success ):

## 属性

| [__ACCOUNT_NAME__](#__ACCOUNT_NAME__) | 只读string |
| --- | --- |
| [__ACCOUNT_PASSWORD__](#__ACCOUNT_PASSWORD__) | 只读string |
| [clientAddr](#clientAddr) | 只读 |
| [clientEnabled](#clientEnabled) | 只读bool |
| [hasClient](#hasClient) | 只读bool |
| [roundTripTime](#roundTripTime) | 只读 |
| [timeSinceHeardFromClient](#timeSinceHeardFromClient) | 只读 |

<a id="detailed_description"></a>

## 详细描述

[Proxy](./Proxy.md)是[Entity](./Entity.md)的一个特殊类型，它继承自Entity，它有一个关联的客户端。本身来说，
它就是一个代理客户端的实体，操控所有服务端向客户端的更新。不能在脚本直接创建[Proxy](./Proxy.md)类对象。

## 成员函数文档

<a id="disconnect"></a>

### def disconnect(self):

断开客户端连接。

<a id="getClientType"></a>

### def getClientType(self):

功能说明：
这个函数返回客户端类型。

返回:

- UNKNOWN_CLIENT_COMPONENT_TYPE = 0,

<a id="getClientDatas"></a>

### def getClientDatas(self):

功能说明：
这个函数返回客户端登录时和注册时所附带的数据。
此数据可用于运营系统扩展，如果连接了第三方账号服务，此数据会经过interfaces进程发往第三方服务系统。

返回:

- tuple, 固定为2个元素的tuple(登陆数据bytes，注册数据bytes)，第一个元素为登陆时客户端调用登陆时传入的datas参数， 第二个元素为注册时客户端调用注册所传入的datas参数。由于可以存储任意二进制数据，因此他们都是以bytes类型存在。

<a id="giveClientTo"></a>

### def giveClientTo(self, proxy):

功能说明：
将当前 Proxy 绑定的客户端连接转交给另一个 Proxy。
它转移的不只是一个“控制标记”，而是整条客户端绑定链：旧 Proxy 会失去 `clientEntityCall`；如果旧 Proxy 已经有 cell，还会通知 cell 侧丢失 Witness；客户端随后销毁旧控制实体；目标 Proxy 接手同一条客户端通道并重新下发 `onCreatedProxies()`，如果目标已经有 cell，则继续重建 Witness、可见集和客户端控制链路。

注意：

- 当前 Proxy 必须已经绑定客户端。
- 目标 Proxy 不能为空、不能是自身、不能已经绑定客户端。
- 如果目标 Proxy 还没有 cell，迁移完成后只会先完成 Base 侧绑定，后续仍要等它获得 cell 才会恢复视野同步。
- 源码里虽然接受 `None` 并转成 `NULL`，但该路径不会真正执行迁移，实际使用应传有效目标 Proxy。

源码解析：

- [空间、AOI 与视野同步：`Proxy.giveClientTo()` 转移的其实是客户端绑定与 Witness 链路](/architecture/source-analysis/space-aoi.html#proxy-give-client-to)

参看：

- [Proxy](./Proxy.md).[onGiveClientToFailure](./Proxy.md#onGiveClientToFailure)

参数：

| proxy | 控制权将转交给这个实体。 |
| --- | --- |

<a id="streamFileToClient"></a>

### def streamFileToClient(self, resourceName, desc="", id=-1):

功能说明：
这个函数类似于[streamStringToClient()](./Proxy.md#streamStringToClient)，将一个资源文件发送给客户端。发送过程在不同的线程上操作，因此不会危及主线程。

参看：

- [Proxy](./Proxy.md).[onStreamComplete](./Proxy.md#onStreamComplete)

参数：

| resourceName | 要发送的资源名称，包含路径。 |
| --- | --- |
| desc | 一个可选的字符串，发送给客户端的资源描述。 |
| id | 一个16位的id，它的值完全取决于调用者。如果传入-1系统将会在队列里面选择一个没有在用的id。可以在客户端根据这个id做资源判断。 |

返回：

- 与这个下载关联的id。

<a id="streamStringToClient"></a>

### def streamStringToClient(self, data, desc="", id=-1):

功能说明：
发送一些数据到当前实体绑定的客户端。如果客户端端口则数据被清除，
当客户端再次绑定到实体的时候才可调用这个函数。16位的id完全取决于调用者。
如果调用者没有指定这个ID则系统会分配一个未用过的id。可以在客户端根据这个id做资源判断。
你可以在Proxy的派生类中定义回调函数（onStreamComplete），所有数据成功发送给客户端时或下载失败时会调用这个回调函数。
参看：[Proxy](./Proxy.md).[onStreamComplete](./Proxy.md#onStreamComplete)与客户端实体Entity.onStreamDataStarted和Entity.onStreamDataRecv还有Entity.onStreamDataCompleted。

参数：

| data | 要发送的字符串。 |
| --- | --- |
| desc | 一个可选的字符串，发送给客户端的描述。 |
| id | 一个16位的id，它的值完全取决于调用者。如果传入-1系统将会在队列里面选择一个没有在用的id。 |

返回：

- 与这个下载关联的id。

## 回调函数文档

<a id="onClientDeath"></a>

### def onClientDeath(self):

如果在脚本中实现了此回调，这个方法将在客户端断开连接时被调用。 这个方法没有参数。

<a id="onClientGetCell"></a>

### def onClientGetCell(self):

如果在脚本中实现了此回调，当客户端能够调用实体的cell属性时，该回调被调用。

<a id="onClientEnabled"></a>

### def onClientEnabled(self):

如果在脚本中实现了此回调，当实体可用时（ 各种初始化完毕并且可以与客户端通讯 ）该回调被调用。 这个方法没有参数。
注意：giveClientTo将控制权赋给了该实体时也会导致该回调被调用。

<a id="onGiveClientToFailure"></a>

### def onGiveClientToFailure(self):

如果在脚本中实现了此回调，当实体调用[giveClientTo](#giveClientTo)失败时，该回调被调用。这个方法没有参数。

<a id="onLogOnAttempt"></a>

### def onLogOnAttempt(self, ip, port, password):

如果在脚本中实现了此回调，这个函数在客户端尝试使用当前账号实体进行登录时触发回调。
这种情况通常是实体存在于内存中处于有效状态，最明显的例子是用户A使用此账号登录了，用户B使用同一账号进行登录，此时回调触发。
这个回调函数可以返回如下常量值：
KBEngine.LOG_ON_ACCEPT：允许新的客户端与实体进行绑定，如果实体已经绑定了一个客户端，之前的客户端将被踢出。
KBEngine.LOG_ON_REJECT：拒绝新的客户端与实体绑定。
KBEngine.LOG_ON_WAIT_FOR_DESTROY：等待实体销毁后再进行客户端绑定。

参数：

| ip | 尝试登录的客户端IP地址。 |
| --- | --- |
| port | 尝试登录的客户端连接的端口。 |
| password | 用户登录时使用的MD5密码。 |

<a id="onStreamComplete"></a>

### def onStreamComplete(self, id, success):

如果在脚本中实现了此回调，当用户使用[Proxy](./Proxy.md).[streamStringToClient](./Proxy.md#streamStringToClient)()或[Proxy](./Proxy.md).[streamFileToClient](./Proxy.md#streamFileToClient)()完成时，该回调被调用。

参数：

| id | 与下载关联的id。 |
| --- | --- |
| success | 成功与否。 |

## 属性文档

<a id="__ACCOUNT_NAME__"></a>

__ACCOUNT_NAME__

说明：
如果proxy是帐号则可以访问__ACCOUNT_NAME__得到帐号名。

<a id="__ACCOUNT_PASSWORD__"></a>

__ACCOUNT_PASSWORD__

说明：
如果proxy是帐号则可以访问__ACCOUNT_PASSWORD__得到帐号MD5密码。

<a id="clientAddr"></a>

clientAddr

这是一个tuple对象，包含了客户端的ip与端口。

<a id="clientEnabled"></a>

clientEnabled

实体是否已经可用。在实体可用之前脚本不能与客户端进行通讯。

<a id="hasClient"></a>

hasClient

Proxy是否绑定了一个客户端连接。

<a id="roundTripTime"></a>

roundTripTime

在一段时间内服务器与这个Proxy绑定的客户端通讯平均往返时间。这个属性只在Linux下生效。

<a id="timeSinceHeardFromClient"></a>

timeSinceHeardFromClient

最后一次收到客户端数据包时到目前为止所过去的时间（秒）。
