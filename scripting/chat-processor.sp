//Pragma
#pragma semicolon 1
#pragma newdecls required

//Defines
#define PLUGIN_NAME "Chat-Processor"
#define PLUGIN_AUTHOR "Keith Warren (Drixevel)"
#define PLUGIN_DESCRIPTION "Replacement for Simple Chat Processor."
#define PLUGIN_VERSION "1.0.6"
#define PLUGIN_CONTACT "http://www.drixevel.com/"

//Includes
#include <sourcemod>
#include <chat-processor>
#include <colorvariables>

//Globals
ConVar hConVars[2];
Handle hForward_OnChatMessage;
Handle hForward_OnChatMessagePost;
bool bProto;
bool bMessageFormats;
Handle hTrie_MessageFormats;
bool bSendingMessage[MAXPLAYERS + 1];

public Plugin myinfo = 
{
	name = PLUGIN_NAME, 
	author = PLUGIN_AUTHOR, 
	description = PLUGIN_DESCRIPTION, 
	version = PLUGIN_VERSION, 
	url = PLUGIN_CONTACT
};

public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
	RegPluginLibrary("chat-processor");

	hForward_OnChatMessage = CreateGlobalForward("OnChatMessage", ET_Hook, Param_CellByRef, Param_Cell, Param_CellByRef, Param_String, Param_String, Param_CellByRef, Param_CellByRef);
	hForward_OnChatMessagePost = CreateGlobalForward("OnChatMessage_Post", ET_Ignore, Param_Cell, Param_Cell, Param_Cell, Param_String, Param_String, Param_Cell, Param_Cell);

	return APLRes_Success;
}

