#pragma once

#include <cstdlib>
#include <filesystem>
#include <fstream>
#include <string>
#include <system_error>

#include <gtest/gtest.h>

#include "resmgr/resmgr.h"

namespace KBEngineTest {

namespace fs = std::filesystem;

struct EnvVarState
{
  bool has_value = false;
  std::string value;
};

inline EnvVarState capture_env_var(const char* name)
{
  const char* value = std::getenv(name);
  if (value == nullptr)
  {
    return {};
  }

  return {true, value};
}

inline bool set_env_var(const char* name, const std::string& value)
{
#if defined(_WIN32)
  return _putenv_s(name, value.c_str()) == 0;
#else
  return ::setenv(name, value.c_str(), 1) == 0;
#endif
}

inline void restore_env_var(const char* name, const EnvVarState& state)
{
  if (state.has_value)
  {
    (void)set_env_var(name, state.value);
    return;
  }

#if defined(_WIN32)
  (void)_putenv_s(name, "");
#else
  (void)::unsetenv(name);
#endif
}

inline void write_file(const fs::path& path, const std::string& contents)
{
  fs::create_directories(path.parent_path());
  std::ofstream out(path, std::ios::binary);
  ASSERT_TRUE(out.is_open()) << path.string();
  out << contents;
}

class ScopedResmgrEnvironment
{
public:
  explicit ScopedResmgrEnvironment(const char* name)
      : root_(fs::temp_directory_path() / name),
        old_root_(capture_env_var("KBE_ROOT")),
        old_res_path_(capture_env_var("KBE_RES_PATH")),
        old_bin_path_(capture_env_var("KBE_BIN_PATH")),
        ready_(false)
  {
    std::error_code ec;
    fs::remove_all(root_, ec);
    fs::create_directories(system_res_dir() / "server");
    fs::create_directories(user_res_dir() / "server");
    fs::create_directories(user_scripts_dir());
    fs::create_directories(bin_dir());

    const std::string res_path =
        system_res_dir().string() + ";" +
        user_assets_dir().string() + ";" +
        user_scripts_dir().string() + ";" +
        user_res_dir().string();

    const bool env_ready =
        set_env_var("KBE_ROOT", root_.string()) &&
        set_env_var("KBE_RES_PATH", res_path) &&
        set_env_var("KBE_BIN_PATH", bin_dir().string());

    if (!env_ready)
    {
      return;
    }

    if (KBEngine::Resmgr::getSingletonPtr() == nullptr)
    {
      new KBEngine::Resmgr();
    }

    ready_ = KBEngine::Resmgr::getSingleton().initialize();
  }

  ~ScopedResmgrEnvironment()
  {
    restore_env_var("KBE_ROOT", old_root_);
    restore_env_var("KBE_RES_PATH", old_res_path_);
    restore_env_var("KBE_BIN_PATH", old_bin_path_);

    if (KBEngine::Resmgr::getSingletonPtr() != nullptr)
    {
      KBEngine::Resmgr::getSingleton().initialize();
    }

    std::error_code ec;
    fs::remove_all(root_, ec);
  }

  bool ready() const
  {
    return ready_;
  }

  const fs::path& root() const
  {
    return root_;
  }

  fs::path system_res_dir() const
  {
    return root_ / "kbe" / "res";
  }

  fs::path user_assets_dir() const
  {
    return root_ / "assets";
  }

  fs::path user_scripts_dir() const
  {
    return user_assets_dir() / "scripts";
  }

  fs::path user_res_dir() const
  {
    return user_assets_dir() / "res";
  }

  fs::path bin_dir() const
  {
    return root_ / "kbe" / "bin" / "server";
  }

private:
  fs::path root_;
  EnvVarState old_root_;
  EnvVarState old_res_path_;
  EnvVarState old_bin_path_;
  bool ready_;
};

}  // namespace KBEngineTest
