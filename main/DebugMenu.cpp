#include <DebugMenu.hpp>
#include <BLE.hpp>
#include <MorphDevice.hpp>
#include <Uart.hpp>

void consoleTask(void *args) {

    Uart console(UART_NUM_0, eBaudRate::BAUD_115000, uart_word_length_t::UART_DATA_8_BITS, uart_parity_t::UART_PARITY_DISABLE,
        uart_stop_bits_t::UART_STOP_BITS_1, uart_hw_flowcontrol_t::UART_HW_FLOWCTRL_DISABLE);

    while(1) {

        printf(R"(
------ DEBUG MENU ------
0. Change MAC-address
1. Enable BLE task (init->scan->connect->get notifications)
2. Disable BLE task (disconnect->deinit)
3. Get charge level (L, R)
4. Get current handset connected
5. Get mic state
6. Get volume
7. Get handstate connection state
8. Toggle mic state
9. Increase volume
a. Decrease volume
b. Reset bonded mac
c. Connect to prev paired device
d. Disconnect all handsets
e. Get ANC state
f. Get ANC current mode
g. Toggle ANC state (on/off)
h. Set ANC mode
i. Send raw bytes
j. Start pairing
k. Register for AUDIO CURATION FEATURE notifications
x. Reboot

Enter menu number from above...
        )");
        const char menuInput {console.readChar(portMAX_DELAY)};
        DBG_PRINTF("menuInput: %d\r\n", menuInput);

        switch (menuInput) {

            case MENU_CHANGE_MAC: {
                BLE_DEVICE.changeCurrentUsedMac();    
                printf("MENU: BLE MAC have changed\n");
            };
            break;

            case MENU_ENABLE_BLE:{

                if (!BLE_DEVICE.isRunning()) {
                    BLE_DEVICE.setMacSwapEnabled(true);
                    BLE_DEVICE.start();
                }
                else {
                    printf("MENU: BLE already running!\n");
                }
            break;
            }

            case MENU_DISABLE_BLE:
                if (BLE_DEVICE.isRunning()) {
                    BLE_DEVICE.stop();
                }
                else {
                    printf("MENU: BLE already disabled!\n");
                }
            break;

            case MENU_GET_BATTERY:
                printf("MENU: %s\n", BLE_DEVICE.requestUpdateChargeLevel() ?
                    "command MORPH_COMMAND_ID_GET_BATTERY_LEVEL sent" : "command MORPH_COMMAND_ID_GET_BATTERY_LEVEL failed");
            break;

            case MENU_GET_HANDSET_CONNECTED:
                printf("MENU: %s\n", BLE_DEVICE.requestUpdateHandsetInfo() ?
                    "command MORPH_COMMAND_ID_GET_HANDSET_CONNECTED sent" : "command MORPH_COMMAND_ID_GET_HANDSET_CONNECTED failed");
            break;

            case MENU_GET_MIC_STATE:
                printf("MENU: %s\n", BLE_DEVICE.requestUpdateMicState() ?
                    "command MORPH_COMMAND_ID_GET_MIC_MUTE_STATE sent" : "command MORPH_COMMAND_ID_GET_MIC_MUTE_STATE failed");
            break;

            case MENU_GET_VOLUME:
                printf("MENU: %s\n", BLE_DEVICE.requestUpdateVolume() ?
                    "command MORPH_COMMAND_ID_GET_VOLUME sent" : "command MORPH_COMMAND_ID_GET_VOLUME failed");
            break;

            case MENU_GET_CONNECTED:
                printf("MENU: %s\n", BLE_DEVICE.requestUpdateHandsetState() ?
                    "command MORPH_COMMAND_ID_IS_HANDSET_CONNECTED sent" : "command MORPH_COMMAND_ID_IS_HANDSET_CONNECTED failed");
            break;

            case MENU_TOGGLE_MIC:
                printf("MENU: %s\n", BLE_DEVICE.requestMicToggle() ?
                    "command MORPH_COMMAND_ID_TOGGLE_MIC_MUTE_STATE sent" : "command MORPH_COMMAND_ID_TOGGLE_MIC_MUTE_STATE failed");
            break;

            case MENU_VOLUME_INC:{
                static const uint8_t maxVolume {127};
                const uint8_t incStep{8};
                const uint8_t newVolume{static_cast<uint8_t>(MORPH_DEVICE.get_volume() < maxVolume - incStep ? (MORPH_DEVICE.get_volume() + incStep) : maxVolume)};
                printf("MENU: %s\n", BLE_DEVICE.requestVolumeChange(newVolume) ?
                    "command MORPH_COMMAND_ID_CHANGE_VOLUME sent" : "command MORPH_COMMAND_ID_CHANGE_VOLUME failed");
            }
            break;

            case MENU_VOLUME_DEC:{
                const uint8_t decStep{8};
                const uint8_t newVolume{static_cast<uint8_t>(MORPH_DEVICE.get_volume() > decStep ? (MORPH_DEVICE.get_volume() - decStep) : 0)};
                printf("MENU: %s\n", BLE_DEVICE.requestVolumeChange(newVolume) ?
                    "command MORPH_COMMAND_ID_CHANGE_VOLUME sent" : "command MORPH_COMMAND_ID_CHANGE_VOLUME failed");
            }
            break;

            case MENU_RESET_MAC: {
                printf("MENU: Reseting mac..\n");
                BLE_DEVICE.deleteBondingInformation();
            }
            break;

            case MENU_CONNECT_PREV:{
                const uint64_t mac {MORPH_DEVICE.get_handsetMac()};
                const uint32_t btProfileMask {MORPH_DEVICE.get_btProfileMask()};

                printf("MENU: %s\n", BLE_DEVICE.requestConnectToHandset(mac, btProfileMask) ?
                    "command MORPH_COMMAND_ID_CONNECT_PREV_HANDSET sent" : "command MORPH_COMMAND_ID_CONNECT_PREV_HANDSET failed");
            }
            break;

            case MENU_DISCONNECT_ALL:{
                printf("MENU: %s\n", BLE_DEVICE.sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_CONNECT_DISCONNECT_ALL), static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID)) ?
                    "command MORPH_COMMAND_ID_CONNECT_DISCONNECT_ALL sent" : "command MORPH_COMMAND_ID_CONNECT_DISCONNECT_ALL failed");
            }
            break;

            case MENU_DISCONNECT_LAST_CONNECTED:{
                printf("MENU: %s\n", BLE_DEVICE.sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_DISCONNECT_PREV), static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID)) ?
                    "command MORPH_COMMAND_ID_DISCONNECT_PREV sent" : "command MORPH_COMMAND_ID_DISCONNECT_PREV failed");
            }
            break;

            case MENU_GET_ANC_STATE:{
                printf("MENU: %s\n", BLE_DEVICE.requestUpdateAncState() ?
                    "command QCC_V3_COMMAND_GET_STATE sent" : "command QCC_V3_COMMAND_GET_STATE failed");
            }
            break;

            case MENU_GET_CURRENT_MODE:{
                printf("MENU: %s\n", BLE_DEVICE.requestUpdateAncMode() ?
                    "command QCC_V3_COMMAND_GET_CURR_MODE sent" : "command QCC_V3_COMMAND_GET_CURR_MODE failed");
            }
            break;

            case MENU_TOGGLE_ANC:{
                printf("MENU: %s\n", BLE_DEVICE.requestAncStateChange(!MORPH_DEVICE.get_anc()) ?
                    "command QCC_V3_COMMAND_SET_STATE sent" : "command QCC_V3_COMMAND_SET_STATE failed");
            }
            break;

            case MENU_SET_ANC_MODE:{
                printf(R"(Enter an anc mode: (number)
1. adaptive ANC
2. static ANC
5. transparency
)");
                const char menuInput {(char)(console.readChar(portMAX_DELAY) - '0')};
                printf("MENU: %s\n", BLE_DEVICE.sendCommand(eVendorType::VENDOR_V3QC, static_cast<uint8_t>(eQccV3Commands_AudioCuration::QCC_V3_COMMAND_SET_MODE), static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_AUDIOCURATION), reinterpret_cast<const uint8_t*>(&menuInput), 1) ?
                    "command QCC_V3_COMMAND_SET_MODE sent" : "command QCC_V3_COMMAND_SET_MODE failed");
            }
            break;

            case MENU_SEND_RAW_BYTES:{
                printf("Enter bytes:\n");
                std::string chars;
                char readByte{};

                do  {
                    readByte = console.readChar(portMAX_DELAY);
                    chars.push_back(readByte);
                }
                while (readByte != '\n');

                chars.push_back('\0');

                if (!(chars.size() % 2)) {
                    printf("Expected odd bytes number!\n");
                    break;
                }
                
                std::vector<uint8_t> bytes;

                for (unsigned int i = 0; i < chars.length(); i += 2) {
                    std::string byteString = chars.substr(i, 2);
                    char byte = (char) strtol(byteString.c_str(), NULL, 16);
                    bytes.push_back(byte);
                }
                
                printf("MENU: %s\n", BLE_DEVICE.sendRawData( bytes.data(), bytes.size()) ? "raw bytes sent!" : 
                    "failed to send bytes..");
            }
            break;

            case MENU_START_PAIRING:{
                printf("MENU: %s\n", BLE_DEVICE.sendCommand(eVendorType::VENDOR_MORPH, static_cast<uint8_t>(eMorphCommands::MORPH_COMMAND_ID_PAIR_HANDSET), static_cast<uint8_t>(eMorphFeatures::MORPH_FEATURE_ID)) ?
                    "command MORPH_COMMAND_ID_PAIR_HANDSET sent" : "command MORPH_COMMAND_ID_PAIR_HANDSET failed");
            }
            break;

            case MENU_START_AUDIO_NOTIFICATIONS:{
                printf("MENU: %s\n", BLE_DEVICE.requestNotificationsRegister(static_cast<uint8_t>(eQccV3Features::QCC_V3_FEATURE_AUDIOCURATION)) ?
                    "audio curation notifications request sent" : "failed to send audio curation notifications request");
            }
            break;

            case MENU_REBOOT:
                printf("MENU: %s\n", "rebooting now..");
                esp_restart();
            break;
        
        default:
            break;
        }
    }
}