public void OnPluginStart()
{
	LoadTranslations("common.phrases");
	//LoadTranslations("chatprocessor.phrases");
	
	CreateConVar("sm_chatprocessor_version", PLUGIN_VERSION, PLUGIN_DESCRIPTION, FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_SPONLY | FCVAR_DONTRECORD);
	
	hConVars[0] = CreateConVar("sm_chatprocessor_status", "1", "Status of the plugin.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	hConVars[1] = CreateConVar("sm_chatprocessor_config", "configs/chat_processor.cfg", "Name of the message formats config.", FCVAR_NOTIFY);

	AutoExecConfig();

	hTrie_MessageFormats = CreateTrie();
}

public void OnConfigsExecuted()
{
	if (!GetConVarBool(hConVars[0]))
	{
		return;
	}
	
	char sGame[64];
	GetGameFolderName(sGame, sizeof(sGame));

	char sConfig[PLATFORM_MAX_PATH];
	GetConVarString(hConVars[1], sConfig, sizeof(sConfig));

	bMessageFormats = GenerateMessageFormats(sConfig, sGame);

	if (bMessageFormats)
	{
		bProto = CanTestFeatures() && GetFeatureStatus(FeatureType_Native, "GetUserMessageType") == FeatureStatus_Available && GetUserMessageType() == UM_Protobuf;

		UserMsg SayText2 = GetUserMessageId("SayText2");

		if (SayText2 != INVALID_MESSAGE_ID)
		{
			HookUserMessage(SayText2, OnSayText2, true);
		}

		/*
		UserMsg SayText = GetUserMessageId("SayText");

		if (SayText != INVALID_MESSAGE_ID)
		{
			HookUserMessage(SayText, OnSayText, true);
		}*/
	}
}

public Action OnSayText2(UserMsg msg_id, BfRead msg, const int[] players, int playersNum, bool reliable, bool init)
{
	if (!GetConVarBool(hConVars[0]) || !bMessageFormats)
	{
		return Plugin_Continue;
	}
	
	int iSender = bProto ? PbReadInt(msg, "ent_idx") : BfReadByte(msg);

	if (iSender <= 0)
	{
		return Plugin_Continue;
	}

	if (bSendingMessage[iSender])
	{
		return Plugin_Handled;
	}

	bSendingMessage[iSender] = true;

	bool bChat = bProto ? view_as<bool>(PbReadInt(msg, "chat")) : view_as<bool>(BfReadByte(msg));

	char sTrans[32];
	switch (bProto)
	{
		case true: PbReadString(msg, "msg_name", sTrans, sizeof(sTrans));
		case false: BfReadString(msg, sTrans, sizeof(sTrans));
	}

	char sFormat[256];
	if (!bMessageFormats || !GetTrieString(hTrie_MessageFormats, sTrans, sFormat, sizeof(sFormat)))
	{
		return Plugin_Continue;
	}

	eChatFlags cFlag = ChatFlag_Invalid;
	if (StrContains(sTrans, "all") != -1)
	{
		cFlag = ChatFlag_All;
	}
	else if (StrContains(sTrans, "team") != -1 || StrContains(sTrans, "survivor") != -1 || StrContains(sTrans, "infected") != -1 || StrContains(sTrans, "Cstrike_Chat_CT") != -1 || StrContains(sTrans, "Cstrike_Chat_T") != -1)
	{
		cFlag = ChatFlag_Team;
	}
	else if (StrContains(sTrans, "spec") != -1)
	{
		cFlag = ChatFlag_Spec;
	}
	else if (StrContains(sTrans, "dead") != -1)
	{
		cFlag = ChatFlag_Dead;
	}

	char sName[MAXLENGTH_NAME];
	switch (bProto)
	{
		case true: PbReadString(msg, "params", sName, sizeof(sName), 0);
		case false: if (BfGetNumBytesLeft(msg)) BfReadString(msg, sName, sizeof(sName));
	}

	char sMessage[MAXLENGTH_MESSAGE];
	switch (bProto)
	{
		case true: PbReadString(msg, "params", sMessage, sizeof(sMessage));
		case false: if (BfGetNumBytesLeft(msg)) BfReadString(msg, sMessage, sizeof(sMessage));
	}

	Handle hRecipients = CreateArray();
	PushArrayCell(hRecipients, iSender);
	for (int i = 0; i < playersNum; i++)
	{
		PushArrayCell(hRecipients, players[i]);
	}

	bool bProcessColors = true;
	bool bRemoveColors = false;

	char sNameCopy[MAXLENGTH_NAME];
	strcopy(sNameCopy, sizeof(sNameCopy), sName);
	
	Call_StartForward(hForward_OnChatMessage);
	Call_PushCellRef(iSender);
	Call_PushCell(hRecipients);
	Call_PushCellRef(cFlag);
	Call_PushStringEx(sName, sizeof(sName), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushStringEx(sMessage, sizeof(sMessage), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushCellRef(bProcessColors);
	Call_PushCellRef(bRemoveColors);
	
	Action iResults;
	int error = Call_Finish(iResults);

	if (error != SP_ERROR_NONE)
	{
		bSendingMessage[iSender] = false;
		ThrowNativeError(error, "Forward has failed to fire.");
		CloseHandle(hRecipients);
		return Plugin_Continue;
	}

	switch (iResults)
	{
		case Plugin_Continue, Plugin_Stop:
		{
			bSendingMessage[iSender] = false;
			CloseHandle(hRecipients);
			return iResults;
		}
	}

	if (StrEqual(sNameCopy, sName))
	{
		Format(sName, sizeof(sName), "\x03%s", sName);
	}
	
	Handle hPack = CreateDataPack();
	WritePackCell(hPack, iSender);
	WritePackCell(hPack, hRecipients);
	WritePackCell(hPack, bChat);
	WritePackString(hPack, sTrans);
	WritePackString(hPack, sName);
	WritePackString(hPack, sMessage);
	WritePackCell(hPack, cFlag);
	WritePackString(hPack, sFormat);
	WritePackCell(hPack, bProcessColors);
	WritePackCell(hPack, bRemoveColors);

	RequestFrame(Frame_OnChatMessage, hPack);
	
	return Plugin_Handled;
}

public void Frame_OnChatMessage(any data)
{
	ResetPack(data);

	int iSender = ReadPackCell(data);

	Handle hRecipients = ReadPackCell(data);

	bool bChat = ReadPackCell(data);

	char sTrans[32];
	ReadPackString(data, sTrans, sizeof(sTrans));

	char sName[MAXLENGTH_NAME];
	ReadPackString(data, sName, sizeof(sName));

	char sMessage[MAXLENGTH_MESSAGE];
	ReadPackString(data, sMessage, sizeof(sMessage));

	eChatFlags cFlags = ReadPackCell(data);

	char sFormat[MAXLENGTH_MESSAGE];
	ReadPackString(data, sFormat, sizeof(sFormat));

	bool bProcessColors = ReadPackCell(data);
	bool bRemoveColors = ReadPackCell(data);

	CloseHandle(data);

	ReplaceString(sFormat, sizeof(sFormat), "{1}", sName);
	ReplaceString(sFormat, sizeof(sFormat), "{2}", sMessage);
	
	if (bProcessColors)
	{
		CProcessVariables(sFormat, sizeof(sFormat), bRemoveColors);
	}

	for (int i = 0; i < GetArraySize(hRecipients); i++)
	{
		int client = GetArrayCell(hRecipients, i);

		if (IsClientInGame(client))
		{
			CSayText2(client, sFormat, iSender, bChat);
		}
	}
	
	Call_StartForward(hForward_OnChatMessagePost);
	Call_PushCell(iSender);
	Call_PushCell(hRecipients);
	Call_PushCell(cFlags);
	Call_PushString(sName);
	Call_PushString(sMessage);
	Call_PushCell(bProcessColors);
	Call_PushCell(bRemoveColors);
	Call_Finish();
	
	CloseHandle(hRecipients);

	bSendingMessage[iSender] = false;
}
/*
public Action OnSayText(UserMsg msg_id, BfRead msg, const int[] players, int playersNum, bool reliable, bool init)
{
	
}
*/

bool GenerateMessageFormats(const char[] config, const char[] game)
{
	Handle hKV = CreateKeyValues("chat-processor");

	char sPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, sPath, sizeof(sPath), config);

	if (!FileToKeyValues(hKV, sPath))
	{
		LogError("Error finding configuration file for message formats: %s", config);
		CloseHandle(hKV);
		return false;
	}

	if (KvJumpToKey(hKV, game) && KvGotoFirstSubKey(hKV, false))
	{
		ClearTrie(hTrie_MessageFormats);

		do {
			char sName[256];
			KvGetSectionName(hKV, sName, sizeof(sName));

			char sValue[256];
			KvGetString(hKV, NULL_STRING, sValue, sizeof(sValue));

			SetTrieString(hTrie_MessageFormats, sName, sValue);

		} while (KvGotoNextKey(hKV, false));
	}
	else
	{
		LogError("Error parsing message format, missing game '%s'.", game);
		CloseHandle(hKV);
		return false;
	}

	LogMessage("Message formats generated for game '%s'.", game);
	CloseHandle(hKV);
	return true;
}