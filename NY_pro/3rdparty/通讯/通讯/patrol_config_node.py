#!/usr/bin/env python3
# -*- coding: utf-8 -*-
"""
简单数据发送节点 - 通过 rosbridge_server 与上位机通信
在 Nvidia 设备上运行，通过 ROS topic 与 Windows 上位机进行 WebSocket 通信
"""

import rospy
from std_msgs.msg import String
import json  # 用于JSON数据的序列化/反序列化
from datetime import datetime  #用于生成时间戳

class PatrolConfigNode:
    """
    简单数据发送节点
    功能：定时发送简单数据供上位机订阅
    """
    
    def __init__(self):
        # 初始化 ROS 节点
        rospy.init_node('patrol_config_node', anonymous=True)
        rospy.loginfo("=" * 50)
        rospy.loginfo("数据发送/接收节点启动")
        rospy.loginfo("通过 rosbridge_server 与上位机通信")
        rospy.loginfo("=" * 50)
        
        # 简单的数据
        self.data = {
            "name": "巡检机器人",
            "status": "运行中",
            "speed": 0.5,
            "battery": 85
        }
        
        # 接收到的消息计数
        self.received_count = 0
        
        # ========== 发布器 ==========
        # 发布数据
        self.data_pub = rospy.Publisher('/patrol/data', String, queue_size=10)
        
        # 发布反馈消息
        self.feedback_pub = rospy.Publisher('/patrol/feedback', String, queue_size=10)
        
        # ========== 订阅器 ==========
        # 订阅上位机的控制指令
        rospy.Subscriber('/patrol/command', String, self.command_callback)
        
        # 订阅上位机的配置更新
        rospy.Subscriber('/patrol/config', String, self.config_callback)
        
        # 订阅上位机的普通消息
        rospy.Subscriber('/patrol/message', String, self.message_callback)
        
        # 定时发布数据（每2秒发送一次）
        rospy.Timer(rospy.Duration(2), self.publish_data_timer)
        
        rospy.loginfo("节点初始化完成")
        rospy.loginfo("上位机 WebSocket 地址: ws://<nvidia_ip>:9090")
        rospy.loginfo("")
        rospy.loginfo("订阅的话题:")
        rospy.loginfo("  - /patrol/command  (控制指令)")
        rospy.loginfo("  - /patrol/config   (配置更新)")
        rospy.loginfo("  - /patrol/message  (普通消息)")
        rospy.loginfo("")
        rospy.loginfo("发布的话题:")
        rospy.loginfo("  - /patrol/data     (机器人数据)")
        rospy.loginfo("  - /patrol/feedback (反馈消息)")
        rospy.loginfo("=" * 50)
    
    def command_callback(self, msg):
        """接收上位机发送的控制指令"""
        try:
            self.received_count += 1
            data = json.loads(msg.data)  # 解析JSON数据
            command = data.get('command', 'unknown')  # 获取指令类型
            
            rospy.loginfo("=" * 50)
            rospy.loginfo(f"[收到指令 #{self.received_count}]")
            rospy.loginfo(f"  指令类型: {command}")
            rospy.loginfo(f"  完整数据: {data}")
            rospy.loginfo("=" * 50)
            
            # 根据不同指令执行不同操作
            if command == 'start':
                self.data['status'] = '运行中'
                rospy.loginfo(">>> 执行: 开始巡检")
            elif command == 'stop':
                self.data['status'] = '已停止'
                rospy.loginfo(">>> 执行: 停止巡检")
            elif command == 'pause':
                self.data['status'] = '已暂停'
                rospy.loginfo(">>> 执行: 暂停巡检")
            elif command == 'resume':
                self.data['status'] = '运行中'
                rospy.loginfo(">>> 执行: 恢复巡检")
            else:
                rospy.logwarn(f">>> 未知指令: {command}")
            
            # 发送反馈
            self.send_feedback('command', f"指令 '{command}' 已执行", data)
            
        except json.JSONDecodeError as e:
            rospy.logerr(f"指令解析失败: {e}")
            rospy.logerr(f"原始消息: {msg.data}")
        except Exception as e:
            rospy.logerr(f"处理指令时出错: {e}")
    
    def config_callback(self, msg):
        """接收上位机发送的配置更新"""
        try:
            self.received_count += 1
            data = json.loads(msg.data)
            
            rospy.loginfo("=" * 50)
            rospy.loginfo(f"[收到配置更新 #{self.received_count}]")
            rospy.loginfo(f"  配置数据: {data}")
            rospy.loginfo("=" * 50)
            
            # 更新配置
            if 'speed' in data:
                self.data['speed'] = data['speed']
                rospy.loginfo(f">>> 更新速度: {data['speed']} m/s")
            
            if 'name' in data:
                self.data['name'] = data['name']
                rospy.loginfo(f">>> 更新名称: {data['name']}")
            
            # 发送反馈
            self.send_feedback('config', '配置已更新', data)
            
        except json.JSONDecodeError as e:
            rospy.logerr(f"配置解析失败: {e}")
        except Exception as e:
            rospy.logerr(f"处理配置时出错: {e}")
    
    def message_callback(self, msg):
        """接收上位机发送的普通消息"""
        try:
            self.received_count += 1
            data = json.loads(msg.data)
            
            rospy.loginfo("=" * 50)
            rospy.loginfo(f"[收到消息 #{self.received_count}]")
            rospy.loginfo(f"  消息内容: {data}")
            rospy.loginfo("=" * 50)
            
            # 发送反馈
            self.send_feedback('message', '消息已收到', data)
            
        except json.JSONDecodeError as e:
            rospy.logerr(f"消息解析失败: {e}")
        except Exception as e:
            rospy.logerr(f"处理消息时出错: {e}")
    
    def send_feedback(self, msg_type, message, original_data=None):
        """发送反馈消息到上位机"""
        feedback = {
            'type': msg_type,
            'status': 'success',
            'message': message,
            'timestamp': datetime.now().strftime("%Y-%m-%d %H:%M:%S"),
            'current_status': self.data['status'],
            'received_count': self.received_count
        }
        
        if original_data:
            feedback['original'] = original_data
        
        msg = json.dumps(feedback, ensure_ascii=False)
        self.feedback_pub.publish(msg)
        rospy.loginfo(f">>> 已发送反馈: {message}")
    
    def publish_data_timer(self, event):
        """定时发布数据"""
        # 更新时间戳和电量（模拟变化）
        self.data['timestamp'] = datetime.now().strftime("%Y-%m-%d %H:%M:%S")
        self.data['battery'] = max(0, self.data['battery'] - 0.1)  # 模拟电量下降
        
        # 发布数据
        msg = json.dumps(self.data, ensure_ascii=False)
        self.data_pub.publish(msg)
        rospy.loginfo(f"已发送数据: {self.data}")
    
    def run(self):
        """运行节点"""
        rospy.loginfo("节点运行中，按 Ctrl+C 退出...")
        rospy.spin()

if __name__ == '__main__':
    try:
        node = PatrolConfigNode()
        node.run()
    except rospy.ROSInterruptException:
        rospy.loginfo("节点被中断")
    except Exception as e:
        rospy.logerr(f"节点运行出错: {e}")
