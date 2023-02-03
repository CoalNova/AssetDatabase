#pragma once

#include <vector>

#include "DatabaseSupplies.h"

namespace AssetDatabase
{
    int StartServ(int port);

    Requisition ProcServ();
    
    int KillServ();
}