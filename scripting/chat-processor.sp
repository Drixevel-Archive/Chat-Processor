//Pragma
#pragma semicolon 1
#pragma newdecls required

//Defines
#define PLUGIN_NAME "Chat-Processor"
#define PLUGIN_AUTHOR "Keith Warren (Drixevel)"
#define PLUGIN_DESCRIPTION "Replacement for Simple Chat Processor."
#define PLUGIN_VERSION "2.0.2"
#define PLUGIN_CONTACT "http://www.drixevel.com/"

//Includes
#include <sourcemod>
#include <chat-processor>
#include <colorvariables>

//Globals
ConVar hConVars[5];
Handle hForward_OnChatMessage;
Handle hForward_OnChatMessagePost;
bool bProto;
Handle hTrie_MessageFormats;
bool bHooked;
bool bNewMsg[MAXPLAYERS + 1];

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
	CreateNative("ChatProcessor_GetFlagFormatString", Native_GetFlagFormatString);

	hForward_OnChatMessage = CreateGlobalForward("CP_OnChatMessage", ET_Hook, Param_CellByRef, Param_Cell, Param_String, Param_String, Param_String, Param_CellByRef, Param_CellByRef);
	hForward_OnChatMessagePost = CreateGlobalForward("CP_OnChatMessagePost", ET_Ignore, Param_Cell, Param_Cell, Param_String, Param_String, Param_String, Param_String, Param_Cell, Param_Cell);

	return APLRes_Success;
}

