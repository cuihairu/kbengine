# PyClientApp类

[KBEngine模块]

PyClientApp是KBEngine模块的一部分，由C++底层模拟一个客户端时创建的客户端对象，脚本层不可直接创建。

## 成员函数

- def [getSpaceData](#getSpaceData)( key ):
- def [player](#player)( ):

## 回调函数

## 属性

| [id](#id) | 只读 Integer |
| --- | --- |
| [entities](#entities) | Entities |

<a id="detailed_description"></a>

## 详细描述

类Entity的实例代表着在client上的游戏对象。
一个Entity可以通过ENTITYCALL访问在base和cell应用程序上的等价的实体。这需要
一组远程调用的函数（在实体的.def文件里指定）。

## 成员函数文档

<a id="getSpaceData"></a>

### def getSpaceData(key):

功能说明：
获取指定key的space数据。
space数据由用户在服务端通过setSpaceData设置。

参数：

| key | string，一个字符串关键字。 |
| --- | --- |

返回：

- string，指定key的字符串数据。

<a id="player"></a>

### def player():

功能说明：
获得当前客户端所控制的实体。

## 回调函数文档

## 属性文档

<a id="entities"></a>

entities

说明：
entities是一个字典对象，包含当前进程上所有的实体。

类型：

- Entities
