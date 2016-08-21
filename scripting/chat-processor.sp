//Pragma
#pragma semicolon 1
#pragma newdecls required

//Defines
#define PLUGIN_NAME "Chat-Processor"
#define PLUGIN_AUTHOR "Keith Warren (Drixevel)"
#define PLUGIN_DESCRIPTION "Replacement for Simple Chat Processor."
#define PLUGIN_VERSION "1.0.1"
#define PLUGIN_CONTACT "http://www.drixevel.com/"

//Includes
#include <sourcemod>
#include <chat-processor>

//Globals
ConVar hConVars[1];
//bool bLateLoad;
Handle hForward_OnChatMessage;
Handle hForward_OnChatMessagePost;
//EngineVersion hEngine;
bool bProto;
//Handle hTrie_ChatFormats;

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

	hForward_OnChatMessage = CreateGlobalForward("OnChatMessage", ET_Hook, Param_CellByRef, Param_Cell, Param_Cell, Param_String, Param_String);
	hForward_OnChatMessagePost = CreateGlobalForward("OnChatMessage_Post", ET_Ignore, Param_Cell, Param_Cell, Param_Cell, Param_String, Param_String);

	//hEngine = GetEngineVersion();
	//bLateLoad = late;
	return APLRes_Success;
}

public void OnPluginStart()
{
	LoadTranslations("common.phrases");
	//LoadTranslations("chatprocessor.phrases");
	
	hConVars[0] = CreateConVar("sm_chatprocessor_status", "1", "Status of the plugin.", FCVAR_NOTIFY, true, 0.0, true, 1.0);

	//AutoExecConfig();

	//hTrie_ChatFormats = CreateTrie();
}

public void OnConfigsExecuted()
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

public Action OnSayText2(UserMsg msg_id, BfRead msg, const int[] players, int playersNum, bool reliable, bool init)
{
	int iSender = bProto ? PbReadInt(msg, "ent_idx") : BfReadByte(msg);

	if (iSender <= 0)
	{
		return Plugin_Continue;
	}

	bool bChat = bProto ? view_as<bool>(PbReadInt(msg, "chat")) : view_as<bool>(BfReadByte(msg));

	char sTrans[32];
	switch (bProto)
	{
		case true: PbReadString(msg, "msg_name", sTrans, sizeof(sTrans));
		case false: BfReadString(msg, sTrans, sizeof(sTrans));
	}

	/*
	int iBuffer;
	if (!GetTrieValue(hTrie_ChatFormats, sTrans, iBuffer))
	{
		return Plugin_Continue;
	}
	*/

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
	for (int i = 0; i < playersNum; i++)
	{
		PushArrayCell(hRecipients, players[i]);
	}

	char sNameCopy[MAXLENGTH_NAME];
	strcopy(sNameCopy, sizeof(sNameCopy), sName);

	Call_StartForward(hForward_OnChatMessage);
	Call_PushCellRef(iSender);
	Call_PushCell(hRecipients);
	Call_PushCell(cFlag);
	Call_PushStringEx(sName, sizeof(sName), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushStringEx(sMessage, sizeof(sMessage), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);

	Action iResults;
	int error = Call_Finish(iResults);

	if (error != SP_ERROR_NONE)
	{
		ThrowNativeError(error, "Forward has failed to fire.");
		CloseHandle(hRecipients);
		return Plugin_Continue;
	}

	switch (iResults)
	{
		case Plugin_Continue, Plugin_Stop:
		{
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

	RequestFrame(Frame_OnChatMessage);

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

	CloseHandle(data);

	int iClientSize = GetArraySize(hRecipients);
	int[] clients = new int[hRecipients];
	int iClients;

	for (int i = 0; i < iClientSize; i++)
	{
		int client = GetArrayCell(hRecipients, i);

		if (IsClientInGame(client))
		{
			clients[iClients++] = client;
		}
	}

	Handle msg = StartMessage("SayText2", clients, iClients, USERMSG_RELIABLE | USERMSG_BLOCKHOOKS);

	switch (bProto)
	{
		case true:
		{
			PbSetInt(msg, "ent_idx", iSender);
			PbSetBool(msg, "chat", bChat);
			PbSetString(msg, "msg_name", sTrans);
			PbAddString(msg, "params", "");
			PbAddString(msg, "params", "");
			PbAddString(msg, "params", "");
			PbAddString(msg, "params", "");
		}
		case false:
		{
			BfWriteByte(msg, iSender);
			BfWriteByte(msg, bChat);
			BfWriteString(msg, sTrans);
		}
	}

	EndMessage();

	Call_StartForward(hForward_OnChatMessagePost);
	Call_PushCell(iSender);
	Call_PushCell(hRecipients);
	Call_PushCell(cFlags);
	Call_PushString(sName);
	Call_PushString(sMessage);
	Call_Finish();

	CloseHandle(hRecipients);
}
/*
public Action OnSayText(UserMsg msg_id, BfRead msg, const int[] players, int playersNum, bool reliable, bool init)
{
	
}
*/