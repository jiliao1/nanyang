#ifndef IRCNETSDKDEF_H
#define IRCNETSDKDEF_H

#if defined (_WIN32) || defined(_WIN64)
#ifdef IRCNETSDK_EXPORTS
#define IRC_NET_API __declspec(dllexport)
#else
#define IRC_NET_API __declspec(dllimport)
#endif
#define IRC_NET_CALL _stdcall
#elif defined (OS_POSIX) || defined (__APPLE__) || defined(ANDROID) || defined (__linux__)
#define IRC_NET_API
#define IRC_NET_CALL
#else
#error os not support!
#endif

#include <stdint.h>

#define IRC_NET_IP_LEN_MAX 16 ///< IP长度最大值
#define IRC_NET_MAC_LEN_MAX 18 ///< MAC地址长度最大值
#define IRC_NET_NAME_LEN_MAX 256 ///< 名称长度最大值
#define IRC_NET_FILE_PATH_LEN_MAX 256 ///< 文件路径最大值
#define IRC_NET_CODE_LEN_MAX 32 ///< 编码长度最大值
#define IRC_NET_RULE_POINT_NUM_MAX 8 ///< 规则点数最大值
#define IRC_NET_QUERY_ALL -1 ///< 查询所有
#define IRC_NET_TOUR_PRESET_NUM_MAX 256 ///< 巡航组预置点个数最大值
#define IRC_NET_PATTERN_NUM_MAX 5 ///< 巡迹个数
#define IRC_NET_TIME_LEN_MAX 256 ///< 时间字符长度最大值
#define IRC_NET_MASK_POINT_NUM_MAX 4 ///< 屏蔽区域点个数
#define IRC_NET_AT30_POINT_NUM_MAX 1 ///< AT30点个数
#define IRC_NET_AT30_LINE_NUM_MAX 2 ///< AT30线点个数
#define IRC_NET_AT30_REGION_NUM_MAX 4 ///< AT30区域点个数
#define IRC_NET_CHANNEL_NUM_MAX 2 ///< 通道个数最大值
#define IRC_NET_FRAME_TEMP_RULE_NUM_MAX 2 ///< 整帧测温报警规则数量
#define IRC_NET_SWIVEL_VERSION_LEN 32 ///< 转台版本长度 
#define IRC_NET_RTSP_LEN_MAX 1024 ///< rtsp的Url长度最大值
#define IRC_NET_COLOR_TYPE_MAX 4 ///< 颜色类型数量


/**
 * @brief 操作句柄
 *
 */
typedef uint64_t IRC_NET_HANDLE;

/**
 * @brief 错误码
 * 
 */
typedef enum
{
    IRC_NET_ERROR_OK = 0, ///< 成功
    IRC_NET_ERROR_FAILED = 1, ///< 失败
    IRC_NET_ERROR_NOT_SUPPORTED = 2, ///< 不支持
    IRC_NET_ERROR_PARAM_WRONG = 3, ///< 参数错误
    IRC_NET_ERROR_TEMP_CALLBACK_WRONG = 4, ///<温度回调未开启

    IRC_NET_ERROR_BLACK_LIST = 1001, ///< 用户不在白名单
    IRC_NET_ERROR_NONE_USER = 1002, ///< 用户名不存在
    IRC_NET_ERROR_PWD_WRONG = 1003, ///< 密码错误
    IRC_NET_ERROR_DEV_NOT_SUPPORTED = 1004, ///< 能力集无设备型号
    IRC_NET_ERROR_ACCOUNT_LOCK = 1005, ///< 账号锁定
    IRC_NET_ERROR_USER_LIMIT = 1006, ///< 用户数量超出限制
    IRC_NET_ERROR_SYSTEM_EXCEPTION = 1007, ///< 操作失败

    IRC_NET_ERROR_TEMP_RULE_LIMIT = 1101, ///< 测温规则上限

} IRC_NET_ERROR;

/**
 * @brief 异常类型
 *
 */
typedef enum
{
    IRC_NET_EXCEPTION_DEV_OFFLINE = 1001, ///< 设备离线
    IRC_NET_EXCEPTION_PREVIEW_OFFLINE = 1002, ///< 预览离线
    IRC_NET_EXCEPTION_ALARM_OFFLINE = 1003, ///< 报警离线
    IRC_NET_EXCEPTION_TEMP_OFFLINE = 1004, ///< 温度离线
} IRC_NET_EXCEPTION_TYPE;

/**
 * @brief 异常回调
 *
 */
typedef void (* IRC_NET_EXCEPTION_CALLBACK)(IRC_NET_HANDLE handle, int exceptionType, void* userData);

/**
 * @brief 报警类型
 *
 */
typedef enum
{
    IRC_NET_ALARM_TEMP = 10001, ///< 温度
    IRC_NET_ALARM_FIRE  = 10002, ///< 火点
    IRC_NET_ALARM_TEMP_RISE = 10003, ///< 温升
    IRC_NET_ALARM_TEMP_DIFF = 10004, ///< 温差
    IRC_NET_ALARM_FIRE_PULSE = 10005, ///< 火点脉冲

    IRC_NET_ALARM_REGION_INTRUSION = 20001, ///< 区域入侵
    IRC_NET_ALARM_LINE_INTRUSION = 20002, ///< 绊线入侵

    IRC_NET_ALARM_SMOKE_DETECT = 30001, ///<烟雾
    IRC_NET_ALARM_LOCAL = 40001 ///< 本地
} IRC_NET_ALARM_TYPE;

/**
 * @brief 报警回调
 *
 */
typedef void (* IRC_NET_ALARM_CALLBACK)(IRC_NET_HANDLE handle, int alarmType, void* alarmInfo, void* userData);

/**
 * @brief 日志等级
 *
 */
typedef enum
{
    IRC_NET_LOG_LEVEL_CLOSE = 0,
    IRC_NET_LOG_LEVEL_TRACE,
    IRC_NET_LOG_LEVEL_DEBUG,
    IRC_NET_LOG_LEVEL_INFO,
    IRC_NET_LOG_LEVEL_WARN,
    IRC_NET_LOG_LEVEL_ERROR
} IRC_NET_LOG_LEVEL;

/**
 * @brief 设备搜索信息
 *
 */
typedef struct
{
    char ip[IRC_NET_IP_LEN_MAX]; ///< 设备IP
} IRC_NET_DEV_SEARCH_INFO;

