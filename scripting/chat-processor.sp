////////////////////
//Pragma
#pragma semicolon 1
#pragma newdecls required

////////////////////
//Defines
#define PLUGIN_NAME "Chat-Processor"
#define PLUGIN_AUTHOR "Keith Warren (Aerial Vanguard)"
#define PLUGIN_DESCRIPTION "Replacement for Simple Chat Processor."
#define PLUGIN_VERSION "2.1.2"
#define PLUGIN_CONTACT "http://www.github.com/aerialvanguard"

////////////////////
//Includes
#include <sourcemod>
#include <chat-processor>
#include <colorvariables>

////////////////////
//Globals
ConVar convar_Status;
ConVar convar_Config;
ConVar convar_Default_ProcessColors;
ConVar convar_Default_RemoveColors;
ConVar convar_StripColors;
ConVar convar_DeadChat;
ConVar convar_AllChat;
ConVar convar_RestrictDeadChat;
ConVar convar_AddGOTV;

EngineVersion engine;

Handle hForward_OnChatMessage;
Handle hForward_OnChatMessagePost;

Handle hTrie_MessageFormats;

bool bProto;
bool bNewMsg[MAXPLAYERS + 1];

////////////////////
// Plugin Info
public Plugin myinfo =
{
	name = PLUGIN_NAME,
	author = PLUGIN_AUTHOR,
	description = PLUGIN_DESCRIPTION,
	version = PLUGIN_VERSION,
	url = PLUGIN_CONTACT
};

////////////////////
// Ask Plugin Load 2
public APLRes AskPluginLoad2(Handle myself, bool late, char[] error, int err_max)
{
	RegPluginLibrary("chat-processor");
	CreateNative("ChatProcessor_GetFlagFormatString", Native_GetFlagFormatString);

	hForward_OnChatMessage = CreateGlobalForward("CP_OnChatMessage", ET_Hook, Param_CellByRef, Param_Cell, Param_String, Param_String, Param_String, Param_CellByRef, Param_CellByRef);
	hForward_OnChatMessagePost = CreateGlobalForward("CP_OnChatMessagePost", ET_Ignore, Param_Cell, Param_Cell, Param_String, Param_String, Param_String, Param_String, Param_Cell, Param_Cell);

	engine = GetEngineVersion();
	return APLRes_Success;
}

