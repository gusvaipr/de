//
// Created by samueli on 2017-10-10.
//

#ifndef INS_SERVER_INS_INCLUDE_DATA_STORE_HPP
#define INS_SERVER_INS_INCLUDE_DATA_STORE_HPP

#include <iostream>
#include <spdlog/spdlog.h>
#include <sqlite3.h>
#include <vector>

#include "types.hpp"

namespace ins_service
{

#ifdef ENABLE_TESTS
class DataStoreFixture;
#endif // ENABLE_TESTS

class DataStore
{
public:
#ifdef ENABLE_TESTS
    friend class DataStoreFixture;
#endif // ENABLE_TESTS

    explicit DataStore()
        : database_(nullptr)
        , console_(spdlog::get(LOGGER_NAME))
    {
        if (console_ == nullptr)
            console_ = spdlog::stdout_logger_mt(LOGGER_NAME);
    }

    void Init(const std::string& db_filename);

    void Close();

    bool UpdateDeviceLocation(const std::string& device_id, Position pos);

    bool
    InsertRSSIReadings(const std::string& device_id, std::vector<std::string> mac_addr_list, std::vector<double> rssi_list);

    bool GetPosition(const std::string& device_id, QueryT queryby, Position& pos);

    bool CreateDeviceTable(const std::string& device_id);

    bool ClearDeviceTable(const std::string& device_id);

private:
    bool CreateLocationTable();

    bool RunQuery(const std::string& sql);

    static int DbCallback(void* not_used, int argc, char** argv, char** azColName); // Not used

    sqlite3*                        database_;
    std::mutex                      database_lock_;
    std::shared_ptr<spdlog::logger> console_;
};

} // namespace ins_service

#endif // INS_SERVER_INS_INCLUDE_DATA_STORE_HPP