/**
 * @brief 设备搜索回调
 *
 */
typedef void (* IRC_NET_DEV_SEARCH_CALLBACK)(IRC_NET_DEV_SEARCH_INFO* searchInfo, void* userData);

/**
 * @brief 登录信息
 * 
 */
typedef struct
{
    char ip[IRC_NET_IP_LEN_MAX]; ///< 设备IP
    int port; ///< 设备端口
    char username[IRC_NET_NAME_LEN_MAX]; ///< 用户名
    char password[IRC_NET_NAME_LEN_MAX]; ///< 密码
} IRC_NET_LOGIN_INFO;

/**
 * @brief 产品代数
 *
 */
typedef enum
{
    IRC_NET_PRODUCT_GENERATION_STANDARD = 0, ///< 标准代
    IRC_NET_PRODUCT_GENERATION_G1, ///< G1代
} IRC_NET_PRODUCT_GENERATION;

/**
 * @brief 设备信息
 * 
 */
typedef struct
{
    int channelNum; ///< 通道数
    int optChannel; ///< 可见光通道
    int irChannel; ///< 红外通道
    int productGeneration; ///<产品代数，参考IRC_NET_PRODUCT_GENERATION
    int model; ///< 设备型号
} IRC_NET_DEV_INFO;

/**
 * @brief 云台能力
 *
 */
typedef struct
{
    int zoom; ///< 变焦
    int focus; ///< 聚焦
    int iris; ///< 光圈
    int ptz; ///< 云台
    int wiper; ///< 雨刷
    int light; ///< 补光灯
    int defrost; ///< 除霜
    int defog; ///< 透雾
    int fan; ///< 风扇
    int heater; ///< 加热器
    int autoFocus; ///< 自动聚焦
    int syncView; ///< 双向随动
    int ptzPosition; ///< 精确定位
    int position3D; ///< 3D定位
    int regionFocus; ///< 区域聚焦
    int manualTrack; ///< 手动跟踪
    int lensInit; ///< 镜头初始化
    int laserDistance; ///< 激光测距
} IRC_NET_PTZ_ABILITY;

/**
 * @brief 测温能力
 *
 */
typedef struct
{
    int siglePointTemp; ///< 单点测温
    int tempLevelEnd; ///< 温度截止
} IRC_NET_TEMP_ABILITY;

/**
 * @brief 设备能力类型
 *
 */
typedef enum
{
    IRC_NET_DEV_ABILITY_PTZ = 0, ///< 云台能力
    IRC_NET_DEV_ABILITY_TEMP, ///< 测温能力
} IRC_NET_DEV_ABILITY_TYPE;

/**
 * @brief 帧格式
 *
 */
typedef enum
{
    IRC_NET_FRAME_FMT_YUV420P = 0, ///< YUV420P
    IRC_NET_FRAME_FMT_RGB24, ///< RGB24
    IRC_NET_FRAME_FMT_RGBA, ///< RGBA
    IRC_NET_FRAME_FMT_BGRA, ///< BGRA
    IRC_NET_FRAME_FMT_GRAY ///< GRAY
} IRC_NET_FRAME_FMT;

/**
 * @brief 码流类型
 *
 */
typedef enum
{
    IRC_NET_STREAM_MAIN = 0, ///< 主码流
    IRC_NET_STREAM_SUB, ///< 辅码流
    IRC_NET_STREAM_SUN, ///< 孙码流
    IRC_NET_STREAM_FRAME /// < 私有帧
} IRC_NET_STREAM_TYPE;

/**
 * @brief 预览信息
 *
 */
typedef struct
{
    int channel; ///< 通道
    int streamType; ///< 码流类型，参考IRC_NET_STREAM_TYPE
    int frameFmt; ///< 帧格式，参考IRC_NET_FRAME_FMT
} IRC_NET_PREVIEW_INFO;

/**
 * @brief 视频回调
 *
 */
typedef void (* IRC_NET_VIDEO_CALLBACK)(IRC_NET_HANDLE handle, char* frame, int width, int height, void* userData);

/**
 * @brief 视频回调视频信息
 *
 */
typedef struct
{
    char* frame; ///< 帧数据
    int width; ///< 宽度
    int height; ///< 高度
    int validWidth; ///< 有效宽度
    int validHeight; ///< 有效高度
} IRC_NET_VIDEO_INFO_CB;

/**
 * @brief 视频回调智能信息
 *
 */
typedef struct
{
    int baseIvsInfoLen; ///< 基础智能信息长度
    char* baseIvsInfo; ///< 基础智能信息
    int tempIvsInfoLen; ///< 温度智能信息长度
    char* tempIvsInfo; ///< 温度智能信息
} IRC_NET_IVS_INFO_CB;

/**
 * @brief 视频回调
 *
 */
typedef void (*IRC_NET_VIDEO_CALLBACK_V2)(IRC_NET_HANDLE handle, IRC_NET_VIDEO_INFO_CB* videoInfo, IRC_NET_IVS_INFO_CB* ivsInfo, void* userData);

/**
 * @brief 坐标点
 *
 */
typedef struct
{
    int x; ///< x坐标
    int y; ///< y坐标
} IRC_NET_POINT;

/**
 * @brief 矩形
 *
 */
typedef struct
{
    int top; ///< 上
    int left; ///< 左
    int right; ///< 右
    int bottom; ///< 下
} IRC_NET_RECT;

/**
 * @brief 环境参数
 *
 */
typedef struct
{
    float atmosphereTemp; ///< 大气温度
    float distance; ///< 目标距离
    float emissivity; ///< 发射率 0.01-1
    float reflectedTemp; ///< 反射温度
    float transmittance; ///< 大气透过率 0.01-1
} IRC_NET_ENV_PARAM;

/**
 * @brief 测温报警规则类型
 *
 */
typedef enum
{
    IRC_NET_TEMP_ALARM_RULE_NONE = 0, ///< 无规则
    IRC_NET_TEMP_ALARM_RULE_AVG_TEMP_GT, ///< 平均温大于
    IRC_NET_TEMP_ALARM_RULE_AVG_TEMP_LT, ///< 平均温小于
    IRC_NET_TEMP_ALARM_RULE_HIGH_TEMP_GT, ///< 高温大于
    IRC_NET_TEMP_ALARM_RULE_HIGH_TEMP_LT, ///< 高温小于
    IRC_NET_TEMP_ALARM_RULE_LOW_TEMP_GT, ///< 低温大于
    IRC_NET_TEMP_ALARM_RULE_LOW_TEMP_LT ///< 低温小于
} IRC_NET_TEMP_ALARM_RULE_TYPE;

