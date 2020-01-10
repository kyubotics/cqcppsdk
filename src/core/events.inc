DEF_EVENT(initialize, ) // 初始化
DEF_EVENT(enable, ) // 应用启用
DEF_EVENT(disable, ) // 应用停用
DEF_EVENT(coolq_start, ) // 酷Q启动
DEF_EVENT(coolq_exit, ) // 酷Q退出

DEF_EVENT(message, const MessageEvent &) // 所有消息事件

DEF_EVENT(private_message, const PrivateMessageEvent &) // 私聊消息事件
DEF_EVENT(group_message, const GroupMessageEvent &) // 群消息事件
DEF_EVENT(discuss_message, const DiscussMessageEvent &) // 讨论组消息事件

DEF_EVENT(notice, const NoticeEvent &) // 所有通知事件

DEF_EVENT(group_upload, const GroupUploadEvent &) // 群文件上传事件
DEF_EVENT(group_admin, const GroupAdminEvent &) // 群管理员变动事件
DEF_EVENT(group_member_decrease, const GroupMemberDecreaseEvent &) // 群成员增加事件
DEF_EVENT(group_member_increase, const GroupMemberIncreaseEvent &) // 群成员减少事件
DEF_EVENT(group_ban, const GroupBanEvent &) // 群禁言事件
DEF_EVENT(friend_add, const FriendAddEvent &) // 好友添加事件

DEF_EVENT(request, const RequestEvent &) // 所有请求事件

DEF_EVENT(friend_request, const FriendRequestEvent &) // 好友请求事件
DEF_EVENT(group_request, const GroupRequestEvent &) // 群请求事件
