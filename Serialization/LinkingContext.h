#pragma once

#include <cstdint>
#include <unordered_map>

class ChatObject;

class LinkingContext
{
public:
	uint32_t GetNetworkId(const ChatObject* inChatObject)
	{
		auto it = std::find_if(mNetworkIdToChatObjectMap.begin(), mNetworkIdToChatObjectMap.end(),
			[&inChatObject](const std::pair<uint32_t, ChatObject*>& pair) -> bool {
				return pair.second == inChatObject;
			});
			
		return (it != mNetworkIdToChatObjectMap.end()) ? it->first : 0;
	}

	ChatObject* GetChatObject(uint32_t inNetworkId) const
	{
		auto it = mNetworkIdToChatObjectMap.find(inNetworkId);
		return (it != mNetworkIdToChatObjectMap.end()) ? it->second : nullptr;
	}

	void AddChatObject(ChatObject* inChatObject, uint32_t inNetworkId)
	{
		mNetworkIdToChatObjectMap[inNetworkId] = inChatObject;
	}

	void RemoveChatObject(ChatObject* inChatObject)
	{
		RemoveChatObject(GetNetworkId(inChatObject));
	}

	void RemoveChatObject(uint32_t inNetworkId)
	{
		mNetworkIdToChatObjectMap.erase(inNetworkId);
	}

private:
	std::unordered_map<uint32_t, ChatObject*> mNetworkIdToChatObjectMap;
};
