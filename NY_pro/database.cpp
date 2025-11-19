#include "database.h"

dataBase::dataBase()
{
    m_connectionName = "qt_sql_default_connection_robot"; // 定义并存储唯一连接名
    
    // 如果已存在连接,先移除它以确保干净的状态
    if(QSqlDatabase::contains(m_connectionName)) {
        qDebug() << "[dataBase构造] 发现已存在的连接,先移除...";
        QSqlDatabase::removeDatabase(m_connectionName);
    }
    
    // 创建新的数据库连接 (使用 QMYSQL)
    qDebug() << "[dataBase构造] 创建QMYSQL连接...";
    db = QSqlDatabase::addDatabase("QMYSQL", m_connectionName);
    
    if (!db.isValid()) {
        qDebug() << "[dataBase构造] ❌ QMYSQL 驱动无效,尝试 QMYSQL3...";
        // 如果 QMYSQL 无效,尝试 QMYSQL3
        QSqlDatabase::removeDatabase(m_connectionName);
        db = QSqlDatabase::addDatabase("QMYSQL3", m_connectionName);
        
        if (!db.isValid()) {
            qDebug() << "[dataBase构造] ❌ QMYSQL3 也无效!";
            qDebug() << "[dataBase构造] 可用驱动:" << QSqlDatabase::drivers();
            qDebug() << "[dataBase构造] 插件路径:" << QCoreApplication::libraryPaths();
        } else {
            qDebug() << "[dataBase构造] ✓ QMYSQL3 驱动有效";
        }
    } else {
        qDebug() << "[dataBase构造] ✓ QMYSQL 驱动有效";
    }

    // 设置连接参数
    db.setHostName("127.0.0.1");
    db.setPort(3306);
    db.setDatabaseName("robot");
    db.setUserName("root");
    db.setPassword("");
    
    qDebug() << "[dataBase构造] 连接参数已设置:";
    qDebug() << "  - Host: 127.0.0.1:3306";
    qDebug() << "  - Database: robot";
    qDebug() << "  - User: root";
    qDebug() << "  - isValid:" << db.isValid();
}

dataBase::~dataBase()
{
    qDebug() << "[dataBase析构] 开始清理数据库连接...";
    if(db.isOpen()) {
        qDebug() << "[dataBase析构] 关闭数据库连接";
        db.close();
    }
    
    // 注意: 不在这里移除连接,因为可能有其他地方还在使用
    // 让Qt在程序退出时自动清理
    // QSqlDatabase::removeDatabase(m_connectionName);
    qDebug() << "[dataBase析构] 完成";
}

bool dataBase::openDatabase()
{
    // 【修复】确保连接始终存在
    if (!QSqlDatabase::contains(m_connectionName)) {
        qDebug() << "【警告】数据库连接丢失，正在重新创建...";
        db = QSqlDatabase::addDatabase("QMYSQL", m_connectionName);
        db.setHostName("127.0.0.1");
        db.setPort(3306);
        db.setDatabaseName("robot");
        db.setUserName("root");
        db.setPassword("");
    }
    
    // 获取数据库连接对象
    QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
    
    // 检查是否有效
    if (!currentDb.isValid()) {
        qDebug() << "【严重错误】数据库连接无效";
        return false;
    }

       // 1. 先检查是否已经打开
       if (currentDb.isOpen()) {
           // qDebug() << "数据库连接已打开 (dataBase::openDatabase)";
           return true;
       }

       // 2. 如果未打开，尝试打开
       qDebug() << "尝试打开数据库连接...";
       bool openSuccess = currentDb.open();

       // 3. 检查 open() 的返回值
       if (openSuccess) {
           // 4. 再次确认 isOpen() 状态
           if (currentDb.isOpen()) {
               qDebug() << "数据库连接成功并且状态确认为 open (dataBase::openDatabase)";
               // 【重要】更新成员变量db的状态，虽然我们后面会按名获取
               db = currentDb;
               return true;
           } else {
               qDebug() << "【警告】db.open() 返回 true，但 db.isOpen() 仍然是 false！";
               qDebug() << "最后错误信息：" << currentDb.lastError().text();
               return false;
           }
       } else {
           qDebug() << "【数据库连接失败】db.open() 返回 false: " << currentDb.lastError().text();
           qDebug() << "当前可用驱动:" << QSqlDatabase::drivers();
           return false;
       }
}

