# Proxy类

| KBEngine |

KBEngine
---

# Proxy类
Proxy类[KBEngine模块]

[[KBEngine模块](../Modules/KBEngine.html)KBEngine模块]Proxy是KBEngine模块的一部分。 更多...

Proxy是[KBEngine](../Modules/KBEngine.html)KBEngine模块的一部分。[更多...](#detailed_description)更多...
```
import KBEngine
```
import KBEngine
## 父类
父类[Entity](Entity.html)Entity
## 成员函数
[成员函数](#)成员函数| def disconnect( self ): |
| --- |
| def getClientType( self ): |
| def getClientDatas( self ): |
| def giveClientTo( self, proxy ): |
| def streamFileToClient( self, resourceName, desc="", id=-1 ): |
| def streamStringToClient( self, data, desc="", id=-1 ): |

def[disconnect](#disconnect)disconnect( self ):def[getClientType](#getClientType)getClientType( self ):def[getClientDatas](#getClientDatas)getClientDatas( self ):def[giveClientTo](#giveClientTo)giveClientTo( self, proxy ):def[streamFileToClient](#streamFileToClient)streamFileToClient( self, resourceName, desc="", id=-1 ):def[streamStringToClient](#streamStringToClient)streamStringToClient( self, data, desc="", id=-1 ):
## 回调函数
[回调函数](#)回调函数| def onClientDeath( self ): |
| --- |
| def onClientGetCell( self ): |
| def onClientEnabled( self ): |
| def onGiveClientToFailure( self ): |
| def onLogOnAttempt( self, ip, port, password ): |
| def onStreamComplete( self, id, success ): |

def[onClientDeath](#onClientDeath)onClientDeath( self ):def[onClientGetCell](#onClientGetCell)onClientGetCell( self ):def[onClientEnabled](#onClientEnabled)onClientEnabled( self ):def[onGiveClientToFailure](#onGiveClientToFailure)onGiveClientToFailure( self ):def[onLogOnAttempt](#onLogOnAttempt)onLogOnAttempt( self, ip, port, password ):def[onStreamComplete](#onStreamComplete)onStreamComplete( self, id, success ):
## 属性
[属性](#)属性| __ACCOUNT_NAME__ | 只读 string |
| --- | --- |
| __ACCOUNT_PASSWORD__ | 只读 string |
| clientAddr | 只读 |
| clientEnabled | 只读 bool |
| hasClient | 只读 bool |
| roundTripTime | 只读 |
| timeSinceHeardFromClient | 只读 |

[__ACCOUNT_NAME__](#__ACCOUNT_NAME__)__ACCOUNT_NAME__只读[string](STRING.html)string[__ACCOUNT_PASSWORD__](#__ACCOUNT_PASSWORD__)__ACCOUNT_PASSWORD__只读[string](STRING.html)string[clientAddr](#clientAddr)clientAddr只读[clientEnabled](#clientEnabled)clientEnabled只读[bool](BOOL.html)bool[hasClient](#hasClient)hasClient只读[bool](BOOL.html)bool[roundTripTime](#roundTripTime)roundTripTime只读[timeSinceHeardFromClient](#timeSinceHeardFromClient)timeSinceHeardFromClient只读
---

## 详细描述
详细描述[Proxy](Proxy.html)Proxy是[Entity](Entity.html)Entity的一个特殊类型，它继承自Entity，它有一个关联的客户端。本身来说，
它就是一个代理客户端的实体，操控所有服务端向客户端的更新。不能在脚本直接创建[Proxy](Proxy.html)Proxy类对象。
---

## 成员函数文档
成员函数文档def disconnect( self ):

def disconnect(self):断开客户端连接。def getClientType( self ):

def getClientType(self):功能说明：这个函数返回客户端类型。返回:


UNKNOWN_CLIENT_COMPONENT_TYPE	= 0,
CLIENT_TYPE_MOBILE				= 1,	// 手机类
CLIENT_TYPE_WIN					= 2,	// pc， 一般都是exe客户端
CLIENT_TYPE_LINUX				= 3		// Linux Application program
CLIENT_TYPE_MAC					= 4		// Mac Application program
CLIENT_TYPE_BROWSER				= 5,	// web应用， html5，flash
CLIENT_TYPE_BOTS				= 6,	// bots
CLIENT_TYPE_MINI				= 7,	// Mini-Client
CLIENT_TYPE_END					= 8		// end

返回:| UNKNOWN_CLIENT_COMPONENT_TYPE	= 0,
CLIENT_TYPE_MOBILE				= 1,	// 手机类
CLIENT_TYPE_WIN					= 2,	// pc， 一般都是exe客户端
CLIENT_TYPE_LINUX				= 3		// Linux Application program
CLIENT_TYPE_MAC					= 4		// Mac Application program
CLIENT_TYPE_BROWSER				= 5,	// web应用， html5，flash
CLIENT_TYPE_BOTS				= 6,	// bots
CLIENT_TYPE_MINI				= 7,	// Mini-Client
CLIENT_TYPE_END					= 8		// end |

UNKNOWN_CLIENT_COMPONENT_TYPE	= 0,CLIENT_TYPE_MOBILE				= 1,	// 手机类CLIENT_TYPE_WIN					= 2,	// pc， 一般都是exe客户端CLIENT_TYPE_LINUX				= 3		// Linux Application programCLIENT_TYPE_MAC					= 4		// Mac Application programCLIENT_TYPE_BROWSER				= 5,	// web应用， html5，flashCLIENT_TYPE_BOTS				= 6,	// botsCLIENT_TYPE_MINI				= 7,	// Mini-ClientCLIENT_TYPE_END					= 8		// enddef getClientDatas( self ):

def getClientDatas(self):功能说明：这个函数返回客户端登录时和注册时所附带的数据。此数据可用于运营系统扩展，如果连接了第三方账号服务，此数据会经过interfaces进程发往第三方服务系统。返回:


tuple, 固定为2个元素的tuple(登陆数据bytes，注册数据bytes)，第一个元素为登陆时客户端调用登陆时传入的datas参数，
第二个元素为注册时客户端调用注册所传入的datas参数。由于可以存储任意二进制数据，因此他们都是以bytes类型存在。

返回:| tuple, 固定为2个元素的tuple(登陆数据bytes，注册数据bytes)，第一个元素为登陆时客户端调用登陆时传入的datas参数，
第二个元素为注册时客户端调用注册所传入的datas参数。由于可以存储任意二进制数据，因此他们都是以bytes类型存在。 |

tuple, 固定为2个元素的tuple(登陆数据bytes，注册数据bytes)，第一个元素为登陆时客户端调用登陆时传入的datas参数，
第二个元素为注册时客户端调用注册所传入的datas参数。由于可以存储任意二进制数据，因此他们都是以bytes类型存在。def giveClientTo( self, proxy ):

def giveClientTo(self, proxy):功能说明：将客户端的控制器转交给另一个Proxy，当前的Proxy必须有一个客户端而目标Proxy则必须没有关联客户端，否则将会提示错误。参看：

Proxy.onGiveClientToFailure

参看：| Proxy.onGiveClientToFailure |

[Proxy](Proxy.html)Proxy.[onGiveClientToFailure](Proxy.html#onGiveClientToFailure)onGiveClientToFailure参数：

proxy
控制权将转交给这个实体。

参数：| proxy | 控制权将转交给这个实体。 |

proxy控制权将转交给这个实体。def streamFileToClient( self, resourceName, desc="", id=-1 ):

def streamFileToClient(self, resourceName, desc="", id=-1):功能说明：这个函数类似于[streamStringToClient()](Proxy.html#streamStringToClient)streamStringToClient()，将一个资源文件发送给客户端。发送过程在不同的线程上操作，因此不会危及主线程。参看：

Proxy.onStreamComplete

参看：| Proxy.onStreamComplete |

[Proxy](Proxy.html)Proxy.[onStreamComplete](Proxy.html#onStreamComplete)onStreamComplete参数：

resourceName
要发送的资源名称，包含路径。

desc
一个可选的字符串，发送给客户端的资源描述。

id
一个16位的id，它的值完全取决于调用者。如果传入-1系统将会在队列里面选择一个没有在用的id。可以在客户端根据这个id做资源判断。

参数：| resourceName | 要发送的资源名称，包含路径。 |
| --- | --- |
| desc | 一个可选的字符串，发送给客户端的资源描述。 |
| id | 一个16位的id，它的值完全取决于调用者。如果传入-1系统将会在队列里面选择一个没有在用的id。可以在客户端根据这个id做资源判断。 |

resourceName要发送的资源名称，包含路径。desc一个可选的字符串，发送给客户端的资源描述。id一个16位的id，它的值完全取决于调用者。如果传入-1系统将会在队列里面选择一个没有在用的id。可以在客户端根据这个id做资源判断。返回：


与这个下载关联的id。

返回：| 与这个下载关联的id。 |

与这个下载关联的id。def streamStringToClient( self, data, desc="", id=-1 ):

def streamStringToClient(self, data, desc="", id=-1):功能说明：发送一些数据到当前实体绑定的客户端。如果客户端端口则数据被清除，
当客户端再次绑定到实体的时候才可调用这个函数。16位的id完全取决于调用者。如果调用者没有指定这个ID则系统会分配一个未用过的id。可以在客户端根据这个id做资源判断。你可以在Proxy的派生类中定义回调函数（onStreamComplete），所有数据成功发送给客户端时或下载失败时会调用这个回调函数。参看：[Proxy](Proxy.html)Proxy.[onStreamComplete](Proxy.html#onStreamComplete)onStreamComplete与客户端实体[Entity](../../client/classes/Entity.html)Entity.[onStreamDataStarted](../../client/classes/Entity.html#onStreamDataStarted)onStreamDataStarted和[Entity](../../client/classes/Entity.html)Entity.[onStreamDataRecv](../../client/classes/Entity.html#onStreamDataRecv)onStreamDataRecv还有[Entity](../../client/classes/Entity.html)Entity.[onStreamDataCompleted](../../client/classes/Entity.html#onStreamDataCompleted)onStreamDataCompleted。参数：

data
要发送的字符串。

desc
一个可选的字符串，发送给客户端的描述。

id
一个16位的id，它的值完全取决于调用者。如果传入-1系统将会在队列里面选择一个没有在用的id。

参数：| data | 要发送的字符串。 |
| --- | --- |
| desc | 一个可选的字符串，发送给客户端的描述。 |
| id | 一个16位的id，它的值完全取决于调用者。如果传入-1系统将会在队列里面选择一个没有在用的id。 |

data要发送的字符串。desc一个可选的字符串，发送给客户端的描述。id一个16位的id，它的值完全取决于调用者。如果传入-1系统将会在队列里面选择一个没有在用的id。返回：


与这个下载关联的id。

返回：| 与这个下载关联的id。 |

与这个下载关联的id。
---

## 回调函数文档
回调函数文档def onClientDeath( self ):

def onClientDeath(self):如果在脚本中实现了此回调，这个方法将在客户端断开连接时被调用。 这个方法没有参数。def onClientGetCell( self ):

def onClientGetCell(self):如果在脚本中实现了此回调，当客户端能够调用实体的[cell](../../keywords.html#cell)cell属性时，该回调被调用。def onClientEnabled( self ):

def onClientEnabled(self):如果在脚本中实现了此回调，当实体可用时（ 各种初始化完毕并且可以与客户端通讯 ）该回调被调用。 这个方法没有参数。注意：giveClientTo将控制权赋给了该实体时也会导致该回调被调用。def onGiveClientToFailure( self ):

def onGiveClientToFailure(self):如果在脚本中实现了此回调，当实体调用[giveClientTo](#onStreamComplete)giveClientTo失败时，该回调被调用。这个方法没有参数。def onLogOnAttempt( self, ip, port, password ):

def onLogOnAttempt(self, ip, port, password):如果在脚本中实现了此回调，这个函数在客户端尝试使用当前账号实体进行登录时触发回调。这种情况通常是实体存在于内存中处于有效状态，最明显的例子是用户A使用此账号登录了，用户B使用同一账号进行登录，此时回调触发。这个回调函数可以返回如下常量值：[KBEngine](../Modules/KBEngine.html)KBEngine.[LOG_ON_ACCEPT](KBEngine.html#LOG_ON_ACCEPT)LOG_ON_ACCEPT：允许新的客户端与实体进行绑定，如果实体已经绑定了一个客户端，之前的客户端将被踢出。[KBEngine](../Modules/KBEngine.html)KBEngine.[LOG_ON_REJECT](KBEngine.html#LOG_ON_REJECT)LOG_ON_REJECT：拒绝新的客户端与实体绑定。[KBEngine](../Modules/KBEngine.html)KBEngine.[LOG_ON_WAIT_FOR_DESTROY](KBEngine.html#LOG_ON_WAIT_FOR_DESTROY)LOG_ON_WAIT_FOR_DESTROY：等待实体销毁后再进行客户端绑定。参数：

ip 尝试登录的客户端IP地址。

port 尝试登录的客户端连接的端口。

password 用户登录时使用的MD5密码。

参数：| ip | 尝试登录的客户端IP地址。 |
| --- | --- |
| port | 尝试登录的客户端连接的端口。 |
| password | 用户登录时使用的MD5密码。 |

ip尝试登录的客户端IP地址。port尝试登录的客户端连接的端口。password用户登录时使用的MD5密码。def onStreamComplete( self, id, success ):

def onStreamComplete(self, id, success):如果在脚本中实现了此回调，当用户使用[Proxy](Proxy.html)Proxy.[streamStringToClient](Proxy.html#streamStringToClient)streamStringToClient()或[Proxy](Proxy.html)Proxy.[streamFileToClient](Proxy.html#streamFileToClient)streamFileToClient()完成时，该回调被调用。参数：

id 与下载关联的id。

success 成功与否。

参数：| id | 与下载关联的id。 |
| --- | --- |
| success | 成功与否。 |

id与下载关联的id。success成功与否。
---

## 属性文档
属性文档__ACCOUNT_NAME__

__ACCOUNT_NAME__说明：如果proxy是帐号则可以访问__ACCOUNT_NAME__得到帐号名。__ACCOUNT_PASSWORD__

__ACCOUNT_PASSWORD__说明：如果proxy是帐号则可以访问__ACCOUNT_PASSWORD__得到帐号MD5密码。clientAddr

clientAddr这是一个tuple对象，包含了客户端的ip与端口。clientEnabled

clientEnabled实体是否已经可用。在实体可用之前脚本不能与客户端进行通讯。hasClient

hasClientProxy是否绑定了一个客户端连接。roundTripTime

roundTripTime在一段时间内服务器与这个Proxy绑定的客户端通讯平均往返时间。这个属性只在Linux下生效。timeSinceHeardFromClient

timeSinceHeardFromClient最后一次收到客户端数据包时到目前为止所过去的时间（秒）。
---
版权归KBEngine所有。

版权归KBEngine所有。