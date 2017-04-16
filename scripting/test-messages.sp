#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <chat-processor>

ConVar cvar_Status;

public Plugin myinfo =
{
	name = "Test Messages",
	author = "Keith Warren (Drixevel)",
	description = "Tests the Chat-Processor plugin.",
	version = "1.0.0",
	url = "http://www.drixevel.com/"
};

public void OnPluginStart()
{
	LogMessage("ONLY RUN THIS PLUGIN IF YOU WANT TO TEST THE FORWARDS FOR CHAT-PROCESSOR!");
	cvar_Status = CreateConVar("sm_chatprocessor_testmessages", "0", "Status for this plugin.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
}

public Action CP_OnChatMessage(int& author, ArrayList recipients, char[] flagstring, char[] name, char[] message, bool& processcolors, bool& removecolors)
{
	if (!GetConVarBool(cvar_Status))
	{
		return Plugin_Continue;
	}

	Format(name, MAXLENGTH_NAME, "[Test] {red}%s", name);
	Format(message, MAXLENGTH_MESSAGE, "{green}%s", message);

	return Plugin_Changed;
}
