"""
KBEngine 实体关联示例代码
演示不同类型的实体关联实现方式
"""

# ==================== 1. Def文件定义 ====================

# Avatar.def
AVATAR_DEF = """
<root>
    <Properties>
        <!-- 基础属性 -->
        <Property>
            <Name>name</Name>
            <Type>STRING</Type>
            <Persistent>true</Persistent>
            <UType>1001</UType>
        </Property>

        <Property>
            <Name>level</Name>
            <Type>UINT32</Type>
            <Persistent>true</Persistent>
            <UType>1002</UType>
        </Property>

        <!-- 关联属性：通过ID关联其他实体 -->
        <Property>
            <Name>accountID</Name>
            <Type>UINT64</Type>
            <Persistent>true</Persistent>
            <UType>2001</UType>
            <Default>0</Default>
        </Property>

        <Property>
            <Name>guildID</Name>
            <Type>UINT64</Type>
            <Persistent>true</Persistent>
            <UType>2002</UType>
            <Default>0</Default>
        </Property>

        <Property>
            <Name>teamID</Name>
            <Type>UINT64</Type>
            <Persistent>true</Persistent>
            <UType>2003</UType>
            <Default>0</Default>
        </Property>

        <!-- ENTITYCALL类型关联（强关联） -->
        <Property>
            <Name>targetEntity</Name>
            <Type>ENTITYCALL</Type>
            <Persistent>true</Persistent>
            <UType>2004</UType>
        </Property>
    </Properties>

    <BaseMethods>
        <!-- 关联查询方法 -->
        <BaseMethod>
            <Name>getGuild</Name>
            <UType>3001</UType>
        </BaseMethod>

        <BaseMethod>
            <Name>setGuild</Name>
            <UType>3002</UType>
            <Arg>
                <Type>UINT64</Type>
            </Arg>
        </BaseMethod>
    </BaseMethods>
</root>
"""

# ==================== 2. 业务逻辑实现 ====================

class Avatar(KBEngine.Entity):
    """角色实体，演示各种关联方式"""

    def __init__(self):
        KBEngine.Entity.__init__(self)

        # ID关联（推荐方式）
        self.accountID = 0       # 关联账号ID
        self.guildID = 0         # 关联公会ID
        self.teamID = 0          # 关联队伍ID

        # ENTITYCALL关联（强关联）
        self.targetEntity = None  # 当前目标实体

        # 运行时缓存（不持久化）
        self.guild = None        # 缓存的公会实体
        self.teamMembers = []    # 队友列表

    # ==================== ID关联实现 ====================

    def getGuild(self):
        """通过ID获取公会实体（懒加载）"""
        if self.guildID == 0:
            return None

        # 检查缓存
        if self.guild is not None:
            if self.guild.id == self.guildID:
                return self.guild
            else:
                self.guild = None  # 缓存失效

        # 查询实体
        self.guild = KBEngine.getEntity(self.guildID)
        return self.guild

    def setGuild(self, guildID):
        """设置公会关联"""
        # 清理旧关联
        if self.guildID > 0:
            oldGuild = KBEngine.getEntity(self.guildID)
            if oldGuild:
                oldGuild.removeMember(self.id)

        # 建立新关联
        self.guildID = guildID
        self.guild = None  # 清除缓存

        # 加入新公会
        if guildID > 0:
            newGuild = KBEngine.getEntity(guildID)
            if newGuild:
                newGuild.addMember(self.id)

        # 保存到数据库
        self.writeToDB()

    # ==================== ENTITYCALL关联实现 ====================

    def setTarget(self, targetEntity):
        """设置目标实体（ENTITYCALL方式）"""
        if targetEntity is None:
            self.targetEntity = None
        else:
            # 自动创建ENTITYCALL
            self.targetEntity = targetEntity.clientEntity

    def getTarget(self):
        """获取目标实体"""
        if self.targetEntity is None:
            return None

        # 从ENTITYCALL获取实体
        targetID = self.targetEntity.id
        return KBEngine.getEntity(targetID)

    # ==================== 复杂关联查询 ====================

    def getTeamMembers(self):
        """获取队友列表（关联表查询）"""
        if self.teamID == 0:
            return []

        # 方式1：通过Team实体查询
        team = KBEngine.getEntity(self.teamID)
        if team:
            return team.getMembers()

        # 方式2：直接数据库查询
        sql = """
            SELECT avatarID FROM tbl_TeamMembers
            WHERE teamID = %d
            AND avatarID != %d
        """ % (self.teamID, self.id)

        KBEngine.executeRawDatabaseCommand(sql, self.onGetTeamMembers)

    def onGetTeamMembers(self, result):
        """队友查询回调"""
        if result and len(result) > 0:
            memberIDs = [row['avatarID'] for row in result]
            self.teamMembers = []

            for memberID in memberIDs:
                member = KBEngine.getEntity(memberID)
                if member:
                    self.teamMembers.append(member)

            # 通知客户端
            self.client.onTeamMembersUpdated(memberIDs)

    # ==================== 实体销毁处理 ====================

    def onDestroy(self):
        """实体销毁时清理关联关系"""
        # 1. 退出公会
        if self.guildID > 0:
            guild = KBEngine.getEntity(self.guildID)
            if guild:
                guild.removeMember(self.id)

        # 2. 退出队伍
        if self.teamID > 0:
            team = KBEngine.getEntity(self.teamID)
            if team:
                team.removeMember(self.id)

        # 3. 清除目标
        self.targetEntity = None

        # 4. 清空所有关联ID
        self.guildID = 0
        self.teamID = 0
        self.accountID = 0


# ==================== 3. 关联表管理示例 ====================

