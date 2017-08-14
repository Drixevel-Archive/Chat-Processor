#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <chat-processor>
#include <cp-scp-wrapper>

ConVar cvar_Status;

Handle g_hForward_OnChatMessage;
Handle g_hForward_OnChatMessage_Post;

#define CHATFLAGS_INVALID		0
#define CHATFLAGS_ALL			(1<<0)
#define CHATFLAGS_TEAM			(1<<1)
#define CHATFLAGS_SPEC			(1<<2)
#define CHATFLAGS_DEAD			(1<<3)

int g_iMessageFlag = CHATFLAGS_INVALID;

public Plugin myinfo =
{
	name = "Chat Processor - Simple Chat Processor Wrapper",
	author = "Keith Warren (Drixevel)",
	description = "A simple plugin to create a wrapper API for backwards SCP support.",
	version = "1.0.0",
	url = "http://www.drixevel.com/"
};

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
	RegPluginLibrary("cp-scp-wrapper");
	g_hForward_OnChatMessage = CreateGlobalForward("OnChatMessage", ET_Event, Param_CellByRef, Param_Cell, Param_String, Param_String);
	g_hForward_OnChatMessage_Post = CreateGlobalForward("OnChatMessage_Post", ET_Ignore, Param_Cell, Param_Cell, Param_String, Param_String);
	CreateNative("GetMessageFlags", Native_GetMessageFlags);
	return APLRes_Success;
}

public void OnPluginStart()
{
	cvar_Status = CreateConVar("sm_chatprocessor_scp_wrapper", "0", "Status for this plugin.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
}

public Action CP_OnChatMessage(int& author, ArrayList recipients, char[] flagstring, char[] name, char[] message, bool& processcolors, bool& removecolors)
{
	if (!GetConVarBool(cvar_Status))
	{
		return Plugin_Continue;
	}

	if (StrContains(flagstring, "all", false) != -1)
	{
		g_iMessageFlag = g_iMessageFlag | CHATFLAGS_ALL;
	}
	if (StrContains(flagstring, "team", false) != -1
	|| 	StrContains(flagstring, "survivor", false) != -1
	||	StrContains(flagstring, "infected", false) != -1
	||	StrContains(flagstring, "Cstrike_Chat_CT", false) != -1
	||	StrContains(flagstring, "Cstrike_Chat_T", false) != -1)
	{
		g_iMessageFlag = g_iMessageFlag | CHATFLAGS_TEAM;
	}
	if (StrContains(flagstring, "spec", false) != -1)
	{
		g_iMessageFlag = g_iMessageFlag | CHATFLAGS_SPEC;
	}
	if (StrContains(flagstring, "dead", false) != -1)
	{
		g_iMessageFlag = g_iMessageFlag | CHATFLAGS_DEAD;
	}

	Call_StartForward(g_hForward_OnChatMessage);
	Call_PushCellRef(author);
	Call_PushCell(recipients);
	Call_PushStringEx(name, MAXLENGTH_NAME, SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushStringEx(message, MAXLENGTH_MESSAGE, SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);

	Action action = Plugin_Continue;
	Call_Finish(action);

	g_iMessageFlag = CHATFLAGS_INVALID;

	return action;
}

public void CP_OnChatMessagePost(int author, ArrayList recipients, const char[] flagstring, const char[] formatstring, const char[] name, const char[] message, bool processcolors, bool removecolors)
{
	if (!GetConVarBool(cvar_Status))
	{
		return;
	}

	Call_StartForward(g_hForward_OnChatMessage_Post);
	Call_PushCell(author);
	Call_PushCell(recipients);
	Call_PushString(name);
	Call_PushString(message);
	Call_Finish();
}

public int Native_GetMessageFlags(Handle plugin, int numParams)
{
	return g_iMessageFlag;
}