public void OnPluginStart()
{
	LoadTranslations("common.phrases");
	//LoadTranslations("chatprocessor.phrases");

	CreateConVar("sm_chatprocessor_version", PLUGIN_VERSION, PLUGIN_DESCRIPTION, FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_SPONLY | FCVAR_DONTRECORD);

	hConVars[0] = CreateConVar("sm_chatprocessor_status", "1", "Status of the plugin.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	hConVars[1] = CreateConVar("sm_chatprocessor_config", "configs/chat_processor.cfg", "Name of the message formats config.", FCVAR_NOTIFY);
	hConVars[2] = CreateConVar("sm_chatprocessor_process_colors_default", "1", "Default setting to give forwards to process colors.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	hConVars[3] = CreateConVar("sm_chatprocessor_remove_colors_default", "0", "Default setting to give forwards to remove colors.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	hConVars[4] = CreateConVar("sm_chatprocessor_strip_colors", "1", "Remove color tags from the name and the message before processing the output.", FCVAR_NOTIFY, true, 0.0, true, 1.0);

	AddCommandListener(Command_Say, "say");
	AddCommandListener(Command_Say, "say_team");

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

	GenerateMessageFormats(sConfig, sGame);

	if (!bHooked)
	{
		bProto = CanTestFeatures() && GetFeatureStatus(FeatureType_Native, "GetUserMessageType") == FeatureStatus_Available && GetUserMessageType() == UM_Protobuf;
		bool bLoaded;

		UserMsg SayText2 = GetUserMessageId("SayText2");
		if (SayText2 != INVALID_MESSAGE_ID)
		{
			HookUserMessage(SayText2, OnSayText2, true);
			bLoaded = true;
			LogMessage("Hooking 'SayText2' chat messages.");
		}

		/*UserMsg SayText = GetUserMessageId("SayText");
		if (SayText != INVALID_MESSAGE_ID && !bLoaded)
		{
			HookUserMessage(SayText, OnSayText, true);
			bLoaded = true;
			LogMessage("Hooking 'SayText' chat messages.");
		}*/

		if (!bLoaded)
		{
			SetFailState("Error loading the plugin, both chat hooks are unavailable. (SayText & SayText2)");
		}

		bHooked = true;
	}
}

////////////////////
// Chat hook
public Action Command_Say(int client, const char[] command, int argc)
{
	bNewMsg[client] = true;
	return Plugin_Continue;
}

////////////////////
//SayText2
public Action OnSayText2(UserMsg msg_id, BfRead msg, const int[] players, int playersNum, bool reliable, bool init)
{
	if (!GetConVarBool(hConVars[0]))
	{
		return Plugin_Continue;
	}

	int iSender = bProto ? PbReadInt(msg, "ent_idx") : BfReadByte(msg);
	if (iSender <= 0)
	{
		return Plugin_Continue;
	}

	bool bChat = bProto ? PbReadBool(msg, "chat") : view_as<bool>(BfReadByte(msg));

	char sFlag[MAXLENGTH_FLAG];
	switch (bProto)
	{
		case true: PbReadString(msg, "msg_name", sFlag, sizeof(sFlag));
		case false: BfReadString(msg, sFlag, sizeof(sFlag));
	}

	// protobuf messages (at least in cs:go) are sent once for every client,
	// but only if that client isn't a spectator
	// since we want to allow modification of recipients, we have to block all other messages
	// and start our own ones
	if(bProto)
	{
		if(StrContains(sFlag, "_Spec") == -1 && !bNewMsg[iSender])
		{
			return Plugin_Stop;
		}
		else
		{
			bNewMsg[iSender] = false;
		}
	}

	char sFormat[MAXLENGTH_BUFFER];
	if (!GetTrieString(hTrie_MessageFormats, sFlag, sFormat, sizeof(sFormat)))
	{
		return Plugin_Continue;
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
		case true: PbReadString(msg, "params", sMessage, sizeof(sMessage), 1);
		case false: if (BfGetNumBytesLeft(msg)) BfReadString(msg, sMessage, sizeof(sMessage));
	}

	if (GetConVarBool(hConVars[4]))
	{
		CRemoveColors(sName, sizeof(sName));
		CRemoveColors(sMessage, sizeof(sMessage));
	}

	Handle hRecipients = CreateArray();

	for (int i = 0; i < playersNum; i++)
	{
		if (FindValueInArray(hRecipients, players[i]) == -1)
		{
			PushArrayCell(hRecipients, players[i]);
		}
	}

	if (FindValueInArray(hRecipients, iSender) == -1)
	{
		PushArrayCell(hRecipients, iSender);
	}

	bool bProcessColors = GetConVarBool(hConVars[2]);
	bool bRemoveColors = GetConVarBool(hConVars[3]);

	char sNameCopy[MAXLENGTH_NAME];
	strcopy(sNameCopy, sizeof(sNameCopy), sName);

	char sFlagCopy[MAXLENGTH_FLAG];
	strcopy(sFlagCopy, sizeof(sFlagCopy), sFlag);

	Call_StartForward(hForward_OnChatMessage);
	Call_PushCellRef(iSender);
	Call_PushCell(hRecipients);
	Call_PushStringEx(sFlag, sizeof(sFlag), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushStringEx(sName, sizeof(sName), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushStringEx(sMessage, sizeof(sMessage), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushCellRef(bProcessColors);
	Call_PushCellRef(bRemoveColors);

	Action iResults;
	int error = Call_Finish(iResults);

	if (error != SP_ERROR_NONE)
	{
		CloseHandle(hRecipients);
		ThrowNativeError(error, "Forward has failed to fire.");
		return Plugin_Continue;
	}

	if (!StrEqual(sFlag, sFlagCopy) && !GetTrieString(hTrie_MessageFormats, sFlag, sFormat, sizeof(sFormat)))
	{
		return Plugin_Continue;
	}

	switch (iResults)
	{
		case Plugin_Continue, Plugin_Stop:
		{
			CloseHandle(hRecipients);
			return iResults;
		}

		case Plugin_Changed, Plugin_Handled:
		{
			if (StrEqual(sNameCopy, sName))
			{
				Format(sName, sizeof(sName), "\x03%s", sName);
			}

			Handle hPack = CreateDataPack();
			WritePackCell(hPack, iSender);
			WritePackCell(hPack, hRecipients);
			WritePackString(hPack, sName);
			WritePackString(hPack, sMessage);
			WritePackString(hPack, sFlag);
			WritePackCell(hPack, bProcessColors);
			WritePackCell(hPack, bRemoveColors);

			WritePackString(hPack, sFormat);
			WritePackCell(hPack, bChat);
			WritePackCell(hPack, iResults);

			RequestFrame(Frame_OnChatMessage_SayText2, hPack);
			return Plugin_Handled;
		}
	}

	return Plugin_Continue;
}

public void Frame_OnChatMessage_SayText2(any data)
{
	ResetPack(data);

	int iSender = ReadPackCell(data);
	Handle hRecipients = ReadPackCell(data);

	char sName[MAXLENGTH_NAME];
	ReadPackString(data, sName, sizeof(sName));

	char sMessage[MAXLENGTH_MESSAGE];
	ReadPackString(data, sMessage, sizeof(sMessage));

	char sFlag[MAXLENGTH_FLAG];
	ReadPackString(data, sFlag, sizeof(sFlag));

	bool bProcessColors = ReadPackCell(data);
	bool bRemoveColors = ReadPackCell(data);

	char sFormat[MAXLENGTH_BUFFER];
	ReadPackString(data, sFormat, sizeof(sFormat));

	bool bChat = ReadPackCell(data);
	Action iResults = view_as<Action>(ReadPackCell(data));

	CloseHandle(data);

	// only used for non-pb messages
	int[] iRecipients = new int[MaxClients];
	int iNumRecipients = GetArraySize(hRecipients);

	for (int i = 0; i < iNumRecipients; i++)
	{
		iRecipients[i] = GetArrayCell(hRecipients, i);
	}

	char sBuffer[MAXLENGTH_BUFFER];
	strcopy(sBuffer, sizeof(sBuffer), sFormat);

	ReplaceString(sBuffer, sizeof(sBuffer), "{1}", sName);
	ReplaceString(sBuffer, sizeof(sBuffer), "{2}", sMessage);

	if (bProcessColors)
	{
		CProcessVariables(sBuffer, sizeof(sBuffer), bRemoveColors);
	}

	if (iResults == Plugin_Changed)
	{
		if (bProto)
		{
			for (int i = 0; i < GetArraySize(hRecipients); i++)
			{
				int client = GetArrayCell(hRecipients, i);

				if (IsClientInGame(client))
				{
					CSayText2(client, sBuffer, iSender, bChat);
				}
			}
		}
		else
		{
			for (int i = 0; i < GetArraySize(hRecipients); i++)
			{
				int client = GetArrayCell(hRecipients, i);

				if (IsClientInGame(client))
				{
					CPrintToChat(client, sBuffer);
				}
			}

			//Broken, will figure it out later..

			/*
			Handle hMsg = StartMessage("SayText2", iRecipients, iNumRecipients, USERMSG_RELIABLE | USERMSG_BLOCKHOOKS);

			BfWriteByte(hMsg, iSender);
			BfWriteByte(hMsg, bChat);
			BfWriteString(hMsg, sBuffer);
			EndMessage();
			*/
		}
	}

	Call_StartForward(hForward_OnChatMessagePost);
	Call_PushCell(iSender);
	Call_PushCell(hRecipients);
	Call_PushString(sFlag);
	Call_PushString(sFormat);
	Call_PushString(sName);
	Call_PushString(sMessage);
	Call_PushCell(bProcessColors);
	Call_PushCell(bRemoveColors);
	Call_Finish();

	CloseHandle(hRecipients);
}

/*////////////////////
//SayText

public Action OnSayText(UserMsg msg_id, BfRead msg, const int[] players, int playersNum, bool reliable, bool init)
{
	int iSender = bProto ? PbReadInt(msg, "ent_idx") : BfReadByte(msg);

	if (iSender <= 0)
	{
		return Plugin_Continue;
	}

	char sMessage[MAXLENGTH_INPUT];
	switch (bProto)
	{
		case true: PbReadString(msg, "text", sMessage, sizeof(sMessage));
		case false: BfReadString(msg, sMessage, sizeof(sMessage));
	}

	if (!bProto)
	{
		BfReadBool(msg);
	}

	Handle hRecipients = CreateArray();
	PushArrayCell(hRecipients, iSender);
	for (int i = 0; i < playersNum; i++)
	{
		PushArrayCell(hRecipients, players[i]);
	}

	char sSenderName[MAX_NAME_LENGTH];
	GetClientName(iSender, sSenderName, sizeof(sSenderName));

	char sBuffer[MAXLENGTH_INPUT];
	Format(sBuffer, sizeof(sBuffer), "%s:", sSenderName);

	int iPos = StrContains(sMessage, sBuffer);

	char sPrefix[64];
	if (iPos == 0)
	{
		sPrefix[0] = '\0';
	}
	else
	{
		Format(sPrefix, iPos + 1, "%s ", sMessage);
	}

	eChatFlags cFlag = ChatFlag_Invalid;

	if (StrContains(sPrefix, "(Team)") != -1)
	{
		cFlag = ChatFlag_Team;
	}

	if (GetClientTeam(iSender) <= 1)
	{
		cFlag = ChatFlag_Spec;
	}

	if (StrContains(sPrefix, "(Dead)") != -1)
	{
		cFlag = ChatFlag_Spec;
	}

	if (cFlag == ChatFlag_Invalid)
	{
		cFlag = ChatFlag_All;
	}

	ReplaceString(sMessage, sizeof(sMessage), "\n", "");

	char sSenderMessage[MAXLENGTH_MESSAGE];
	strcopy(sSenderMessage, sizeof(sSenderMessage), sMessage[iPos + strlen(sSenderName) + 2]);

	bool bProcessColors = true;
	bool bRemoveColors = false;

	Call_StartForward(hForward_OnChatMessage);
	Call_PushCellRef(iSender);
	Call_PushCell(hRecipients);
	Call_PushCellRef(cFlag);
	Call_PushStringEx(sSenderName, sizeof(sSenderName), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushStringEx(sSenderMessage, sizeof(sSenderMessage), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushCellRef(bProcessColors);
	Call_PushCellRef(bRemoveColors);

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
		case Plugin_Continue, Plugin_Handled:
		{
			CloseHandle(hRecipients);
			return iResults;
		}
	}

	char sTemp[MAX_NAME_LENGTH];
	GetClientName(iSender, sTemp, sizeof(sTemp));

	if (StrEqual(sSenderName, sTemp))
	{
		Format(sSenderName, sizeof(sSenderName), "\x03%s", sSenderName);
	}

	Handle hPack = CreateDataPack();
	WritePackCell(hPack, iSender);
	WritePackCell(hPack, hRecipients);
	WritePackString(hPack, sPrefix);
	WritePackString(hPack, sSenderName);
	WritePackString(hPack, sSenderMessage);
	WritePackCell(hPack, cFlag);
	WritePackCell(hPack, bProcessColors);
	WritePackCell(hPack, bRemoveColors);

	RequestFrame(Frame_OnChatMessage_SayText, hPack);

	return Plugin_Handled;
}

public void Frame_OnChatMessage_SayText(any data)
{
	ResetPack(data);

	int iSender = ReadPackCell(data);

	Handle hRecipients = ReadPackCell(data);

	char sPrefix[64];
	ReadPackString(data, sPrefix, sizeof(sPrefix));

	char sSenderName[MAXLENGTH_NAME];
	ReadPackString(data, sSenderName, sizeof(sSenderName));

	char sSenderMessage[MAXLENGTH_MESSAGE];
	ReadPackString(data, sSenderMessage, sizeof(sSenderMessage));

	eChatFlags cFlags = ReadPackCell(data);

	bool bProcessColors = ReadPackCell(data);
	bool bRemoveColors = ReadPackCell(data);

	CloseHandle(data);

	int iTeamColor;
	switch (GetClientTeam(iSender))
	{
		case 0, 1: iTeamColor = 0xCCCCCC;
		case 2: iTeamColor = 0x4D7942;
		case 3: iTeamColor = 0xFF4040;
	}

	char sBuffer[32];
	Format(sBuffer, sizeof(sBuffer), "\x07%06X", iTeamColor);

	ReplaceString(sSenderName, sizeof(sSenderName), "\x03", sBuffer);
	ReplaceString(sSenderMessage, sizeof(sSenderMessage), "\x03", sBuffer);

	char sDisplayMessage[MAXLENGTH_MESSAGE];
	Format(sDisplayMessage, sizeof(sDisplayMessage), "\x01%s%s\x01: %s", sPrefix, sSenderName, sSenderMessage);

	if (bProcessColors)
	{
		CProcessVariables(sDisplayMessage, sizeof(sDisplayMessage), bRemoveColors);
	}

	for (int i = 0; i < GetArraySize(hRecipients); i++)
	{
		int client = GetArrayCell(hRecipients, i);

		if (IsClientInGame(client))
		{
			CSayText2(client, sDisplayMessage, iSender);
		}
	}

	Call_StartForward(hForward_OnChatMessagePost);
	Call_PushCell(iSender);
	Call_PushCell(hRecipients);
	Call_PushCell(cFlags);
	Call_PushString(sSenderName);
	Call_PushString(sSenderMessage);
	Call_PushCell(bProcessColors);
	Call_PushCell(bRemoveColors);
	Call_Finish();

	CloseHandle(hRecipients);
}*/

bool GenerateMessageFormats(const char[] config, const char[] game)
{
	Handle hKV = CreateKeyValues("chat-processor");

	char sPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, sPath, sizeof(sPath), config);

	if (FileToKeyValues(hKV, sPath) && KvJumpToKey(hKV, game) && KvGotoFirstSubKey(hKV, false))
	{
		ClearTrie(hTrie_MessageFormats);

		do {
			char sName[256];
			KvGetSectionName(hKV, sName, sizeof(sName));

			char sValue[256];
			KvGetString(hKV, NULL_STRING, sValue, sizeof(sValue));

			SetTrieString(hTrie_MessageFormats, sName, sValue);

		} while (KvGotoNextKey(hKV, false));

		LogMessage("Message formats generated for game '%s'.", game);
		CloseHandle(hKV);
		return true;
	}

	LogError("Error parsing the flag message formatting config for game '%s', please verify its integrity.", game);
	CloseHandle(hKV);
	return false;
}

public int Native_GetFlagFormatString(Handle plugin, int numParams)
{
	int iSize;
	GetNativeStringLength(1, iSize);

	char[] sFlag = new char[iSize + 1];
	GetNativeString(1, sFlag, iSize + 1);

	char sFormat[MAXLENGTH_BUFFER];
	GetTrieString(hTrie_MessageFormats, sFlag, sFormat, sizeof(sFormat));

	SetNativeString(2, sFormat, GetNativeCell(3));
}
