#include <Utils/WifiUtils.hpp>
#include <algorithm>
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <cstring>
#include <Utils/ThreadSafeDbg.hpp>

void WifiUtils::removeSSIDDuplicates(apInfoList &apList) {

    for (auto first = apList.begin(); first != apList.end();) {

        bool erased = false;

        for (auto second = std::next(first, 1); second != apList.end(); second++) {

            if (strcmp((char *) first->ssid, (char *) second->ssid) == 0) {
                first = apList.erase(first);
                erased = true;
                break;
            }
        }
        if (false == erased) {
            ++first;
        }
    }
}

void WifiUtils::removeBSSIDDuplicates(apInfoList &apList) {

    for (auto first = apList.begin(); first != apList.end();) {

        bool erased = false;

        for (auto second = std::next(first, 1); second != apList.end(); second++) {

            if (memcmp(first->bssid, second->bssid, BSSID_DIGITS) == 0) {
                DBG_PRINT_TAG("WIFIUTILS", "removing ssid duplicate: %s", first->ssid);
                first = apList.erase(first);
                erased = true;
                break;
            }
        }
        if (false == erased) {
            ++first;
        }
    }
}

void WifiUtils::sortByRSSIDecrease(apInfoList &apList) {

    for (uint32_t i = 0; i < apList.size(); i++) {
        for (uint32_t j = i + 1; j < apList.size(); j++) {
            if (apList[j].rssi > apList[i].rssi) {
                std::swap(apList[i], apList[j]);
            }
        }
    }
}

void WifiUtils::filterBySSID(const char *SSID, apInfoList &apList) {

    auto removeAps = [&](wifi_ap_record_t rec) -> bool {
        return (0 != strcmp(SSID, (char *) rec.ssid));
    };
    auto removeIt = std::remove_if(apList.begin(), apList.end(), removeAps);
    apList.erase(removeIt, apList.end());
}

void WifiUtils::bssidFromStrToArray(const char *bssidStr, uint8_t bssidNum[BSSID_DIGITS]) {

    string stringToken(bssidStr);
    char *token;
    int digitIndex = 0;
    char *savePtr;

    token = strtok_r((char *) stringToken.data(), ":", &savePtr);

    while (token != nullptr) {

        if (digitIndex >= BSSID_DIGITS) {
            break;
        }
        bssidNum[digitIndex++] = strtoul(token, nullptr, 16);
        token = strtok_r(nullptr, ":", &savePtr);
    }
}

string WifiUtils::bssidArrayToStr(const uint8_t bssidNum[BSSID_DIGITS]) {

    string ret;
    char buffer[4] = "";

    for (int i = 0; i < BSSID_DIGITS; i++) {
        if (bssidNum[i] <= 15) {
            ret += "0";
        }
        itoa(bssidNum[i], buffer, 16);
        ret += buffer;
        ret += ":";
    }

    //Delete last colon
    ret.pop_back();

    // Transfrom to upper-case
    for (auto &c: ret) {
        c = toupper(c);
    }
    return ret;
}

// namespace WifiUtils
// {

//     static void removeAllExceptFirst(apIndices_t &list)
//     {
//         if(!list.empty())
//         {
//             list.erase(list.begin() + 1, list.end());
//         }
//     }



//     int8_t removeSSIDDuplicates(apIndices_t &APIndicesList)
//     {

//         for (auto i = APIndicesList.begin(); i != APIndicesList.end();) 
//         {
//             bool erased = false;

//             for (auto j = std::next(i, 1); j != APIndicesList.end(); j++) 
//             {
//                 if (WiFi.SSID(*i) == WiFi.SSID(*j)) 
//                 {
//                     i = APIndicesList.erase(i);
//                     erased = true;
//                     break;
//                 }
//             }

//             if(!erased) ++i;
//         }

//         return 0;
//     }

//     int8_t removeBSSIDDuplicates(apIndices_t &APIndicesList)
//     {
//         for (auto i = APIndicesList.begin(); i != APIndicesList.end();) 
//         {
//             bool erased = false;

//             for (auto j = std::next(i, 1); j != APIndicesList.end(); j++) 
//             {
//                 if (isBSSIDsEqual(WiFi.BSSID(*i), WiFi.BSSID(*j))) 
//                 {
//                     i = APIndicesList.erase(i);
//                     erased = true;
//                     break;
//                 }
//             }

//             if(!erased) ++i;
//         }

//         return 0;
//     }

//     bool isBSSIDsEqual(const uint8_t first[BSSID_DIGITS], const uint8_t second[BSSID_DIGITS])
//     {
//         for (uint8_t i = 0; i < BSSID_DIGITS; i++)
//         {
//             if (first[i] != second[i])
//             {
//                 return false;
//             }
//         }
//         return true;
//     }

//     bool isBSSIDempty(const uint8_t bssid[BSSID_DIGITS])
//     {
//         uint8_t zeroBSSID[BSSID_DIGITS];
//         std::fill(zeroBSSID, zeroBSSID + BSSID_DIGITS, 0);

//         return isBSSIDsEqual(bssid, zeroBSSID);
//     }

//     apIndices_t rescanApIndices(){

//         WifiUtils::apIndices_t retIndicesList;
//         int8_t numAP;

//         cleanScanData();
//         numAP = WiFi.scanNetworks();

//         for (uint8_t i = 0; i < numAP; i++){

//             retIndicesList.push_back(i);
//         }

//         return retIndicesList;        
//     }

//     void cleanScanData()
//     {
//         WiFi.scanDelete();
//     }



//     bool isBssidCorrespondToSsid(const char * ssid, uint8_t bssid[BSSID_DIGITS])
//     {
//         apIndices_t apList;
//         bool retVal = false;

//         apList = rescanApIndices();

//         filterByBSSID(bssid, apList);

//         for(auto apIndex : apList)
//         {
//             if(0 == strcmp(WiFi.SSID(apIndex).c_str(), ssid))
//             {
//                 retVal = true;
//             }
//         }

//         return retVal;
//     }

//     bool isBSSIDOnline(uint8_t bssid[])
//     {
//         apIndices_t apList;

//         apList = rescanApIndices();

//         filterByBSSID(bssid, apList);

//         return (false == apList.empty());
//     }

//     apIndices_t seekStrongestRSSIBySSID(const char * ssid)
//     {
//         apIndices_t apList;

//         apList = rescanApIndices();

//         filterBySSID(ssid, apList);
//         sortByRSSIDecrease(apList);

//         removeAllExceptFirst(apList);

//         return apList;
//     }





// };
