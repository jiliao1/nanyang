#ifndef MYSQLHANDLER_H
#define MYSQLHANDLER_H

#include <mysql/mysql.h>  // MySQL C API
#include <string>
#include <vector>

/**
 * @struct DeviceStatusRecord
 * @brief 用于表示数据库表 `device_status_log` 中的一行数据。
 *
 * 该结构体作为数据传输对象 (DTO)，在应用程序逻辑与数据库访问层之间传递设备状态信息。
 * 所有字段均为字符串类型，与数据库表结构保持一致。
 *
 * @note
 *   - 字段名与数据库列名严格对应。
 *   - 默认构造函数将所有字段初始化为空字符串。
 *   - 无复杂资源管理，可安全拷贝。
 */
struct DeviceStatusRecord {
    std::string record_time;           ///< [in/out] 记录时间戳 (格式: YYYY-MM-DD HH:MM:SS)
    std::string type;                  ///< [in/out] 设备类型标识 (e.g., "camera", "sensor")
    std::string device_name;           ///< [in/out] 设备唯一名称 (e.g., "camera_01")
    std::string data;                  ///< [in/out] 设备上报的原始数据或解析后数据 (JSON 字符串等)
    std::string status;                ///< [in/out] 设备当前状态 (e.g., "online", "offline", "anomaly")
    std::string anomaly_video_path;    ///< [in/out] 异常事件关联的视频存储路径 (若无异常则为空)

    /**
     * @brief 默认构造函数
     * 初始化所有字段为空字符串。
     */
    DeviceStatusRecord() = default;
};

/**
 * @class MySQLHandler
 * @brief 封装 MySQL 数据库连接与操作的工具类
 *
 * 该类提供对 `device_status_log` 表的增查操作，封装了底层 MySQL C API 的复杂性。
 * 采用 RAII (Resource Acquisition Is Initialization) 设计模式，确保数据库连接
 * 在对象生命周期结束时自动释放，避免资源泄漏。
 *
 * @requirements
 *   - 必须链接 libmysqlclient (MySQL 客户端库)
 *   - 构造函数参数必须有效，否则 connect() 将失败
 *
 * @thread_safety
 *   - **非线程安全**。同一实例不可被多个线程并发调用。
 *   - 如需多线程使用，应为每个线程创建独立实例，或使用外部同步机制。
 *
 * @resource_management
 *   - 自动管理 MYSQL* 句柄的生命周期。
 *   - 禁止拷贝，防止悬挂指针；支持移动语义以提高效率。
 *
 * @example
 *   MySQLHandler db("localhost", "user", "pass", "test_db");
 *   if (!db.connect()) {
 *       std::cerr << "Connect failed: " << db.getLastError() << std::endl;
 *       return -1;
 *   }
 *   std::vector<DeviceStatusRecord> records;
 *   if (db.queryAllDeviceStatus(records)) {
 *       for (const auto& r : records) {
 *           std::cout << r.device_name << ": " << r.status << std::endl;
 *       }
 *   }
 */
class MySQLHandler {
public:
    /**
     * @brief 构造数据库处理器对象
     *
     * 初始化连接参数。此操作不建立实际连接，仅存储配置信息。
     *
     * @param[in] host 数据库服务器主机名或 IP 地址
     * @param[in] user 登录用户名
     * @param[in] password 用户密码
     * @param[in] database 要操作的数据库名称
     * @param[in] port 数据库服务端口 (默认 3306)
     * @param[in] unix_socket 本地 Unix 套接字路径 (用于本地连接优化，留空则使用 TCP)
     * @param[in] client_flag MySQL 客户端标志位 (如 CLIENT_SSL, 通常为 0)
     *
     * @post
     *   - 成员变量被初始化为传入参数值。
     *   - mysql_ 指针被设置为 nullptr，表示未连接状态。
     */
    MySQLHandler(const std::string& host,
                 const std::string& user,
                 const std::string& password,
                 const std::string& database,
                 unsigned int port = 3306,
                 const std::string& unix_socket = "",
                 unsigned long client_flag = 0);

    /**
     * @brief 析构函数
     *
     * 自动调用 disconnect() 关闭数据库连接并释放所有相关资源。
     * 遵循 RAII 原则，确保无资源泄漏。
     *
     * @post mysql_ == nullptr
     */
    ~MySQLHandler();

    // 禁止拷贝：防止因浅拷贝导致的双重释放
    MySQLHandler(const MySQLHandler&) = delete;
    MySQLHandler& operator=(const MySQLHandler&) = delete;

    // 允许移动：支持高效转移资源所有权
    MySQLHandler(MySQLHandler&&) = default;
    MySQLHandler& operator=(MySQLHandler&&) = default;