/**
 * @brief 测温报警规则
 *
 */
typedef struct
{
    int type; ///< 报警规则类型，参考IRC_NET_TEMP_ALARM_RULE_TYPE
    int debounce; ///< 去抖动时间，默认5s
    float thresholdTemp; ///< 温度阈值
    float toleranceTemp; ///< 容差温度
} IRC_NET_TEMP_ALARM_RULE_INFO;

/**
* @brief 测温规则类型
*
*/
typedef enum
{
    IRC_NET_TEMP_RULE_POINT = 0, ///< 点
    IRC_NET_TEMP_RULE_LINE, ///< 线
    IRC_NET_TEMP_RULE_RECT, ///< 矩形
    IRC_NET_TEMP_RULE_CIRCLE, ///< 圆
    IRC_NET_TEMP_RULE_POLYGON ///< 多边形
} IRC_NET_TEMP_RULE_TYPE;

/**
* @brief 色板类型
*
*/
typedef enum
{
    IRC_NET_PALETTE_WHITE_HOT = 0, ///< 白热
    IRC_NET_PALETTE_BLACK_HOT, ///< 黑热
    IRC_NET_PALETTE_RAINBOW, ///< 彩虹
    IRC_NET_PALETTE_RAINBOW_HC, ///< 高对比度彩虹
    IRC_NET_PALETTE_IRON, ///< 铁红
    IRC_NET_PALETTE_LAVA, ///< 熔岩
    IRC_NET_PALETTE_SKY, ///< 天空
    IRC_NET_PALETTE_MID_GREY, ///< 中灰
    IRC_NET_PALETTE_RDGY, ///< 灰红
    IRC_NET_PALETTE_PUOR, ///< 紫橙
    IRC_NET_PALETTE_SPECIAL, ///< 特殊
    IRC_NET_PALETTE_RED, ///< 警示红
    IRC_NET_PALETTE_ICE_FIRE, ///< 冰火
    IRC_NET_PALETTE_GREE_RED, ///< 青红
    IRC_NET_PALETTE_SPECIAL2, ///< 特殊2
    IRC_NET_PALETTE_RED_HOT, ///< 渐变红
    IRC_NET_PALETTE_GREEN_HOT, ///< 渐变绿
    IRC_NET_PALETTE_BLUE_HOT, ///< 渐变蓝
    IRC_NET_PALETTE_GREEN, ///< 警示绿
    IRC_NET_PALETTE_BLUE, ///警示蓝
} IRC_NET_PALETTE_TYPE;


/**
 * @brief 报警联动信息
 *
 */
typedef struct
{
    bool enable; ///< 使能开关
    bool channel[IRC_NET_CHANNEL_NUM_MAX]; ///< 联动通道
    int64_t delay; ///< 联动延时
} IRC_NET_ALARM_LINKAGE_INFO;

/**
 * @brief 报警联动集合
 *
 */
typedef struct
{
    IRC_NET_ALARM_LINKAGE_INFO snapshotLinkageInfo; ///< 抓图联动信息
    IRC_NET_ALARM_LINKAGE_INFO recordLinkageInfo; ///< 录像联动信息
} IRC_NET_ALARM_LINKAGE_SET;

/**
 * @brief 温度回调
 *
 */
typedef void (* IRC_NET_TEMP_CALLBACK)(IRC_NET_HANDLE handle, char* temp, int width, int height, void* userData);

/**
 * @brief 温度回调温度信息
 *
 */
typedef struct
{
    char* temp; ///< 帧数据
    int width; ///< 宽度
    int height; ///< 高度
} IRC_NET_TEMP_INFO_CB;

/**
 * @brief 温度回调扩展信息
 *
 */
typedef struct
{
    uint64_t utcTime; ///< UTC毫秒时间戳
    uint32_t emiss;//发射率
    uint32_t humidity;//
    uint32_t reflectTempK10;      //反射温度
    uint32_t envTempK10;          //大气温度
    uint32_t distance;           //距离
    uint32_t sensorTemp;
    int A0;
    int B0;
    int C0;
    int D0;
    int A1;
    int B1;
    int C1;
    int D1;
} IRC_NET_TEMP_EXT_INFO_CB;

/**
 * @brief 温度回调
 *
 */
typedef void (*IRC_NET_TEMP_CALLBACK_V2)(IRC_NET_HANDLE handle, IRC_NET_TEMP_INFO_CB* tempInfo, IRC_NET_TEMP_EXT_INFO_CB* extInfo, void* userData);

/**
 * @brief 测温规则信息
 *
 */
typedef struct
{
    bool enable; ///< 使能开关
    int presetId; ///< 预置点ID，从0开始
    int id; ///< 测温规则ID，从1开始，添加规则时无效
    char name[IRC_NET_NAME_LEN_MAX]; ///< 测温规则名称
    int type; ///< 测温规则类型，参考IRC_NET_TEMP_RULE_TYPE
    IRC_NET_POINT points[IRC_NET_RULE_POINT_NUM_MAX]; ///< 测温规则坐标点
    int pointNum; ///< 测温规则坐标点个数
    bool envParamEnable; ///< 环境参数使能开关
    IRC_NET_ENV_PARAM envParam; ///< 环境参数
    IRC_NET_TEMP_ALARM_RULE_INFO alarmRuleInfo; ///< 报警规则信息
    IRC_NET_ALARM_LINKAGE_SET alarmLinkageSet; ///< 报警联动
} IRC_NET_TEMP_RULE_INFO;

/**
 * @brief 测温规则索引
 * @note 测温规则类型与测温规则ID不可二者仅一个为-1
 *
 */
typedef struct
{
    bool enable; ///<使能开关
    int presetId; ///< 预置点ID，从0开始
    int type; ///< 测温规则类型，参考IRC_NET_TEMP_RULE_TYPE
    int id; ///< 测温规则ID，从1开始
} IRC_NET_TEMP_RULE_INDEX;

/**
 * @brief 测温报警规则类型（G1设备）
 *
 */
