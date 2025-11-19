1. **必须先连接才能操作**
   ```cpp
   MySQLHandler db("localhost", "user", "pass", "mydb");
   if (!db.connect()) {
       std::cerr << "连接失败" << std::endl;
       return -1;
   }
   ```

2. **查询时需用 vector 接收结果**
   ```cpp
   std::vector<DeviceStatusRecord> records;
   if (db.queryAllDeviceStatus(records)) {
       for (const auto& r : records) {
           std::cout << r.device_name << std::endl;
       }
   }
   ```

3. **必须检查查询/插入的返回值**
   ```cpp
   if (!db.insertDeviceStatus("cam", "cam01", "{}", "online", "")) {
       std::cerr << "插入失败: " << db.getLastError() << std::endl;
   }
   ```

4. **查询字段顺序必须与 SELECT 一致**
   ```cpp
   // SELECT 顺序：
   // record_time, type, device_name, data, status, anomaly_video_path
   record.record_time = row[0];
   record.type        = row[1];
   record.device_name = row[2];
   // ... 以此类推
   ```

5. **数据库 NULL 值会转为空字符串**
   ```cpp
   // 数据库中为 NULL，代码中是 ""
   if (record.anomaly_video_path.empty()) {
       // 可能是 NULL 或原本就是空
   }
   ```

6.  **编译时需链接 MySQL 客户端库**
    ```bash
    g++ main.cpp MySQLHandler.cpp -lmysqlclient
    ```