#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
//#include <sourcemod-misc>
#include <chat-processor>

//ConVar cvar_Status;

public Plugin myinfo =
{
	name = "Test Messages",
	author = "Keith Warren (Shaders Allen)",
	description = "Tests the Chat-Processor plugin.",
	version = "1.0.0",
	url = "http://www.github.com/shadersallen"
};

public void OnPluginStart()
{
	LogMessage("ONLY RUN THIS PLUGIN IF YOU WANT TO TEST THE FORWARDS FOR CHAT-PROCESSOR!");
	/*cvar_Status = CreateConVar("sm_chatprocessor_testmessages", "0", "Status for this plugin.", FCVAR_NOTIFY, true, 0.0, true, 1.0);

	int shaders = GetShaders();

	if (GetUserMessageType() == UM_Protobuf)
	{
		PbSayText2(shaders, shaders, true, "This is a test message.");
	}
	else
	{
		SayText2(shaders, shaders, true, "This is a test message.", "1");
	}*/
}

/*public Action CP_OnChatMessage(int& author, ArrayList recipients, char[] flagstring, char[] name, char[] message, bool& processcolors, bool& removecolors)
{
	if (!GetConVarBool(cvar_Status))
	{
		return Plugin_Continue;
	}

	Format(name, MAXLENGTH_NAME, "{green}[Test] {red}%s", name);
	Format(message, MAXLENGTH_MESSAGE, "{green}%s", message);

	return Plugin_Changed;
}

void PbSayText2(int client, int author = 0, bool bWantsToChat = false, const char[] szFormat, any ...)
{
	char szSendMsg[192];
	VFormat(szSendMsg, sizeof(szSendMsg), szFormat, 5);
	StrCat(szSendMsg, sizeof(szSendMsg), "\n");

	Handle pb = StartMessageOne("SayText2", client);

	if (pb != null)
	{
		PbSetInt(pb, "ent_idx", author);
		PbSetBool(pb, "chat", bWantsToChat);
		PbSetString(pb, "msg_name", szSendMsg);
		PbAddString(pb, "params", "");
		PbAddString(pb, "params", "");
		PbAddString(pb, "params", "");
		PbAddString(pb, "params", "");
		EndMessage();
	}
}

void SayText2(int to, int from, bool chat, const char[] param1, const char[] param2)
{
	Handle hBf = StartMessageOne("SayText2", to);

	BfWriteByte(hBf, from);
	BfWriteByte(hBf, chat);
	BfWriteString(hBf, param1);
	BfWriteString(hBf, param2);
	EndMessage();
}
*/