typedef enum
{
    IRC_NET_TEMP_ALARM_RULE_G1_NONE = 0, ///< 无规则
    IRC_NET_TEMP_ALARM_RULE_G1_HIGH_TEMP, ///< 高温
    IRC_NET_TEMP_ALARM_RULE_G1_LOW_TEMP, ///< 低温
    IRC_NET_TEMP_ALARM_RULE_G1_HIGH_LOW_TEMP ///< 高低温
} IRC_NET_TEMP_ALARM_RULE_TYPE_G1;

/**
 * @brief 测温报警阈值规则（G1设备）
 *
 */
typedef struct
{
    float thresholdTemp; ///< 温度阈值
    bool enable; ///< 等级使能开关
    float tempLevel1; ///< 一级
    float tempLevel2; ///< 二级
    float tempLevel3; ///< 三级
} IRC_NET_TEMP_ALARM_RULE_INFO_G1;

/**
 * @brief 测温规则信息（G1设备）
 *
 */
typedef struct
{
    int id; ///< 测温规则ID，从1开始，添加规则时无效
    int type; ///< 测温规则类型，参考IRC_NET_TEMP_RULE_TYPE
    IRC_NET_POINT startPoint; ///< 起点坐标
    IRC_NET_POINT endPoint; ///< 终点坐标
    int alarmType; ///< 报警规则类型，参考IRC_NET_TEMP_ALARM_RULE_TYPE_G1
    IRC_NET_TEMP_ALARM_RULE_INFO_G1 lowTempAlarmRuleInfo; ///< 低温报警规则信息
    IRC_NET_TEMP_ALARM_RULE_INFO_G1 highTempAlarmRuleInfo; ///< 高温报警规则信息
    int debounce; ///< 去抖动时间，默认5s
    bool alarmLinkageSnapshotEnable; ///< 报警联动抓图使能
}IRC_NET_TEMP_RULE_INFO_G1;

/**
 * @brief 温度信息
 *
 */
typedef struct
{
    float avgTemp; ///< 平均温
    float centerTemp; ///< 中心温
    float maxTemp; ///< 最高温
    float minTemp; ///< 最低温
    IRC_NET_POINT maxTempPoint;///< 最高温坐标
    IRC_NET_POINT minTempPoint;///< 最低温坐标
} IRC_NET_TEMP_INFO;

/**
 * @brief 规则温度信息
 *
 */
typedef struct
{
    IRC_NET_TEMP_RULE_INDEX ruleIndex; ///< 规则索引
    IRC_NET_TEMP_INFO tempInfo; ///< 温度信息
} IRC_NET_RULE_TEMP_INFO;

/**
 * @brief 整帧报警配置
 *
 */
typedef struct
{
    bool enable; ///< 使能开关
    IRC_NET_TEMP_ALARM_RULE_INFO alarmRuleInfo[IRC_NET_FRAME_TEMP_RULE_NUM_MAX]; ///< 报警规则信息
    IRC_NET_ALARM_LINKAGE_SET alarmLinkageSet; ///< 报警联动
} IRC_NET_FRAME_TEMP_ALARM_CONFIG;

/**
 * @brief 整帧报警配置（G1设备）
 *
 */
typedef struct
{
    int alarmType; ///< 报警规则类型，参考IRC_NET_TEMP_ALARM_RULE_TYPE_G1
    IRC_NET_TEMP_ALARM_RULE_INFO_G1 lowTempAlarmRuleInfo; ///< 低温报警阈值规则信息
    IRC_NET_TEMP_ALARM_RULE_INFO_G1 highTempAlarmRuleInfo; ///< 高温报警阈值规则信息
    int debounce; ///< 去抖动时间，默认5s
    bool alarmLinkageSnapshotEnable; ///< 报警联动抓图使能
} IRC_NET_FRAME_TEMP_ALARM_CONFIG_G1;

/**
 * @brief 测温屏蔽区域索引
 *
 */
typedef struct
{
    int presetId; ///< 预置点ID，从0开始
    int id; ///< 屏蔽区域ID，从1开始
} IRC_NET_TEMP_MASK_INDEX;

/**
 * @brief 测温屏蔽区域信息
 *
 */
typedef struct
{
    bool enable; ///< 使能开关
    IRC_NET_TEMP_MASK_INDEX index; ///< 屏蔽区域索引
    char name[IRC_NET_NAME_LEN_MAX]; ///< 屏蔽区域名称
    IRC_NET_POINT points[IRC_NET_MASK_POINT_NUM_MAX]; ///< 屏蔽区域坐标点
} IRC_NET_TEMP_MASK_INFO;

/**
* @brief 测温档位类型
*
*/
typedef enum
{
    IRC_NET_TEMP_LEVEL_HG = 0, ///< 高增益
    IRC_NET_TEMP_LEVEL_LG, ///< 低增益
    IRC_NET_TEMP_LEVEL_AUTO ///< 自动
} IRC_NET_TEMP_LEVEL_TYPE;

/**
 * @brief OSD时间标题信息
 *
 */
typedef struct
{
    bool enable; ///< 使能
    IRC_NET_RECT rect; ///< 标题位置，8192坐标系
} IRC_NET_OSD_TIME_TITLE_INFO;

/**
 * @brief OSD通道标题信息
 *
 */
typedef struct
{
    bool enable; ///< 使能
    char name[IRC_NET_NAME_LEN_MAX]; ///< 通道名称，UTF-8字符串，最大64个字符或汉字
    IRC_NET_RECT rect; ///< 标题位置，8192坐标系
} IRC_NET_OSD_CHANNEL_TITLE_INFO;

/**
 * @brief 温宽信息
 *
 */
typedef struct
{
    bool enable; ///< 使能
    float lowTemp; ///< 低温阈值
    float highTemp; ///< 高温阈值
} IRC_NET_TEMP_SPAN_INFO;

/**
 * @brief IP配置
 *
 */
typedef struct
{
    char name[IRC_NET_NAME_LEN_MAX]; ///< 网卡名称，只读
    bool dhcpEnable; ///< dhcp使能
    char mac[IRC_NET_MAC_LEN_MAX]; ///< MAC地址，只读
    char ip[IRC_NET_IP_LEN_MAX]; ///< ip地址
    char subnetMask[IRC_NET_IP_LEN_MAX]; ///< 子网掩码
    char gateway[IRC_NET_IP_LEN_MAX]; ///< 默认网关
    char defaultDns[IRC_NET_IP_LEN_MAX]; ///< 首选DNS
    char standbyDns[IRC_NET_IP_LEN_MAX]; ///< 备选DNS
} IRC_NET_IP_CONFIG;

