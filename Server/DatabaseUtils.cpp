#include "DatabaseUtils.h"
#include <cstdio>
#include <string>
#include <iostream>

namespace AssetDatabase
{

    const std::string ITMTable = "CREATE TABLE ITM_SUPPLIES( "
        "ITM_CODE TEXT      NOT NULL, "
        "DESCRIPT TEXT      NOT NULL, "
        "STCK_QTY INT       NOT NULL, "
        "INV_LCTN TEXT      NOT NULL);";


    int SupplyCallback(void * data, int argc, char** argv, char** azColumn)
    {
        *(int*)data = std::stoi(argv[0]);
        return 0;
    }

    int PrintCallback(void * data, int argc, char** argv, char** azColumn)
    {
        for(int i =0; i < argc; i++)
        {
            std::cout << argv[i] << ", ";
        }
        std::cout << '\n';
        return 0;
    }


    int LoadDatabase(const char* databaseName , sqlite3** db)
    {
        return sqlite3_open(databaseName, db);
    }

    int MakeITMTable(sqlite3* db, char** outputMessage)
    {
        return sqlite3_exec(db,ITMTable.c_str(),NULL, 0, outputMessage);
    }

    int AddSupplyItem(SupplyItem supplyitem, sqlite3* db, int quantity, char** outputMessage)
    {
        std::string ITMCode = std::to_string(supplyitem.itmCode);
        
        std::string request = "SELECT STCK_QTY FROM ITM_SUPPLIES WHERE ITM_CODE= " + ITMCode;
        int count = -1;
        int result = sqlite3_exec(db, request.c_str(), SupplyCallback, &count, outputMessage ); 
        if (count == -1)
        {
            request = "INSERT INTO ITM_SUPPLIES(ITM_CODE, DESCRIPT, STCK_QTY, INV_LCTN) VALUES (\"" 
                + ITMCode + "\", \"" + supplyitem.description + "\", \"" 
                + std::to_string(supplyitem.quantity) + "\", \"" + supplyitem.location + "\");";
        }
        else
        {
            request = "UPDATE ITM_SUPPLIES SET STCK_QTY = " + std::to_string(count + 1) + " WHERE ITM_CODE = " + ITMCode + ";";
        }

        return result = sqlite3_exec(db, request.c_str(), nullptr, nullptr, outputMessage);
    }

    int RetrieveSupplyItem(SupplyItem supplyitem, sqlite3* db, int quantity, char** outputMessage)
    {
        std::string ITMCode = std::to_string(supplyitem.itmCode);
        
        std::string request = "SELECT STCK_QTY FROM ITM_SUPPLIES WHERE ITM_CODE= " + ITMCode;
        int count = -1;
        int result = sqlite3_exec(db, request.c_str(), SupplyCallback, &count, outputMessage ); 
        if (count > quantity - 1)
        {
            request = "UPDATE ITM_SUPPLIES SET STCK_QTY = " + std::to_string(count - quantity) + " WHERE ITM_CODE = " + ITMCode + ";";
        }
        return result = sqlite3_exec(db, request.c_str(), nullptr, nullptr, outputMessage);
    }

    int QuerySupplyItemCount(SupplyItem supplyitem, sqlite3* db, char** outputMessage)
    {
        std::string request = "SELECT STCK_QTY FROM ITM_SUPPLIES WHERE ITM_CODE = ";
        std::string ITMCode = std::to_string(supplyitem.itmCode);
        int count = -1;
        int result = sqlite3_exec(db, request.c_str(), SupplyCallback, &count, outputMessage ); 
        return result;
    }


    std::vector<AssetDatabase::Requisition> ProcessRequesitions(std::vector<AssetDatabase::Requisition> requisitions, sqlite3* db)
    {
        auto fulfilledReqs = std::vector<AssetDatabase::Requisition>();
        std::string op = "BEGIN TRANSACTION; \n\n";
        char** message;

        //iterate over all requests
        for(auto req : requisitions)
        {
            auto fulfilledSupplies = std::vector<AssetDatabase::SupplyItem>();
            for (auto item : req.supplyItems)
            {
                int inventoryCount = AssetDatabase::QuerySupplyItemCount(item, db, message);

                if (inventoryCount < item.quantity) 
                    item.quantity = inventoryCount;

                if (item.quantity > 0)
                    AssetDatabase::RetrieveSupplyItem(item, db, item.quantity, message);

                fulfilledSupplies.push_back(item);           
            }
            fulfilledReqs.push_back(req);
        }
        op += "COMMIT;";

        return fulfilledReqs;
    }