void dataBase::ShowUserDB(QTableView *tv_user, QString name, QString purview, QString situation)
{
    // 1. 连接检查
        if (!openDatabase()) {
            qDebug() << "ShowUserDB 失败：数据库未连接。";
            // 可以考虑清空模型或显示错误信息
            if (tv_user->model()) tv_user->model()->deleteLater();
            tv_user->setModel(nullptr);
            return;
        }

        // 2. 获取连接
        QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
        if (!currentDb.isValid() || !currentDb.isOpen()) {
             qDebug() << "【错误】ShowUserDB: 获取到的数据库连接无效或未打开。";
             if (tv_user->model()) tv_user->model()->deleteLater();
             tv_user->setModel(nullptr);
             return;
        }

        // 3. 创建模型并执行查询 (使用 currentDb)
        QString str = "select username,name,purview,situation,time from user where 1=1 ";
        // (构建 str 的代码不变)
        if (name != "all") {
            str += QString(" and name like '%%1%' ").arg(name);
        }
        if ( purview.compare("全部") != 0 ) {
            str += QString(" and purview like '%%1%' ").arg(purview);
        }
        if (situation.compare("全部") != 0) {
            str += QString(" and situation like '%%1%' ").arg(situation);
        }

        // 确保每次都创建新模型，避免旧数据残留
        QSqlQueryModel* model = new QSqlQueryModel(tv_user); // parent 设为 tv_user
        model->setQuery(str, currentDb); // <--- 使用 currentDb

        // 4. 错误检查
        if (model->lastError().isValid()) {
            qDebug() << "【SQL 错误】ShowUserDB setQuery: " << model->lastError().text();
            model->deleteLater(); // 清理失败的模型
            tv_user->setModel(nullptr);
            return;
        }

        // 5. 设置模型和视图 (与之前基本一致)
        QSortFilterProxyModel* sqlproxy = new QSortFilterProxyModel(tv_user); // parent 设为 tv_user
        sqlproxy->setSourceModel(model);
        model->setHeaderData(0, Qt::Horizontal, "账号");
        model->setHeaderData(1, Qt::Horizontal, "用户名");
        model->setHeaderData(2, Qt::Horizontal, "权限");
        model->setHeaderData(3, Qt::Horizontal, "状态");
        model->setHeaderData(4, Qt::Horizontal, "添加时间");

        // 清理旧模型（如果存在）
        QAbstractItemModel* oldModel = tv_user->model();
        if (oldModel && oldModel != sqlproxy) {
            oldModel->deleteLater();
        }

        tv_user->setModel(sqlproxy);
        tv_user->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tv_user->setSortingEnabled(true);
        tv_user->sortByColumn(0, Qt::AscendingOrder);
        tv_user->show(); // 可选，通常不需要显式调用
}

bool dataBase::exchangeuser(QString username, QString time, QString info)
{
    if (!openDatabase()) { return false; }

        QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
        if (!currentDb.isValid() || !currentDb.isOpen()) { return false; }

        QSqlQuery query(currentDb);
        // ... (设置 situation 的逻辑) ...
        QString situation = (info.compare("正常") == 0) ? "停用" : "正常";

        if (!query.prepare(QStringLiteral("update user set situation=? where username=? and time=?"))) {
            qDebug() << "【SQL 错误】exchangeuser prepare: " << query.lastError().text();
            return false;
        }
        query.addBindValue(situation);
        query.addBindValue(username);
        query.addBindValue(time);

        if (!query.exec()) {
            qDebug() << "【SQL 错误】exchangeuser exec: " << query.lastError().text();
            return false;
        }
        return true;
}