    /**
     * @brief 建立到 MySQL 服务器的物理连接
     *
     * 使用构造函数中配置的参数尝试建立数据库连接。
     *
     * @return bool
     *   - `true`: 连接成功建立，对象进入“已连接”状态。
     *   - `false`: 连接失败。可通过 getLastError() 获取详细错误信息。
     *
     * @post
     *   - 成功: mysql_ 指向有效的 MYSQL 连接句柄。
     *   - 失败: mysql_ 为 nullptr，连接未建立。
     *
     * @error_handling
     *   - 错误信息通过标准错误流 (std::cerr) 输出。
     *   - 可通过 getLastError() 获取底层 MySQL 错误码和消息。
     *
     * @note
     *   - 若已存在连接，此函数会先关闭旧连接再尝试新连接。
     *   - 失败时会自动清理部分资源，确保对象处于一致状态。
     */
    bool connect();

    /**
     * @brief 断开当前数据库连接
     *
     * 如果存在活动连接，则关闭它并释放 MYSQL* 句柄。
     * 多次调用此函数是安全的（幂等性）。
     *
     * @return void
     *
     * @post mysql_ == nullptr
     */
    void disconnect();

    /**
     * @brief 查询 `device_status_log` 表中的所有设备状态记录
     *
     * 执行全表扫描查询，获取所有设备的最新状态日志。
     *
     * @param[out] records 引用参数，用于接收查询结果。
     *                     - 输入: 内容将被清空。
     *                     - 输出: 填充查询到的 DeviceStatusRecord 对象列表。
     *                     - 若查询失败，结果为空。
     *
     * @return bool
     *   - `true`: 查询成功，records 包含有效数据。
     *   - `false`: 查询失败（数据库未连接、SQL 执行错误等）。
     *
     * @error_handling
     *   - 错误信息通过 std::cerr 输出。
     *   - 可通过 getLastError() 获取 MySQL 错误详情。
     *
     * @note
     *   - 该函数执行 SELECT 语句，并自动管理结果集 (MYSQL_RES) 的生命周期。
     *   - 调用者无需手动释放 records 中的数据。
     */
    bool queryAllDeviceStatus(std::vector<DeviceStatusRecord>& records);

    /**
     * @brief 向 `device_status_log` 表插入一条新的设备状态记录
     *
     * 安全地插入设备状态数据。函数内部会对所有字符串参数进行 SQL 转义，
     * 有效防止 SQL 注入攻击。
     *
     * @param[in] type 设备类型
     * @param[in] device_name 设备名称
     * @param[in] data 设备上报数据
     * @param[in] status 设备当前状态
     * @param[in] anomaly_video_path 异常视频存储路径 (可为空)
     *
     * @return bool
     *   - `true`: 插入成功，数据库已新增一条记录。
     *   - `false`: 插入失败（连接问题、SQL 语法错误、约束冲突等）。
     *
     * @error_handling
     *   - 错误信息通过 std::cerr 输出。
     *   - 可通过 getLastError() 获取 MySQL 错误详情。
     *
     * @note
     *   - `record_time` 字段由数据库自动生成（通常为 CURRENT_TIMESTAMP）。
     *   - 所有输入字符串均经过 mysql_real_escape_string 处理，确保安全。
     */
    bool insertDeviceStatus(std::string type,
                            std::string device_name,
                            std::string data,
                            std::string status,
                            std::string anomaly_video_path);

    /**
     * @brief 获取最后一次数据库操作的错误信息
     *
     * 当 connect()、queryAllDeviceStatus() 或 insertDeviceStatus() 返回 false 时，
     * 此函数可用于获取底层 MySQL 返回的错误描述。
     *
     * @return std::string
     *   - 如果存在有效连接：返回 mysql_error(mysql_) 的内容。
     *   - 如果未连接：返回 "No connection"。
     *
     * @note
     *   - 返回的是临时字符串对象，可立即使用。
     *   - 错误信息为 UTF-8 编码。
     */
    std::string getLastError() const;

private:
    // === 成员变量 ===
    std::string host_;          ///< [config] 数据库主机地址
    std::string user_;          ///< [config] 登录用户名
    std::string password_;      ///< [config] 登录密码
    std::string database_;      ///< [config] 目标数据库名
    unsigned int port_;         ///< [config] 服务端口
    std::string unix_socket_;   ///< [config] Unix 套接字路径 (可选)
    unsigned long client_flag_; ///< [config] 客户端连接标志

    MYSQL* mysql_;              ///< [state] MySQL 连接句柄，nullptr 表示未连接
};

#endif // MYSQLHANDLER_H