KBEngine
========

> **⚠️ Development Fork Notice**
>
> This is a **development fork** for contributing improvements to the upstream KBEngine project.
>
> - **Branch**: `dev` contains active development work
> - **Branch**: `master` tracks upstream `kbengine/kbengine`
>
> **For production use**, please use the official repository:
> - Official: https://github.com/kbengine/kbengine
> - Releases: https://github.com/kbengine/kbengine/releases
> - Documentation: https://kbengine.github.io/docs/
>
> This fork does **NOT** provide stable releases or downloads.

[![CI](https://github.com/cuihairu/kbengine/workflows/CI/badge.svg)](https://github.com/cuihairu/kbengine/actions)
[![Coverage](https://codecov.io/gh/cuihairu/kbengine/branch/chore/cmake-bootstrap/graph/badge.svg)](https://codecov.io/gh/cuihairu/kbengine)
[![C++17](https://img.shields.io/badge/C++-17-blue.svg)](https://en.wikipedia.org/wiki/C%2B%2B#Standard_version)
[![Python 3.12](https://img.shields.io/badge/python-3.12-blue.svg)](https://www.python.org/)


## Homepage

	https://kbengine.github.io/


## Releases

	Sources		: https://github.com/kbengine/kbengine/releases/latest


## Demo sources

	Unity3d		: https://github.com/kbengine/kbengine_unity3d_demo/releases/latest
	Unity3d		: https://github.com/kbengine/kbengine_unity3d_warring/releases/latest
	UE4		: https://github.com/kbengine/kbengine_ue4_demo/releases/latest
	Ogre		: https://github.com/kbengine/kbengine_ogre_demo/releases/latest
	Cocos2d_js	: https://github.com/kbengine/kbengine_cocos2d_js_demo/releases/latest
	Godot Engine	: https://github.com/krogank9/kbe_godot_demo/releases/latest


## Docs

	Docs		: https://kbengine.github.io/docs/
	API		: https://github.com/kbengine/kbengine/tree/master/docs


## Support

	Email		: kbesrv?gmail.com
	QQ		: 3603661
	BBS		: http://bbs.kbengine.org
	Maillist	: https://groups.google.com/d/forum/kbengine_maillist


## Stresstest

	https://www.youtube.com/watch?v=sWtk3CfxyGY
	http://v.youku.com/v_show/id_XMjgyMjM0MTYwNA==.html?spm=a2h3j.8428770.3416059.1


## What is KBEngine?

	An MMOG server engine. 
	Just use Python scripting to be able to complete any game logic simply and efficiently (supports hotfixing).
	Various KBEngine plugins can be quickly combined with (Unity3D, OGRE, Cocos2d-x, HTML5, etc.) technology to 
	form a complete game client.

	The engine is written in C++, and saves developers from having to re-implement common server-side 
	technology, allowing them to concentrate on game logic development, to quickly create a variety of games.

	(Because it is often asked what the upper limit of the load is that KBEngine can handle, the underlying 
	architecture has been designed as a multi-process distributed dynamic load balancing solution. In theory, 
	by continuously expanding the hardware, the upper limit of the load can also be continuously increased. 
	The upper limit of the capacity of a single machine depends on the complexity of the game logic itself.)


## Create a new game asset library

	Execute:
		new_assets.bat

	Output:
		server_assets


	## CMake and vcpkg

		The CMake build now supports vcpkg manifest mode.
		When configured with the vcpkg toolchain, KBEngine will prefer vcpkg-provided fmt, zlib, OpenSSL, libmariadb, hiredis and GTest.
		If vcpkg is not active, the current vendored/fallback path remains available.
		The manifest is pinned with a vcpkg builtin baseline for reproducible dependency resolution.
		The current manifest resolves to:
			fmt 12.1.0
			OpenSSL 3.6.1
			curl 8.18.0
			zlib 1.3.1
			GTest 1.17.0
			hiredis 1.3.0
			libmariadb 3.4.7
		libmariadb 3.4.7 is the MariaDB Connector/C client library used by KBEngine.
		It is suitable for MariaDB and MySQL deployments, including MySQL 8 authentication flows introduced after older 3.0.x connector releases.

		Example:
			export VCPKG_ROOT=/path/to/vcpkg
			cmake --preset vcpkg
			cmake --build --preset vcpkg
			ctest --preset vcpkg

## Building with CMake

KBEngine uses CMake 3.21+ for cross-platform building. The build system supports two modes:

1. **vcpkg mode** (Recommended): Uses vcpkg for dependency management
2. **Fallback mode**: Uses vendored dependencies

### Prerequisites

**Common Requirements:**
- CMake 3.21 or higher
- C++17 compatible compiler

**Platform-Specific:**
- **Windows**: Visual Studio 2019/2022 with C++ development tools
- **Linux**: GCC 8+ or Clang 10+
- **macOS**: Xcode 13+ or Clang 10+

**Dependencies:**
- All dependencies are managed by **vcpkg** when using vcpkg mode
- This includes Python 3.12, which is embedded into KBEngine (requires Python headers for compilation)
- Two libraries use vendored source code: **tmxparser** (map parser) and **g3dlite** (3D math library)

---

### Quick Start (vcpkg mode)

#### Windows

```powershell
# 1. Set VCPKG_ROOT environment variable
$env:VCPKG_ROOT = "D:\path\to\vcpkg"

# 2. Configure CMake
cmake --preset vcpkg

# 3. Build
cmake --build build/presets/vcpkg --parallel

# 4. Run tests (optional)
ctest --preset vcpkg --output-on-failure
```

**Or use the provided build script:**
```cmd
REM Modify build.bat to match your Visual Studio path
build.bat
```

#### Linux

```bash
# 1. Set VCPKG_ROOT
export VCPKG_ROOT=/path/to/vcpkg

# 2. Configure CMake
cmake --preset vcpkg

# 3. Build
cmake --build build/presets/vcpkg --parallel

# 4. Run tests (optional)
ctest --preset vcpkg --output-on-failure
```

#### macOS

```bash
# 1. Set VCPKG_ROOT
export VCPKG_ROOT=/path/to/vcpkg

# 2. Configure CMake
cmake --preset vcpkg

# 3. Build
cmake --build build/presets/vcpkg --parallel

# 4. Run tests (optional)
ctest --preset vcpkg --output-on-failure
```

---

### Platform-Specific Details

#### Windows

**Key Differences:**
- **Generator**: Uses Ninja by default (via CMake presets)
- **Runtime Library**: Uses DLL runtime (`/MD` or `/MDd`)
- **Compiler**: MSVC with UTF-8 support enabled (`/utf-8`)
- **Path Handling**: Windows paths use backslashes

**Important Notes:**
1. Must run from **x64 Native Tools Command Prompt** or setup environment via script
2. Visual Studio 2019/2022 required
3. VCPKG_ROOT must be set before configuring
4. Build output: `kbe\bin\server\`

**Troubleshooting:**
```cmd
REM If CMake can't find Visual Studio
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64

REM If vcpkg dependencies fail
vcpkg install fmt:x64-windows openssl:x64-windows
```

#### Linux

**Key Differences:**
- **Generator**: Uses Ninja (preferred) or Unix Makefiles
- **Compiler Flags**: Uses GCC/Clang flags for coverage and optimization
- **Runtime**: Static linking by default
- **Dependencies**: May need system packages installed

**System Dependencies:**
```bash
# Ubuntu/Debian
sudo apt-get install build-essential cmake ninja-build

# Fedora/RHEL
sudo dnf install gcc-c++ cmake ninja-build

# Arch Linux
sudo pacman -S base-devel cmake ninja
```

**Important Notes:**
1. Use `ninja` for faster builds (automatically used by presets)
2. Build output: `kbe/bin/server/`
3. May need to set `LD_LIBRARY_PATH` for runtime dependencies

#### macOS

**Key Differences:**
- **Compiler**: Clang from Xcode Command Line Tools
- **Frameworks**: Uses macOS frameworks instead of Linux libraries
- **Universal Binary**: Can build for x86_64, arm64, or universal
- **Code Signing**: May require for distribution

**System Requirements:**
```bash
# Install Xcode Command Line Tools
xcode-select --install

# Or install full Xcode from App Store

# Install Homebrew for additional dependencies
brew install cmake ninja python3
```

**Architecture-Specific Builds:**
```bash
# Apple Silicon (M1/M2/M3)
cmake --preset vcpkg -DCMAKE_OSX_ARCHITECTURES=arm64

# Intel Mac
cmake --preset vcpkg -DCMAKE_OSX_ARCHITECTURES=x86_64

# Universal Binary (both architectures)
cmake --preset vcpkg -DCMAKE_OSX_ARCHITECTURES="arm64;x86_64"
```

**Important Notes:**
1. macOS 11 (Big Sur) or later recommended
2. Build output: `kbe/bin/server/`
3. May need to adjust SDK version for older macOS support

---

### Build Modes

#### Debug Mode (Default)
```bash
cmake --preset vcpkg -DCMAKE_BUILD_TYPE=Debug
```

#### Release Mode
```bash
cmake --preset vcpkg -DCMAKE_BUILD_TYPE=Release
```

#### Release with Debug Info
```bash
cmake --preset vcpkg -DCMAKE_BUILD_TYPE=RelWithDebInfo
```

---

### Testing

```bash
# Run all tests
ctest --preset vcpkg --output-on-failure

# Run specific test
ctest --preset vcpkg -R <test_name> --verbose

# Run tests with parallel execution
ctest --preset vcpkg --parallel 4
```

---

### vcpkg Dependencies

The project uses **vcpkg manifest mode** for dependency management.

**Required dependencies (provided by vcpkg):**
- fmt 12.1.0 - Text formatting
- OpenSSL 3.6.1 - Cryptography & SSL/TLS
- curl 8.18.0 - HTTP client
- zlib 1.3.1 - Compression
- GTest 1.17.0 - Testing framework
- hiredis 1.3.0 - Redis client
- libmariadb 3.4.7 - MySQL/MariaDB connector
- Python 3.12 - Embedded scripting (KBEngine embeds Python interpreter)
- tinyxml2 - XML parsing
- utf8cpp - UTF-8 string handling
- log4cxx - Logging
- apr/apr-util - Apache Portable Runtime
- jemalloc - Memory allocator

**Vendored libraries (not in vcpkg):**
- **tmxparser** - Map tile format parser
- **g3dlite** - 3D math and geometry library

**Note:** In vcpkg mode, all dependencies must come from vcpkg. The build will fail if any required vcpkg dependency is not found.

**Updating dependencies:**
```bash
# Modify vcpkg.json to change versions
# Then rebuild
cmake --preset vcpkg
cmake --build build/presets/vcpkg
```

---

### Output Locations

After successful build:

| Platform | Executables | Libraries |
|----------|-------------|-----------|
| Windows | `kbe\bin\server\` | `kbe\src\libs\` |
| Linux | `kbe/bin/server/` | `kbe/src/libs/` |
| macOS | `kbe/bin/server/` | `kbe/src/libs/` |

---

### Troubleshooting

#### CMake Configuration Fails

```bash
# Clear CMake cache
rm -rf build/presets/vcpkg

# Reconfigure
cmake --preset vcpkg
```

#### vcpkg Dependencies Fail

```bash
# Clean vcpkg build
vcpkg remove --outdated

# Rebuild dependencies
vcpkg install --recurse
```

#### Compiler Not Found

**Windows:**
```cmd
# Ensure Visual Studio environment is set
call "C:\Program Files\Microsoft Visual Studio\2022\Enterprise\Common7\Tools\VsDevCmd.bat" -arch=x64
```

**Linux/macOS:**
```bash
# Check compiler
gcc --version
clang --version

# Install if missing
# Ubuntu/Debian: sudo apt-get install build-essential
# macOS: xcode-select --install
```

#### Python Not Found

```bash
# If CMake cannot find Python3, ensure vcpkg is properly configured
# Python 3.12 is provided by vcpkg, no system installation needed
cmake --preset vcpkg -DVCPKG_ROOT=/path/to/vcpkg
```

---

### Advanced Options

#### Disable Tests

```bash
cmake --preset vcpkg -DBUILD_TESTING=OFF
```

#### Enable Code Coverage (Linux/macOS only)

```bash
cmake --preset vcpkg -DKBE_ENABLE_CODE_COVERAGE=ON
```

#### Custom Installation Prefix

```bash
cmake --preset vcpkg -DCMAKE_INSTALL_PREFIX=/opt/kbengine
```

---

### Additional Resources

- **CMake Documentation**: https://cmake.org/documentation/
- **vcpkg Documentation**: https://vcpkg.io/en/
- **Upstream Documentation**: https://kbengine.github.io/docs/
- **Issue Tracker**: https://github.com/kbengine/kbengine/issues


## 中文

[官网](https://kbengine.github.io/cn/)，[论坛](https://github.com/kbengine/kbengine/discussions)，QQ交流群：461368412、16535321、716679842


## 什么是KBEngine?

	一款MMOG游戏服务端引擎，
	仅Python脚本即可简单高效的完成任何游戏逻辑(支持热更新)，
	使用配套客户端插件能够快速与（Unity3D、UE4、OGRE、HTML5、等等）结合形成一个完整的客户端。

	引擎使用C++编写，开发者无需重复的实现游戏服务端通用的底层技术，
	将精力真正集中到游戏开发层面上来，稳定可靠并且快速的打造各种网络游戏。

	(经常被问到承载上限，KBEngine底层架构被设计为多进程分布式动态负载均衡方案，
	理论上只需要不断扩展硬件就能够不断增加承载上限，单台机器的承载上限取决于游戏逻辑本身的复杂度。)