bool dataBase::ExchangeUserPur(QString username, QString time, QString purview)
{
    if (!openDatabase()) { return false; } // 添加连接检查
    QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(currentDb);
    query.prepare(QStringLiteral("update user set purview='%1' where username='%2' and time='%3'").arg(purview).arg(username).arg(time));
    if (!query.exec()) {
        qDebug() << "【SQL 错误】ExchangeUserPur: " << query.lastError().text();
        return false;
    }
    return true;
}

bool dataBase::exchangeUserpwd(QString username, QString time)
{
    if (!openDatabase()) { return false; } // 添加连接检查
    QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(currentDb);
    query.prepare(QStringLiteral("update user set  password='%1' where username='%2' and time='%3'").arg(QString::fromLocal8Bit("1234")).arg(username).arg(time));
    if (!query.exec()) {
        qDebug() << "【SQL 错误】exchangeUserpwd: " << query.lastError().text();
        return false;
    }
    return true;
}

bool dataBase::deleteUser(QString username, QString time)
{
    if (!openDatabase()) { return false; } // 添加连接检查
    QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(currentDb);
    query.prepare(QString("delete from user where username='%1' and time='%2'").arg(username).arg(time));
    if (!query.exec()) {
        qDebug() << "【SQL 错误】deleteUser: " << query.lastError().text();
        return false;
    }
    return true;
}

void dataBase::showReport(QTableView *tv_rp, QString type, QString start_time, QString end_time, QString result)
{
    // 1. 连接检查
        if (!openDatabase()) {
            qDebug() << "showReport 失败：数据库未连接。";
            if (tv_rp->model()) tv_rp->model()->deleteLater();
            tv_rp->setModel(nullptr);
            return;
        }

        // 2. 获取连接
        QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
        if (!currentDb.isValid() || !currentDb.isOpen()) {
             qDebug() << "【错误】showReport: 获取到的数据库连接无效或未打开。";
             if (tv_rp->model()) tv_rp->model()->deleteLater();
             tv_rp->setModel(nullptr);
             return;
        }

        // 3. 创建模型并执行查询 (使用 currentDb)
        QString start = start_time + " 00:00:00"; // 增加秒精度
        QString end = end_time + " 23:59:59";     // 增加秒精度
        QString str = QString("select id,type,name,starttime,endtime,time,result,controler from report where starttime >= '%1' and starttime <= '%2' ").arg(start).arg(end);
        // (构建 str 的代码不变)
        if (type.compare("所有") != 0) {
            str += QString(" and type = '%1' ").arg(type); // 修正：使用 = 而不是 LIKE
        }
        if (result.compare("所有") != 0) {
            str += QString(" and result = '%1' ").arg(result); // 修正：使用 = 而不是 LIKE
        }
        str += " ORDER BY starttime ASC"; // 明确排序

        QSqlQueryModel* model = new QSqlQueryModel(tv_rp); // parent 设为 tv_rp
        model->setQuery(str, currentDb); // <--- 使用 currentDb

        // 4. 错误检查
        if (model->lastError().isValid()) {
                qDebug() << "【SQL 错误】showReport setQuery: " << model->lastError().text();
                qDebug() << "执行的SQL语句是: " << str; // 打印 SQL 语句帮助调试
                model->deleteLater();
                tv_rp->setModel(nullptr);
                return;
            } else { // <--- 添加 else 分支
                // 【新增】打印返回的行数
                qDebug() << "showReport: Query successful. Rows returned:" << model->rowCount();
                if (model->rowCount() == 0) {
                     // 【新增】如果行数为0，打印使用的SQL语句，方便核对
                    qDebug() << "showReport: No records found matching the criteria.";
                    qDebug() << "Executed SQL:" << str;
                }
            }

        // 5. 设置模型和视图
        QSortFilterProxyModel* sqlproxy = new QSortFilterProxyModel(tv_rp); // parent 设为 tv_rp
        sqlproxy->setSourceModel(model);
        model->setHeaderData(0, Qt::Horizontal, "任务ID"); // 改进表头名称
        model->setHeaderData(1, Qt::Horizontal, "任务类型");
        model->setHeaderData(2, Qt::Horizontal, "任务名称");
        model->setHeaderData(3, Qt::Horizontal, "开始时间");
        model->setHeaderData(4, Qt::Horizontal, "结束时间");
        model->setHeaderData(5, Qt::Horizontal, "任务时长");
        model->setHeaderData(6, Qt::Horizontal, "任务结论");
        model->setHeaderData(7, Qt::Horizontal, "操作员");

        QAbstractItemModel* oldModel = tv_rp->model();
        if (oldModel && oldModel != sqlproxy) {
            oldModel->deleteLater();
        }

        tv_rp->setModel(sqlproxy);
        // (tv_rp 设置不变)
        tv_rp->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
        tv_rp->setSortingEnabled(true);
        // tv_rp->sortByColumn(3, Qt::AscendingOrder); // 由 SQL 的 ORDER BY 控制初始排序
        tv_rp->show();
}

