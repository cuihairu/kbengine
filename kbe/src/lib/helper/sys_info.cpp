// Copyright 2008-2018 Yolo Technologies, Inc. All Rights Reserved. https://www.comblockengine.com

#include "sys_info.h"

#ifndef CODE_INLINE
#include "sys_info.inl"
#endif

#include <algorithm>
#include <cerrno>
#include <chrono>
#include <cstdio>
#include <cstring>
#include <mutex>
#include <thread>
#include <unordered_map>
#include <vector>

#if KBE_PLATFORM == PLATFORM_WIN32
#include <Iphlpapi.h>
#include <Psapi.h>
#include <processthreadsapi.h>
#include <sysinfoapi.h>
#pragma comment (lib, "iphlpapi.lib")
#pragma comment (lib, "psapi.lib")
#else
#include <signal.h>
#include <unistd.h>
#endif

#if KBE_PLATFORM == PLATFORM_APPLE
#include <ifaddrs.h>
#include <libproc.h>
#include <mach/host_info.h>
#include <mach/mach.h>
#include <mach/processor_info.h>
#include <mach/task.h>
#include <mach/task_info.h>
#include <net/if_dl.h>
#include <sys/sysctl.h>
#elif KBE_PLATFORM != PLATFORM_WIN32
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/if.h>
#include <sys/ioctl.h>
#if defined(__linux__)
#include <sys/sysinfo.h>
#endif
#endif

