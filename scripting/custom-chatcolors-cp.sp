#pragma semicolon 1

#include <sourcemod>
#include <chat-processor>

#define PLUGIN_VERSION		"3.1.0 CP"

public Plugin:myinfo = {
	name        = "[Source 2013] Custom Chat Colors",
	author      = "Dr. McKay, Fixed up by Keith Warren (Drixevel)",
	description = "Processes chat and provides colors for Source 2013 games",
	version     = PLUGIN_VERSION,
	url         = "http://www.doctormckay.com"
};

new Handle:colorForward;
new Handle:nameForward;
new Handle:tagForward;
new Handle:applicationForward;
new Handle:messageForward;
new Handle:preLoadedForward;
new Handle:loadedForward;
new Handle:configReloadedForward;

new String:tag[MAXPLAYERS + 1][32];
new String:tagColor[MAXPLAYERS + 1][12];
new String:usernameColor[MAXPLAYERS + 1][12];
new String:chatColor[MAXPLAYERS + 1][12];

new String:defaultTag[MAXPLAYERS + 1][32];
new String:defaultTagColor[MAXPLAYERS + 1][12];
new String:defaultUsernameColor[MAXPLAYERS + 1][12];
new String:defaultChatColor[MAXPLAYERS + 1][12];

new Handle:configFile;

enum CCC_ColorType {
	CCC_TagColor,
	CCC_NameColor,
	CCC_ChatColor
};

#define COLOR_NONE		-1
#define COLOR_GREEN		-2
#define COLOR_OLIVE		-3
#define COLOR_TEAM		-4

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max) {
	CreateNative("CCC_GetColor", Native_GetColor);
	CreateNative("CCC_SetColor", Native_SetColor);
	CreateNative("CCC_GetTag", Native_GetTag);
	CreateNative("CCC_SetTag", Native_SetTag);
	CreateNative("CCC_ResetColor", Native_ResetColor);
	CreateNative("CCC_ResetTag", Native_ResetTag);
	
	RegPluginLibrary("ccc");
	
	return APLRes_Success;
} 

public OnPluginStart() {
	RegAdminCmd("sm_reloadccc", Command_ReloadConfig, ADMFLAG_CONFIG, "Reloads Custom Chat Colors config file");
	colorForward = CreateGlobalForward("CCC_OnChatColor", ET_Event, Param_Cell);
	nameForward = CreateGlobalForward("CCC_OnNameColor", ET_Event, Param_Cell);
	tagForward = CreateGlobalForward("CCC_OnTagApplied", ET_Event, Param_Cell);
	applicationForward = CreateGlobalForward("CCC_OnColor", ET_Event, Param_Cell, Param_String, Param_Cell);
	messageForward = CreateGlobalForward("CCC_OnChatMessage", ET_Ignore, Param_Cell, Param_String, Param_Cell);
	preLoadedForward = CreateGlobalForward("CCC_OnUserConfigPreLoaded", ET_Event, Param_Cell);
	loadedForward = CreateGlobalForward("CCC_OnUserConfigLoaded", ET_Ignore, Param_Cell);
	configReloadedForward = CreateGlobalForward("CCC_OnConfigReloaded", ET_Ignore);
	LoadConfig();
}

LoadConfig() {
	if(configFile != INVALID_HANDLE) {
		CloseHandle(configFile);
	}
	configFile = CreateKeyValues("admin_colors");
	decl String:path[64];
	BuildPath(Path_SM, path, sizeof(path), "configs/custom-chatcolors.cfg");
	if(!FileToKeyValues(configFile, path)) {
		SetFailState("Config file missing");
	}
	for(new i = 1; i <= MaxClients; i++) {
		if(!IsClientInGame(i) || IsFakeClient(i)) {
			continue;
		}
		ClearValues(i);
		OnClientPostAdminCheck(i);
	}
}