/**
 * @brief 联动动作集合
 *
 */
typedef struct
{
    IRC_NET_ALARM_LINKAGE_INFO snapshotLinkageInfo; ///< 抓图联动信息
    IRC_NET_ALARM_LINKAGE_INFO recordLinkageInfo; ///< 录像联动信息
} IRC_NET_LINKAGE_SET;

/**
 * @brief 通道目标识别配置
 *
 */
typedef struct
{
    bool enable; ///< 使能
    int sensitivity; ///< 灵敏度，范围：0-100，默认是70
    IRC_NET_LINKAGE_SET linkageSet; ///< 联动动作
} IRC_NET_IP_TARGET_RECOGNITION_CONFIG;

/**
 * @brief 温度单位
 *
 */
typedef enum
{
    IRC_NET_TEMP_CENTIGRADE = 0, ///< 摄氏度
    IRC_NET_TEMP_FAHRENHEIT, ///< 华氏度
    IRC_NET_TEMP_KELVIN ///< 开尔文
} IRC_NET_TEMP_UNIT;

/**
 * @brief 报警行为类型
 *
 */
typedef enum
{
    IRC_NET_ALARM_ACTION_SINGLE = 0, ///< 单次脉冲报警
    IRC_NET_ALARM_ACTION_START, ///< 报警开始
    IRC_NET_ALARM_ACTION_END ///< 报警结束
} IRC_NET_ALARM_ACTION_TYPE;

/**
 * @brief 报警推送图片信息
 *
 */
typedef struct
{
    char* pictureInfo; ///< 图片数据信息
    int size; ///< 图片大小
} IRC_NET_ALARM_PICTURE_INFO;

/**
 * @brief 报警基础信息
 *
 */
typedef struct
{
    int channel; ///< 通道
    int64_t timestamp; ///< 时间戳
    int alarmAction; ///< 报警行为，参考IRC_NET_ALARM_ACTION
    IRC_NET_ALARM_PICTURE_INFO pictureInfos[IRC_NET_CHANNEL_NUM_MAX]; ///< 报警推送图片信息
} IRC_NET_ALARM_BASE_INFO;

/**
 * @brief 温度报警信息
 *
 */
typedef struct
{
    char name[IRC_NET_NAME_LEN_MAX]; ///< 测温规则名称
    IRC_NET_ALARM_BASE_INFO alarmBaseInfo; ///< 报警基础信息
    IRC_NET_RULE_TEMP_INFO ruleTempInfo; ///< 规则温度信息
    int tempUnit; ///< 温度单位，参考IRC_NET_TEMP_UNIT
    int alarmRuleType; ///< 报警规则类型，参考IRC_NET_TEMP_ALARM_RULE_TYPE
    float thresholdTemp; ///< 阈值温度
} IRC_NET_TEMP_ALARM_INFO;

/**
 * @brief 火点报警信息
 *
 */
typedef struct
{
    char name[IRC_NET_NAME_LEN_MAX]; ///< 火点规则名称
    int id; ///< 火点规则ID，从1开始
    IRC_NET_ALARM_BASE_INFO alarmBaseInfo; ///< 报警基础信息
    IRC_NET_RECT alarmPositionInfo; ///< 报警坐标信息
} IRC_NET_FIRE_ALARM_INFO;

/**
 * @brief 区域入侵报警信息
 *
 */
typedef struct
{
    char name[IRC_NET_NAME_LEN_MAX]; ///< 区域入侵规则名称
    int id; ///< 区域规则ID，从1开始
    IRC_NET_ALARM_BASE_INFO alarmBaseInfo; ///< 报警基础信息
} IRC_NET_REGION_INTRUSION_ALARM_INFO;

/**
 * @brief 绊线入侵报警信息
 *
 */
typedef struct
{
    char name[IRC_NET_NAME_LEN_MAX]; ///< 绊线入侵规则名称
    int id; ///< 绊线规则ID，从1开始
    IRC_NET_ALARM_BASE_INFO alarmBaseInfo; ///< 报警基础信息
} IRC_NET_LINE_INTRUSION_ALARM_INFO;

/**
 * @brief 温差报警温度规则
 *
 */
typedef struct
{
    char name[IRC_NET_NAME_LEN_MAX]; ///< 区域规则名称
    int id; ///< 区域规则ID，从1开始
    IRC_NET_TEMP_INFO tempInfo; ///< 温度信息
} IRC_NET_TEMP_DIFF_RULE_INFO;

/**
 * @brief 温差报警信息
 *
 */
typedef struct
{
    char name[IRC_NET_NAME_LEN_MAX]; ///< 温差规则名称
    int id; ///< 温差规则ID，从1开始
    int type; ///< 报警规则类型，参考IRC_NET_TEMP_ALARM_RULE_TYPE
    IRC_NET_ALARM_BASE_INFO alarmBaseInfo; ///< 报警基础信息
    IRC_NET_TEMP_DIFF_RULE_INFO firstTempDiffRuleInfo; ///< 区域1温度规则
    IRC_NET_TEMP_DIFF_RULE_INFO secondTempDiffRuleInfo; ///< 区域2温度规则
    float thresholdTemp; ///< 阈值温度
} IRC_NET_TEMP_DIFF_ALARM_INFO;

/**
 * @brief 烟雾报警信息
 *
 */
typedef struct
{
    char name[IRC_NET_NAME_LEN_MAX]; ///< 温差规则名称
    int id; ///< 温差规则ID，从1开始
    IRC_NET_ALARM_BASE_INFO alarmBaseInfo; ///< 报警基础信息
} IRC_NET_SMOKE_DETECT_ALARM_INFO;

/**
 * @brief 本地报警信息
 *
 */
typedef struct
{
    bool enable; ///< 报警使能
    int id; ///< 报警ID
    int debounce; ///< 去抖时间
    int sensorType; ///< 传感器类型
    IRC_NET_ALARM_BASE_INFO alarmBaseInfo; ///< 报警基础信息
} IRC_NET_LOCAL_ALARM_INFO;


