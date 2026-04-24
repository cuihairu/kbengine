# 关键词释义

<a id="EntityCall"></a>

### EntityCall:

脚本层与实体远程交互的常规手段(其他参考：[allClients](./cellapp/Entity.md#allClients)、[otherClients](./cellapp/Entity.md#otherClients)、[clientEntity](./cellapp/Entity.md#clientEntity))。
EntityCall对象在C++底层实现非常简单，它只包含了实体的ID、目的地的地址、实体类型、EntityCall类型。当用户请求一次远程交互时，底层首先能够通过实体类型找到实体定义的描述，
通过实体定义的描述对用户输入的数据进行检查，如果检查合法那么底层将数据打包并发往目的地，目的地进程根据协议进行解包最终调用到脚本层。
注意: EntityCall只能调用其对应def文件中声明过的方法，不能访问实体的属性以及其他任何信息。
一个实体最多可以包含三个部分：
client： 当实体包括客户端部分时(通常为玩家)，在服务端可以访问实体的client(EntityCall)属性。
base：当实体的一部分创建在Baseapp时，在非当前Baseapp中可以访问实体的base(EntityCall)属性。
cell：当实体的一部分创建在Cellapp时，在非当前Cellapp中可以访问实体的cell(EntityCall)属性。
举例:
Avatar.def中定义client远程方法:

```python
	<ClientMethods>
		<hello>
		</hello>
	</ClientMethods>
```

client\Avatar.py

```python
	class Avatar:
		def hello(self):
			print("hello")
```

在GUIConsole工具的Debug页输入框中输入(请先在左边列表中勾选要调试的进程)：
首先在服务端Baseapp的日志找到玩家实体(Avatar)的ID, 然后通过实体ID获得玩家实体(Avatar)或者EntityCall:
>>> KBEngine.entities[玩家ID].client.hello()
此时客户端log文件中将输出"hello", 一次远程调用过程完成。

<a id="KBE_ROOT"></a>

### KBE_ROOT:

这是一个KBEngine的环境变量，描述的是KBEngine所在的根目录。

<a id="KBE_RES_PATH"></a>

### KBE_RES_PATH:

这是一个KBEngine的环境变量，描述的是KBEngine引擎能够读取到的资源目录。

<a id="KBE_HYBRID_PATH"></a>

### KBE_HYBRID_PATH:

这是一个KBEngine的环境变量，描述的是KBEngine引擎可执行文件所在的目录。

<a id="entities.xml"></a>

### entities.xml:

服务端所有有效的实体类型必须在此进行注册，引擎初始化时会根据此处依次加载实体的描述信息。

<a id="kbengine_defaults.xml"></a>

### kbengine_defaults.xml:

服务端默认配置，在此用户可以修改[cellapp](./cellapp/)、[baseapp](./baseapp/)、loginapp等所有的组件配置。
注意：你可能经常需要对引擎进行升级，直接修改此处可能在升级时产生冲突，另外也不适合多个项目放在同一KBEngine环境的情况。
建议在[kbengine.xml](#kbengine.xml)中进行重载修改，你只需要对想修改的部分按照格式在xml中重写就可以了。

<a id="kbengine.xml"></a>

### kbengine.xml:

服务端配置，在此用户可以修改[cellapp](./cellapp/)、[baseapp](./baseapp/)、loginapp等所有的组件配置。
详细请参考[kbengine_defaults.xml](#kbengine_defaults.xml)

<a id="cn_entity"></a>

### 实体

实体被定义为服务端最基本的对象，类似Python的基础对象object。
什么时候需要定义实体? 请参见[http://www.kbengine.org/docs/programming/entitydef.html](http://www.kbengine.org/docs/programming/entitydef.html)

<a id="entity"></a>

### entity

参见:[实体](#cn_entity)

<a id="View"></a>

### View

每一个连接到服务器的客户端实体都将拥有一个View，View类似一种视图，让客户端能够对自身View内的事件传达给客户端。
View与空间相关，每个View都能够设定独立的大小范围。
注意：这里描述的空间是一种抽象的概念，不一定需要和物理空间概念绑定（MMORPG除外），对于一个卡牌游戏的核心玩法，也可以认为一个房间内的玩家在一个逻辑空间中。
事件包括: 实体移动、客户端广播类型的属性改变、死亡销毁等等。

<a id="Witness"></a>

### Witness

目击者。
只有绑定了Witness的[cell](#cell)实体View才能产生作用，换句话来说witness就是客户端的一个[cell](#cell)代理，cellapp将View内的信息不断的通过Witness同步给客户端。
服务端一个NPC被目击者目击时会调用实体的onWitness回调，服务端可以依赖于此特性降低CPU的消耗，当一个实体
没有被目击时，用户可以停止它的任何行为。

<a id="Space"></a>

### Space

空间，KBEngine在cellapp上分配一个空间，这个空间与其他空间隔离，View、陷阱、实体碰撞等等只在当前空间相互影响。
空间具体是什么由用户来定义，它可以是一个场景、副本、房间...

<a id="cn_Space"></a>

### 空间

参见:[Space](#Space)

<a id="cell"></a>

### cell

cell在文档中存在二个不同的意思。
通常如果描述的是[Entity.cell](./baseapp/Entity.md#cell)属性， 此时实际是在描述实体的[CellEntityCall](#EntityCall)。
如果是在cellapp上有关cell的描述通常是在描述一个[空间](#cn_Space)的一部分，一个空间在[cellapp](./cellapp/)进行负载平衡时有可能被分割成N份，每一份称之为一个cell，每个cell由不同进程维护。

<a id="base"></a>

### base

通常是指[baseapp](./baseapp/)上的Base实体或者是一个指向Base实体的[BaseEntityCall](#EntityCall), 例如：[Entity.base](./cellapp/Entity.md#base)。

<a id="client"></a>

### client

通常是指客户端或者是一个指向客户端实体的[EntityCall](#EntityCall), 例如：[Entity.client](./baseapp/Entity.md#client)。

<a id="cellapp"></a>

### cellapp

Cellapp进程主要负责与位置有关类游戏逻辑、View、AI、场景房间等等。
参见:[cellapp](./cellapp/)

<a id="baseapp"></a>

### baseapp

Baseapp进程主要负责与客户端通讯、与位置无关类游戏逻辑(公会管理器、聊天系统、游戏大厅、排行榜等等)、存档与备份等等。
参见:[baseapp](./baseapp/)

<a id="real"></a>

### real

指的是一个cell上的实体，这个实体是真实存在于当期cell上的（有一种实体只是另一个cell广播过来的影像，参见：[ghost](#ghost)）。

<a id="ghost"></a>

### ghost

这种实体其实是由于cellapp动态负载均衡机制将一个完整的space分割成N份并交给不同的进程中的cell共同维护从而产生的一种边界区影像实体。
由于space被分割成多个区域，所以space存在边界。要让客户端无法感知到边界的存在我们将每个cell边界区域一定范围内的实体同步一份到另一个边邻的cell上，
这样就不会在边界区域时无法和另一个边界区的实体进行交互了。而这种实体只有部分数据被同步过来(CELL_PUBLIC等cell广播类型的属性)。
非ghost实体我们称为[real](#real)实体。

<a id="vector3"></a>

### vector3

描述和管理3D空間的向量。
其中有x，y，z三个属性代表不同的轴向。
脚本中使用的例子：
import Math
v = Math.Vector3()