////////////////////
// On Plugin Start
public void OnPluginStart()
{
	LoadTranslations("common.phrases");
	//LoadTranslations("chatprocessor.phrases");

	CreateConVar("sm_chatprocessor_version", PLUGIN_VERSION, PLUGIN_DESCRIPTION, FCVAR_REPLICATED | FCVAR_NOTIFY | FCVAR_SPONLY | FCVAR_DONTRECORD);

	convar_Status = CreateConVar("sm_chatprocessor_status", "1", "Status of the plugin.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	convar_Config = CreateConVar("sm_chatprocessor_config", "configs/chat_processor.cfg", "Name of the message formats config.", FCVAR_NOTIFY);
	convar_Default_ProcessColors = CreateConVar("sm_chatprocessor_process_colors_default", "1", "Default setting to give forwards to process colors.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	convar_Default_RemoveColors = CreateConVar("sm_chatprocessor_remove_colors_default", "0", "Default setting to give forwards to remove colors.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	convar_StripColors = CreateConVar("sm_chatprocessor_strip_colors", "1", "Remove color tags from the name and the message before processing the output.", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	convar_DeadChat = CreateConVar("sm_chatprocessor_deadchat", "1", "Controls how dead communicate.\n(0 = off, 1 = on)", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	convar_AllChat = CreateConVar("sm_chatprocessor_allchat", "0", "Allows both teams to communicate with each other through team chat.\n(0 = off, 1 = on)", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	convar_RestrictDeadChat = CreateConVar("sm_chatprocessor_restrictdeadchat", "0", "Restricts all chat for the dead entirely.\n(0 = off, 1 = on)", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	convar_AddGOTV = CreateConVar("sm_chatprocessor_addgotv", "1", "Add GOTV client to recipients list", FCVAR_NOTIFY, true, 0.0, true, 1.0);
	AutoExecConfig();

	hTrie_MessageFormats = CreateTrie();
}

////////////////////
// On Configs Executed
public void OnConfigsExecuted()
{
	if (!GetConVarBool(convar_Status))
	{
		return;
	}

	char sGame[64];
	GetGameFolderName(sGame, sizeof(sGame));

	char sConfig[PLATFORM_MAX_PATH];
	GetConVarString(convar_Config, sConfig, sizeof(sConfig));

	GenerateMessageFormats(sConfig, sGame);

	bProto = CanTestFeatures() && GetFeatureStatus(FeatureType_Native, "GetUserMessageType") == FeatureStatus_Available && GetUserMessageType() == UM_Protobuf;

	UserMsg SayText2 = GetUserMessageId("SayText2");
	if (SayText2 != INVALID_MESSAGE_ID)
	{
		HookUserMessage(SayText2, OnSayText2, true);
		LogMessage("Successfully hooked a SayText2 chat hook.");
	}
	else
	{
		SetFailState("Error loading the plugin, SayText2 is unavailable.");
	}
}

////////////////////
// Chat hook
public Action OnClientSayCommand(int client, const char[] command, const char[] sArgs)
{
	if (client > 0 && StrContains(command, "say") != -1)
	{
		bNewMsg[client] = true;
	}
}

////////////////////
//SayText2
public Action OnSayText2(UserMsg msg_id, BfRead msg, const int[] players, int playersNum, bool reliable, bool init)
{
	//Check if the plugin is disabled.
	if (!GetConVarBool(convar_Status))
	{
		return Plugin_Continue;
	}

	//Retrieve the client sending the message to other clients.
	int iSender = bProto ? PbReadInt(msg, "ent_idx") : BfReadByte(msg);
	if (iSender <= 0)
	{
		return Plugin_Continue;
	}

	//Stops double messages in-general.
	if (bNewMsg[iSender])
	{
		bNewMsg[iSender] = false;
	}
	else
	{
		return Plugin_Stop;
	}

	//Chat Type
	bool bChat = bProto ? PbReadBool(msg, "chat") : view_as<bool>(BfReadByte(msg));

	//Retrieve the name of template name to use when getting the format.
	char sFlag[MAXLENGTH_FLAG];
	switch (bProto)
	{
		case true: PbReadString(msg, "msg_name", sFlag, sizeof(sFlag));
		case false: BfReadString(msg, sFlag, sizeof(sFlag));
	}

	//Retrieve the format template based on the flag name above we retrieved.
	char sFormat[MAXLENGTH_BUFFER];
	if (!GetTrieString(hTrie_MessageFormats, sFlag, sFormat, sizeof(sFormat)))
	{
		return Plugin_Continue;
	}

	//Get the name string of the client.
	char sName[MAXLENGTH_NAME];
	switch (bProto)
	{
		case true: PbReadString(msg, "params", sName, sizeof(sName), 0);
		case false: if (BfGetNumBytesLeft(msg)) BfReadString(msg, sName, sizeof(sName));
	}

	//Get the message string that the client is wanting to send.
	char sMessage[MAXLENGTH_MESSAGE];
	switch (bProto)
	{
		case true: PbReadString(msg, "params", sMessage, sizeof(sMessage), 1);
		case false: if (BfGetNumBytesLeft(msg)) BfReadString(msg, sMessage, sizeof(sMessage));
	}

	//Clients have the ability to color their chat if they manually type in color tags, this allows server operators to choose if they want their players the ability to do so.
	//Example: {red}This {white}is {green}a {blue}random {yellow}message.
	//Goes for both the name and the message.
	if (GetConVarBool(convar_StripColors))
	{
		CRemoveColors(sName, sizeof(sName));
		CRemoveColors(sMessage, sizeof(sMessage));
	}

	//It's easier just to use a handle here for an array instead of passing 2 arguments through both forwards with static arrays.
	Handle hRecipients = CreateArray();

	bool bDeadTalk = GetConVarBool(convar_DeadChat);
	bool bAllTalk = GetConVarBool(convar_AllChat);
	bool bRestrictDeadChat = GetConVarBool(convar_RestrictDeadChat);
	int team = GetClientTeam(iSender);

	for (int i = 1; i < MaxClients; i++)
	{
		if (!IsClientInGame(i) || (!convar_AddGOTV.BoolValue && IsFakeClient(i)))
		{
			continue;
		}

		if (convar_AddGOTV.BoolValue && IsFakeClient(i) && IsClientSourceTV(i))
		{
			if (FindValueInArray(hRecipients, GetClientUserId(i)) == -1)
			{
				PushArrayCell(hRecipients, GetClientUserId(i));
			}
		}

		if (bRestrictDeadChat && !IsPlayerAlive(iSender))
		{
			continue;
		}

		if (!IsPlayerAlive(iSender) && !bDeadTalk && IsPlayerAlive(i))
		{
			continue;
		}

		if (!bAllTalk && StrContains(sFlag, "_All") == -1 && team != GetClientTeam(i))
		{
			continue;
		}

		PushArrayCell(hRecipients, GetClientUserId(i));
	}

	//Retrieve the default values for coloring and use these as a base for developers to change later.
	bool bProcessColors = GetConVarBool(convar_Default_ProcessColors);
	bool bRemoveColors = GetConVarBool(convar_Default_RemoveColors);

	//We need to make copy of these strings for checks after the pre-forward has fired.
	char sNameCopy[MAXLENGTH_NAME];
	strcopy(sNameCopy, sizeof(sNameCopy), sName);

	char sMessageCopy[MAXLENGTH_MESSAGE];
	strcopy(sMessageCopy, sizeof(sMessageCopy), sMessage);

	char sFlagCopy[MAXLENGTH_FLAG];
	strcopy(sFlagCopy, sizeof(sFlagCopy), sFlag);

	//Fire the pre-forward. https://i.ytimg.com/vi/A2a0Ht01qA8/maxresdefault.jpg
	Call_StartForward(hForward_OnChatMessage);
	Call_PushCellRef(iSender);
	Call_PushCell(hRecipients);
	Call_PushStringEx(sFlag, sizeof(sFlag), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushStringEx(sName, sizeof(sName), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushStringEx(sMessage, sizeof(sMessage), SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushCellRef(bProcessColors);
	Call_PushCellRef(bRemoveColors);

	//Retrieve the results here and use manage it.
	Action iResults;
	int error = Call_Finish(iResults);

	//We ran into a native error, gotta report it.
	if (error != SP_ERROR_NONE)
	{
		delete hRecipients;
		ThrowNativeError(error, "Global Forward 'CP_OnChatMessage' has failed to fire. [Error code: %i]", error);
		return Plugin_Continue;
	}

	//Check if the flag string was changed after the pre-forward and if so, re-retrieve the format string.
	if (!StrEqual(sFlag, sFlagCopy) && !GetTrieString(hTrie_MessageFormats, sFlag, sFormat, sizeof(sFormat)))
	{
		delete hRecipients;
		return Plugin_Continue;
	}

	if (StrEqual(sNameCopy, sName))
	{
		Format(sName, sizeof(sName), "\x03%s", sName);
	}

	if (StrEqual(sMessageCopy, sMessage))
	{
		Format(sMessage, sizeof(sMessage), "\x01%s", sMessage);
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
	WritePackCell(hPack, bRestrictDeadChat);

	RequestFrame(Frame_OnChatMessage_SayText2, hPack);

	return Plugin_Stop;
}

public void Frame_OnChatMessage_SayText2(any data)
{
	//Retrieve pack contents and what not, this part is obvious.
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
	Action iResults = ReadPackCell(data);

	bool bRestrictDeadChat = ReadPackCell(data);

	CloseHandle(data);

	if (bRestrictDeadChat)
	{
		PrintToChat(iSender, "Dead chat is currently restricted.");
	}

	//Make a copy of the format buffer and use that as the print so the format string stays the same.
	char sBuffer[MAXLENGTH_BUFFER];
	strcopy(sBuffer, sizeof(sBuffer), sFormat);

	//Make sure that the text is default for the message if no colors are present.
	if (iResults != Plugin_Changed && !bProcessColors || bRemoveColors)
	{
		Format(sMessage, sizeof(sMessage), "\x03%s", sMessage);
	}

	//Replace the specific characters for the name and message strings.
	ReplaceString(sBuffer, sizeof(sBuffer), "{1}", sName);
	ReplaceString(sBuffer, sizeof(sBuffer), "{2}", sMessage);
	ReplaceString(sBuffer, sizeof(sBuffer), "{3}", "\x01");

	//Process colors based on the final results we have.
	if (iResults == Plugin_Changed && bProcessColors)
	{
		CProcessVariables(sBuffer, sizeof(sBuffer), bRemoveColors);

		//CSGO quirk where the 1st color in the line won't work..
		if (engine == Engine_CSGO)
		{
			Format(sBuffer, sizeof(sBuffer), " %s", sBuffer);
		}
	}

	if (iResults != Plugin_Stop)
	{
		//Send the message to clients.
		int client;
		for (int i = 0; i < GetArraySize(hRecipients); i++)
		{
			client = GetClientOfUserId(GetArrayCell(hRecipients, i));

			if (client > 0 && IsClientInGame(client))
			{
				if (bProto)
				{
					CSayText2(client, sBuffer, iSender, bChat);
				}
				else
				{
					CSetNextAuthor(iSender);
					CPrintToChat(client, sBuffer);
				}
			}
		}
	}

	//Finally... fire the post-forward after the message has been sent and processed. https://s-media-cache-ak0.pinimg.com/564x/a5/bb/3c/a5bb3c3e05089a40ef01ea082ac39e24.jpg
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

	//Close the recipients handle.
	delete hRecipients;
}

////////////////////
//Parse message formats for flags.
bool GenerateMessageFormats(const char[] config, const char[] game)
{
	KeyValues kv = CreateKeyValues("chat-processor");

	char sPath[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, sPath, sizeof(sPath), config);

	if (FileToKeyValues(kv, sPath) && KvJumpToKey(kv, game) && KvGotoFirstSubKey(kv, false))
	{
		ClearTrie(hTrie_MessageFormats);

		do {
			char sName[256];
			KvGetSectionName(kv, sName, sizeof(sName));

			char sValue[256];
			KvGetString(kv, NULL_STRING, sValue, sizeof(sValue));

			SetTrieString(hTrie_MessageFormats, sName, sValue);

		} while (KvGotoNextKey(kv, false));

		LogMessage("Message formats generated for game '%s'.", game);
		delete kv;
		return true;
	}

	LogError("Error parsing the flag message formatting config for game '%s', please verify its integrity.", game);
	delete kv;
	return false;
}

////////////////////
//Flag format string native
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