/**
 * @brief 云台基本控制功能类型
 *
 */
typedef enum
{
    IRC_NET_PTZ_CMD_UP = 0, ///< 上
    IRC_NET_PTZ_CMD_DOWN, ///< 下
    IRC_NET_PTZ_CMD_LEFT, ///< 左
    IRC_NET_PTZ_CMD_RIGHT, ///< 右
    IRC_NET_PTZ_CMD_LEFT_TOP, ///< 左上
    IRC_NET_PTZ_CMD_RIGHT_TOP, ///< 右上
    IRC_NET_PTZ_CMD_LEFT_DOWN, ///< 左下
    IRC_NET_PTZ_CMD_RIGHT_DOWN, ///< 右下
    IRC_NET_PTZ_CMD_ZOOM_OUT, ///< 焦距变短
    IRC_NET_PTZ_CMD_ZOOM_IN, ///< 焦距变长
    IRC_NET_PTZ_CMD_FOCUS_NEAR, ///< 聚焦变近
    IRC_NET_PTZ_CMD_FOCUS_FAR, ///< 聚焦边远
    IRC_NET_PTZ_CMD_IRIS_CLOSE, ///< 光圈变小
    IRC_NET_PTZ_CMD_IRIS_OPEN, ///< 光圈变大
    IRC_NET_PTZ_CMD_WIPER, ///< 雨刷
    IRC_NET_PTZ_CMD_LIGHT, ///< 补光灯
    IRC_NET_PTZ_CMD_DEFROST, ///< 除霜
    IRC_NET_PTZ_CMD_DEFOG, ///< 透雾
    IRC_NET_PTZ_CMD_FAN, ///< 风扇
    IRC_NET_PTZ_CMD_HEATER, ///< 加热器
    IRC_NET_PTZ_CMD_AUTO_FOCUS, ///< 自动聚焦
    IRC_NET_PTZ_CMD_SYNC_VIEW, ///< 双向随动
} IRC_NET_PTZ_CMD_TYPE;

/**
 * @brief 云台控制信息
 *
 */
typedef struct
{
    int channel; ///< 通道
    int cmd; ///< 云台控制功能类型，参考IRC_NET_PTZ_CMD_TYPE
    int param1; ///< 参数1，详见文档云台命令参数
    int param2; ///< 参数2，详见文档云台命令参数
    int param3; ///< 参数3，详见文档云台命令参数
    int stop; ///< 是否停止，0-开始，1-停止
} IRC_NET_PTZ_CONTROL_INFO;

/**
 * @brief 云台辅助功能使能状态
 *
 */
typedef struct
{
    int wiperState; ///< 雨刷状态
    int lightState; ///< 补光灯状态
    int defrostState; ///< 除霜状态
    int defogState; ///< 透雾状态
    int fanState; ///< 风扇状态
    int heaterState; ///< 加热器状态
    int autoFocusState; ///< 自动聚焦状态
    int syncViewState; ///< 双向随动状态
} IRC_NET_PTZ_AUX_STATE;

/**
 * @brief 预置点信息
 *
 */
typedef struct
{
    bool enable;
    int id; ///< 预置点id，从0开始
    char name[IRC_NET_NAME_LEN_MAX]; ///< 预置点名称
} IRC_NET_PTZ_PRESET_INFO;

/**
 * @brief 预置点控制功能类型
 *
 */
typedef enum
{
    IRC_NET_PTZ_PRESET_CMD_ADD = 0, ///< 添加预置点
    IRC_NET_PTZ_PRESET_CMD_UPDATE_NAME, ///< 更新预置点名称
    IRC_NET_PTZ_PRESET_CMD_GOTO, ///< 移动至预置点
    IRC_NET_PTZ_PRESET_CMD_DELETE, ///< 删除单个预置点
    IRC_NET_PTZ_PRESET_CMD_DELETE_ALL, ///< 删除全部预置点
} IRC_NET_PTZ_PRESET_CMD_TYPE;

/**
 * @brief 巡航组预置点信息
 *
 */
typedef struct
{
    IRC_NET_PTZ_PRESET_INFO presetInfo; ///< 巡航组预置点信息
    int residenceTime; ///< 停留时间
} IRC_NET_PTZ_TOUR_PRESET_INFO;

/**
 * @brief 巡航组信息
 *
 */
typedef struct
{
    bool enable; ///< 使能
    int id; ///< 巡航组id，从1开始
    char name[IRC_NET_NAME_LEN_MAX]; ///< 巡航组点名称
    bool running; ///< 运行状态
    IRC_NET_PTZ_TOUR_PRESET_INFO presetInfos[IRC_NET_TOUR_PRESET_NUM_MAX]; ///< 巡航组预置点信息
    int presetNum; ///< 巡航组预置点个数
} IRC_NET_PTZ_TOUR_INFO;

/**
 * @brief 巡航组控制功能类型
 *
 */
typedef enum
{
    IRC_NET_PTZ_TOUR_CMD_SAVE = 0, ///< 保存巡航组
    IRC_NET_PTZ_TOUR_CMD_START, ///< 开始巡航
    IRC_NET_PTZ_TOUR_CMD_STOP, ///< 停止巡航
    IRC_NET_PTZ_TOUR_CMD_DELETE, ///< 删除单个巡航组
    IRC_NET_PTZ_TOUR_CMD_DELETE_ALL ///< 删除全部巡航组
} IRC_NET_PTZ_TOUR_CMD_TYPE;

/**
 * @brief 巡迹信息
 *
 */
typedef struct
{
    bool enable; ///< 使能
    int id; ///< 巡迹id，范围[1，5]
    int running; ///< 运行状态，0-停止，1-正在记录，2-正在巡迹
} IRC_NET_PTZ_PATTERN_INFO;

/**
 * @brief 巡迹控制功能类型
 *
 */
typedef enum
{
    IRC_NET_PTZ_PATTERN_CMD_START_RECOED = 0, ///< 开始记录
    IRC_NET_PTZ_PATTERN_CMD_STOP_RECOED, ///< 结束记录
    IRC_NET_PTZ_PATTERN_CMD_START_REPLAY, ///< 开始重放
    IRC_NET_PTZ_PATTERN_CMD_STOP_REPLAY, ///< 结束重放
    IRC_NET_PTZ_PATTERN_CMD_DELETE_ALL ///< 删除全部巡迹
} IRC_NET_PTZ_PATTERN_CMD_TYPE;