    int ProcessIntakes(std::vector<SupplyItem> supplyItems, sqlite3* db, char** message)
    {
        auto itemsToAdd = std::vector<SupplyItem>();
        std::string op = "BEGIN TRANSACTION; \n\n";

        for (auto item : supplyItems)
        {
            //check if item exists in database
            if (QuerySupplyItemCount(item, db, message)> -1)
            {
                op += "INSERT INTO ITM_SUPPLIES (ITM_CODE, DESCRIPT, STCK_QTY, INV_LCTN) VALUES (\"" 
                + std::to_string(item.itmCode) + "\", \"" + item.description 
                + "\", " + std::to_string(item.quantity) + ", " + item.location + ");\n\n";
            }
            else 
            {
                op += "UPDATE ITM_TABLE SET STCK_QTY = STCK_QTY + " + std::to_string(item.quantity);
                op += " WHERE ITM_CODE = " + std::to_string(item.quantity) + ";";
            }
            

        }
        op += "COMMIT;";

        return sqlite3_exec(db, op.c_str(), nullptr, 0, message ); 
    }

    int ListDatabase(sqlite3* db)
    {
        char* message;
        const char* op = "SELECT * FROM ITM_SUPPLIES";
        return sqlite3_exec(db, op, PrintCallback, 0, &message);
    }

    int InitializeNewDatabase(sqlite3* db)
    {
        FILE* file = fopen("InventoryLevels.csv", "r");
        char byte = 0, * message;
        int offset = 0, IDoffset = 0;;
        AssetDatabase::SupplyItem item = {};
        AssetDatabase::MakeITMTable(db, &message);

        auto initialList = std::vector<AssetDatabase::SupplyItem>();

        while((byte = fgetc(file)) != EOF)
        {
            if (byte != ',')
            {
                switch (offset % 4) 
                {
                    case 0:
                        item.itmCode = item.itmCode * 10 + byte - 48;
                        break;
                    case 1:
                        item.quantity = item.quantity * 10 + byte - 48;
                        break;
                    case 2:
                        item.description += byte;
                        break;
                    case 3:
                        item.location += byte;
                        break;
                }
            }
            else
            {
                offset++;

                if ((offset % 4) == 0)
                {
                    //add item to database;
                    initialList.push_back(item);
                    item = {};
                }
            }
        }
        fclose(file);
        for(auto supplyItem : initialList)
        {
            std::cout << "Added " << supplyItem.itmCode << ", " << supplyItem.description << ", " << supplyItem.quantity << ", to " << supplyItem.location << '\n';
            int sqlStatus = AssetDatabase::AddSupplyItem(supplyItem, db, item.quantity, &message);
            if (sqlStatus != SQLITE_OK)
            {
                std::cout << "Initialization failed: \n  " << message << '\n';
                return sqlStatus;
            }
        }
        return 0;
    }

    void ParseInputs(int argc, char ** argv, sqlite3* db)
    {

        char* message;
        int ITM = 0, qua = 0;
        if (std::string(argv[1]).compare("get") == 0 && argc == 4)
        {
            ITM = std::stoi(argv[2]);
            qua = std::stoi(argv[3]);
            if (ITM > 99999 && ITM < 1000000 && qua > 0 && qua < 128)
                AssetDatabase::RetrieveSupplyItem({ITM, 0, "", ""}, db, qua, &message);
            std::cout << "\n\n";
            AssetDatabase::ListDatabase(db);
        }
        else if (std::string(argv[1]).compare("add") == 0 && argc == 7)
        {
            ITM = std::stoi(argv[2]);
            qua = std::stoi(argv[3]);
            if (ITM > 99999 && ITM < 1000000 && qua > 0 && qua < 128)
                AssetDatabase::AddSupplyItem({ITM, 0, "", ""}, db, qua, &message);
            std::cout << "\n\n";
            AssetDatabase::ListDatabase(db);
        }
        else
        {
            std::cout << "could not parse: ";
            for(int i = 1; i < argc; i++)
                std::cout << "  " << argv[i] << ' ';            
        }
    }

}