namespace KBEngine
{
KBE_SINGLETON_INIT(SystemInfo);

SystemInfo g_SystemInfo;

namespace
{
struct CpuSnapshot
{
  uint64 totalTicks = 0;
  uint64 busyTicks = 0;
};

struct ProcessCpuSnapshot
{
  uint64 cpuTicks = 0;
  uint64 timestampMicros = 0;
};

std::mutex g_cpuStateMutex;
CpuSnapshot g_lastCpuSnapshot;
bool g_hasCpuSnapshot = false;
std::unordered_map<uint32, ProcessCpuSnapshot> g_lastProcessCpuSamples;

#if KBE_PLATFORM != PLATFORM_WIN32
uint64 nowMicros()
{
  return static_cast<uint64>(
    std::chrono::duration_cast<std::chrono::microseconds>(
      std::chrono::steady_clock::now().time_since_epoch()).count());
}
#endif

#if defined(__linux__)
bool readProcStatTotals(CpuSnapshot& snapshot)
{
  FILE* file = std::fopen("/proc/stat", "r");
  if(file == NULL)
  {
    return false;
  }

  unsigned long long user = 0;
  unsigned long long nice = 0;
  unsigned long long system = 0;
  unsigned long long idle = 0;
  unsigned long long iowait = 0;
  unsigned long long irq = 0;
  unsigned long long softirq = 0;
  unsigned long long steal = 0;

  const int count = std::fscanf(file, "cpu %llu %llu %llu %llu %llu %llu %llu %llu",
    &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);
  std::fclose(file);

  if(count < 4)
  {
    return false;
  }

  snapshot.busyTicks = user + nice + system + irq + softirq + steal;
  snapshot.totalTicks = snapshot.busyTicks + idle + iowait;
  return snapshot.totalTicks > 0;
}

bool readProcessStat(uint32 pid, uint64& cpuTicks, uint64& rssBytes)
{
  char path[128];
  std::snprintf(path, sizeof(path), "/proc/%u/stat", pid);

  FILE* file = std::fopen(path, "r");
  if(file == NULL)
  {
    return false;
  }

  char buffer[4096];
  if(std::fgets(buffer, sizeof(buffer), file) == NULL)
  {
    std::fclose(file);
    return false;
  }

  std::fclose(file);

  char* afterName = std::strrchr(buffer, ')');
  if(afterName == NULL)
  {
    return false;
  }

  ++afterName;
  while(*afterName == ' ')
  {
    ++afterName;
  }

  std::vector<unsigned long long> values;
  values.reserve(64);

  char* token = std::strtok(afterName, " ");
  while(token != NULL)
  {
    values.push_back(std::strtoull(token, NULL, 10));
    token = std::strtok(NULL, " ");
  }

  if(values.size() <= 21)
  {
    return false;
  }

  const uint64 utime = values[11];
  const uint64 stime = values[12];
  const long pageSize = ::sysconf(_SC_PAGE_SIZE);
  cpuTicks = utime + stime;
  rssBytes = pageSize > 0 ? values[21] * static_cast<uint64>(pageSize) : 0;
  return true;
}
#endif

#if KBE_PLATFORM == PLATFORM_APPLE
bool readSystemCpuLoad(host_cpu_load_info_data_t& info)
{
  mach_msg_type_number_t count = HOST_CPU_LOAD_INFO_COUNT;
  return host_statistics(mach_host_self(), HOST_CPU_LOAD_INFO,
    reinterpret_cast<host_info_t>(&info), &count) == KERN_SUCCESS;
}

bool readProcessTaskInfo(uint32 pid, proc_taskinfo& info)
{
  return proc_pidinfo(static_cast<int>(pid), PROC_PIDTASKINFO, 0, &info, sizeof(info)) ==
    static_cast<int>(sizeof(info));
}
#endif

#if KBE_PLATFORM == PLATFORM_WIN32
uint64 fileTimeToUInt64(const FILETIME& value)
{
  ULARGE_INTEGER converted;
  converted.LowPart = value.dwLowDateTime;
  converted.HighPart = value.dwHighDateTime;
  return converted.QuadPart;
}
#endif
}

SystemInfo::SystemInfo() :
  totalmem_(0)
{
}

SystemInfo::~SystemInfo()
{
}

void SystemInfo::clear()
{
  std::lock_guard<std::mutex> lock(g_cpuStateMutex);
  g_hasCpuSnapshot = false;
  g_lastCpuSnapshot = CpuSnapshot();
  g_lastProcessCpuSamples.clear();
}

bool SystemInfo::_autocreate()
{
  return true;
}

bool SystemInfo::hasProcess(uint32 pid)
{
  if(pid == 0)
  {
    pid = static_cast<uint32>(getProcessPID());
  }

#if KBE_PLATFORM == PLATFORM_WIN32
  HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if(process == NULL)
  {
    return false;
  }

  DWORD exitCode = 0;
  const BOOL ok = GetExitCodeProcess(process, &exitCode);
  CloseHandle(process);
  return ok && exitCode == STILL_ACTIVE;
#else
  if(::kill(static_cast<pid_t>(pid), 0) == 0)
  {
    return true;
  }

  return errno == EPERM;
#endif
}

void SystemInfo::update()
{
}

uint32 SystemInfo::countCPU()
{
  const unsigned int count = std::thread::hardware_concurrency();
  return count > 0 ? count : 1;
}

SystemInfo::PROCESS_INFOS SystemInfo::getProcessInfo(uint32 pid)
{
  if(pid == 0)
  {
    pid = static_cast<uint32>(getProcessPID());
  }

  PROCESS_INFOS infos;
  infos.cpu = getCPUPerByPID(pid);
  infos.memused = getMemUsedByPID(pid);
  infos.error = !hasProcess(pid);
  return infos;
}

float SystemInfo::getCPUPer()
{
#if defined(__linux__)
  CpuSnapshot snapshot;
  if(!readProcStatTotals(snapshot))
  {
    return 0.f;
  }

  std::lock_guard<std::mutex> lock(g_cpuStateMutex);
  if(!g_hasCpuSnapshot)
  {
    g_lastCpuSnapshot = snapshot;
    g_hasCpuSnapshot = true;
    return 0.f;
  }

  const uint64 totalDelta = snapshot.totalTicks - g_lastCpuSnapshot.totalTicks;
  const uint64 busyDelta = snapshot.busyTicks - g_lastCpuSnapshot.busyTicks;
  g_lastCpuSnapshot = snapshot;

  if(totalDelta == 0)
  {
    return 0.f;
  }

  return static_cast<float>((static_cast<double>(busyDelta) * 100.0) /
    static_cast<double>(totalDelta));
#elif KBE_PLATFORM == PLATFORM_APPLE
  host_cpu_load_info_data_t info;
  if(!readSystemCpuLoad(info))
  {
    return 0.f;
  }

  CpuSnapshot snapshot;
  snapshot.busyTicks = info.cpu_ticks[CPU_STATE_USER] +
    info.cpu_ticks[CPU_STATE_SYSTEM] +
    info.cpu_ticks[CPU_STATE_NICE];
  snapshot.totalTicks = snapshot.busyTicks + info.cpu_ticks[CPU_STATE_IDLE];

  std::lock_guard<std::mutex> lock(g_cpuStateMutex);
  if(!g_hasCpuSnapshot)
  {
    g_lastCpuSnapshot = snapshot;
    g_hasCpuSnapshot = true;
    return 0.f;
  }

  const uint64 totalDelta = snapshot.totalTicks - g_lastCpuSnapshot.totalTicks;
  const uint64 busyDelta = snapshot.busyTicks - g_lastCpuSnapshot.busyTicks;
  g_lastCpuSnapshot = snapshot;

  if(totalDelta == 0)
  {
    return 0.f;
  }

  return static_cast<float>((static_cast<double>(busyDelta) * 100.0) /
    static_cast<double>(totalDelta));
#elif KBE_PLATFORM == PLATFORM_WIN32
  static bool hasSnapshot = false;
  static uint64 lastIdle = 0;
  static uint64 lastKernel = 0;
  static uint64 lastUser = 0;

  FILETIME idleTime;
  FILETIME kernelTime;
  FILETIME userTime;
  if(!GetSystemTimes(&idleTime, &kernelTime, &userTime))
  {
    return 0.f;
  }

  const uint64 idle = fileTimeToUInt64(idleTime);
  const uint64 kernel = fileTimeToUInt64(kernelTime);
  const uint64 user = fileTimeToUInt64(userTime);

  if(!hasSnapshot)
  {
    hasSnapshot = true;
    lastIdle = idle;
    lastKernel = kernel;
    lastUser = user;
    return 0.f;
  }

  const uint64 idleDelta = idle - lastIdle;
  const uint64 kernelDelta = kernel - lastKernel;
  const uint64 userDelta = user - lastUser;
  const uint64 totalDelta = kernelDelta + userDelta;

  lastIdle = idle;
  lastKernel = kernel;
  lastUser = user;

  if(totalDelta == 0 || totalDelta < idleDelta)
  {
    return 0.f;
  }

  return static_cast<float>((static_cast<double>(totalDelta - idleDelta) * 100.0) /
    static_cast<double>(totalDelta));
#else
  return 0.f;
#endif
}

float SystemInfo::getCPUPerByPID(uint32 pid)
{
  if(pid == 0)
  {
    pid = static_cast<uint32>(getProcessPID());
  }

#if defined(__linux__)
  uint64 cpuTicks = 0;
  uint64 rssBytes = 0;
  if(!readProcessStat(pid, cpuTicks, rssBytes))
  {
    return 0.f;
  }

  const uint64 now = nowMicros();
  std::lock_guard<std::mutex> lock(g_cpuStateMutex);
  ProcessCpuSnapshot& last = g_lastProcessCpuSamples[pid];
  if(last.timestampMicros == 0)
  {
    last.cpuTicks = cpuTicks;
    last.timestampMicros = now;
    return 0.f;
  }

  const uint64 cpuDelta = cpuTicks - last.cpuTicks;
  const uint64 timeDeltaMicros = now - last.timestampMicros;
  last.cpuTicks = cpuTicks;
  last.timestampMicros = now;

  if(timeDeltaMicros == 0)
  {
    return 0.f;
  }

  const long ticksPerSecond = ::sysconf(_SC_CLK_TCK);
  if(ticksPerSecond <= 0)
  {
    return 0.f;
  }

  const double cpuSeconds = static_cast<double>(cpuDelta) / static_cast<double>(ticksPerSecond);
  const double wallSeconds = static_cast<double>(timeDeltaMicros) / 1000000.0;
  if(wallSeconds <= 0.0)
  {
    return 0.f;
  }

  return static_cast<float>((cpuSeconds / wallSeconds) * 100.0);
#elif KBE_PLATFORM == PLATFORM_APPLE
  proc_taskinfo info;
  if(!readProcessTaskInfo(pid, info))
  {
    return 0.f;
  }

  const uint64 cpuMicros = (info.pti_total_user + info.pti_total_system) / 1000;
  const uint64 now = nowMicros();

  std::lock_guard<std::mutex> lock(g_cpuStateMutex);
  ProcessCpuSnapshot& last = g_lastProcessCpuSamples[pid];
  if(last.timestampMicros == 0)
  {
    last.cpuTicks = cpuMicros;
    last.timestampMicros = now;
    return 0.f;
  }

  const uint64 cpuDelta = cpuMicros - last.cpuTicks;
  const uint64 timeDeltaMicros = now - last.timestampMicros;
  last.cpuTicks = cpuMicros;
  last.timestampMicros = now;

  if(timeDeltaMicros == 0)
  {
    return 0.f;
  }

  return static_cast<float>((static_cast<double>(cpuDelta) * 100.0) /
    static_cast<double>(timeDeltaMicros));
#elif KBE_PLATFORM == PLATFORM_WIN32
  HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
  if(process == NULL)
  {
    return 0.f;
  }

  FILETIME creationTime;
  FILETIME exitTime;
  FILETIME kernelTime;
  FILETIME userTime;
  if(!GetProcessTimes(process, &creationTime, &exitTime, &kernelTime, &userTime))
  {
    CloseHandle(process);
    return 0.f;
  }

  CloseHandle(process);

  const uint64 cpuTicks = fileTimeToUInt64(kernelTime) + fileTimeToUInt64(userTime);
  const uint64 now = GetTickCount64() * 1000;

  std::lock_guard<std::mutex> lock(g_cpuStateMutex);
  ProcessCpuSnapshot& last = g_lastProcessCpuSamples[pid];
  if(last.timestampMicros == 0)
  {
    last.cpuTicks = cpuTicks;
    last.timestampMicros = now;
    return 0.f;
  }

  const uint64 cpuDelta = cpuTicks - last.cpuTicks;
  const uint64 timeDeltaMicros = now - last.timestampMicros;
  last.cpuTicks = cpuTicks;
  last.timestampMicros = now;

  if(timeDeltaMicros == 0)
  {
    return 0.f;
  }

  const double cpuMicros = static_cast<double>(cpuDelta) / 10.0;
  return static_cast<float>((cpuMicros * 100.0) / static_cast<double>(timeDeltaMicros));
#else
  return 0.f;
#endif
}

SystemInfo::MEM_INFOS SystemInfo::getMemInfos()
{
  MEM_INFOS infos = {0, 0, 0};

#if defined(__linux__)
  struct sysinfo data;
  if(::sysinfo(&data) == 0)
  {
    infos.total = static_cast<uint64>(data.totalram) * static_cast<uint64>(data.mem_unit);
    infos.free = static_cast<uint64>(data.freeram) * static_cast<uint64>(data.mem_unit);
  }
#elif KBE_PLATFORM == PLATFORM_APPLE
  uint64 total = 0;
  size_t totalSize = sizeof(total);
  if(::sysctlbyname("hw.memsize", &total, &totalSize, NULL, 0) == 0)
  {
    infos.total = total;
  }

  mach_port_t hostPort = mach_host_self();
  vm_size_t pageSize = 0;
  host_page_size(hostPort, &pageSize);

  vm_statistics64_data_t vmStats;
  mach_msg_type_number_t count = HOST_VM_INFO64_COUNT;
  if(host_statistics64(hostPort, HOST_VM_INFO64, reinterpret_cast<host_info64_t>(&vmStats), &count) == KERN_SUCCESS)
  {
    infos.free = static_cast<uint64>(vmStats.free_count + vmStats.inactive_count) *
      static_cast<uint64>(pageSize);
  }
#elif KBE_PLATFORM == PLATFORM_WIN32
  MEMORYSTATUSEX statex;
  std::memset(&statex, 0, sizeof(statex));
  statex.dwLength = sizeof(statex);
  if(GlobalMemoryStatusEx(&statex))
  {
    infos.total = statex.ullTotalPhys;
    infos.free = statex.ullAvailPhys;
  }
#else
#if defined(_SC_PHYS_PAGES) && defined(_SC_PAGE_SIZE)
  const long pageSize = ::sysconf(_SC_PAGE_SIZE);
  const long totalPages = ::sysconf(_SC_PHYS_PAGES);
  if(pageSize > 0 && totalPages > 0)
  {
    infos.total = static_cast<uint64>(pageSize) * static_cast<uint64>(totalPages);
  }
#endif

#if defined(_SC_AVPHYS_PAGES) && defined(_SC_PAGE_SIZE)
  const long availPages = ::sysconf(_SC_AVPHYS_PAGES);
  const long availPageSize = ::sysconf(_SC_PAGE_SIZE);
  if(availPages > 0 && availPageSize > 0)
  {
    infos.free = static_cast<uint64>(availPageSize) * static_cast<uint64>(availPages);
  }
#endif
#endif

  if(infos.total >= infos.free)
  {
    infos.used = infos.total - infos.free;
  }

  return infos;
}

uint64 SystemInfo::totalmem()
{
  if(totalmem_ == 0)
  {
    totalmem_ = getMemInfos().total;
  }

  return totalmem_;
}

uint64 SystemInfo::getMemUsedByPID(uint32 pid)
{
  if(pid == 0)
  {
    pid = static_cast<uint32>(getProcessPID());
  }

#if defined(__linux__)
  uint64 cpuTicks = 0;
  uint64 rssBytes = 0;
  if(!readProcessStat(pid, cpuTicks, rssBytes))
  {
    return 0;
  }

  return rssBytes;
#elif KBE_PLATFORM == PLATFORM_APPLE
  proc_taskinfo info;
  if(!readProcessTaskInfo(pid, info))
  {
    return 0;
  }

  return static_cast<uint64>(info.pti_resident_size);
#elif KBE_PLATFORM == PLATFORM_WIN32
  HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION | PROCESS_VM_READ, FALSE, pid);
  if(process == NULL)
  {
    return 0;
  }

