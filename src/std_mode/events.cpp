/**
 * 本文件用于定义提供给酷Q调用的事件处理函数, 参数类型和参数名保持与酷Q原生一致.
 */

#include "../core/common.h"

#include "../core/api.h"
#include "../core/event.h"
#include "../core/event_callback.h"
#include "../core/event_export_def.h"
#include "../core/init.h"
#include "../utils/function.h"
#include "../utils/string.h"

using namespace std;
using namespace cq;
using utils::call_all;
using utils::string_from_coolq;

#pragma region Lifecycle

/**
 * 返回 API 版本和 App Id.
 */
__CQ_EVENT(const char *, AppInfo, 0)
() {
    return "9," APP_ID;
}

/**
 * 生命周期: 初始化.
 */
__CQ_EVENT(int32_t, Initialize, 4)
(const int32_t auth_code) {
    __ac = auth_code;
    __init();
    __init_api();
    call_all(_initialize_callbacks);
    return 0;
}

/**
 * 生命周期: 应用启用.
 */
__CQ_EVENT(int32_t, cq_event_enable, 0)
() {
    call_all(_enable_callbacks);
    return 0;
}

/**
 * 生命周期: 应用停用.
 */
__CQ_EVENT(int32_t, cq_event_disable, 0)
() {
    call_all(_disable_callbacks);
    return 0;
}

/**
 * 生命周期: 酷Q启动.
 */
__CQ_EVENT(int32_t, cq_event_coolq_start, 0)
() {
    call_all(_coolq_start_callbacks);
    return 0;
}

/**
 * 生命周期: 酷Q退出.
 */
__CQ_EVENT(int32_t, cq_event_coolq_exit, 0)
() {
    call_all(_coolq_exit_callbacks);
    return 0;
}

#pragma endregion

#pragma region Message

/**
 * Type=21 私聊消息
 * sub_type 子类型，11/来自好友 1/来自在线状态 2/来自群 3/来自讨论组
 */
