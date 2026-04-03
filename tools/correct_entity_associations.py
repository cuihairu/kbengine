"""
KBEngine 实体关联的正确实现方式
基于实际API的准确示例
"""

class Avatar(KBEngine.Entity):
    """角色实体 - 使用正确的API"""

    def __init__(self):
        KBEngine.Entity.__init__(self)

        # 关联ID
        self.guildID = 0
        self.teamID = 0

        # 运行时缓存
        self._guild = None

    # ==================== 正确的实体访问方式 ====================

    def getGuild(self):
        """获取公会实体 - 使用正确的API"""
        if self.guildID == 0:
            return None

        # ✅ 正确：通过entities字典访问
        if self.guildID in KBEngine.entities:
            return KBEngine.entities[self.guildID]

        # ✅ 或者使用get方法
        return KBEngine.entities.get(self.guildID)

    def getTeamMembers(self):
        """获取队友 - 遍历实体字典"""
        if self.teamID == 0:
            return []

        members = []
        # ✅ 正确：遍历entities字典
        for entityID, entity in KBEngine.entities.items():
            if entity.__class__.__name__ == "Avatar":
                if hasattr(entity, 'teamID') and entity.teamID == self.teamID:
                    members.append(entity)

        return members

    # ==================== 通过数据库ID关联 ====================

    def loadAccount(self):
        """从数据库加载账号实体"""
        if self.accountID == 0:
            return None

        # ✅ 正确：通过数据库ID创建实体
        KBEngine.createEntityFromDBID(
            "Account",
            self.accountID,
            self.onAccountLoaded
        )

    def onAccountLoaded(self, accountRef):
        """账号加载回调"""
        if accountRef is not None:
            account = accountRef if hasattr(accountRef, 'id') else None
            if account:
                self.accountName = account.name
                self.client.onAccountData(account.name, account.level)

    # ==================== 使用executeRawDatabaseCommand ====================

    def getFriendsFromDB(self):
        """从数据库直接查询好友关系"""
        sql = """
            SELECT avatarID2 FROM tbl_Friends
            WHERE avatarID1 = %d
        """ % self.id

        # ✅ 正确：使用数据库查询获取关联实体ID列表
        KBEngine.executeRawDatabaseCommand(
            sql,
            self.onFriendsQueryResult
        )

    def onFriendsQueryResult(self, result):
        """好友查询结果回调"""
        if not result or len(result) == 0:
            self.client.onFriendsList([])
            return

        friendIDs = [row['avatarID2'] for row in result]
        friends = []

        # ✅ 正确：通过entities字典获取实体
        for friendID in friendIDs:
            friend = KBEngine.entities.get(friendID)
            if friend:
                friends.append({
                    "id": friend.id,
                    "name": friend.name,
                    "level": friend.level
                })

        self.client.onFriendsList(friends)

    # ==================== 批量创建关联实体 ====================

    def loadGuildMembers(self, guildID):
        """批量加载公会成员"""
        # 先查询数据库获取所有成员ID
        sql = """
            SELECT avatarID FROM tbl_GuildMembers
            WHERE guildID = %d
        """ % guildID

        KBEngine.executeRawDatabaseCommand(
            sql,
            lambda result: self.onGuildMembersQuery(result, guildID)
        )

    def onGuildMembersQuery(self, result, guildID):
        """公会成员查询结果"""
        if not result or len(result) == 0:
            return

        memberIDs = [row['avatarID'] for row in result]
        loadedMembers = []

        # ✅ 对于已加载的实体，直接获取
        for memberID in memberIDs:
            member = KBEngine.entities.get(memberID)
            if member:
                loadedMembers.append(member)

        # ✅ 对于未加载的实体，从数据库加载
        unloadedMemberIDs = [
            mid for mid in memberIDs
            if mid not in KBEngine.entities
        ]

        if unloadedMemberIDs:
            # 批量从数据库加载（逐个加载）
            for memberID in unloadedMemberIDs:
                KBEngine.createEntityFromDBID(
                    "Avatar",
                    memberID,
                    lambda ref, mid=memberID: self.onMemberLoaded(ref, mid, loadedMembers)
                )
        else:
            # 全部加载完成，通知客户端
            self.client.onGuildMembersLoaded(loadedMembers)

    def onMemberLoaded(self, memberRef, memberID, membersList):
        """单个成员加载回调"""
        if memberRef is not None:
            member = memberRef if hasattr(memberRef, 'id') else None
            if member:
                membersList.append(member)

        # 这里需要计数判断是否全部加载完成
        # 实际实现中需要更复杂的逻辑

    # ==================== 客户端实体访问 ====================

    def getTargetEntityClient(self, targetID):
        """获取目标实体（客户端调用）"""
        # 这个方法在服务端，但给客户端参考

        # ✅ 客户端使用findEntity
        # target = KBEngine.findEntity(targetID)
        # if target:
        #     self.client.onTargetFound(target.id, target.position)

        pass


# ==================== 关联表管理示例 ====================

