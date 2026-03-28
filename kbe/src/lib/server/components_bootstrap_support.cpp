#include "common/platform.h"

#include "baseapp/baseapp_interface.h"
#include "baseappmgr/baseappmgr_interface.h"
#include "cellapp/cellapp_interface.h"
#include "cellappmgr/cellappmgr_interface.h"
#include "dbmgr/dbmgr_interface.h"
#include "loginapp/loginapp_interface.h"
#include "machine/machine_interface.h"
#include "tools/bots/bots_interface.h"
#include "tools/interfaces/interfaces_interface.h"
#include "tools/logger/logger_interface.h"

namespace KBEngine
{

COMPONENT_ORDER g_componentGlobalOrder = -1;
COMPONENT_ORDER g_componentGroupOrder = -1;
COMPONENT_GUS g_genuuid_sections = -1;

}