public Action:Command_ReloadConfig(client, args) {
	LoadConfig();
	LogAction(client, -1, "Reloaded Custom Chat Colors config file");
	ReplyToCommand(client, "[CCC] Reloaded config file.");
	Call_StartForward(configReloadedForward);
	Call_Finish();
	return Plugin_Handled;
}

ClearValues(client) {
	Format(tag[client], sizeof(tag[]), "");
	Format(tagColor[client], sizeof(tagColor[]), "");
	Format(usernameColor[client], sizeof(usernameColor[]), "");
	Format(chatColor[client], sizeof(chatColor[]), "");
	
	Format(defaultTag[client], sizeof(defaultTag[]), "");
	Format(defaultTagColor[client], sizeof(defaultTagColor[]), "");
	Format(defaultUsernameColor[client], sizeof(defaultUsernameColor[]), "");
	Format(defaultChatColor[client], sizeof(defaultChatColor[]), "");
}

public OnClientConnected(client) {
	ClearValues(client);
}

public OnClientDisconnect(client) {
	ClearValues(client); // On connect and on disconnect, just to be safe
}

public OnClientPostAdminCheck(client) {
	if(!ConfigForward(client)) {
		return; // Another plugin wants to block this
	}
	// check the Steam ID first
	decl String:auth[32];
	GetClientAuthId(client, AuthId_Steam2, auth, sizeof(auth));
	KvRewind(configFile);
	if(!KvJumpToKey(configFile, auth)) {
		KvRewind(configFile);
		KvGotoFirstSubKey(configFile);
		new AdminId:admin = GetUserAdmin(client);
		new AdminFlag:flag;
		decl String:configFlag[2];
		decl String:section[32];
		new bool:found = false;
		do {
			KvGetSectionName(configFile, section, sizeof(section));
			KvGetString(configFile, "flag", configFlag, sizeof(configFlag));
			if(strlen(configFlag) > 1) {
				LogError("Multiple flags given in section \"%s\", which is not allowed. Using first character.", section);
			}
			if(strlen(configFlag) == 0 && StrContains(section, "STEAM_", false) == -1 && StrContains(section, "[U:1:", false) == -1) {
				found = true;
				break;
			}
			if(!FindFlagByChar(configFlag[0], flag)) {
				if(strlen(configFlag) > 0) {
					LogError("Invalid flag given for section \"%s\", skipping", section);
				}
				continue;
			}
			if(GetAdminFlag(admin, flag)) {
				found = true;
				break;
			}
		} while(KvGotoNextKey(configFile));
		if(!found) {
			return;
		}
	}
	decl String:clientTagColor[12];
	decl String:clientNameColor[12];
	decl String:clientChatColor[12];
	KvGetString(configFile, "tag", tag[client], sizeof(tag[]));
	KvGetString(configFile, "tagcolor", clientTagColor, sizeof(clientTagColor));
	KvGetString(configFile, "namecolor", clientNameColor, sizeof(clientNameColor));
	KvGetString(configFile, "textcolor", clientChatColor, sizeof(clientChatColor));
	ReplaceString(clientTagColor, sizeof(clientTagColor), "#", "");
	ReplaceString(clientNameColor, sizeof(clientNameColor), "#", "");
	ReplaceString(clientChatColor, sizeof(clientChatColor), "#", "");
	new tagLen = strlen(clientTagColor);
	new nameLen = strlen(clientNameColor);
	new chatLen = strlen(clientChatColor);
	if(tagLen == 6 || tagLen == 8 || StrEqual(clientTagColor, "T", false) || StrEqual(clientTagColor, "G", false) || StrEqual(clientTagColor, "O", false)) {
		strcopy(tagColor[client], sizeof(tagColor[]), clientTagColor);
	}
	if(nameLen == 6 || nameLen == 8 || StrEqual(clientNameColor, "G", false) || StrEqual(clientNameColor, "O", false)) {
		strcopy(usernameColor[client], sizeof(usernameColor[]), clientNameColor);
	}
	if(chatLen == 6 || chatLen == 8 || StrEqual(clientChatColor, "T", false) || StrEqual(clientChatColor, "G", false) || StrEqual(clientChatColor, "O", false)) {
		strcopy(chatColor[client], sizeof(chatColor[]), clientChatColor);
	}
	strcopy(defaultTag[client], sizeof(defaultTag[]), tag[client]);
	strcopy(defaultTagColor[client], sizeof(defaultTagColor[]), tagColor[client]);
	strcopy(defaultUsernameColor[client], sizeof(defaultUsernameColor[]), usernameColor[client]);
	strcopy(defaultChatColor[client], sizeof(defaultChatColor[]), chatColor[client]);
	Call_StartForward(loadedForward);
	Call_PushCell(client);
	Call_Finish();
}