  PROCESS_MEMORY_COUNTERS counters;
  std::memset(&counters, 0, sizeof(counters));
  if(!GetProcessMemoryInfo(process, &counters, sizeof(counters)))
  {
    CloseHandle(process);
    return 0;
  }

  CloseHandle(process);
  return static_cast<uint64>(counters.WorkingSetSize);
#else
  return 0;
#endif
}

std::vector<std::string> SystemInfo::getMacAddresses()
{
  std::vector<std::string> macAddresses;

#if KBE_PLATFORM == PLATFORM_WIN32
  PIP_ADAPTER_INFO adapterInfo = new IP_ADAPTER_INFO();
  bool adapterInfoIsArray = false;
  unsigned long size = sizeof(IP_ADAPTER_INFO);

  int result = ::GetAdaptersInfo(adapterInfo, &size);
  if(result == ERROR_BUFFER_OVERFLOW)
  {
    delete adapterInfo;
    adapterInfo = reinterpret_cast<PIP_ADAPTER_INFO>(new unsigned char[size]);
    adapterInfoIsArray = true;
    result = ::GetAdaptersInfo(adapterInfo, &size);
  }

  if(result == ERROR_SUCCESS)
  {
    for(PIP_ADAPTER_INFO current = adapterInfo; current != NULL; current = current->Next)
    {
      std::string mac;
      char buffer[4];

      for(UINT i = 0; i < current->AddressLength; ++i)
      {
        std::snprintf(buffer, sizeof(buffer), "%02x", current->Address[i]);
        mac += buffer;
      }

      std::transform(mac.begin(), mac.end(), mac.begin(), tolower);
      macAddresses.push_back(mac);
    }
  }

  if(adapterInfoIsArray)
  {
    delete[] reinterpret_cast<unsigned char*>(adapterInfo);
  }
  else
  {
    delete adapterInfo;
  }
#elif KBE_PLATFORM == PLATFORM_APPLE
  struct ifaddrs* interfaces = NULL;
  if(::getifaddrs(&interfaces) != 0)
  {
    return macAddresses;
  }

  for(struct ifaddrs* iter = interfaces; iter != NULL; iter = iter->ifa_next)
  {
    if(iter->ifa_addr == NULL || iter->ifa_addr->sa_family != AF_LINK)
    {
      continue;
    }

    const sockaddr_dl* sdl = reinterpret_cast<const sockaddr_dl*>(iter->ifa_addr);
    if(sdl->sdl_alen != 6)
    {
      continue;
    }

    const unsigned char* base = reinterpret_cast<const unsigned char*>(LLADDR(sdl));
    char mac[19];
    std::snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
      base[0], base[1], base[2], base[3], base[4], base[5]);

    if(std::find(macAddresses.begin(), macAddresses.end(), mac) == macAddresses.end())
    {
      macAddresses.push_back(mac);
    }
  }

  ::freeifaddrs(interfaces);