__CQ_EVENT(int32_t, cq_event_private_message, 24)
(int32_t sub_type, int32_t msg_id, int64_t from_qq, const char *msg, int32_t font) {
    auto e = PrivateMessageEvent(static_cast<int64_t>(msg_id),
                                 string_from_coolq(msg),
                                 font,
                                 from_qq,
                                 static_cast<PrivateMessageEvent::SubType>(sub_type));
    call_all(_private_message_callbacks, e);
    call_all(_message_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

/**
 * Type=2 群消息
 */
__CQ_EVENT(int32_t, cq_event_group_message, 36)
(int32_t sub_type, int32_t msg_id, int64_t from_group, int64_t from_qq, const char *from_anonymous_base64,
 const char *msg, int32_t font) {
    Anonymous anonymous;
    try {
        anonymous = ObjectHelper::from_base64<Anonymous>(from_anonymous_base64);
    } catch (ParseError &) {
    }
    auto e = GroupMessageEvent(
        static_cast<int64_t>(msg_id), string_from_coolq(msg), font, from_qq, from_group, std::move(anonymous));
    call_all(_group_message_callbacks, e);
    call_all(_message_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

/**
 * Type=4 讨论组消息
 */
__CQ_EVENT(int32_t, cq_event_discuss_message, 32)
(int32_t sub_type, int32_t msg_id, int64_t from_discuss, int64_t from_qq, const char *msg, int32_t font) {
    auto e = DiscussMessageEvent(static_cast<int64_t>(msg_id), string_from_coolq(msg), font, from_qq, from_discuss);
    call_all(_discuss_message_callbacks, e);
    call_all(_message_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

#pragma endregion

#pragma region Notice

/**
 * Type=11 群事件-文件上传
 */
__CQ_EVENT(int32_t, cq_event_group_upload, 28)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *file_base64) {
    File file;
    try {
        file = ObjectHelper::from_base64<File>(file_base64);
    } catch (ParseError &) {
    }
    auto e = GroupUploadEvent(from_qq, from_group, std::move(file));
    call_all(_group_upload_callbacks, e);
    call_all(_notice_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

/**
 * Type=101 群事件-管理员变动
 * sub_type 子类型，1/被取消管理员 2/被设置管理员
 */
__CQ_EVENT(int32_t, cq_event_group_admin, 24)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t being_operate_qq) {
    auto e = GroupAdminEvent(being_operate_qq, from_group, static_cast<GroupAdminEvent::SubType>(sub_type));
    call_all(_group_admin_callbacks, e);
    call_all(_notice_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

/**
 * Type=102 群事件-群成员减少
 * sub_type 子类型，1/群员离开 2/群员被踢 3/自己(即登录号)被踢
 * from_qq 操作者QQ(仅subType为2、3时存在)
 * being_operate_qq 被操作QQ
 */
__CQ_EVENT(int32_t, cq_event_group_member_decrease, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    using SubType = GroupMemberDecreaseEvent::SubType;
    auto e = GroupMemberDecreaseEvent(being_operate_qq, from_group, from_qq, static_cast<SubType>(sub_type));
    if (being_operate_qq == get_login_user_id() && e.sub_type == SubType::KICK) {
        e.sub_type = SubType::KICK_ME;
    }
    if (e.sub_type == SubType::LEAVE) {
        e.operator_id = e.user_id; // 主动退群, 操作者是退群者自己
    }
    call_all(_group_member_decrease_callbacks, e);
    call_all(_notice_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

/**
 * Type=103 群事件-群成员增加
 * sub_type 子类型，1/管理员已同意 2/管理员邀请
 * from_qq 操作者QQ(即管理员QQ)
 * being_operate_qq 被操作QQ(即加群的QQ)
 */
__CQ_EVENT(int32_t, cq_event_group_member_increase, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq) {
    auto e = GroupMemberIncreaseEvent(
        being_operate_qq, from_group, from_qq, static_cast<GroupMemberIncreaseEvent::SubType>(sub_type));
    call_all(_group_member_increase_callbacks, e);
    call_all(_notice_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

/**
 * Type=104 群事件-群禁言
 * sub_type 子类型，1/被解禁 2/被禁言
 * from_group 来源群号
 * from_qq 操作者QQ(即管理员QQ)
 * being_operate_qq 被操作QQ(若为全群禁言/解禁，则本参数为 0)
 * duration 禁言时长(单位 秒，仅子类型为2时可用)
 */
__CQ_EVENT(int32_t, cq_event_group_ban, 40)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, int64_t being_operate_qq, int64_t duration) {
    auto e =
        GroupBanEvent(being_operate_qq, from_group, from_qq, static_cast<GroupBanEvent::SubType>(sub_type), duration);
    call_all(_group_ban_callbacks, e);
    call_all(_notice_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

/**
 * Type=201 好友事件-好友添加
 */
__CQ_EVENT(int32_t, cq_event_friend_add, 16)
(int32_t sub_type, int32_t send_time, int64_t from_qq) {
    auto e = FriendAddEvent(from_qq);
    call_all(_friend_add_callbacks, e);
    call_all(_notice_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

#pragma endregion

#pragma region Request

/**
 * Type=301 请求-好友添加
 * msg 附言
 * response_flag 反馈标识(处理请求用)
 */
__CQ_EVENT(int32_t, cq_event_friend_request, 24)
(int32_t sub_type, int32_t send_time, int64_t from_qq, const char *msg, const char *response_flag) {
    auto e = FriendRequestEvent(string_from_coolq(msg), {string_from_coolq(response_flag)}, from_qq);
    call_all(_friend_request_callbacks, e);
    call_all(_request_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

/**
 * Type=302 请求-群添加
 * sub_type 子类型，1/他人申请入群 2/自己(即登录号)受邀入群
 * msg 附言
 * response_flag 反馈标识(处理请求用)
 */
__CQ_EVENT(int32_t, cq_event_group_request, 32)
(int32_t sub_type, int32_t send_time, int64_t from_group, int64_t from_qq, const char *msg, const char *response_flag) {
    auto e = GroupRequestEvent(string_from_coolq(msg),
                               {string_from_coolq(response_flag)},
                               from_qq,
                               from_group,
                               static_cast<GroupRequestEvent::SubType>(sub_type));
    call_all(_group_request_callbacks, e);
    call_all(_request_callbacks, e);
    return static_cast<int32_t>(e.operation);
}

#pragma endregion
