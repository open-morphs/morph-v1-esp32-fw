// #include "SettingsSaver.hpp"
// #include "NVS.hpp"
// #include "Utils/ThreadSafeDbg.hpp"

// static const char* const TAG = "SETTINGS_SAVER";


// void SettingsSaver::run(void* pData) {

//     while(1) {

//         static message_t message = {};
//         bool messageReceived = consumeMessage(message, _periodMs);

//         if (true == messageReceived) {
//             switch (message.messageCode) { 
//                 case eMessageCode::MESSAGE_SAFE_REBOOT:
//                 case eMessageCode::MESSAGE_POWER_LOSS: 
//                     saveAll();
//                 break;

//                 default:
//                     assert(false); // shouldn't be here
//                 break;
//             }
//         }

//         for (auto &profile : _storage) {
//             if (true == profile.getSettingPointer()->wasChanged()) {
//                 switch (profile.getOnChangedPolicy()) {
//                     case eOnChangedPolicy::SAVE_ALLWAYS:
//                         save(profile);
//                     break;
//                     case eOnChangedPolicy::SAVE_WHEN_NO_POWER:
//                         if (false == CONTROLLER.hasPower()) {
//                             save(profile);
//                         }
//                         else {
//                         }
//                     break;
                    
//                     default:
//                         assert(false); //shouldn't be here
//                     break;
//                 }                
//             }
//         }
//     }
// }

// void SettingsSaver::addProfile(const SettingsSaveProfile& profile) {
//     _storage.push_back(profile);
// }

// void SettingsSaver::erase(const SettingsSaveProfile& profile) {

//     NVS nvs;
//     nvs.erase(profile.getNamespace().c_str(), profile.getSettingPointer()->getKeyName());
// }

// void SettingsSaver::save(const SettingsSaveProfile& profile) {

//     NVS nvs;
//     MultiType* setting = profile.getSettingPointer();

//     switch (profile.getSettingPointer()->getType()) {
//         case Internals::JSON_INTEGER:
//             nvs.write(profile.getNamespace().c_str(), setting->getKeyName(), setting->as<int>());
//         break;

//         case Internals::JSON_POSITIVE_INTEGER:
//             nvs.write(profile.getNamespace().c_str(), setting->getKeyName(), setting->as<uint32_t>());
//         break;

//         case Internals::JSON_BOOLEAN:
//             nvs.write(profile.getNamespace().c_str(), setting->getKeyName(), setting->as<bool>()? 1:0);
//         break;

//         case Internals::JSON_STRING:
//             nvs.write(profile.getNamespace().c_str(), setting->getKeyName(), setting->as<const char*>());
//         break;
        
//         default:
//             assert(false); //shouldn't be here
//         break;
//     }

//     DBG_PRINT_TAG(TAG, "setting %s saved!", profile.getSettingPointer()->getKeyName());
//     setting->clearChangedFlag();
// }

// SettingsSaveProfile SettingsSaver::findProfileBySettingPointer(MultiType* value)const {

//     SettingsSaveProfile ret = invalidProfile();

//     for (auto elem : _storage){
//         if(value == elem.getSettingPointer()){
//             ret = elem;
//             break;
//         }
//     }
//     return ret;
// }

// SettingsSaveProfile SettingsSaver::invalidProfile()const {
//     static SettingsSaveProfile inval(nullptr, SAVE_ALLWAYS, "invalidValue");
//     return inval;
// }

// bool SettingsSaver::load(const SettingsSaveProfile& profile) {
//     bool ret = false;

//     NVS nvs;
//     MultiType* setting = profile.getSettingPointer();

//     switch (profile.getSettingPointer()->getType()) {
//         case Internals::JSON_INTEGER:{
//             int64_t readRes = 0;
//             if (true == nvs.read(profile.getNamespace().c_str(), setting->getKeyName(), readRes)) {
//                 setting->set((int)readRes);
//                 ret = true;
//             }
//         }
//         break;

//         case Internals::JSON_POSITIVE_INTEGER:{
//             int64_t readRes = 0;
//             if (true == nvs.read(profile.getNamespace().c_str(), setting->getKeyName(), readRes)){
//                 setting->set((uint32_t)readRes);
//                 ret = true;
//             }
//         }
//         break;

//         case Internals::JSON_BOOLEAN:{
//             int64_t readRes = 0;
//             if (true == nvs.read(profile.getNamespace().c_str(), setting->getKeyName(), readRes)){
//                 setting->set(readRes? 1:0);
//                 ret = true;
//             }
//         }
//         break;

//         case Internals::JSON_STRING:{
//             string readRes = "";
//             if (true == nvs.read(profile.getNamespace().c_str(), setting->getKeyName(), readRes)){
//                 setting->set(readRes.c_str());
//                 ret = true;
//             }
//         }
//         break;
        
//         default:
//             assert(false); //shouldn't be here
//         break;
//     }
//     setting->clearChangedFlag();
//     return ret;
// }

// bool SettingsSaver::loadAll() {

//     bool ret = true;
//     for (auto &profile : _storage) {
//         if (false == load(profile)){
//             ret = false;
//         }
//     }
//     return ret;
// }

// void SettingsSaver::saveAll() {
//     for (auto &profile : _storage) {
//         save(profile);
//     }
// }