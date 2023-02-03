#pragma once
#include <string>
#include <vector>

namespace AssetDatabase
{
    struct SupplyItem
    {
        int itmCode = 0,
            quantity = 0;
        std::string 
            description = "", 
            location = "";
    };

    union ITMCode
    {
        unsigned short num;
        char bytes[6];
        std::string string;
    };

    struct Requisition
    {
        int siteCode = 0;
        std::vector<SupplyItem> supplyItems = std::vector<SupplyItem>();
        std::string requestor = "";
        std::string address = "";
        std::string billingInfo = "";
    };

}