class GuildManager:
    """公会管理器 - 正确实现"""

    @staticmethod
    def addMember(guildID, avatarID):
        """添加公会成员"""
        # 1. 更新数据库关联表
        sql = """
            INSERT INTO tbl_GuildMembers (guildID, avatarID, joinTime)
            VALUES (%d, %d, %d)
            ON DUPLICATE KEY UPDATE guildID=%d
        """ % (guildID, avatarID, KBEngine.time(), guildID)

        KBEngine.executeRawDatabaseCommand(sql, None)

        # 2. 更新实体的guildID属性
        avatar = KBEngine.entities.get(avatarID)
        if avatar:
            avatar.guildID = guildID
            avatar.writeToDB()

    @staticmethod
    def getGuildMembers(guildID, callback):
        """获取公会成员列表"""
        sql = """
            SELECT avatarID FROM tbl_GuildMembers
            WHERE guildID = %d
            ORDER BY joinTime ASC
        """ % guildID

        KBEngine.executeRawDatabaseCommand(
            sql,
            lambda result: GuildManager.onMembersQuery(result, callback)
        )

    @staticmethod
    def onMembersQuery(result, callback):
        """成员查询结果处理"""
        if not result or len(result) == 0:
            callback([])
            return

        members = []
        for row in result:
            avatarID = row['avatarID']
            # ✅ 正确：从entities字典获取
            avatar = KBEngine.entities.get(avatarID)
            if avatar:
                members.append({
                    "id": avatar.id,
                    "name": avatar.name,
                    "level": avatar.level
                })

        callback(members)


# ==================== 客户端实体关联示例 ====================

class ClientEntityManager:
    """客户端实体管理（在客户端脚本中使用）"""

    @staticmethod
    def findEntityAndInteract(entityID):
        """查找实体并交互"""
        # ✅ 客户端使用findEntity
        entity = KBEngine.findEntity(entityID)

        if entity:
            # 执行交互
            entity.interact()
        else:
            print(f"实体 {entityID} 不存在")

    @staticmethod
    def getTargetEntity(targetID):
        """获取目标实体"""
        # ✅ 客户端方式
        return KBEngine.findEntity(targetID)

    @staticmethod
    def listAllEntities():
        """列出所有客户端可见实体"""
        entities = []
        # ✅ 客户端遍历entities
        for entityID, entity in KBEngine.entities.items():
            entities.append({
                "id": entity.id,
                "position": entity.position,
                "modelName": entity.modelName
            })
        return entities


# ==================== 性能优化建议 ====================

class AvatarOptimized(KBEngine.Entity):
    """优化后的实体关联实现"""

    def __init__(self):
        KBEngine.Entity.__init__(self)

        self.guildID = 0
        self._guildCache = None
        self._lastCacheCheck = 0

    def getGuildOptimized(self):
        """带缓存优化的公会获取"""
        # 检查缓存是否过期（5秒）
        if KBEngine.time() - self._lastCacheCheck < 5:
            return self._guildCache

        self._lastCacheCheck = KBEngine.time()

        # ✅ 重新从entities获取
        if self.guildID in KBEngine.entities:
            self._guildCache = KBEngine.entities[self.guildID]
        else:
            self._guildCache = None

        return self._guildCache

    def batchGetEntities(self, entityIDs):
        """批量获取实体（避免重复查询）"""
        found = []
        notFound = []

        for entityID in entityIDs:
            # ✅ 批量从entities获取
            if entityID in KBEngine.entities:
                found.append(KBEngine.entities[entityID])
            else:
                notFound.append(entityID)

        return found, notFound


# ==================== 错误示例对比 ====================

class WrongAvatarExamples:
    """错误的实现示例（不要这样做）"""

    def wrongMethod1(self):
        # ❌ 错误：没有KBEngine.getEntity函数
        # entity = KBEngine.getEntity(12345)
        pass

    def wrongMethod2(self):
        # ❌ 错误：直接索引可能引发KeyError
        # entity = KBEngine.entities[12345]
        pass

    def wrongMethod3(self):
        # ❌ 错误：假设实体一定存在
        # entity = KBEngine.entities.get(12345)
        # entity.name = "new_name"  # entity可能为None
        pass


class CorrectAvatarExamples:
    """正确的实现示例"""

    def correctMethod1(self):
        # ✅ 正确：使用get方法安全访问
        entity = KBEngine.entities.get(12345)
        if entity:
            entity.name = "new_name"

    def correctMethod2(self):
        # ✅ 正确：先检查再访问
        entityID = 12345
        if entityID in KBEngine.entities:
            entity = KBEngine.entities[entityID]
            entity.name = "new_name"

    def correctMethod3(self):
        # ✅ 正确：遍历所有实体
        avatars = [
            entity for entity in KBEngine.entities.values()
            if entity.__class__.__name__ == "Avatar"
        ]

    def correctMethod4(self):
        # ✅ 正确：使用数据库查询获取ID列表，再获取实体
        sql = "SELECT avatarID FROM tbl_TeamMembers WHERE teamID = %d" % self.teamID
        KBEngine.executeRawDatabaseCommand(sql, self.onTeamQuery)

    def onTeamQuery(self, result):
        if result:
            for row in result:
                avatarID = row['avatarID']
                avatar = KBEngine.entities.get(avatarID)
                if avatar:
                    print(f"队友: {avatar.name}")


# ==================== 总结 ====================

"""
KBEngine实体访问的正确方式：

服务端（baseapp/cellapp）：
- KBEngine_entities.get(entityID)     # 推荐
- KBEngine_entities[entityID]        # 需要先检查存在
- entityID in KBEngine_entities      # 检查存在性

客户端：
- KBEngine.findEntity(entityID)       # 专用查找函数

从数据库加载：
- KBEngine.createEntityFromDBID(entityType, dbID, callback)

重要提醒：
1. ❌ 没有KBEngine.getEntity()函数
2. ✅ 服务端用KBEngine.entities字典
3. ✅ 客户端用KBEngine.findEntity()函数
4. ✅ 复杂关联使用关联表 + executeRawDatabaseCommand
"""