// #pragma once

// #include <vector>
// #include "SettingsSaveProfile.hpp"
// #include "ITask.hpp"
// #include "MessageConsumer.hpp"

// class SettingsSaver : public ITask, public MessageConsumer{
    
//     public:
//         void addProfile(const SettingsSaveProfile& profile);
//         bool loadAll();
//         void saveAll();
//         bool isReady();
//     private:
//         void run (void* pData) override;
//         void erase(const SettingsSaveProfile& profile);
//         void save(const SettingsSaveProfile& profile);
//         bool load(const SettingsSaveProfile& profile);
//         SettingsSaveProfile findProfileBySettingPointer(MultiType* value)const;
//         SettingsSaveProfile invalidProfile()const;
//     private:
//         std::vector<SettingsSaveProfile> _storage;
// };