void dataBase::showRpmore(QTableView *tv_rps, QString id)
{
    // 1. 连接检查
       if (!openDatabase()) {
           qDebug() << "showRpmore 失败：数据库未连接。";
           if (tv_rps->model()) tv_rps->model()->deleteLater();
           tv_rps->setModel(nullptr);
           return;
       }

       // 2. 获取连接
       QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
       if (!currentDb.isValid() || !currentDb.isOpen()) {
            qDebug() << "【错误】showRpmore: 获取到的数据库连接无效或未打开。";
            if (tv_rps->model()) tv_rps->model()->deleteLater();
            tv_rps->setModel(nullptr);
            return;
       }

       // 3. 创建模型并执行查询
       QSqlQueryModel* model = new QSqlQueryModel(tv_rps);
       QString str = QString("select point_id, point_time, equipment_name, equipment_data, status, recognition_image_path, video_path from report_details where task_id = '%1' ORDER BY point_time ASC").arg(id); // 使用 = 并明确排序
       model->setQuery(str, currentDb); // <--- 使用 currentDb

       // 4. 错误检查
       if (model->lastError().isValid()) {
           qDebug() << "【SQL 错误】showRpmore setQuery: " << model->lastError().text();
           qDebug() << "执行的SQL语句是: " << str;
           model->deleteLater();
           tv_rps->setModel(nullptr);
           return;
       }

       // 5. 设置模型和视图
       QSortFilterProxyModel* sqlproxy = new QSortFilterProxyModel(tv_rps);
       sqlproxy->setSourceModel(model);
       model->setHeaderData(0, Qt::Horizontal, "巡检点编号");
       model->setHeaderData(1, Qt::Horizontal, "巡检时间");
       model->setHeaderData(2, Qt::Horizontal, "设备名称");
       model->setHeaderData(3, Qt::Horizontal, "设备数据");
       model->setHeaderData(4, Qt::Horizontal, "设备状态");
       model->setHeaderData(5, Qt::Horizontal, "识别图片");
       model->setHeaderData(6, Qt::Horizontal, "异常视频");

       QAbstractItemModel* oldModel = tv_rps->model();
       if (oldModel && oldModel != sqlproxy) {
           oldModel->deleteLater();
       }

       tv_rps->setModel(sqlproxy);
       // (tv_rps 设置不变)
       tv_rps->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
       tv_rps->setSortingEnabled(true);
       // tv_rps->sortByColumn(1, Qt::AscendingOrder); // 由 SQL 的 ORDER BY 控制
       tv_rps->verticalHeader()->setVisible(false);
       tv_rps->show();
}

