#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
Windows 上位机 ROS 客户端
通过 rosbridge WebSocket 与 Nvidia 上的 ROS 节点通信
无需安装 ROS，只需要 Python 和 websocket 库

安装依赖：
pip install websocket-client

使用方法：
python windows_client.py
"""

import websocket
import json
import threading
import time
from datetime import datetime

class ROSBridgeClient:
    """ROS Bridge WebSocket 客户端"""
    
    def __init__(self, host='192.168.1.100', port=9090):
        """
        初始化客户端
        :param host: Nvidia 设备的 IP 地址
        :param port: rosbridge_server 端口，默认 9090
        """
        self.host = host
        self.port = port
        self.url = f"ws://{host}:{port}" ## 构造 WebSocket URL: ws://192.168.1.100:9090
        self.ws = None  # WebSocket 连接对象（初始为空）
        self.connected = False  #  # 连接状态标志
        self.running = False
        
        print("=" * 60)
        print("Windows 上位机 ROS 客户端")
        print(f"目标地址: {self.url}")
        print("=" * 60)
    
    def connect(self):
        """连接到 rosbridge_server"""
        try:
            print(f"\n正在连接到 {self.url} ...")
            """
            # 1. 创建 WebSocketApp 对象
            self.ws = websocket.WebSocketApp(
                self.url,                    # WebSocket URL
                on_open=self.on_open,       # 连接成功时调用
                on_message=self.on_message, # 收到消息时调用
                on_error=self.on_error,     # 发生错误时调用
                on_close=self.on_close      # 连接关闭时调用
            )

            # 2. 在新线程中运行 WebSocket（避免阻塞主程序）
            ws_thread = threading.Thread(target=self.ws.run_forever)
            ws_thread.daemon = True  # 设为守护线程，主程序结束时自动关闭
            ws_thread.start()        # 启动线程

             # 3. 等待连接建立（最多等待 5 秒）
             timeout = 5
             start_time = time.time()
             while not self.connected and time.time() - start_time < timeout:
                time.sleep(0.1)  # 每 0.1 秒检查一次连接状态
            """
            self.ws = websocket.WebSocketApp(
                self.url,
                on_open=self.on_open,
                on_message=self.on_message,
                on_error=self.on_error,
                on_close=self.on_close
            )
            self.running = True
            
            # 在新线程中运行 WebSocket
            ws_thread = threading.Thread(target=self.ws.run_forever)
            ws_thread.daemon = True
            ws_thread.start()
            
            # 等待连接建立
            timeout = 5
            start_time = time.time()
            while not self.connected and time.time() - start_time < timeout:
                time.sleep(0.1)
            
            if self.connected:
                print("✓ 连接成功！")
                return True
            else:
                print("✗ 连接超时")
                return False
                
        except Exception as e:
            print(f"✗ 连接失败: {e}")
            return False
    
    def on_open(self, ws):
        """连接建立时的回调"""
        self.connected = True
        print(f"[{datetime.now().strftime('%H:%M:%S')}] WebSocket 连接已建立")
    
    def on_message(self, ws, message):
        """接收到消息时的回调"""
        try:
            data = json.loads(message)
            
            # 处理不同类型的消息
            if data.get('op') == 'publish':
                topic = data.get('topic', 'unknown')
                msg_data = data.get('msg', {})
                self.handle_topic_message(topic, msg_data)
                
        except json.JSONDecodeError:
            print(f"JSON 解析失败: {message}")
        except Exception as e:
            print(f"处理消息时出错: {e}")
    
    def handle_topic_message(self, topic, msg_data):
        """处理接收到的话题消息"""
        timestamp = datetime.now().strftime('%H:%M:%S')
        
        if topic == '/patrol/data':
            # 处理巡检数据
            try:
                data = json.loads(msg_data.get('data', '{}'))
                print(f"\n[{timestamp}] 📡 收到巡检数据:")
                print(f"  名称: {data.get('name')}")
                print(f"  状态: {data.get('status')}")
                print(f"  速度: {data.get('speed')} m/s")
                print(f"  电量: {data.get('battery'):.1f}%")
                print(f"  时间: {data.get('timestamp')}")
            except:
                print(f"[{timestamp}] /patrol/data: {msg_data}")
        
        elif topic == '/patrol/feedback':
            # 处理反馈消息（节点的响应）
            try:
                data = json.loads(msg_data.get('data', '{}'))
                print(f"\n[{timestamp}] ✓ 收到反馈:")
                print(f"  类型: {data.get('type')}")
                print(f"  状态: {data.get('status')}")
                print(f"  消息: {data.get('message')}")
                print(f"  当前状态: {data.get('current_status')}")
                print(f"  已接收: {data.get('received_count')} 条消息")
            except:
                print(f"[{timestamp}] /patrol/feedback: {msg_data}")
        
        elif topic == '/patrol/status':
            # 处理状态反馈
            print(f"[{timestamp}] 📊 状态反馈: {msg_data}")
        
        else:
            print(f"[{timestamp}] {topic}: {msg_data}")
    
    def on_error(self, ws, error):
        """发生错误时的回调"""
        print(f"✗ WebSocket 错误: {error}")
    
    def on_close(self, ws, close_status_code, close_msg):
        """连接关闭时的回调"""
        self.connected = False
        print(f"\n[{datetime.now().strftime('%H:%M:%S')}] WebSocket 连接已关闭")
        if close_msg:
            print(f"关闭信息: {close_msg}")
    
    def subscribe(self, topic, msg_type='std_msgs/String'):
        """
        订阅 ROS 话题
        :param topic: 话题名称，如 '/patrol/data'
        :param msg_type: 消息类型，默认 'std_msgs/String'
        """
        if not self.connected:
            print("✗ 未连接，无法订阅")
            return False
        
        subscribe_msg = {
            'op': 'subscribe',
            'topic': topic,
            'type': msg_type
        }
        
        try:
            self.ws.send(json.dumps(subscribe_msg))
            print(f"✓ 已订阅话题: {topic}")
            return True
        except Exception as e:
            print(f"✗ 订阅失败: {e}")
            return False
    
    def unsubscribe(self, topic):
        """取消订阅话题"""
        if not self.connected:
            return False
        
        unsubscribe_msg = {
            'op': 'unsubscribe',
            'topic': topic
        }
        
        try:
            self.ws.send(json.dumps(unsubscribe_msg))
            print(f"✓ 已取消订阅: {topic}")
            return True
        except Exception as e:
            print(f"✗ 取消订阅失败: {e}")
            return False
    
    def publish(self, topic, msg_data, msg_type='std_msgs/String'):
        """
        发布消息到 ROS 话题
        :param topic: 话题名称
        :param msg_data: 消息数据（字典格式）
        :param msg_type: 消息类型
        """
        if not self.connected:
            print("✗ 未连接，无法发布")
            return False
        
        publish_msg = {
            'op': 'publish',
            'topic': topic,
            'type': msg_type,
            'msg': msg_data
        }
        
        try:
            self.ws.send(json.dumps(publish_msg))
            return True
        except Exception as e:
            print(f"✗ 发布失败: {e}")
            return False
    
    def send_command(self, command):
        """
        发送控制指令
        :param command: 指令类型，如 'start', 'stop', 'pause', 'resume'
        """
        cmd_data = {
            "command": command,
            "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        }
        
        msg = {
            'data': json.dumps(cmd_data, ensure_ascii=False)
        }
        
        if self.publish('/patrol/command', msg):
            print(f"✓ 已发送指令: {command}")
            return True
        return False
    
    def update_config(self, config_data):
        """
        更新配置
        :param config_data: 配置数据字典
        """
        msg = {
            'data': json.dumps(config_data, ensure_ascii=False)
        }
        
        if self.publish('/patrol/config', msg):
            print(f"✓ 已发送配置更新: {config_data}")
            return True
        return False
    
    def send_message(self, message_data):
        """
        发送普通消息
        :param message_data: 消息数据（字典或字符串）
        """
        if isinstance(message_data, str):
            message_data = {
                "text": message_data,
                "timestamp": datetime.now().strftime("%Y-%m-%d %H:%M:%S")
            }
        
        msg = {
            'data': json.dumps(message_data, ensure_ascii=False)
        }
        
        if self.publish('/patrol/message', msg):
            print(f"✓ 已发送消息: {message_data}")
            return True
        return False
    
    def disconnect(self):
        """断开连接"""
        self.running = False
        if self.ws:
            self.ws.close()
        print("已断开连接")


def print_menu():
    """打印菜单"""
    print("\n" + "=" * 60)
    print("操作菜单:")
    print("  1. 订阅巡检数据 (/patrol/data)")
    print("  2. 订阅反馈消息 (/patrol/feedback)")
    print("  3. 发送开始指令 (start)")
    print("  4. 发送停止指令 (stop)")
    print("  5. 发送暂停指令 (pause)")
    print("  6. 发送恢复指令 (resume)")
    print("  7. 更新配置 (速度)")
    print("  8. 发送自定义消息")
    print("  9. 取消所有订阅")
    print("  0. 退出")
    print("=" * 60)


def main():
    """主函数"""
    # 修改这里的 IP 地址为你的 Nvidia 设备 IP
    NVIDIA_IP = '192.168.73.130'  # ← 修改为实际的 IP 地址
    
    # 创建客户端
    client = ROSBridgeClient(host=NVIDIA_IP, port=9090)
    
    # 连接到 rosbridge
    if not client.connect():
        print("\n连接失败，请检查：")
        print("1. Nvidia 设备的 IP 地址是否正确")
        print("2. rosbridge_server 是否已启动")
        print("   启动命令: roslaunch rosbridge_server rosbridge_websocket.launch")
        print("3. 网络连接是否正常")
        return
    
    # 等待连接稳定
    time.sleep(1)
    
    try:
        while client.connected:
            print_menu()
            choice = input("\n请选择操作 (0-9): ").strip()
            
            if choice == '1':
                # 订阅巡检数据
                client.subscribe('/patrol/data', 'std_msgs/String')
                print("\n✓ 已订阅巡检数据，将持续接收...")
            
            elif choice == '2':
                # 订阅反馈消息
                client.subscribe('/patrol/feedback', 'std_msgs/String')
                print("\n✓ 已订阅反馈消息，将接收节点的响应...")
            
            elif choice == '3':
                # 发送开始指令
                client.send_command('start')
            
            elif choice == '4':
                # 发送停止指令
                client.send_command('stop')
            
            elif choice == '5':
                # 发送暂停指令
                client.send_command('pause')
            
            elif choice == '6':
                # 发送恢复指令
                client.send_command('resume')
            
            elif choice == '7':
                # 更新配置
                print("\n--- 配置更新 ---")
                speed_input = input("请输入新速度 (m/s，默认1.0): ").strip()
                name_input = input("请输入新名称 (默认'更新路线'): ").strip()
                
                speed = float(speed_input) if speed_input else 1.0
                name = name_input if name_input else '更新路线'
                
                config = {
                    'speed': speed,
                    'name': name
                }
                client.update_config(config)
            
            elif choice == '8':
                # 发送自定义消息
                message = input("\n请输入要发送的消息: ").strip()
                if message:
                    client.send_message(message)
                else:
                    print("✗ 消息不能为空")
            
            elif choice == '9':
                # 取消所有订阅
                client.unsubscribe('/patrol/data')
                client.unsubscribe('/patrol/feedback')
                print("\n✓ 已取消所有订阅")
            
            elif choice == '0':
                print("\n正在退出...")
                break
            
            else:
                print("✗ 无效的选择")
            
            time.sleep(0.5)
    
    except KeyboardInterrupt:
        print("\n\n收到中断信号...")
    
    finally:
        client.disconnect()
        print("程序已退出")


if __name__ == '__main__':
    print("""
    ╔══════════════════════════════════════════════════════════╗
    ║         Windows 上位机 ROS 客户端                         ║
    ║         通过 WebSocket 与 ROS 通信                        ║
    ╚══════════════════════════════════════════════════════════╝
    
    使用前请确保：
    1. 已安装依赖: pip install websocket-client
    2. Nvidia 端已启动 rosbridge_server
    3. 修改脚本中的 NVIDIA_IP 为实际 IP 地址
    """)
    
    try:
        main()
    except Exception as e:
        print(f"\n程序异常: {e}")
        import traceback
        traceback.print_exc()