/**
 * @brief 云台精确定位配置
 *
 */
typedef struct
{
    float pan; ///< 水平角度
    float tilt; ///< 垂直角度
    float zoom; ///< 变倍
} IRC_NET_PTZ_POSITION_PARAM;

/**
 * @brief 云台精确定位配置V1
 *
 */
typedef struct
{
    float pan; ///< 水平角度
    float tilt; ///< 垂直角度
    float visZoom; ///< 可见光变倍
    float irZoom; ///< 红外变倍
    float visFovH; ///< 可见光水平视场角
    float visFovV; ///< 可见光垂直视场角
    float irFovH; ///< 红外水平视场角
    float irFovV; ///< 红外垂直视场角
} IRC_NET_PTZ_POSITION_PARAM_V1;

/**
 * @brief 云台精确跟踪定位配置
 *
 */
typedef struct
{
    float panSpeed; ///< 水平转速 0-655.35
    float tiltSpeed; ///< 垂直转速 0-655.35
    int derection; ///< 方向 0x00 顺时针向下 0x01 顺时针向上 0x10逆时针向下 0x11 逆时针向上
} IRC_NET_PTZ_TRACKING_POSITION_PARAM; 

/**
 * @brief 3D定位配置
 *
 */
typedef struct
{
    int channel; ///< 通道
    IRC_NET_POINT startPoint; ///< 起始点
    IRC_NET_POINT endPoint; ///< 结束点
} IRC_NET_PTZ_3D_POSITION_PARAM;

/**
 * @brief 区域聚焦配置
 *
 */
typedef struct
{
    int channel; ///< 通道
    IRC_NET_POINT startPoint; ///< 起始点
    IRC_NET_POINT endPoint; ///< 结束点
    int stop; ///< 是否停止，0-开始，1-停止
} IRC_NET_PTZ_REGION_FOCUS_PARAM;

/**
 * @brief 手动跟踪配置
 *
 */
typedef struct
{
    int channel; ///< 通道
    IRC_NET_POINT startPoint; ///< 起始点
    IRC_NET_POINT endPoint; ///< 结束点
    int stop; ///< 是否停止，0-开始，1-停止
} IRC_NET_PTZ_MANUAL_TRACK_PARAM;

/**
 * @brief 文件类型
 *
 */
typedef enum
{
    IRC_NET_FILE_PICTURE = 0, ///< 图像
    IRC_NET_FILE_VIDEO ///< 视频
} IRC_NET_FILE_TYPE;

/**
 * @brief 文件查询类型
 *
 */
typedef enum 
{
    IRC_NET_FILE_QUERY_ALL = 0, ///< 查询所有
    IRC_NET_FILE_QUERY_ALARM, ///< 查询报警
    IRC_NET_FILE_QUERY_COMMON, ///< 查询普通
} RC_NET_FILE_QUERY_TYPE;

/**
 * @brief 查询参数
 *
 */
typedef struct
{
    int channel; ///< 通道
    int type; ///< 文件类型，参考IRC_NET_FILE_TYPE
    int queryType; ///< 查询类型，参考RC_NET_FILE_QUERY_TYPE
    char startTime[IRC_NET_TIME_LEN_MAX]; ///< 开始时间
    char endTime[IRC_NET_TIME_LEN_MAX]; ///< 结束时间
    int count; ///< 期望返回数目，默认50
    int offset; ///< 查询位置偏移量，初次填0
} IRC_NET_FILE_QUERY_PARAM;

/**
 * @brief 文件信息
 *
 */
typedef struct
{
    int id; ///< id序号
    int channel; ///< 通道
    int type; ///< 文件类型，参考IRC_NET_FILE_TYPE
    char startTime[IRC_NET_TIME_LEN_MAX]; ///< 开始时间
    char endTime[IRC_NET_TIME_LEN_MAX]; ///< 结束时间
    char path[IRC_NET_FILE_PATH_LEN_MAX]; ///< 文件路径
} IRC_NET_FILE_INFO;

/**
 * @brief 文件下载信息
 *
 */
typedef struct
{
    IRC_NET_FILE_INFO fileInfo;
    char downloadPath[IRC_NET_FILE_PATH_LEN_MAX]; ///< 下载路径
} IRC_NET_FILE_DOWNLOAD_INFO;

/**
 * @brief 文件下载进度
 *
 */
typedef struct
{
    int64_t totalSize; ///< 下载总长度，单位：字节
    int64_t downloadSize; ///< 已下载长度，单位：字节
    int error; ///< 下载错误，0-正常，-1-异常
} IRC_NET_FILE_DOWNLOAD_PROGRESS;

/**
 * @brief 转台基本控制功能类型
 *
 */
typedef enum
{
    IRC_NET_SWIVEL_CMD_UP = 0, ///< 上仰
    IRC_NET_SWIVEL_CMD_DOWN ///< 下俯
} IRC_NET_SWIVEL_CMD_TYPE;

/**
 * @brief 区域扫描参数信息
 *
 */
typedef struct
{
    int id; ///< 区域编号
    bool enable; ///< 区域扫描使能
    int derection; ///< 区域扫描方向，1-顺时针，-1-逆时针
    int speed; ///< 扫描速度
    int startPresetId; ///< 开始预置点
    int stopPresetId; ///< 结束预置点
    float tiltStepAngle; ///< 梯度值
} IRC_NET_REGION_SCAN_INFO;

/**
 * @brief 动作类型
 *
 */
typedef enum
{
    IRC_NET_ACTION_AUTO_RESET = 0, ///< 自动归位
    IRC_NET_ACTION_PRESET, ///< 预置点
    IRC_NET_ACTION_TOUR, ///< 巡航组
    IRC_NET_ACTION_PATTERN, ///< 巡迹
    IRC_NET_ACTION_LINE_SCAN, ///< 线性扫描
    IRC_NET_ACTION_AREA_SCAN, ///< 区域扫描  
} IRC_NET_ACTION_TYPE;

/**
 * @brief 开机动作参数信息
 *
 */
