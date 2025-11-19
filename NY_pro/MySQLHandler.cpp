#include "MySQLHandler.h"
#include <iostream>

// 构造函数：初始化连接参数，不建立实际连接
MySQLHandler::MySQLHandler(const std::string& host,
                           const std::string& user,
                           const std::string& password,
                           const std::string& database,
                           unsigned int port,
                           const std::string& unix_socket,
                           unsigned long client_flag)
    : host_(host), user_(user), password_(password), database_(database),
      port_(port), unix_socket_(unix_socket), client_flag_(client_flag),
      mysql_(nullptr) {
}

// 析构函数：自动关闭连接（RAII）
MySQLHandler::~MySQLHandler() {
    disconnect();
}

// 建立数据库连接
bool MySQLHandler::connect() {
    mysql_ = mysql_init(nullptr);
    if (!mysql_) {
        std::cerr << "mysql_init() 失败: " << mysql_error(nullptr) << std::endl;
        return false;
    }

    std::cout << "正在连接到数据库: " << host_ << ":" << port_ << " 数据库: " << database_ << std::endl;

    if (!mysql_real_connect(mysql_, host_.c_str(), user_.c_str(), password_.c_str(),
                            database_.c_str(), port_, unix_socket_.empty() ? nullptr : unix_socket_.c_str(),
                            client_flag_)) {
        std::cerr << "连接失败: " << mysql_error(mysql_) << std::endl;
        mysql_close(mysql_);
        mysql_ = nullptr;
        return false;
    }

    std::cout << "数据库连接成功！" << std::endl;
    return true;
}

// 断开连接，安全释放资源
void MySQLHandler::disconnect() {
    if (mysql_) {
        mysql_close(mysql_);
        mysql_ = nullptr;
        std::cout << "数据库连接已关闭。" << std::endl;
    }
}

// 查询所有设备状态日志
bool MySQLHandler::queryAllDeviceStatus(std::vector<DeviceStatusRecord>& records) {
    records.clear();

    if (!mysql_) {
        std::cerr << "错误: 数据库未连接。" << std::endl;
        return false;
    }

    // 修复了字段名：record_time（原为 recode_time）
    const char* query = "SELECT `record_time`, `type`, `device_name`, `data`, `status`, `anomaly_video_path` FROM device_status_log";

    if (mysql_query(mysql_, query)) {
        std::cerr << "查询执行失败: " << mysql_error(mysql_) << std::endl;
        return false;
    }

    MYSQL_RES* result = mysql_store_result(mysql_);
    if (!result) {
        std::cerr << "获取结果集失败: " << mysql_error(mysql_) << std::endl;
        return false;
    }

    my_ulonglong num_rows = mysql_num_rows(result);
    std::cout << "查询完成，共获取 " << num_rows << " 条记录。" << std::endl;

    MYSQL_ROW row;
    while ((row = mysql_fetch_row(result))) {
        DeviceStatusRecord record;
        // 按 SELECT 顺序填充字段
        record.record_time = row[0] ? row[0] : "";
        record.type = row[1] ? row[1] : "";
        record.device_name = row[2] ? row[2] : "";
        record.data = row[3] ? row[3] : "";
        record.status = row[4] ? row[4] : "";
        record.anomaly_video_path = row[5] ? row[5] : "";

        records.push_back(record);
    }

    mysql_free_result(result);
    return true;
}

// 插入设备状态记录（已转义防 SQL 注入）
bool MySQLHandler::insertDeviceStatus(std::string type, std::string device_name,
                                      std::string data, std::string status,
                                      std::string anomaly_video_path) {
    if (!mysql_) {
        std::cerr << "错误: 数据库未连接。" << std::endl;
        return false;
    }

    // 转义字符串，防止 SQL 注入
    std::string escaped_type(type.size() * 2 + 1, '\0');
    std::string escaped_device_name(device_name.size() * 2 + 1, '\0');
    std::string escaped_data(data.size() * 2 + 1, '\0');
    std::string escaped_status(status.size() * 2 + 1, '\0');
    std::string escaped_path(anomaly_video_path.size() * 2 + 1, '\0');

    size_t len_type = mysql_real_escape_string(mysql_, &escaped_type[0], type.c_str(), type.length());
    size_t len_device = mysql_real_escape_string(mysql_, &escaped_device_name[0], device_name.c_str(), device_name.length());
    size_t len_data = mysql_real_escape_string(mysql_, &escaped_data[0], data.c_str(), data.length());
    size_t len_status = mysql_real_escape_string(mysql_, &escaped_status[0], status.c_str(), status.length());
    size_t len_path = mysql_real_escape_string(mysql_, &escaped_path[0], anomaly_video_path.c_str(), anomaly_video_path.length());

    escaped_type.resize(len_type);
    escaped_device_name.resize(len_device);
    escaped_data.resize(len_data);
    escaped_status.resize(len_status);
    escaped_path.resize(len_path);

    // 构建并执行 SQL
    char query[2048];
    int ret = snprintf(query, sizeof(query),
             "INSERT INTO `device_status_log`(`type`,`device_name`,`data`,`status`,`anomaly_video_path`) "
             "VALUES ('%s','%s','%s','%s','%s')",
             escaped_type.c_str(), 
             escaped_device_name.c_str(), 
             escaped_data.c_str(),
             escaped_status.c_str(), 
             escaped_path.c_str());

    if (ret < 0 || ret >= static_cast<int>(sizeof(query))) {
        std::cerr << "错误: SQL 语句构建失败（缓冲区不足）。" << std::endl;
        return false;
    }

    if (mysql_query(mysql_, query)) {
        std::cerr << "插入执行失败: " << mysql_error(mysql_) << std::endl;
        return false;
    }

    std::cout << std::endl << "新记录插入成功: " << device_name << " - " << status << std::endl;
    return true;
}

// 获取最后一次错误信息
std::string MySQLHandler::getLastError() const {
    if (mysql_) {
        return std::string(mysql_error(mysql_));
    }
    return "No connection";
}