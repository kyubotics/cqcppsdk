DEF_EVENT(initialize, )
DEF_EVENT(enable, )
DEF_EVENT(disable, )
DEF_EVENT(coolq_start, )
DEF_EVENT(coolq_exit, )

DEF_EVENT(private_message, const PrivateMessageEvent &)
DEF_EVENT(group_message, const GroupMessageEvent &)
DEF_EVENT(discuss_message, const DiscussMessageEvent &)

DEF_EVENT(group_upload, const GroupUploadEvent &)
DEF_EVENT(group_admin, const GroupAdminEvent &)
DEF_EVENT(group_member_decrease, const GroupMemberDecreaseEvent &)
DEF_EVENT(group_member_increase, const GroupMemberIncreaseEvent &)
DEF_EVENT(group_ban, const GroupBanEvent &)
DEF_EVENT(friend_add, const FriendAddEvent &)

DEF_EVENT(friend_request, const FriendRequestEvent &)
DEF_EVENT(group_request, const GroupRequestEvent &)
