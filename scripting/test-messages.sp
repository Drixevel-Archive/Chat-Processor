#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <chat-processor>

public Plugin myinfo =
{
	name = "Test Messages",
	author = "Keith Warren (Drixevel)",
	description = "Tests the Chat-Processor plugin.",
	version = "1.0.1",
	url = "http://www.drixevel.com/"
};

public void OnPluginStart()
{
	LogMessage("ONLY RUN THIS PLUGIN IF YOU WANT TO TEST THE FORWARDS FOR CHAT-PROCESSOR!");
}

public Action OnChatMessage(int& author, ArrayList recipients, eChatFlags& flag, char[] name, char[] message, bool& bProcessColors, bool& bRemoveColors)
{
	Format(message, MAXLENGTH_MESSAGE, "{lightgreen}%s", message);
	return Plugin_Changed;
}

public void OnChatMessagePost(int author, ArrayList recipients, eChatFlags flag, const char[] name, const char[] message, bool bProcessColors, bool bRemoveColors)
{
	PrintToServer("[RESULTS] %i - %s: %s [Process Colors: %s / Remove Colors: %s]", author, name, message, bProcessColors ? "True" : "False", bRemoveColors ? "True" : "False");
}