#else
  int fd = ::socket(AF_INET, SOCK_DGRAM, 0);
  if(fd < 0)
  {
    return macAddresses;
  }

  struct ifreq buffer[16];
  struct ifconf ifc;
  ifc.ifc_len = sizeof(buffer);
  ifc.ifc_buf = reinterpret_cast<caddr_t>(buffer);

  if(::ioctl(fd, SIOCGIFCONF, reinterpret_cast<char*>(&ifc)) == 0)
  {
    int interfaceCount = ifc.ifc_len / sizeof(struct ifreq);
    while(interfaceCount-- > 0)
    {
      if(::ioctl(fd, SIOCGIFHWADDR, reinterpret_cast<char*>(&buffer[interfaceCount])) == 0)
      {
        char mac[19];
        std::snprintf(mac, sizeof(mac), "%02x:%02x:%02x:%02x:%02x:%02x",
          static_cast<unsigned char>(buffer[interfaceCount].ifr_hwaddr.sa_data[0]),
          static_cast<unsigned char>(buffer[interfaceCount].ifr_hwaddr.sa_data[1]),
          static_cast<unsigned char>(buffer[interfaceCount].ifr_hwaddr.sa_data[2]),
          static_cast<unsigned char>(buffer[interfaceCount].ifr_hwaddr.sa_data[3]),
          static_cast<unsigned char>(buffer[interfaceCount].ifr_hwaddr.sa_data[4]),
          static_cast<unsigned char>(buffer[interfaceCount].ifr_hwaddr.sa_data[5]));

        macAddresses.push_back(mac);
      }
    }
  }

  ::close(fd);
#endif

  return macAddresses;
}
}
