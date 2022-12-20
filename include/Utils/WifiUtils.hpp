#pragma once

#include <vector>
#include "esp_wifi.h"
#include <string>

using namespace std;

#define BSSID_DIGITS 6


using index_t = uint8_t;
using apInfoList = std::vector<wifi_ap_record_t>;

class WifiUtils {

public:
    static void removeSSIDDuplicates(apInfoList &apList);

    static void removeBSSIDDuplicates(apInfoList &apList);

    static void sortByRSSIDecrease(apInfoList &apList);

    static void filterBySSID(const char *SSID, apInfoList &apList);

    static void filterByBSSID(const char *SSID, apInfoList &apList);

    static void bssidFromStrToArray(const char *bssidStr, uint8_t bssidNum[BSSID_DIGITS]);

    static string bssidArrayToStr(const uint8_t bssidNum[BSSID_DIGITS]);
    // bool isBSSIDOnline(uint8_t bssid[]);
    // WifiUtils::apIndices_t seekStrongestRSSIBySSID(const char * ssid);
    // bool isBSSIDsEqual(const uint8_t first[BSSID_DIGITS],const uint8_t second[BSSID_DIGITS]);
    // bool isBSSIDempty(const uint8_t bssid[BSSID_DIGITS]);
    // apIndices_t rescanApIndices();
    // void cleanScanData();
    // bool isBssidCorrespondToSsid(const char * ssid, uint8_t bssid[BSSID_DIGITS]);

};
