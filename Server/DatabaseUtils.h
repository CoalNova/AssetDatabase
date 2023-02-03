#pragma once
#include <sqlite3.h>
#include <vector>
#include "DatabaseSupplies.h"

namespace AssetDatabase
{
    int LoadDatabase(const char* databaseName, sqlite3** db);
    int MakeITMTable(sqlite3* db, char** outputMessage);

    int AddSupplyItem(SupplyItem supplyitem, sqlite3* db, int quantity, char** outputMessage);

    int RetrieveSupplyItem(SupplyItem supplyitem, sqlite3* db, int quantity, char** outputMessage);

    int QuerySupplyItemCount(SupplyItem supplyitem, sqlite3* db, char** outputMessage);

    std::vector<Requisition> ProcessRequesitions(std::vector<Requisition> requisitions, sqlite3* db);

    int ProcessIntakes(std::vector<SupplyItem> supplyItems);

    int ListDatabase(sqlite3* db);

    int InitializeNewDatabase(sqlite3* db);

    void ParseInputs(int argc, char ** argv, sqlite3* db);
}