typedef struct
{
    bool enable; ///< 开机动作使能
    int actionType; ///< 开机动作类型，参考IRC_NET_ACTION_TYPE
    int lineScanId; ///< 线性扫描id
    int regionScanId; ///< 区域扫描id
    int presetId; ///< 预置点id
    int patternId; ///< 巡迹id
    int tourId; ///< 巡航组id
}IRC_NET_BOOT_ACTION_INFO;

/**
 * @brief 空闲动作参数信息
 *
 */
typedef struct
{
    bool enable; ///< 空闲动作使能
    int actionType; ///< 空闲动作类型，参考IRC_NET_ACTION_TYPE 但是 0 为None
    int lineScanId; ///< 线性扫描id
    int regionScanId; ///< 区域扫描id
    int presetId; ///< 预置点id
    int patternId; ///< 巡迹id
    int tourId; ///< 巡航组id
    int runningFunction; ///< 参考IRC_NET_ACTION_TYPE 但是 0 为None
    int second; ///< 空闲时间
    bool running; ///< 运行状态
}IRC_NET_PARK_ACTION_INFO;

/**
 * @brief 激光测距OSD标题格式
 *
 */
typedef struct
{
    int alignType; ///< 对齐方式 0-左对齐，1-右对齐
    int fontSize; ///< 字体大小 0-小，1-中，2-大
    int autoTurn; ///< 反色处理 0-不处理，1-自动反色，2-勾边
    int bgColor[IRC_NET_COLOR_TYPE_MAX]; ///< 背景颜色
    int fgColor[IRC_NET_COLOR_TYPE_MAX]; ///< 前景颜色
    IRC_NET_RECT titlePosition; ///< 标题位置
}IRC_NET_LASER_DISTANCE_OSD_TITLE_FORMAT;

/**
 * @brief 激光测距OSD参数
 *
 */
typedef struct
{
    bool enable; ///< 使能
    int rangingDuration; ///< 单次测距持续时间
    IRC_NET_LASER_DISTANCE_OSD_TITLE_FORMAT titleFormat; ///< 激光测距OSD标题格式
}IRC_NET_LASER_DISTANCE_OSD_PARAM;

/**
 * @brief 补光灯配置信息
 *
 */
typedef struct
{
    int fillLightMode; ///< 补光灯模式 0-自动，1-手动
    int infraredLight; ///< 红外灯控制 0-100
    int whiteLight; ///< 白光灯灯控制 0-100
    int hFov; ///< 补光灯水平视场角 0-100
    float hFovFactor; ///< 补光灯水平视场角角度系数
}IRC_NET_FILL_LIGHT_CONFIG_INFO;

/**
 * @brief 红外温度点坐标信息
 *
 */
typedef struct
{
    float temp; ///< 点温度
    IRC_NET_POINT point; ///< 温度点坐标 
}IRC_NET_IR_TEMP_POINT_INFO;

/**
 * @brief 加速度坐标轴数据
 *
 */
typedef struct
{
    float x; ///< x轴数据
    float y; ///< y轴数据
    float z; ///< z轴数据  
}IRC_NET_COORDINATE_AXIS_DATA;

/**
 * @brief 加速度角度数据
 *
 */
typedef struct
{
    IRC_NET_COORDINATE_AXIS_DATA coordinateData; ///< 坐标轴数据
    float pan; ///< 云台的 P 值
    float tilt; ///< 云台的 T 值
    float rt; ///< 加速度计修正后的 T 值
}IRC_NET_ANGLE_DATA;

/**
 * @brief 加速度数据
 *
 */
typedef struct
{
    IRC_NET_ANGLE_DATA angleData; ///< 加速度角度数据;
    IRC_NET_COORDINATE_AXIS_DATA accelerationData; ///< 各个轴的加速度
    IRC_NET_COORDINATE_AXIS_DATA rotationData; ///< 各个轴的旋转角加速度
}IRC_NET_ACCELERATION_DATA;

/**
 * @brief 目标跟踪配置信息
 *
 */
typedef struct
{
    bool enable; ///< 使能;
    IRC_NET_ALARM_LINKAGE_INFO snapshotLinkageInfo; ///< 抓图联动信息
    IRC_NET_ALARM_LINKAGE_INFO recordLinkageInfo; ///< 录像联动信息
}IRC_NET_TARGET_TRACK_DATA;

/**
 * @brief 可见光日夜模式参数
 *
 */
typedef struct
{
    int dayNightMode; ///< 日夜切换模式，0-自动， 1-白天, 2-黑夜, 3-时间段;
    int delay; ///< 延迟时间
    int start; ///< 白天时间段开始时间, 单位秒
    int end; ///< 白天时间段结束时间, 单位秒
    int sensitivity; ///< 灵敏度范围1-7
}IRC_NET_DAY_NIGHT_MODE_PARAM;

/**
 * @brief 视频等级
 *
 */
typedef struct
{
    bool enable; ///< 使能;
    int level; ///< 等级;
}IRC_NET_VIDEOIN_LEVEL;

/**
 * @brief 区域视频增强范围
 *
 */
typedef struct
{
    int level; ///< 等级;
    IRC_NET_POINT region[2];///< 区域坐标;
}IRC_NET_REGIONAL_VIDEO_ENHANCE_RANGE;

/**
 * @brief 区域视频增强
 *
 */
typedef struct
{
    bool enable; ///< 使能;
    IRC_NET_REGIONAL_VIDEO_ENHANCE_RANGE range;///< 区域视频增强范围;
}IRC_NET_REGIONAL_VIDEO_ENHANCE;

/**
 * @brief 热成像图像增强信息
 *
 */
typedef struct
{
    bool brightMutationSuppression; ///< 亮度突变抑制;
    IRC_NET_VIDEOIN_LEVEL denoise2D; ///< 2D降噪
    IRC_NET_VIDEOIN_LEVEL denoise3D; ///< 3D降噪
    IRC_NET_VIDEOIN_LEVEL detailEnhance;///< 细节增强
    IRC_NET_REGIONAL_VIDEO_ENHANCE regionalVideoEnhance;///< 区域视频增强
    int flipMode;///< 翻转模式
}IRC_NET_THERMAL_IMAGE_MODE_ENHANCE_INFO;

/**
 * @brief 温度坐标信息
 *
 */
typedef struct
{
    float temp; ///< 温度数据
    IRC_NET_POINT point;///< 坐标信息
}IRC_NET_TEMP_POINT_INFO;

#endif // IRCNETSDKDEF_H