public Action CP_OnChatMessage(int& author, ArrayList recipients, char[] flagstring, char[] name, char[] message, bool& processcolors, bool& removecolors) {
	if(CheckForward(author, message, CCC_NameColor)) {
		if(StrEqual(usernameColor[author], "G", false)) {
			Format(name, MAXLENGTH_NAME, "\x04%s", name);
		} else if(StrEqual(usernameColor[author], "O", false)) {
			Format(name, MAXLENGTH_NAME, "\x05%s", name);
		} else if(strlen(usernameColor[author]) == 6) {
			Format(name, MAXLENGTH_NAME, "\x07%s%s", usernameColor[author], name);
		} else if(strlen(usernameColor[author]) == 8) {
			Format(name, MAXLENGTH_NAME, "\x08%s%s", usernameColor[author], name);
		} else {
			Format(name, MAXLENGTH_NAME, "\x03%s", name); // team color by default!
		}
	} else {
		Format(name, MAXLENGTH_NAME, "\x03%s", name); // team color by default!
	}
	if(CheckForward(author, message, CCC_TagColor)) {
		if(strlen(tag[author]) > 0) {
			if(StrEqual(tagColor[author], "T", false)) {
				Format(name, MAXLENGTH_NAME, "\x03%s%s", tag[author], name);
			} else if(StrEqual(tagColor[author], "G", false)) {
				Format(name, MAXLENGTH_NAME, "\x04%s%s", tag[author], name);
			} else if(StrEqual(tagColor[author], "O", false)) {
				Format(name, MAXLENGTH_NAME, "\x05%s%s", tag[author], name);
			} else if(strlen(tagColor[author]) == 6) {
				Format(name, MAXLENGTH_NAME, "\x07%s%s%s", tagColor[author], tag[author], name);
			} else if(strlen(tagColor[author]) == 8) {
				Format(name, MAXLENGTH_NAME, "\x08%s%s%s", tagColor[author], tag[author], name);
			} else {
				Format(name, MAXLENGTH_NAME, "\x01%s%s", tag[author], name);
			}
		}
	}
	
	new MaxMessageLength = MAXLENGTH_MESSAGE - strlen(name) - 5; // MAXLENGTH_MESSAGE = maximum characters in a chat message, including name. Subtract the characters in the name, and 5 to account for the colon, spaces, and null terminator
	
	if(strlen(chatColor[author]) > 0 && CheckForward(author, message, CCC_ChatColor)) {
		if(StrEqual(chatColor[author], "T", false)) {
			Format(message, MaxMessageLength, "\x03%s", message);
		} else if(StrEqual(chatColor[author], "G", false)) {
			Format(message, MaxMessageLength, "\x04%s", message);
		} else if(StrEqual(chatColor[author], "O", false)) {
			Format(message, MaxMessageLength, "\x05%s", message);
		} else if(strlen(chatColor[author]) == 6) {
			Format(message, MaxMessageLength, "\x07%s%s", chatColor[author], message);
		} else if(strlen(chatColor[author]) == 8) {
			Format(message, MaxMessageLength, "\x08%s%s", chatColor[author], message);
		}
	}
	decl String:game[64];
	GetGameFolderName(game, sizeof(game));
	if(StrEqual(game, "csgo")) {
		Format(name, MAXLENGTH_NAME, "\x01\x0B%s", name);
	}
	
	Call_StartForward(messageForward);
	Call_PushCell(author);
	Call_PushStringEx(message, MaxMessageLength, SM_PARAM_STRING_UTF8|SM_PARAM_STRING_COPY, SM_PARAM_COPYBACK);
	Call_PushCell(MaxMessageLength);
	Call_Finish();
	
	return Plugin_Changed;
}