void dataBase::taskmore(QTableView *tv_coms, QString start_time, QString end_time)
{
    // 1. 连接检查
       if (!openDatabase()) {
           qDebug() << "taskmore 失败：数据库未连接。";
           if (tv_coms->model()) tv_coms->model()->deleteLater();
           tv_coms->setModel(nullptr);
           return;
       }

       // 2. 获取连接
       QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
       if (!currentDb.isValid() || !currentDb.isOpen()) {
            qDebug() << "【错误】taskmore: 获取到的数据库连接无效或未打开。";
            if (tv_coms->model()) tv_coms->model()->deleteLater();
            tv_coms->setModel(nullptr);
            return;
       }

       // 3. 创建模型并执行查询
       QSqlQueryModel* model = new QSqlQueryModel(tv_coms);
       // [修正] 原 SQL 查询 * 可能包含 endtime 列，如果允许 NULL 会有问题
       // 显式列出需要的列，并使用 COALESCE 处理可能的 NULL endtime
       QString str = QString("SELECT id, name, type, startday, starttime, "
                             "COALESCE(endtime, '') AS endtime_display, situation "
                             "FROM task WHERE startday >= '%1' AND startday <= '%2' ORDER BY startday ASC, starttime ASC")
                             .arg(start_time).arg(end_time);

       model->setQuery(str, currentDb); // <--- 使用 currentDb

       // 4. 错误检查
       if (model->lastError().isValid()) {
           qDebug() << "【SQL 错误】taskmore setQuery: " << model->lastError().text();
           qDebug() << "执行的SQL语句是: " << str;
           model->deleteLater();
           tv_coms->setModel(nullptr);
           return;
       }

       // 5. 设置模型和视图
       QSortFilterProxyModel* sqlproxy = new QSortFilterProxyModel(tv_coms);
       sqlproxy->setSourceModel(model);
       model->setHeaderData(0, Qt::Horizontal, "任务ID");
       model->setHeaderData(1, Qt::Horizontal, "巡检名称");
       model->setHeaderData(2, Qt::Horizontal, "巡检位置"); // (类型 Type)
       model->setHeaderData(3, Qt::Horizontal, "任务日期");
       model->setHeaderData(4, Qt::Horizontal, "开始时间");
       model->setHeaderData(5, Qt::Horizontal, "结束时间"); // (显示的是 endtime_display)
       model->setHeaderData(6, Qt::Horizontal, "任务状态");

       QAbstractItemModel* oldModel = tv_coms->model();
       if (oldModel && oldModel != sqlproxy) {
           oldModel->deleteLater();
       }

       tv_coms->setModel(sqlproxy);
       // (tv_coms 设置不变)
       tv_coms->horizontalHeader()->setSectionResizeMode(QHeaderView::Stretch);
       tv_coms->setSortingEnabled(true);
       // tv_coms->sortByColumn(3, Qt::AscendingOrder); // 由 SQL 控制
       tv_coms->show();
}

bool dataBase::deletetask(QString id)
{
    if (!openDatabase()) { return false; } // 添加连接检查

        QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
        if (!currentDb.isValid() || !currentDb.isOpen()) { return false; }

        bool success = true; // 假设成功
        QSqlQuery query(currentDb);

        // 删除 task 表记录 (report_details 会级联删除)
        if (!query.prepare("DELETE FROM task WHERE id = ?")) {
             qDebug() << "【SQL 错误】deletetask prepare (task): " << query.lastError().text();
             return false; // Prepare 失败是严重错误
        }
        query.addBindValue(id);
        if (!query.exec()) {
            qDebug() << "【SQL 错误】deletetask exec (task): " << query.lastError().text();
            success = false; // 标记失败，但继续尝试删除 report
        }

        // 删除 report 表记录
        if (!query.prepare("DELETE FROM report WHERE id = ?")) {
             qDebug() << "【SQL 错误】deletetask prepare (report): " << query.lastError().text();
             // 即使 Prepare 失败，也可能返回之前的 success 状态
        } else {
            query.addBindValue(id);
            if (!query.exec()) {
                qDebug() << "【SQL 错误】deletetask exec (report): " << query.lastError().text();
                // 不改变 success 状态，主任务删除成功即可认为成功
            }
        }

        return success; // 返回 task 删除是否成功
}

