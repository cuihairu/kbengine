# 安装指南

本指南将帮助你安装和配置 KBEngine。

## 系统要求

### 操作系统
- Windows 10/11
- Linux (Ubuntu 20.04+, Debian 11+, CentOS 8+)
- macOS 11+

### 软件依赖

#### 必需依赖
- **Python**: 3.12 或更高版本
- **CMake**: 3.20 或更高版本
- **C++ 编译器**:
  - Windows: Visual Studio 2019/2022
  - Linux: GCC 10+ 或 Clang 12+
  - macOS: Xcode 14+ / Clang 14+

#### 可选依赖
- **MySQL**: 8.0+ (用于数据库存储)
- **Redis**: 6.0+ (用于缓存)

## Windows 安装

### 1. 安装依赖

```powershell
# 安装 vcpkg
git clone https://github.com/Microsoft/vcpkg.git
cd vcpkg
.\bootstrap-vcpkg.bat

# 安装依赖（使用 vcpkg）
.\vcpkg install openssl mysql libhiredis fmt --triplet=x64-windows
```

### 2. 克隆项目

```powershell
git clone https://github.com/cuihairu/kbengine.git
cd kbengine
```

### 3. 配置和构建

```powershell
# 配置 CMake
cmake -B build -S . -DCMAKE_TOOLCHAIN_FILE=vcpkg/scripts/buildsystems/vcpkg.cmake

# 构建项目
cmake --build build --config Release
```

### 4. 配置服务器

编辑 `kbengine.xml` 配置文件，设置数据库连接和其他参数。

## Linux 安装

### 1. 安装系统依赖

```bash
# Ubuntu/Debian
sudo apt update
sudo apt install -y python3 python3-pip cmake build-essential \
    libmysqlclient-dev libhiredis-dev libfmt-dev

# CentOS/RHEL
sudo yum install -y python3 python3-pip cmake gcc-c++ \
    mysql-devel hiredis-devel fmt-devel
```

### 2. 克隆项目

```bash
git clone https://github.com/cuihairu/kbengine.git
cd kbengine
```

### 3. 配置和构建

```bash
# 配置 CMake
cmake -B build -S .

# 构建项目
cmake --build build -j$(nproc)

# 安装
sudo cmake --install build
```

### 4. 配置服务器

```bash
# 复制配置文件
cp etc/kbengine_defaults.xml /etc/kbengine/

# 编辑配置
sudo vi /etc/kbengine/kbengine.xml
```

## macOS 安装

### 1. 安装依赖

```bash
# 使用 Homebrew
brew install python3 cmake openssl mysql redis hiredis fmt
```

### 2. 克隆和构建

```bash
git clone https://github.com/cuihairu/kbengine.git
cd kbengine

# 配置 CMake
cmake -B build -S .

# 构建项目
cmake --build build -j$(sysctl -n hw.ncpu)
```

## 验证安装

### 启动服务器

```bash
# 启动所有服务
./kbe/bin/start_all.sh

# 或者单独启动
./kbe/bin/machine
./kbe/bin/logger
./kbe/bin/baseapp
./kbe/bin/cellapp
```

### 检查服务状态

访问 WebConsole (默认端口: 8080):
```
http://localhost:8080
```

## 常见问题

### Python 版本问题

确保使用 Python 3.12 或更高版本：
```bash
python3 --version
```

### CMake 配置失败

检查 CMake 版本：
```bash
cmake --version
```

### 编译错误

确保所有依赖都已正确安装：
- Windows: 检查 vcpkg 安装
- Linux: 使用 `ldconfig` 检查库路径
- macOS: 检查 Homebrew 安装

## 下一步

- 阅读 [配置说明](configuration.md)
- 查看 [WebConsole 指南](webconsole.md)
- 浏览 [API 参考](/api/)

## 参考文档

- [Python 3.12 升级笔记](../../python-3.12-upgrade-notes.md)
- [代码覆盖率设置](../../CODE_COVERAGE_SETUP.md)
