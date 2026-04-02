# PyClientApp类

| KBEngine |

KBEngine
---

# PyClientApp类
PyClientApp类[KBEngine模块]

[[KBEngine模块](../Modules/KBEngine.html)KBEngine模块]PyClientApp是KBEngine模块的一部分，由C++底层模拟一个客户端时创建的客户端对象，脚本层不可直接创建。

PyClientApp是[KBEngine](../Modules/KBEngine.html)KBEngine模块的一部分，由C++底层模拟一个客户端时创建的客户端对象，脚本层不可直接创建。
## 成员函数
[成员函数](#)成员函数| def getSpaceData( key ): |
| --- |
| def player( ): |

def[getSpaceData](#getSpaceData)getSpaceData( key ):def[player](#player)player( ):
## 回调函数
[回调函数](#)回调函数
## 属性
[属性](#)属性| id | 只读 Integer |
| --- | --- |
| entities | Entities |

[id](#id)id只读 Integer[entities](#entities)entities[Entities](Entities.html)Entities
---

## 详细描述
详细描述类[Entity](../../keywords.html#entity)Entity的实例代表着在client上的游戏对象。一个[Entity](../../keywords.html#entity)Entity可以通过[ENTITYCALL](../../keywords.html#EntityCall)ENTITYCALL访问在base和cell应用程序上的等价的实体。这需要
一组远程调用的函数（在实体的.def文件里指定）。
---

## 成员函数文档
成员函数文档def getSpaceData( key  ):

def getSpaceData(key):功能说明：获取指定key的space数据。space数据由用户在服务端通过[setSpaceData](../../cellapp/Modules/KBEngine.html#setSpaceData)setSpaceData设置。参数：

key
string，一个字符串关键字。

参数：| key | string，一个字符串关键字。 |

keystring，一个字符串关键字。返回：


string，指定key的字符串数据。

返回：| string，指定key的字符串数据。 |

string，指定key的字符串数据。def player(  ):

def player():功能说明：获得当前客户端所控制的实体。
---

## 回调函数文档
回调函数文档
---

## 属性文档
属性文档entities

entities说明：entities是一个字典对象，包含当前进程上所有的实体。类型：

Entities

类型：| Entities |

[Entities](Entities.html)Entities
---
版权归KBEngine所有。

版权归KBEngine所有。