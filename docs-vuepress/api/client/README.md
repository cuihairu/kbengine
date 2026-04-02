# KBEngine模块

| KBEngine |

KBEngine
---

# KBEngine模块
KBEngine模块[KBEngine](KBEngine.html)KBEngine模块提供了逻辑脚本层访问[实体](../Classes/Entity.html)实体的部分, 以及当前space的数据等等。
## 类
[类](#)类| Entity |

[Entity](../Classes/Entity.html)Entity
## 成员函数
[成员函数](#)成员函数| def login( username, password ): |
| --- |
| def createAccount( username, password ): |
| def reloginBaseapp(): |
| def player(  ): |
| def resetPassword(username): |
| def bindAccountEmail( emailaddress ): |
| def newPassword( oldpassword, newpassword ): |
| def findEntity( entityID ): |
| def getSpaceData( key ): |

def[login](#login)login( username, password ):def[createAccount](#createAccount)createAccount( username, password ):def[reloginBaseapp](#reloginBaseapp)reloginBaseapp():def[player](#player)player(  ):def[resetPassword](#resetPassword)resetPassword(username):def[bindAccountEmail](#bindAccountEmail)bindAccountEmail( emailaddress ):def[newPassword](#newPassword)newPassword( oldpassword, newpassword ):def[findEntity](#findEntity)findEntity( entityID ):def[getSpaceData](#getSpaceData)getSpaceData( key ):
## 回调函数
[回调函数](#)回调函数
## 属性
[属性](#)属性| component | 只读 string |
| --- | --- |
| entities | Entities
entity_uuid  uint64

entity_id  int32

spaceID  int32 | entity_uuid | uint64

entity_id  int32

spaceID  int32 | entity_id | int32

spaceID  int32 | spaceID | int32 |
| entity_uuid | uint64

entity_id  int32

spaceID  int32 | entity_id | int32

spaceID  int32 | spaceID | int32 |
| entity_id | int32

spaceID  int32 | spaceID | int32 |
| spaceID | int32 |

[component](#component)component只读[string](STRING.html)string[entities](#entities)entities[Entities](Entities.html)Entities[entity_uuid](#entity_uuid)entity_uuiduint64[entity_id](#entity_id)entity_idint32[spaceID](#spaceID)spaceIDint32
---

## 成员函数文档
成员函数文档def login( username, password  ):

def login(username, password):功能说明：登录账号到KBEngine服务端。注意：如果插件与UI层使用事件交互模式，在UI层不要直接调用，请触发一个"login"事件给插件，事件附带数据username和password。参数：

username
string，用户名。

password
string，密码。

参数：| username | string，用户名。 |
| --- | --- |
| password | string，密码。 |

usernamestring，用户名。passwordstring，密码。def createAccount( username, password  ):

def createAccount(username, password):功能说明：请求向KBEngine服务端创建一个登录账号。注意：如果插件与UI层使用事件交互模式，在UI层不要直接调用，请触发一个"createAccount"事件给插件，事件附带数据username和password。参数：

username
string，用户名。

password
string，密码。

参数：| username | string，用户名。 |
| --- | --- |
| password | string，密码。 |

usernamestring，用户名。passwordstring，密码。def reloginBaseapp(   ):

def reloginBaseapp():功能说明：请求重登录到KBEngine服务端(通常在掉线之后希望更及时的连接到服务端并继续控制服务端角色时使用)。注意：如果插件与UI层使用事件交互模式，在UI层不要直接调用，请触发一个"reloginBaseapp"事件给插件，事件附带数据为空。def player(  ):

def player():功能说明：获得当前客户端所控制的实体。返回：


Entity，返回控制的实体, 如果不存在(如：未能连接到服务端)则返回空。

返回：| Entity，返回控制的实体, 如果不存在(如：未能连接到服务端)则返回空。 |

[Entity](../../keywords.html#entity)Entity，返回控制的实体, 如果不存在(如：未能连接到服务端)则返回空。def resetPassword( username ):

def resetPassword(username):功能说明：请求loginapp重置账号的密码, 服务端将会向该账号绑定的邮箱发送一封重置密码邮件(通常是忘记密码功能使用)。参数：

username
string，用户名。

参数：| username | string，用户名。 |

usernamestring，用户名。def bindAccountEmail( emailaddress ):

def bindAccountEmail(emailaddress):功能说明：请求baseapp绑定账号的email地址。参数：

emailaddress
string，邮箱地址。

参数：| emailaddress | string，邮箱地址。 |

emailaddressstring，邮箱地址。def newPassword( oldpassword, newpassword ):

def newPassword(oldpassword, newpassword):功能说明：请求设置账号的新密码。参数：

oldpassword
string，旧密码。

newpassword
string，新密码。

参数：| oldpassword | string，旧密码。 |
| --- | --- |
| newpassword | string，新密码。 |

oldpasswordstring，旧密码。newpasswordstring，新密码。def findEntity( entityID  ):

def findEntity(entityID):功能说明：通过实体的ID查找实体的实例对象。参数：

entityID
int32，实体ID。

参数：| entityID | int32，实体ID。 |

entityIDint32，实体ID。返回：


Entity，存在返回实体实例，不存在返回空。

返回：| Entity，存在返回实体实例，不存在返回空。 |

[Entity](../../keywords.html#entity)Entity，存在返回实体实例，不存在返回空。def getSpaceData( key  ):

def getSpaceData(key):功能说明：获取指定key的space数据。space数据由用户在服务端通过[setSpaceData](../../cellapp/Modules/KBEngine.html#setSpaceData)setSpaceData设置。参数：

key
string，一个字符串关键字。

参数：| key | string，一个字符串关键字。 |

keystring，一个字符串关键字。返回：


string，指定key的字符串数据。

返回：| string，指定key的字符串数据。 |

string，指定key的字符串数据。
---

## 回调函数文档
回调函数文档
---

## 属性文档
属性文档component

component说明：这是正运行在当前脚本环境的组件。（至今为止）可能值有'cellapp', 'baseapp', 'client', 'dbmgr', 'bots' 和 'editor'。entities

entities说明：entities是一个字典对象，包含当前进程上所有的实体。类型：

Entities

类型：| Entities |

[Entities](Entities.html)Entitiesentity_uuid

entity_uuid说明：实体的uuid，改ID与实体本次登录绑定。当使用重登陆功能时服务端会与此ID进行比对，判断合法性。entity_id

entity_id说明：当前客户端所控制的实体的ID。spaceID

spaceID说明：当前客户端控制的实体所在的[空间](../../keywords.html#cn_Space)空间ID(也可以理解为所在对应的场景、房间、副本)。
---
版权归KBEngine所有。

版权归KBEngine所有。