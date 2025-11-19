# ROS控制功能使用说明

## 启动步骤

1. **启动程序**
   - 运行 `启动程序.bat` 或 `启动程序.ps1`
   
2. **配置rosbridge连接**
   - 程序启动时会弹出IP配置对话框
   - **IP地址**: 192.168.16.146 (已默认填写)
   - **端口**: 8079 (已默认填写为rosbridge端口)
   - 点击 "OK" 确认连接

3. **查看连接状态**
   - 在软件的输出信息区应该看到:
     ```
     正在连接: ws://192.168.16.146:8079
     WebSocket已连接
     ```

## 控制功能

### 底盘控制按钮
- **前进/后退/左转/右转**按钮
  - 按下时: 以10Hz频率发送控制指令到 `/cmd_vel` 话题
  - 速度: linear.x = 0.2 m/s, angular.z = 0.2 rad/s
  - 松开时: 立即发送停止指令

### 云台控制按钮
- **上/下/左/右**按钮
  - 按下时: 以5Hz频率发送控制指令到 `/ptz_scan_control` 话题
  - 速度: pitch/yaw = ±16
  - 松开时: 立即发送停止指令

### 云台变焦滑动条
- 滑动条范围: 1-23
- 实际变焦值: 16-16384 (对数映射)
- 话题: `/ptz_zoom_control`
- 移动滑动条时实时发送控制指令

## ROS话题格式

### /cmd_vel (geometry_msgs/Twist)
```json
{
  "op": "publish",
  "topic": "/cmd_vel",
  "msg": {
    "linear": {"x": 0.2, "y": 0.0, "z": 0.0},
    "angular": {"x": 0.0, "y": 0.0, "z": 0.2}
  }
}
```

### /ptz_scan_control (geometry_msgs/Vector3)
```json
{
  "op": "publish",
  "topic": "/ptz_scan_control",
  "msg": {
    "x": 0.0,
    "y": 16.0,    // pitch速度
    "z": -16.0    // yaw速度
  }
}
```

### /ptz_zoom_control (geometry_msgs/Vector3)
```json
{
  "op": "publish",
  "topic": "/ptz_zoom_control",
  "msg": {
    "x": 1024.0,  // 变焦值 16-16384
    "y": 0.0,
    "z": 0.0
  }
}
```

## 调试信息

在控制台(qDebug输出)中可以看到:
- 按钮按下/释放事件
- WebSocket连接状态
- 发送的JSON消息内容

## 故障排查

### 1. 按钮没反应
- 检查WebSocket是否连接成功
- 查看控制台是否有 "WebSocket未连接" 的错误信息
- 确认rosbridge_server正在运行: `rosrun rosbridge_server rosbridge_websocket`

### 2. 未显示连接信息
- 确认在启动对话框中点击了 "OK" 而不是 "Cancel"
- 确认端口号为 8079

### 3. ROS端未收到消息
- 检查rosbridge_server是否正常运行
- 使用 `rostopic echo /cmd_vel` 监听话题
- 检查防火墙设置