bool:CheckForward(author, const String:message[], CCC_ColorType:type) {
	new Action:result = Plugin_Continue;
	Call_StartForward(applicationForward);
	Call_PushCell(author);
	Call_PushString(message);
	Call_PushCell(type);
	Call_Finish(result);
	if(result >= Plugin_Handled) {
		return false;
	}
	
	// Compatibility
	switch(type) {
		case CCC_TagColor: return TagForward(author);
		case CCC_NameColor: return NameForward(author);
		case CCC_ChatColor: return ColorForward(author);
	}
	
	return true;
}

bool:ColorForward(author) {
	new Action:result = Plugin_Continue;
	Call_StartForward(colorForward);
	Call_PushCell(author);
	Call_Finish(result);
	if(result >= Plugin_Handled) {
		return false;
	}
	
	return true;
}

bool:NameForward(author) {
	new Action:result = Plugin_Continue;
	Call_StartForward(nameForward);
	Call_PushCell(author);
	Call_Finish(result);
	if(result >= Plugin_Handled) {
		return false;
	}
	
	return true;
}

bool:TagForward(author) {
	new Action:result = Plugin_Continue;
	Call_StartForward(tagForward);
	Call_PushCell(author);
	Call_Finish(result);
	if(result >= Plugin_Handled) {
		return false;
	}
	
	return true;
}

bool:ConfigForward(client) {
	new Action:result = Plugin_Continue;
	Call_StartForward(preLoadedForward);
	Call_PushCell(client);
	Call_Finish(result);
	if(result >= Plugin_Handled) {
		return false;
	}
	
	return true;
}

public Native_GetColor(Handle:plugin, numParams) {
	new client = GetNativeCell(1);
	if(client < 1 || client > MaxClients || !IsClientInGame(client)) {
		ThrowNativeError(SP_ERROR_PARAM, "Invalid client or client is not in game");
		return COLOR_NONE;
	}
	switch(GetNativeCell(2)) {
		case CCC_TagColor: {
			if(StrEqual(tagColor[client], "T", false)) {
				SetNativeCellRef(3, false);
				return COLOR_TEAM;
			} else if(StrEqual(tagColor[client], "G", false)) {
				SetNativeCellRef(3, false);
				return COLOR_GREEN;
			} else if(StrEqual(tagColor[client], "O", false)) {
				SetNativeCellRef(3, false);
				return COLOR_OLIVE;
			} else if(strlen(tagColor[client]) == 6 || strlen(tagColor[client]) == 8) {
				SetNativeCellRef(3, strlen(tagColor[client]) == 8);
				return StringToInt(tagColor[client], 16);
			} else {
				SetNativeCellRef(3, false);
				return COLOR_NONE;
			}
		}
		case CCC_NameColor: {
			if(StrEqual(usernameColor[client], "G", false)) {
				SetNativeCellRef(3, false);
				return COLOR_GREEN;
			} else if(StrEqual(usernameColor[client], "O", false)) {
				SetNativeCellRef(3, false);
				return COLOR_OLIVE;
			} else if(strlen(usernameColor[client]) == 6 || strlen(usernameColor[client]) == 8) {
				SetNativeCellRef(3, strlen(usernameColor[client]) == 8);
				return StringToInt(usernameColor[client], 16);
			} else {
				SetNativeCellRef(3, false);
				return COLOR_TEAM;
			}
		}
		case CCC_ChatColor: {
			if(StrEqual(chatColor[client], "T", false)) {
				SetNativeCellRef(3, false);
				return COLOR_TEAM;
			} else if(StrEqual(chatColor[client], "G", false)) {
				SetNativeCellRef(3, false);
				return COLOR_GREEN;
			} else if(StrEqual(chatColor[client], "O", false)) {
				SetNativeCellRef(3, false);
				return COLOR_OLIVE;
			} else if(strlen(chatColor[client]) == 6 || strlen(chatColor[client]) == 8) {
				SetNativeCellRef(3, strlen(chatColor[client]) == 8);
				return StringToInt(chatColor[client], 16);
			} else {
				SetNativeCellRef(3, false);
				return COLOR_NONE;
			}
		}
	}
	return COLOR_NONE;
}

