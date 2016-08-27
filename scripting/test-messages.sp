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

public Action OnChatMessage(int& author, Handle recipients, eChatFlags& flag, char[] name, char[] message, bool& bProcessColors, bool& bRemoveColors)
{
	Format(name, MAXLENGTH_NAME, "{red}%s", name);
	Format(message, MAXLENGTH_MESSAGE, "{blue}%s", message);

	for (int i = 0; i < GetArraySize(recipients); i++)
	{
		int client = GetArrayCell(recipients, i);
		PrintToServer("Array Index %i: %N", i, client);
	}

	return Plugin_Changed;
}

public void OnChatMessagePost(int author, Handle recipients, eChatFlags flag, const char[] name, const char[] message, bool bProcessColors, bool bRemoveColors)
{

}