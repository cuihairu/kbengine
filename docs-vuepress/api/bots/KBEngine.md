# KBEngine模块

[KBEngine](./KBEngine.md)模块提供了逻辑脚本层访问实体的部分, 以及当前进程其他客户端的数据等等。

## 类

- Entity
- PyClientApp

## 成员函数

- def [addBots](#addBots)( reqCreateAndLoginTotalCount, reqCreateAndLoginTickCount=0, reqCreateAndLoginTickTime=0 ):
- def [callback](#callback)( initialOffset, callbackObj ):
- def [cancelCallback](#cancelCallback)( id ):
- def [genUUID64](#genUUID64)( ):
- def [getWatcher](#getWatcher)( path ):
- def [getWatcherDir](#getWatcherDir)( path ):
- def [scriptLogType](#scriptLogType)( logType ):
- def [urlopen](#urlopen)( url, callback, postData, headers ):

## 回调函数

- def [onInit](#onInit)( isReload ):
- def [onFinish](#onFinish)( ):

## 属性

| [bots](#bots) | bots[component](#component)只读string |
| --- | --- |

## 成员函数文档

<a id="addBots"></a>

### def addBots(reqCreateAndLoginTotalCount, reqCreateAndLoginTickCount=0, reqCreateAndLoginTickTime=0):

功能说明：
向服务端添加机器人。
例子:

```python
# 这里是使用addBots的一个例子
        import 
KBEngine

 
        # 一次性向服务器添加5个机器人（瞬时完成）
        KBEngine.addBots( 5 )
 
        # 一共向服务器添加1000个机器人，每次添加5个，每次添加所用时间(s)
        KBEngine.addBots( 1000, 5, 10 )
```

参数：

| reqCreateAndLoginTotalCount | integer，向服务器添加的机器人总数。 |
| --- | --- |
| reqCreateAndLoginTickCount | integer，每次向服务器添加的机器人数量。 |
| reqCreateAndLoginTickTime | integer，每次添加所用时间(秒)。 |

<a id="callback"></a>

### def callback(initialOffset, callbackObj):

功能说明：
注册一个回调，回调函数callbackObj触发，回调函数将在"initialOffset"秒后被执行。
例子:

```python
# 这里是使用callback的一个例子
        import 
KBEngine

 
        # 增加一个定时器，1秒后执行
        KBEngine.callback( 1, onCallbackfun )
 
    def onCallbackfun( ):
        print "onCallbackfun called“
	
```

参数：

| initialOffset | float，指定回调从注册到回调的时间间隔（秒）。 |
| --- | --- |
| callbackObj | function，指定的回调函数对象。 |

返回:

- integer，该函数返回callback的内部id，这个id可用于[cancelCallback](#cancelCallback)移除回调。

<a id="cancelCallback"></a>

### def cancelCallback(id):

功能说明：
函数cancelCallback用于移除一个注册但还未触发的回调，移除后的回调不再执行。
如果cancelCallback函数使用一个无效的id（例如已经移除），将会产生错误。
到KBEngine.callback参考回调的一个使用例子。

参数：

| id | integer，它指定要移除的回调id。 |
| --- | --- |

<a id="genUUID64"></a>

### def genUUID64():

功能说明：
该函数生成一个64位的唯一ID。
注意：这个函数依赖于Cellapps服务进程启动参数gus，请正确设置启动参数保持唯一性。
另外如果gus超过65535则该函数只能在当前进程上保持唯一性。
用途：
多个服务进程上产生唯一物品ID并且在合服时不会产生冲突。
多个服务进程上产生一个房间ID，不需要进行唯一性校验。

返回：

- 返回一个64位的integer。

<a id="getWatcher"></a>

### def getWatcher(path):

功能说明：
从KBEngine调试系统中获取一个监视变量的值。
例子：在baseapp1的Python命令行输入:
>>>KBEngine.getWatcher("/root/stats/runningTime")
12673648533
>>>KBEngine.getWatcher("/root/scripts/players")
32133

参数：

| path | string，该变量的绝对路径包括变量名(可以在GUIConsole的watcher页查看)。 |
| --- | --- |

返回：

- 该变量的值。

<a id="getWatcherDir"></a>

### def getWatcherDir(path):

功能说明：
从KBEngine调试系统中获取一个监视目录下的元素列表(目录、变量名)。
例子：在baseapp1的Python命令行输入:
>>>KBEngine.getWatcher("/root")
('stats', 'objectPools', 'network', 'syspaths', 'ThreadPool', 'cprofiles', 'scripts', 'numProxices', 'componentID', 'componentType', 'uid', 'numClients', 'globalOrder', 'username', 'load', 'gametime', 'entitiesSize', 'groupOrder')

参数：

| path | string，该变量的绝对路径(可以在GUIConsole的watcher页查看)。 |
| --- | --- |

返回：

- 监视目录下的元素列表(目录、变量名)。

<a id="scriptLogType"></a>

### def scriptLogType(logType):

功能说明：
设置当前Python.print输出的信息类型(参考: KBEngine.LOG_TYPE_*)。

<a id="urlopen"></a>

### def urlopen(url, callback, postData, headers):

功能说明：
这个脚本函数在提供对外HTTP/HTTPS异步请求。

参数：

| url | 有效的HTTP/HTTPS网址，字符串类型。 |
| --- | --- |
| callback | 可选参数，带有请求执行结果的回调对象（比如说是一个函数）。这个回调带有5个参数：HTTP请求返回码（如：200)，返回的内容，返回的HTTP协议头，是否成功，请求的网址。 声明样例： def onHttpCallback(httpcode, data, headers, success, url): print(httpcode, data, headers, success, url) 如同上面的例子所示:<br>httpcode:参数对应的就是“HTTP请求返回码”，这个结果集合参数是一个整形值。<br>data:参数则是“返回的内容”，它是一个字符串。<br>headers:参数是“服务器返回的HTTP协议头”，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。<br>success:则对应了“执行是否成功”，当请求执行有错误时，为False，可以通过httpcode进一步判断错误信息。<br>url:是“请求所用的网址。 |
| postData | 可选参数，默认是GET方式请求服务器，如果需要POST方式请提供需要POST的内容，引擎将自动使用POST方式请求服务器，它是一个bytes。 |
| headers | 可选参数，请求时使用的HTTP头，如：{"Content-Type": "application/x-www-form-urlencoded"}，它是一个字典。 |

## 回调函数文档

<a id="onInit"></a>

### def onInit(isReload):

功能说明：
当引擎启动后初始化完所有的脚本后这个接口被调用。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

参数：

| isReload | bool，是否是被重写加载脚本后触发的。 |
| --- | --- |

<a id="onFinish"></a>

### def onFinish():

功能说明：
进程关闭会回调此函数。
注意：该回调接口必须实现在入口模块(kbengine_defaults.xml->entryScriptFile)中。

## 属性文档

<a id="bots"></a>

bots

说明：
bots是一个字典对象，包含当前进程上所有的客户端对象。

类型：

- PyBots

<a id="component"></a>

component

说明：
这是正运行在当前脚本环境的组件。（至今为止）可能值有'cellapp', 'baseapp', 'client', 'dbmgr', 'bots' 和 'editor'。