bool dataBase::add_task(QString id, QString name, QString type, QString startday, QString starttime, QString endtime, QString situation)
{
    qDebug() << "[add_task] 开始执行...";
    qDebug() << "[add_task] 参数: name=" << name << ", starttime=" << starttime << ", situation=" << situation;
    
    // 1. 确保连接可用
    if (!openDatabase()) {
        qDebug() << "[add_task] ❌ 失败：数据库未连接 (openDatabase返回false)";
        return false;
    }
    qDebug() << "[add_task] ✓ openDatabase() 成功";

    // 2. 获取数据库连接
    QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
    qDebug() << "[add_task] 连接名称:" << m_connectionName;
    qDebug() << "[add_task] 连接有效性: isValid=" << currentDb.isValid() << ", isOpen=" << currentDb.isOpen();
    
    if (!currentDb.isValid() || !currentDb.isOpen()) {
        qDebug() << "[add_task] ❌ 错误：获取到的数据库连接无效或未打开";
        return false;
    }

    // 3. 使用正确的表结构 (task_name, start_time, end_time, status, operator, notes)
    QSqlQuery query(currentDb);
    
    // 实际表结构: id(自增), task_name, start_time, end_time, status, operator, notes, created_at(自动)
    QString insert = "INSERT INTO task (task_name, start_time, end_time, status, operator, notes) VALUES (?, ?, ?, ?, ?, ?)";
    qDebug() << "[add_task] SQL语句:" << insert;

    if (!query.prepare(insert)) {
        qDebug() << "[add_task] ❌ SQL错误 - prepare()失败:" << query.lastError().text();
        return false;
    }
    qDebug() << "[add_task] ✓ query.prepare() 成功";

    // 绑定值 (参数重新映射到正确字段)
    query.addBindValue(name);           // task_name
    query.addBindValue(starttime);      // start_time (完整时间戳)
    if (endtime.isEmpty()) {
        query.addBindValue(QVariant(QVariant::String)); // end_time = NULL
    } else {
        query.addBindValue(endtime);
    }
    query.addBindValue(situation);      // status (例如: "正在进行")
    query.addBindValue("admin");        // operator (默认操作员)
    query.addBindValue(type);           // notes (存储方案类型)
    
    qDebug() << "[add_task] 绑定的值:";
    qDebug() << "  - task_name:" << name;
    qDebug() << "  - start_time:" << starttime;
    qDebug() << "  - end_time:" << (endtime.isEmpty() ? "NULL" : endtime);
    qDebug() << "  - status:" << situation;
    qDebug() << "  - operator: admin";
    qDebug() << "  - notes:" << type;

    // 执行插入
    qDebug() << "[add_task] 准备执行 query.exec()...";
    if (!query.exec()) {
        qDebug() << "[add_task] ❌ SQL错误 - exec()失败:" << query.lastError().text();
        qDebug() << "[add_task] 错误类型:" << query.lastError().type();
        qDebug() << "[add_task] 数据库错误号:" << query.lastError().nativeErrorCode();
        return false;
    }
    
    qDebug() << "[add_task] ✅ 成功！任务已记录到数据库:" << name;
    return true;
}

// 获取最后插入的自增ID
QString dataBase::getLastInsertId()
{
    if (!openDatabase()) {
        qDebug() << "getLastInsertId 失败：数据库未连接";
        return "";
    }
    
    QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(currentDb);
    
    if (query.exec("SELECT LAST_INSERT_ID()")) {
        if (query.next()) {
            QString id = query.value(0).toString();
            qDebug() << "获取到最后插入的ID: " << id;
            return id;
        }
    }
    
    qDebug() << "获取LAST_INSERT_ID()失败: " << query.lastError().text();
    return "";
//    QSqlQuery query(db);
//    QString insert = "insert into task values(?,?,?,?,?,?,?)";
//    query.prepare(insert);
//    query.addBindValue(id);
//    query.addBindValue(name);
//    query.addBindValue(type);
//    query.addBindValue(startday);
//    query.addBindValue(starttime);
//    // 修正：允许 endtime 为空
//    if (endtime.isEmpty()) {
//        query.addBindValue(QVariant(QVariant::String)); // 绑定一个 NULL 字符串
//    } else {
//        query.addBindValue(endtime);
//    }

//    query.addBindValue(situation);
//    if (!query.exec()) {
//        qDebug() << "【SQL 错误】add_task: " << query.lastError().text();
//        return false;
//    }
//    return true;
}