class GuildManager:
    """公会管理器（演示关联表使用）"""

    @staticmethod
    def createGuild(avatarID, guildName):
        """创建公会（建立关联关系）"""
        # 1. 创建公会实体
        guildParams = {
            "name": guildName,
            "leaderID": avatarID,
            "level": 1
        }
        guild = KBEngine.createEntityAnywhere("Guild", guildParams)

        # 2. 在关联表中建立关系
        sql = """
            INSERT INTO tbl_GuildMembers (guildID, avatarID, memberType)
            VALUES (%d, %d, 0)
        """ % (guild.id, avatarID)

        KBEngine.executeRawDatabaseCommand(sql, None)

        # 3. 更新角色的公会ID
        avatar = KBEngine.getEntity(avatarID)
        if avatar:
            avatar.setGuild(guild.id)

        return guild

    @staticmethod
    def getGuildMembers(guildID):
        """获取公会成员列表（关联表查询）"""
        sql = """
            SELECT avatarID, memberType, joinTime
            FROM tbl_GuildMembers
            WHERE guildID = %d
            ORDER BY memberType ASC, joinTime ASC
        """ % guildID

        KBEngine.executeRawDatabaseCommand(sql,
            lambda result: GuildManager.onGetGuildMembers(result, guildID))

    @staticmethod
    def onGetGuildMembers(result, guildID):
        """公会成员查询回调"""
        if not result or len(result) == 0:
            return

        memberData = []
        for row in result:
            memberID = row['avatarID']
            member = KBEngine.getEntity(memberID)

            if member:
                memberData.append({
                    "id": member.id,
                    "name": member.name,
                    "level": member.level,
                    "memberType": row['memberType']
                })

        # 通知公会实体
        guild = KBEngine.getEntity(guildID)
        if guild:
            guild.onMembersLoaded(memberData)


# ==================== 4. 性能优化示例 ====================

class AvatarOptimized(KBEngine.Entity):
    """优化版本的角色实体"""

    def __init__(self):
        KBEngine.Entity.__init__(self)

        self.guildID = 0
        self._guildCache = None
        self._guildCacheTime = 0
        self._cacheTimeout = 60  # 缓存60秒

    @property
    def guild(self):
        """带缓存和超时检查的公会获取"""
        current_time = KBEngine.getTime()

        # 检查缓存有效性
        if self._guildCache is not None:
            if (current_time - self._guildCacheTime) < self._cacheTimeout:
                # 检查实体是否仍然有效
                try:
                    if self._guildCache.id == self.guildID:
                        return self._guildCache
                except:
                    pass  # 实体已销毁

        # 缓存失效，重新加载
        if self.guildID > 0:
            self._guildCache = KBEngine.getEntity(self.guildID)
            self._guildCacheTime = current_time
            return self._guildCache

        return None

    def batchLoadFriends(self, friendIDs):
        """批量加载好友（减少数据库查询）"""
        if not friendIDs:
            return

        # 批量查询
        sql = """
            SELECT * FROM tbl_Avatar
            WHERE id IN (%s)
        """ % ','.join(map(str, friendIDs))

        KBEngine.executeRawDatabaseCommand(sql, self.onBatchLoadFriends)

    def onBatchLoadFriends(self, result):
        """批量好友加载回调"""
        friends = []
        for row in result:
            friend = KBEngine.getEntity(row['id'])
            if friend:
                friends.append(friend)

        # 一次性通知客户端
        self.client.onFriendsLoaded(friends)


# ==================== 5. 数据完整性保护 ====================

class AvatarSafe(KBEngine.Entity):
    """带完整性保护的角色实体"""

    def setGuild(self, guildID):
        """安全的公会设置（带完整性检查）"""
        # 1. 验证目标实体存在
        if guildID > 0:
            targetGuild = KBEngine.getEntity(guildID)
            if not targetGuild:
                self.client.onError("公会不存在")
                return False

            # 2. 检查是否已加入其他公会
            if self.guildID > 0 and self.guildID != guildID:
                self.client.onError("已加入其他公会")
                return False

            # 3. 检查公会是否已满员
            if targetGuild.isFull():
                self.client.onError("公会已满员")
                return False

        # 4. 执行关联操作
        oldGuildID = self.guildID
        self.guildID = guildID

        # 5. 保存到数据库
        self.writeToDB()

        # 6. 更新关联表
        if oldGuildID > 0:
            self.removeFromGuildTable(oldGuildID)
        if guildID > 0:
            self.addToGuildTable(guildID)

        return True

    def removeFromGuildTable(self, guildID):
        """从公会关联表移除"""
        sql = "DELETE FROM tbl_GuildMembers WHERE guildID=%d AND avatarID=%d" % (guildID, self.id)
        KBEngine.executeRawDatabaseCommand(sql, None)

    def addToGuildTable(self, guildID):
        """添加到公会关联表"""
        sql = """
            INSERT INTO tbl_GuildMembers (guildID, avatarID, joinTime)
            VALUES (%d, %d, %d)
            ON DUPLICATE KEY UPDATE guildID=%d
        """ % (guildID, self.id, KBEngine.getTime(), guildID)

        KBEngine.executeRawDatabaseCommand(sql, None)


# ==================== 6. 使用示例 ====================

def example_usage():
    """使用示例"""

    # 1. 创建角色
    avatar = KBEngine.createEntityAnywhere("Avatar", {
        "name": "玩家1",
        "level": 10
    })

    # 2. 设置公会关联
    avatar.setGuild(12345)  # 通过ID关联

    # 3. 获取公会实体
    guild = avatar.getGuild()
    if guild:
        print(f"公会名称: {guild.name}")

    # 4. 设置目标实体
    target = KBEngine.getEntity(67890)
    avatar.setTarget(target)

    # 5. 批量查询队友
    avatar.getTeamMembers()

    # 6. 实体销毁时自动清理关联
    # avatar.destroy()