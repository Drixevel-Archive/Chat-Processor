//Pragma
#pragma semicolon 1
#pragma newdecls required

//Defines
#define PLUGIN_NAME "Test Messages"
#define PLUGIN_AUTHOR "Keith Warren (Drixevel)"
#define PLUGIN_DESCRIPTION "Tests the Chat-Processor plugin."
#define PLUGIN_VERSION "1.0.0"
#define PLUGIN_CONTACT "http://www.drixevel.com/"

//Includes
#include <sourcemod>
#include <chat-processor>

public Plugin myinfo = 
{
	name = PLUGIN_NAME, 
	author = PLUGIN_AUTHOR, 
	description = PLUGIN_DESCRIPTION, 
	version = PLUGIN_VERSION, 
	url = PLUGIN_CONTACT
};

public Action OnChatMessage(int& author, ArrayList recipients, eChatFlags& flag, char[] name, char[] message, bool& bProcessColors, bool& bRemoveColors)
{
	Format(message, MAXLENGTH_MESSAGE, "{red}%s 2", message);
	return Plugin_Changed;
}

public void OnChatMessagePost(int author, ArrayList recipients, eChatFlags flag, const char[] name, const char[] message, bool bProcessColors, bool bRemoveColors)
{
	//PrintToServer("[TEST] %s: %s [%b/%b]", name, message, bProcessColors, bRemoveColors);
	//PrintToChatAll("%s: %s 222", name, message);
}