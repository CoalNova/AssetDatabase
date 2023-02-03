#include <cstddef>
#include <iostream>
#include <string>
#include <string.h>
#include <sqlite3.h>
#include <bits/types/FILE.h>
#include <cstdio>
#include <thread>
#include <chrono>
#include <vector>

#include "DatabaseNet.h"
#include "DatabaseUtils.h"
#include "DatabaseSupplies.h"

#define DATABASE_NAME "itm_db.db"
#define PORT 9864


int main(int argc, char** argv)
{
    sqlite3* db = nullptr;   
    char* outputMessage;

    // check if file exists
    FILE* file = nullptr;
    bool fileExists = file = fopen(DATABASE_NAME, "r");
    if (fileExists)
        fclose(file);

    int sqlStatus = AssetDatabase::LoadDatabase( DATABASE_NAME, &db);
    //catch error opening db
    if (sqlStatus != SQLITE_OK)
    {
        std::cout << "error opening database, error code: " << sqlStatus << '\n';
        return (sqlStatus);
    }
    std::cout << "Database " << DATABASE_NAME << " loaded\n";
    
    if (!fileExists)
        AssetDatabase::InitializeNewDatabase(db);

    //check if operation is trying to be performed
    if (argc > 1)
        AssetDatabase::ParseInputs(argc, argv, db);
    else
    {
        AssetDatabase::StartServ(PORT);
        while (true)
        {
            auto reqs =std::vector<AssetDatabase::Requisition>();
            reqs.push_back(AssetDatabase::ProcServ());
            for(auto req : reqs)
                for(auto item : req.supplyItems)
                    std::cout << "Item: " << item.itmCode << ", quantity: " << item.quantity << std::endl;
        }
        AssetDatabase::KillServ();
    }   
    sqlite3_close(db);
}
