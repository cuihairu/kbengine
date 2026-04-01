#include <gtest/gtest.h>

#include "server/machine_infos.h"

TEST(ServerMachineInfosBootstrapTest, IncludeSucceeds)
{
  SUCCEED();
}

TEST(ServerMachineInfosBootstrapTest, MachineInfosInstance)
{
  new KBEngine::MachineInfos();
  KBEngine::MachineInfos& mi = KBEngine::MachineInfos::getSingleton();
  (void)mi.machineName();
  (void)mi.cpuInfo();
  (void)mi.memInfo();
  delete &mi;
}
