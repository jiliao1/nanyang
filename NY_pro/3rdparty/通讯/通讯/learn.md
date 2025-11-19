1. rosbridge_server  桥接ROS与websocket,支持多种语言的客户端
将 ROS 话题映射为 WebSocket 消息
支持 JSON 格式的数据收发
允许任意支持 WebSocket 的客户端（如 Python、JS、C#）与 ROS 通讯
roslaunch rosbridge_server rosbridge_websocket.launch  所有消息必须遵循rosbridge的JSON格式规范
✅ 在 9090 端口监听 WebSocket 连接
✅ 自动处理 JSON ↔ ROS 消息的转换
✅ 不需要配置话题、消息类型
✅ 即插即用
op 字段用于告诉 rosbridge_server 要执行什么操作，类似于 HTTP 中的 GET、POST 等方法。

2. WebSocket 通讯   全双工、实时、跨平台，适合机器人与上位机、网页等通讯
Windows 客户端通过 websocket-client 库连接 rosbridge_server
发送/接收 JSON 格式的消息
支持订阅、发布、取消订阅等操作
通过WebSocket协议(TCP/IP)进行网络通讯。


3. JSON格式  通用、易解析，适合结构化数据交换

Windows PC                    Nvidia 设备
┌─────────────┐              ┌──────────────┐              ┌──────────────┐
│             │              │              │              │              │
│  windows_   │  WebSocket   │  rosbridge_  │  ROS Topics  │  patrol_     │
│  client.py  │◄───────────► │  server      │◄────────────►│  config_node │
│             │  TCP/IP      │  (9090端口)   │              │              │
│             │  JSON格式     │              │  ROS消息     │              │
└─────────────┘              └──────────────┘              └──────────────┘

代码中出现的两次JSON处理：
# 第一次：构造要发送的数据
cmd_data = {"command": "start"}

# 第二次：包装成 std_msgs/String 格式
msg = {
    'data': json.dumps(cmd_data)  # 转为 JSON 字符串
}

# 第三次：包装成 rosbridge 格式
publish_msg = {
    'op': 'publish',
    'topic': '/patrol/command',
    'msg': msg
}