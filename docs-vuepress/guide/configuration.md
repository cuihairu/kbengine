# 配置说明

本文档详细说明 KBEngine 的配置选项。

## 配置文件结构

KBEngine 使用 XML 格式的配置文件，主要配置文件包括：

- `kbengine.xml` - 主配置文件
- `kbengine_defaults.xml` - 默认配置值
- `kbengine_res.xml` - 资源配置

## 主配置文件

### 服务器组配置

```xml
<root>
    <cellapp>
        <externalPorts>
            <any>50001</any>
        </externalPorts>
        <internalNetwork>
            <interfaceAddress>localhost</interfaceAddress>
        </internalNetwork>
    </cellapp>

    <baseapp>
        <externalPorts>
            <any>50002</any>
        </externalPorts>
        <internalNetwork>
            <interfaceAddress>localhost</interfaceAddress>
        </internalNetwork>
    </baseapp>
</root>
```

### 数据库配置

```xml
<dbmgr>
    <databaseName>kbengine</databaseName>
    <ip>localhost</ip>
    <port>3306</port>
    <user>root</user>
    <password>password</password>
    <auth>
        <default>
            <accountName>test</accountName>
            <password>test</password>
        </default>
    </auth>
</dbmgr>
```

### 日志配置

```xml
<logger>
    <logPath>logs/</logPath>
    <logLevel>INFO</logLevel>
    <logNums>10</logNums>
</logger>
```

## 配置选项详解

### CellApp 配置

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `externalPorts` | 外部端口 | 50001 |
| `internalNetwork` | 内网配置 | localhost |
| `maxConnections` | 最大连接数 | 1000 |

### BaseApp 配置

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `externalPorts` | 外部端口 | 50002 |
| `entityDefsPath` | 实体定义路径 | ./server/entity_defs/ |
| `scriptsPath` | 脚本路径 | ./server/scripts/ |

### Logger 配置

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `logPath` | 日志路径 | logs/ |
| `logLevel` | 日志级别 | INFO |
| `logNums` | 日志文件数量 | 10 |
| `logSize` | 单个日志文件大小 | 500M |

### 数据库配置

| 选项 | 说明 | 默认值 |
|------|------|--------|
| `databaseName` | 数据库名称 | kbengine |
| `ip` | 数据库地址 | localhost |
| `port` | 数据库端口 | 3306 |
| `user` | 数据库用户 | root |
| `password` | 数据库密码 | (空) |

## 高级配置

### 性能调优

#### 线程池配置

```xml
<threadPool>
    <initThreads>2</initThreads>
    <maxThreads>8</maxThreads>
</threadPool>
```

#### 网络缓冲区

```xml
<network>
    <tcpBufferSize>1048576</tcpBufferSize>
    <udpBufferSize>65536</udpBufferSize>
</network>
```

### 安全配置

#### IP 白名单

```xml
<allowedIPs>
    <ip>192.168.1.*</ip>
    <ip>10.0.0.*</ip>
</allowedIPs>
```

#### 访问控制

```xml
<auth>
    <default>
        <accountName>admin</accountName>
        <password>admin_password</password>
    </default>
</auth>
```

## 环境变量

支持通过环境变量覆盖配置：

```bash
export KBE_DB_HOST=localhost
export KBE_DB_PORT=3306
export KBE_LOG_LEVEL=DEBUG
```

## 配置验证

启动服务器时自动验证配置：

```bash
./kbe/bin/machine --check-config
```

## 常见配置场景

### 开发环境

```xml
<root>
    <cellapp>
        <externalPorts><any>50001</any></externalPorts>
    </cellapp>
    <baseapp>
        <externalPorts><any>50002</any></externalPorts>
    </baseapp>
    <logger>
        <logLevel>DEBUG</logLevel>
    </logger>
</root>
```

### 生产环境

```xml
<root>
    <cellapp>
        <externalPorts>
            <any>50001</any>
            <any>50002</any>
            <any>50003</any>
        </externalPorts>
    </cellapp>
    <baseapp>
        <externalPorts>
            <any>50101</any>
            <any>50102</any>
        </externalPorts>
    </baseapp>
    <logger>
        <logLevel>INFO</logLevel>
        <logSize>1G</logSize>
    </logger>
</root>
```

### 集群部署

```xml
<root>
    <cellapp>
        <internalNetwork>
            <interfaceAddress>192.168.1.10</interfaceAddress>
        </internalNetwork>
    </cellapp>
    <baseapp>
        <internalNetwork>
            <interfaceAddress>192.168.1.11</interfaceAddress>
        </internalNetwork>
    </baseapp>
</root>
```

## 故障排查

### 配置文件加载失败

检查 XML 语法：
```bash
xmllint --noout kbengine.xml
```

### 端口冲突

检查端口占用：
```bash
# Linux/macOS
netstat -tuln | grep 50001

# Windows
netstat -ano | findstr 50001
```

### 数据库连接失败

测试数据库连接：
```bash
mysql -h localhost -u root -p
```

## 相关文档

- [安装指南](installation.md)
- [WebConsole 指南](webconsole.md)
- [API 参考](/api/)