public Native_SetColor(Handle:plugin, numParams) {
	new client = GetNativeCell(1);
	if(client < 1 || client > MaxClients || !IsClientInGame(client)) {
		ThrowNativeError(SP_ERROR_PARAM, "Invalid client or client is not in game");
		return false;
	}
	decl String:color[32];
	if(GetNativeCell(3) < 0) {
		switch(GetNativeCell(3)) {
			case COLOR_GREEN: {
				Format(color, sizeof(color), "G");
			}
			case COLOR_OLIVE: {
				Format(color, sizeof(color), "O");
			}
			case COLOR_TEAM: {
				Format(color, sizeof(color), "T");
			}
			case COLOR_NONE: {
				Format(color, sizeof(color), "");
			}
		}
	} else {
		if(!GetNativeCell(4)) {
			// No alpha
			Format(color, sizeof(color), "%06X", GetNativeCell(3));
		} else {
			// Alpha specified
			Format(color, sizeof(color), "%08X", GetNativeCell(3));
		}
	}
	if(strlen(color) != 6 && strlen(color) != 8 && !StrEqual(color, "G", false) && !StrEqual(color, "O", false) && !StrEqual(color, "T", false)) {
		return false;
	}
	switch(GetNativeCell(2)) {	
		case CCC_TagColor: {
			strcopy(tagColor[client], sizeof(tagColor[]), color);
		}
		case CCC_NameColor: {
			strcopy(usernameColor[client], sizeof(usernameColor[]), color);
		}
		case CCC_ChatColor: {
			strcopy(chatColor[client], sizeof(chatColor[]), color);
		}
	}
	return true;
}

public Native_GetTag(Handle:plugin, numParams) {
	new client = GetNativeCell(1);
	if(client < 1 || client > MaxClients || !IsClientInGame(client)) {
		ThrowNativeError(SP_ERROR_PARAM, "Invalid client or client is not in game");
		return;
	}
	SetNativeString(2, tag[client], GetNativeCell(3));
}

public Native_SetTag(Handle:plugin, numParams) {
	new client = GetNativeCell(1);
	if(client < 1 || client > MaxClients || !IsClientInGame(client)) {
		ThrowNativeError(SP_ERROR_PARAM, "Invalid client or client is not in game");
		return;
	}
	GetNativeString(2, tag[client], sizeof(tag[]));
}

public Native_ResetColor(Handle:plugin, numParams) {
	new client = GetNativeCell(1);
	if(client < 1 || client > MaxClients || !IsClientInGame(client)) {
		ThrowNativeError(SP_ERROR_PARAM, "Invalid client or client is not in game");
		return;
	}
	switch(GetNativeCell(2)) {
		case CCC_TagColor: {
			strcopy(tagColor[client], sizeof(tagColor[]), defaultTagColor[client]);
		}
		case CCC_NameColor: {
			strcopy(usernameColor[client], sizeof(usernameColor[]), defaultUsernameColor[client]);
		}
		case CCC_ChatColor: {
			strcopy(chatColor[client], sizeof(chatColor[]), defaultChatColor[client]);
		}
	}
}

public Native_ResetTag(Handle:plugin, numParams) {
	new client = GetNativeCell(1);
	if(client < 1 || client > MaxClients || !IsClientInGame(client)) {
		ThrowNativeError(SP_ERROR_PARAM, "Invalid client or client is not in game");
		return;
	}
	strcopy(tag[client], sizeof(tag[]), defaultTag[client]);
}