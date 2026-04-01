# Codecov 设置指南

## 📋 概述

KBEngine 项目使用 Codecov 进行代码覆盖率追踪和报告。本指南说明如何配置 Codecov。

## 🔑 Token 配置

### 公开仓库
对于公开仓库 (`cuihairu/kbengine`)，Codecov 通常**不需要 token**。
- GitHub Actions 会自动处理上传
- Codecov 会通过 GitHub 集成自动识别仓库

### 私有仓库
如果仓库变为私有，需要配置 `CODECOV_TOKEN`：

#### 方式 1：通过 GitHub Actions Secrets（推荐）

1. 访问 [Codecov](https://codecov.io) 并登录
2. 获取你的专属 token: `Settings > Copy Token`
3. 在 GitHub 仓库中设置 Secret:
   - 进入: `Settings > Secrets and variables > Actions`
   - 点击: `New repository secret`
   - 名称: `CODECOV_TOKEN`
   - 值: 粘贴你的 Codecov token
   - 点击: `Add secret`

#### 方式 2：组织级别的 Secret

如果你有多个仓库，可以在组织级别设置：
- 进入: `Organization settings > Secrets > Actions`
- 添加组织级别的 `CODECOV_TOKEN`

## 🔧 配置文件

### `.codecov.yml` 配置

项目根目录的 `.codecov.yml` 文件包含：
- **覆盖率目标**: 项目 80%，补丁 75%
- **排除规则**: 测试文件、依赖库、生成文件
- **分支策略**: 监听 master 和 chore/* 分支
- **GitHub 集成**: PR 注释和状态检查

### GitHub Actions 集成

在 `.github/workflows/ci.yml` 中：
```yaml
- name: Upload to Codecov
  uses: codecov/codecov-action@v4
  with:
    token: ${{ secrets.CODECOV_TOKEN }}  # 可选，公开仓库不需要
```

## ✅ 验证配置

### 检查上传状态

1. **GitHub Actions 日志**
   - 进入: `Actions > 选择 workflow 运行`
   - 查看: `Upload to Codecov` 步骤的输出

2. **Codecov 仪表板**
   - 访问: `https://codecov.io/gh/cuihairu/kbengine`
   - 查看最新的覆盖率报告

3. **PR 中的注释**
   - 提交 PR 后，Codecov 会在 PR 中添加覆盖率注释

## 🎯 覆盖率目标

| 指标 | 目标 | 说明 |
|------|------|------|
| **项目覆盖率** | 80% ± 1% | 整体代码覆盖率 |
| **补丁覆盖率** | 75% ± 1% | 新增代码的覆盖率 |
| **失败条件** | 低于目标 | 覆盖率下降超过阈值会报错 |

## 🚨 故障排除

### 常见问题

1. **上传失败**
   ```
   Error: Missing upload token
   ```
   **解决**: 对于私有仓库，检查 `CODECOV_TOKEN` secret 是否正确设置

2. **覆盖率显示为 0**
   ```
   Coverage: 0.00%
   ```
   **解决**:
   - 检查 `.codecov.yml` 中的 `ignore` 规则
   - 确认编译时添加了 `--coverage` 标志
   - 查看 GitHub Actions 日志中的覆盖率收集步骤

3. **PR 中没有 Codecov 注释**
   ```
   No codecov comment on PR
   ```
   **解决**:
   - 确认仓库已在 Codecov 中同步
   - 检查 `.codecov.yml` 中的 `comment` 配置
   - 确认 Codecov 有权限访问你的仓库

## 📊 覆盖率徽章

在 README.md 中显示覆盖率徽章：

```markdown
[![Coverage](https://codecov.io/gh/cuihairu/kbengine/branch/chore/cmake-bootstrap/graph/badge.svg)]
```

徽章会自动更新为当前分支的覆盖率状态。

## 🔗 相关链接

- **Codecov 网站**: https://codecov.io
- **项目页面**: https://codecov.io/gh/cuihairu/kbengine
- **GitHub Actions**: https://github.com/cuihairu/kbengine/actions
- **Codecov 文档**: https://docs.codecov.com/

## 💡 最佳实践

1. **持续监控**: 定期检查覆盖率趋势
2. **增量改进**: 重点关注新增代码的覆盖率
3. **PR 审查**: 使用 Codecov PR 注释进行代码审查
4. **质量门控**: 设置合理的覆盖率目标，避免过度追求覆盖率而影响开发效率

---

**注意**: 对于当前公开仓库 `cuihairu/kbengine`，通常不需要设置 token。但如果遇到上传问题，请按上述步骤配置。