void dataBase::insertreport(QString task_name, QString start_time, QString status, QString operator_name, QString notes)
{
    if (!openDatabase()) { return; }
    
    QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
    QSqlQuery query(currentDb);
    QString end_time = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    
    // 插入到 report 表 (id为自增，created_at有默认值)
    QString insert = "INSERT INTO report (task_name, start_time, end_time, status, operator, notes) VALUES (?, ?, ?, ?, ?, ?)";
    query.prepare(insert);
    query.addBindValue(task_name);
    query.addBindValue(start_time);
    query.addBindValue(end_time);
    query.addBindValue(status);
    query.addBindValue(operator_name);
    query.addBindValue(notes);
    
    if (!query.exec()) {
        qDebug() << "【SQL 错误】insertreport: " << query.lastError().text();
        qDebug() << "【SQL 语句】" << insert;
    } else {
        qDebug() << "【成功】任务报告已保存: " << task_name << ", 状态: " << status;
    }
}

/**
 * @brief [新功能] 向 report_details 表插入一条巡检点明细
 */
bool dataBase::add_report_detail(const QString &task_id, const QString &point_id,
                                 const QString &equipment_name, const QString &equipment_data,
                                 const QString &status, const QString &image_path, const QString &video_path)
{
    // 1. 确保连接是打开的
    if (!openDatabase()) {
        qDebug() << "add_report_detail 失败：数据库未连接。";
        return false;
    }
    
    // 2. 获取连接并详细检查
    QSqlDatabase currentDb = QSqlDatabase::database(m_connectionName);
    if (!currentDb.isValid()) {
        qDebug() << "【错误】add_report_detail: 数据库连接无效 (isValid=false)";
        return false;
    }
    if (!currentDb.isOpen()) {
        qDebug() << "【错误】add_report_detail: 数据库连接未打开 (isOpen=false)";
        return false;
    }

    QSqlQuery query(currentDb); // 使用获取到的连接
    // 2. SQL 语句必须与 setup_database.sql 中的列名完全一致
    // (recognition_result 已更名为 recognition_image_path)
    QString sql = "INSERT INTO report_details (task_id, point_id, point_time, "
                  "equipment_name, equipment_data, status, "
                  "recognition_image_path, video_path) "
                  "VALUES (?, ?, ?, ?, ?, ?, ?, ?)";

    if (!query.prepare(sql)) { // 检查 prepare
            qDebug() << "【SQL 错误】add_report_detail: query.prepare() failed:" << query.lastError().text();
            return false;
       }

    query.addBindValue(task_id);
    query.addBindValue(point_id);
    query.addBindValue(QDateTime::currentDateTime()); // 自动填入当前时间
    query.addBindValue(equipment_name);
    query.addBindValue(equipment_data);
    query.addBindValue(status);
    query.addBindValue(image_path);
    query.addBindValue(video_path);

    // 3. 执行并进行错误检查
    if (!query.exec()) {
        qDebug() << "【SQL 错误】add_report_detail 执行失败: " << query.lastError().text();
        return false;
    }

    return true;
}

QString dataBase::GetTheSpendTime(QString starttime, QString endtime)
{
    QDateTime start = QDateTime::fromString(starttime, "yyyy-MM-dd hh:mm:ss");
        QDateTime end = QDateTime::fromString(endtime, "yyyy-MM-dd hh:mm:ss");
        qint64 spend = start.secsTo(end); // 总秒数

        int h = spend / 3600;
        int m = (spend % 3600) / 60;
        int s = spend % 60;

        QString s_str;
        if (h > 0) {
            s_str = QString("%1 h %2 m %3 s").arg(h).arg(m).arg(s);
        } else if (m > 0) {
            s_str = QString("%1 m %2 s").arg(m).arg(s);
        } else {
            s_str = QString("%1 s").arg(s);
        }
        return s_str;
}
