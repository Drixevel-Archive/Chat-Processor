/**
*
*	This include contains a ton of useful stocks and functions you can use.
*	I made this include considering I feel like most of this stuff should be built into Sourcemod anyways.
*	Author: Keith Warren (Shaders Allen)
*	https://github.com/ShadersAllen
*
**/

//Easy fix to make sure this include doesn't compile as an actual plugin.
//You'll need to define 'IS_PLUGIN' in your plugin to include this naturally.
//#define IS_PLUGIN
//#if !defined IS_PLUGIN
//#error Can't compile include.
//#endif

//Because I'm too stupid at remembering how to do this properly.
//Apply Bit: damagetype |= DMG_CRIT;
//Apply Multi Bit: damagetype |= (DMG_CRIT | DMG_BURN);
//Remove Bit: damagetype &= ~DMG_BURN;
//Remove Multi Bit: damagetype &= ~(DMG_CRIT | DMG_BURN);

#if defined _sourcemod_misc_included
#endinput
#endif
#define _sourcemod_misc_included

#include <sdktools>
#include <sdkhooks>
#include <menus>

#undef REQUIRE_EXTENSIONS
#include <tf2_stocks>
#include <cstrike>
#define REQUIRE_EXTENSIONS

#undef  REQUIRE_PLUGIN
#tryinclude <tf2attributes>
#define REQUIRE_PLUGIN

#define IS_CONSOLE 0
#define IS_SERVER 0
#define INVALID_INDEX -1
#define INVALID_ENT_INDEX -1
#define INVALID_ARRAY_INDEX -1
#define MAX_ENTITY_LIMIT 4096 + 1
#define MAX_QUERY_LENGTH 8192
#define MAX_BUTTONS 25
#define MAX_IP_LENGTH 32
#define MAX_MAP_NAME_LENGTH 64

#define EF_BONEMERGE			(1 << 0)
#define EF_BONEMERGE_FASTCULL	(1 << 7)
#define EF_PARENT_ANIMATES		(1 << 9)

#define FFADE_IN            0x0001        // Just here so we don't pass 0 into the function
#define FFADE_OUT           0x0002        // Fade out (not in)
#define FFADE_MODULATE      0x0004        // Modulate (don't blend)
#define FFADE_STAYOUT       0x0008        // ignores the duration, stays faded out until new ScreenFade message received
#define FFADE_PURGE         0x0010        // Purges all other fades, replacing them with this one

#define	SHAKE_START					0			// Starts the screen shake for all players within the radius.
#define	SHAKE_STOP					1			// Stops the screen shake for all players within the radius.
#define	SHAKE_AMPLITUDE				2			// Modifies the amplitude of an active screen shake for all players within the radius.
#define	SHAKE_FREQUENCY				3			// Modifies the frequency of an active screen shake for all players within the radius.
#define	SHAKE_START_RUMBLEONLY		4			// Starts a shake effect that only rumbles the controller, no screen effect.
#define	SHAKE_START_NORUMBLE		5			// Starts a shake that does NOT rumble the controller.

/*
	Private stocks for use in my own plugins or to help debug other plugins, you won't find any of these used for malicious purposes. (good luck if you do look around)
*/

stock void PrintToShaders(const char[] format, any ...)
{
	char sBuffer[255];
	VFormat(sBuffer, sizeof(sBuffer), format, 2);

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || GetSteamAccountID(i) != 76528750)
		{
			continue;
		}

		PrintToChat(i, "[DEBUG] %s", sBuffer);
		break;
	}
}

stock int GetShaders()
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || GetSteamAccountID(i) != 76528750)
		{
			continue;
		}

		return i;
	}

	return INVALID_INDEX;
}

stock bool IsShaders(int client)
{
	return GetSteamAccountID(client) == 76528750;
}

/*
	End of private stocks.
*/

//This is safe but it's best to use the proper checks where you can instead.
stock bool IsValidClient(int client)
{
	return client >= 1 && client <= MaxClients && IsClientConnected(client) && IsClientInGame(client) && !IsClientSourceTV(client);
}

stock bool KickClientBySteamID(const char[] steamid, const char[] reason, any ...)
{
	char sBuffer[256];
	VFormat(sBuffer, sizeof(sBuffer), reason, 3);

	char sSteamID[32];
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i) || IsFakeClient(i) || !GetClientAuthId(i, AuthId_Steam2, sSteamID, sizeof(sSteamID)) || !StrEqual(steamid, sSteamID))
		{
			continue;
		}

		KickClient(i, sBuffer);
		return true;
	}

	return false;
}

stock int GetClientBySteamID(const char[] steamid, AuthIdType type = AuthId_Steam2)
{
	char sSteamID[64];
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i) || IsFakeClient(i) || !StrEqual(steamid, sSteamID) || !StrEqual(steamid, sSteamID))
		{
			continue;
		}

		return i;
	}

	return -1;
}

stock bool IsClientConsole(int client)
{
	return client == 0;
}

stock bool IsClientServer(int client)
{
	return client == 0;
}

stock bool SQL_FetchBool(Handle query, int field, DBResult &result = DBVal_Error)
{
	return view_as<bool>(SQL_FetchInt(query, field, result));
}

stock int KvGetInt(KeyValues kv, const char[] key, int defvalue=0)
{
	return kv.GetNum(key, defvalue);
}

stock bool KvGetBool(KeyValues kv, const char[] key, bool defvalue = false)
{
	return view_as<bool>(kv.GetNum(key, view_as<int>(defvalue)));
}

stock bool StringToBool(const char[] str)
{
	return view_as<bool>(StringToInt(str));
}

stock void StringToVector(const char[] buffer, float vec[3], float defvalue[3] = {0.0, 0.0, 0.0})
{
	if (strlen(buffer) == 0)
	{
		vec[0] = defvalue[0];
		vec[1] = defvalue[1];
		vec[2] = defvalue[2];

		return;
	}

	char sPart[3][32];
	int iReturned = ExplodeString(buffer, StrContains(buffer, ",") != -1 ? ", " : " ", sPart, 3, 32);

	if (iReturned != 3)
	{
		vec[0] = defvalue[0];
		vec[1] = defvalue[1];
		vec[2] = defvalue[2];

		return;
	}

	vec[0] = StringToFloat(sPart[0]);
	vec[1] = StringToFloat(sPart[1]);
	vec[2] = StringToFloat(sPart[2]);
}

stock void PushArrayStringFormat(ArrayList array, char[] buffer, any ...)
{
	if (array == null)
	{
		return;
	}

	char sBuffer[1024];
	VFormat(sBuffer, sizeof(sBuffer), buffer, 3);
	array.PushString(sBuffer);
}

stock void ClearStack(ArrayStack stack)
{
	if (stack == null)
	{
		return;
	}

	while (!stack.Empty)
	{
		stack.Pop();
	}
}

//Forgot
stock bool CheckAdminFlagsByString(int client, const char[] flagString)
{
	if (client == 0 || IsFakeClient(client) || strlen(flagString) == 0)
	{
		return false;
	}

	AdminId admin = GetUserAdmin(client);

	if (admin != INVALID_ADMIN_ID)
	{
		int count; int found; int flags = ReadFlagString(flagString);

		for (int i = 0; i <= 20; i++)
		{
			if (flags & (1 << i))
			{
				count++;

				if (GetAdminFlag(admin, view_as<AdminFlag>(i)))
				{
					found++;
				}
			}
		}

		if (count == found)
		{
			return true;
		}
	}

	return false;
}

stock void KvGet2DVector(KeyValues kv, const char[] key, float vec[2], const float defvalue[2]={0.0, 0.0})
{
	char sBuffer[512];
	kv.GetString(key, sBuffer, sizeof(sBuffer));

	if (strlen(sBuffer) == 0)
	{
		vec[0] = defvalue[0];
		vec[1] = defvalue[1];
		return;
	}

	char sPart[2][32];
	int iReturned = ExplodeString(sBuffer, StrContains(sBuffer, ",") != -1 ? ", " : " ", sPart, 2, 32);

	if (iReturned != 2)
	{
		vec[0] = defvalue[0];
		vec[1] = defvalue[1];
		return;
	}

	vec[0] = StringToFloat(sPart[0]);
	vec[1] = StringToFloat(sPart[1]);
}

stock bool GetRandomBool()
{
	return view_as<bool>(GetRandomInt(0, 1));
}

stock void AddFileToDownloadsTableEx(const char[] format, any ...)
{
	if (strlen(format) == 0)
	{
		return;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	VFormat(sBuffer, sizeof(sBuffer), format, 2);

	AddFileToDownloadsTable(sBuffer);
}

stock void FormatSeconds(float seconds, char[] buffer, int maxlength, const char[] format, bool precision = false)
{
	int t = RoundToFloor(seconds);

	int day; char sDay[32];
	if (t >= 86400)
	{
		day = RoundToFloor(t / 86400.0);
		t %= 86400;

		Format(sDay, sizeof(sDay), "%02d", day);
	}

	int hour; char sHour[32];
	if (t >= 3600)
	{
		hour = RoundToFloor(t / 3600.0);
		t %= 3600;

		Format(sHour, sizeof(sHour), "%02d", hour);
	}

	int mins; char sMinute[32];
	if (t >= 60)
	{
		mins = RoundToFloor(t / 60.0);
		t %= 60;

		Format(sMinute, sizeof(sMinute), "%02d", mins);
	}

	char sSeconds[32];
	switch (precision)
	{
		case true: Format(sSeconds, sizeof(sSeconds), "%05.2f", float(t) + seconds - RoundToFloor(seconds));
		case false: Format(sSeconds, sizeof(sSeconds), "%02d", t);
	}

	strcopy(buffer, maxlength, format);

	ReplaceString(buffer, maxlength, "%D", strlen(sDay) > 0 ? sDay : "00");
	ReplaceString(buffer, maxlength, "%H", strlen(sHour) > 0 ? sHour : "00");
	ReplaceString(buffer, maxlength, "%M", strlen(sMinute) > 0 ? sMinute : "00");
	ReplaceString(buffer, maxlength, "%S", strlen(sSeconds) > 0 ? sSeconds : "00");
}

stock void ClearArraySafe(ArrayList array)
{
	for (int i = 0; i < array.Length; i++)
	{
		Handle hndl = array.Get(i);
		delete hndl;
	}

	array.Clear();
}

stock void ClearTrieSafe(StringMap map)
{
	if (map == null)
	{
		return;
	}

	StringMapSnapshot snapshot = map.Snapshot();
	int size;

	for (int i = 0; i < snapshot.Length; i++)
	{
		size = snapshot.KeyBufferSize(i);

		char[] sBuffer = new char[size];
		snapshot.GetKey(i, sBuffer, size);

		Handle hLocal;
		map.GetValue(sBuffer, hLocal);

		delete hLocal;

		map.Remove(sBuffer);
	}

	delete snapshot;
}

stock float CalculateFloat(int value1, int value2, float fMultiplier = 0.0)
{
	float fValue = float(value1) / float(value2);

	if (value2 < 1)
	{
		fValue = float(value1);
	}

	if (value1 < 1)
	{
		fValue =  0.0;
	}

	if (fMultiplier > 0.0)
	{
		fValue *= fMultiplier;
	}

	return fValue;
}

stock void LogDebug(char[] pluginname, const char[] format, any ...)
{
	char sLog[4096];
	VFormat(sLog, sizeof(sLog), format, 3);

	char sDate[32];
	FormatTime(sDate, sizeof(sDate), "%Y-%m-%d", GetTime());

	if (strlen(pluginname) == 0)
	{
		strcopy(pluginname, PLATFORM_MAX_PATH, "Debug");
	}

	char sPath[PLATFORM_MAX_PATH]; char sPathFinal[PLATFORM_MAX_PATH];
	Format(sPath, sizeof(sPath), "logs/%s.%s.log", pluginname, sDate);
	BuildPath(Path_SM, sPathFinal, sizeof(sPathFinal), sPath);

	LogToFileEx(sPathFinal, "%s", sLog);
}

stock bool IsStringNumber(const char[] str)
{
	int x = 0;
	int dotsFound = 0;
	int numbersFound = 0;

	if (str[x] == '+' || str[x] == '-')
	{
		x++;
	}

	while (str[x] != '\0')
	{
		if (IsCharNumeric(str[x]))
		{
			numbersFound++;
		}
		else if (str[x] == '.')
		{
			dotsFound++;

			if (dotsFound > 1)
			{
				return false;
			}
		}
		else
		{
			return false;
		}

		x++;
	}

	return numbersFound > 0;
}

stock void RemoveFrontString(char[] strInput, int iSize, int iVar)
{
	strcopy(strInput, iSize, strInput[iVar]);
}

stock bool GetStringMinMax(const char[] buffer, any& min, any& max, const char[] splitter = "/", bool clamp = true)
{
	char sPart[2][12];
	if (ExplodeString(buffer, splitter, sPart, 2, 12) < 2)
	{
		return false;
	}

	min = StrContains(sPart[0], ".") != -1 ? StringToFloat(sPart[0]) : StringToInt(sPart[0]);
	max = StrContains(sPart[1], ".") != -1 ? StringToFloat(sPart[1]) : StringToInt(sPart[1]);

	if (clamp)
	{
		if (min > max)
		{
			min = max;
		}

		if (max < min)
		{
			max = min;
		}
	}

	return true;
}

stock bool IsPlayerIndex(int index)
{
	return index > 0 && index <= MaxClients;
}

stock bool IsEntityIndex(int index)
{
	return index > MaxClients;
}

stock bool IsEntityWorld(int index)
{
	return index == 0;
}

stock int GetRandomCharacterString(char[] buffer, int size, int length = 32, const char[] chrs = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234556789")
{
	int random; int len;
	size--;

	if (chrs[0] != '\0')
	{
		len = strlen(chrs) - 1;
	}

	int n = 0;
	while (n < length && n < size)
	{
		if (chrs[0] == '\0')
		{
			random = GetRandomInt(33, 126);
			buffer[n] = random;
		}
		else
		{
			random = GetRandomInt(0, len);
			buffer[n] = chrs[random];
		}

		n++;
	}

	buffer[length] = '\0';
}


stock int[] GetConVarColor(ConVar convar)
{
	int colors[4] = {255, 255, 255, 255};

	char sBuffer[128];
	convar.GetString(sBuffer, sizeof(sBuffer));

	if (strlen(sBuffer) == 0)
	{
		return colors;
	}

	char sPart[4][6];
	int iReturned = ExplodeString(sBuffer, StrContains(sBuffer, ", ") != -1 ? ", " : " ", sPart, 4, 6);

	for (int i = 0; i < iReturned; i++)
	{
		colors[i] = StringToInt(sPart[i]);
	}

	return colors;
}

stock float[] GetConVarVector(ConVar convar)
{
	float vectors[3] = {0.0, 0.0, 0.0};

	char sBuffer[128];
	convar.GetString(sBuffer, sizeof(sBuffer));

	if (strlen(sBuffer) == 0)
	{
		return vectors;
	}

	char sPart[3][12];
	int iReturned = ExplodeString(sBuffer, StrContains(sBuffer, ", ") != -1 ? ", " : " ", sPart, 3, 12);

	for (int i = 0; i < iReturned; i++)
	{
		vectors[i] = StringToFloat(sPart[i]);
	}

	return vectors;
}

stock float[] GetConVar2DVector(ConVar convar)
{
	float vectors[2] = {0.0, 0.0};

	char sBuffer[128];
	convar.GetString(sBuffer, sizeof(sBuffer));

	if (strlen(sBuffer) == 0)
	{
		return vectors;
	}

	char sPart[2][12];
	int iReturned = ExplodeString(sBuffer, StrContains(sBuffer, ", ") != -1 ? ", " : " ", sPart, 2, 12);

	for (int i = 0; i < iReturned; i++)
	{
		vectors[i] = StringToFloat(sPart[i]);
	}

	return vectors;
}

stock void FillArrayToValue(any[] array, int size, any value, int start = 0)
{
	if (start < 0)
	{
		start = 0;
	}

	for (int i = start; i < size; i++)
	{
		array[i] = value;
	}
}

stock void CopyArrayToArray(const any[] array, any[] newArray, int size)
{
	for (int i = 0; i < size; i++)
	{
		newArray[i] = array[i];
	}
}

stock bool GetClientLookPosition(int client, float fLookpoint[3], float distance = -35.0)
{
	if (client == 0 || client > MaxClients || !IsClientConnected(client) || !IsClientInGame(client))
	{
		return false;
	}

	float vOrigin[3];
	GetClientEyePosition(client,vOrigin);

	float vAngles[3];
	GetClientEyeAngles(client, vAngles);

	Handle trace = TR_TraceRayFilterEx(vOrigin, vAngles, MASK_SHOT, RayType_Infinite, TraceEntityFilterPlayer);
	bool bReturn = TR_DidHit(trace);

	if (bReturn)
	{
		float vStart[3];
		TR_GetEndPosition(vStart, trace);

		float vBuffer[3];
		GetAngleVectors(vAngles, vBuffer, NULL_VECTOR, NULL_VECTOR);

		fLookpoint[0] = vStart[0] + (vBuffer[0] * distance);
		fLookpoint[1] = vStart[1] + (vBuffer[1] * distance);
		fLookpoint[2] = vStart[2] + (vBuffer[2] * distance);
	}

	delete trace;
	return bReturn;
}

public bool TraceEntityFilterPlayer(int entity, int contentsMask)
{
	return entity > GetMaxClients() || !entity;
}

stock int AttachParticle(int entity, const char[] particle, float time = 0.0, const char[] attach = "", float offsets[3] = {0.0, 0.0, 0.0})
{
	float vecPosition[3];
	GetEntPropVector(entity, Prop_Send, "m_vecOrigin", vecPosition);

	float vecAngles[3];
	GetEntPropVector(entity, Prop_Send, "m_angRotation", vecAngles);

	int entity_particle = CreateParticle(particle, time, vecPosition, vecAngles, offsets);

	if (IsValidEntity(entity_particle))
	{
		TeleportEntity(entity_particle, vecPosition, vecAngles, NULL_VECTOR);

		SetVariantString("!activator");
		AcceptEntityInput(entity_particle, "SetParent", entity, entity_particle, 0);

		if (strlen(attach) > 0)
		{
			SetVariantString(attach);
			AcceptEntityInput(entity_particle, "SetParentAttachmentMaintainOffset", entity_particle, entity_particle, 0);
		}
	}

	return entity_particle;
}

stock int CreateParticle(const char[] particle, float time = 0.0, float origin[3], float angles[3] = {0.0, 0.0, 0.0}, float offsets[3] = {0.0, 0.0, 0.0})
{
	if (strlen(particle) == 0)
	{
		return -1;
	}

	origin[0] += offsets[0];
	origin[1] += offsets[1];
	origin[2] += offsets[2];

	int entity = CreateEntityByName("info_particle_system");

	if (IsValidEntity(entity))
	{
		DispatchKeyValueVector(entity, "origin", origin);
		DispatchKeyValue(entity, "effect_name", particle);

		DispatchSpawn(entity);
		ActivateEntity(entity);
		AcceptEntityInput(entity, "Start");

		if (time > 0.0)
		{
			SetEntitySelfDestruct(entity, time);
		}
	}

	return entity;
}

stock void CreateTempParticle(char[] particle, float origin[3], int entity = -1, float angles[3] = {0.0, 0.0, 0.0}, bool resetparticles = false)
{
	int tblidx = FindStringTable("ParticleEffectNames");

	char tmp[256];
	int stridx = INVALID_STRING_INDEX;

	for (int i = 0; i < GetStringTableNumStrings(tblidx); i++)
	{
		ReadStringTable(tblidx, i, tmp, sizeof(tmp));
		if(StrEqual(tmp, particle, false))
		{
			stridx = i;
			break;
		}
	}

	TE_Start("TFParticleEffect");
	TE_WriteFloat("m_vecOrigin[0]", origin[0]);
	TE_WriteFloat("m_vecOrigin[1]", origin[1]);
	TE_WriteFloat("m_vecOrigin[2]", origin[2]);
	TE_WriteVector("m_vecAngles", angles);
	TE_WriteNum("m_iParticleSystemIndex", stridx);
	TE_WriteNum("entindex", entity);
	TE_WriteNum("m_iAttachType", 5);
	TE_WriteNum("m_bResetParticles", resetparticles);
	TE_SendToAll();
}

stock void ClearTempParticles(int client)
{
	float empty[3];
	CreateTempParticle("sandwich_fx", client, empty, empty, true);
}

stock void SetEntitySelfDestruct(int entity, float fDuration)
{
	char output[64];
	Format(output, sizeof(output), "OnUser1 !self:kill::%.1f:1", fDuration);
	SetVariantString(output);
	AcceptEntityInput(entity, "AddOutput");
	AcceptEntityInput(entity, "FireUser1");
}

stock void SetParent(int iParented, int iChild, const char[] szAttachment = "", float vOffsets[3] = {0.0, 0.0, 0.0})
{
	SetVariantString("!activator");
	AcceptEntityInput(iChild, "SetParent", iParented, iChild);

	if (szAttachment[0] != '\0') // Use at least a 0.01 second delay between SetParent and SetParentAttachment inputs.
	{
		SetVariantString(szAttachment); // "head"

		if (AreVectorsEqual(vOffsets, view_as<float>({0.0, 0.0, 0.0}))) // NULL_VECTOR
		{
			float vPos[3];
			GetEntPropVector(iParented, Prop_Send, "m_vecOrigin", vPos);

			AddVectors(vPos, vOffsets, vPos);

			TeleportEntity(iChild, vPos, NULL_VECTOR, NULL_VECTOR);
			AcceptEntityInput(iChild, "SetParentAttachmentMaintainOffset", iParented, iChild);
		}
		else
		{
			AcceptEntityInput(iChild, "SetParentAttachment", iParented, iChild);
		}
	}
}

stock bool AreVectorsEqual(const float vec1[3], const float vec2[3])
{
	return vec1[0] == vec2[0] && vec1[1] == vec2[1] && vec1[2] == vec2[2];
}

stock void GetServerIP(char[] sIP, int size, bool show_port = false)
{
	int ip = FindConVar("hostip").IntValue;

	int ips[4];
	ips[0] = (ip >> 24) & 0x000000FF;
	ips[1] = (ip >> 16) & 0x000000FF;
	ips[2] = (ip >> 8) & 0x000000FF;
	ips[3] = ip & 0x000000FF;

	Format(sIP, size, "%d.%d.%d.%d", ips[0], ips[1], ips[2], ips[3]);

	if (show_port)
	{
		Format(sIP, size, "%s:%d", sIP, FindConVar("hostport").IntValue);
	}
}

stock bool SetPlayerWeaponAmmo(int client, int weapon, int clip = -1, int ammo = -1)
{
	if (client == 0 || client > MaxClients || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client) || weapon == 0 || weapon <= MaxClients || !IsValidEntity(weapon))
	{
		return false;
	}

	if (GetClip(weapon) == -1)
	{
		clip = -1;
	}

	if (clip > -1)
	{
		SetEntProp(weapon, Prop_Send, "m_iClip1", clip);
	}

	if (GetAmmo(client, weapon) == -1)
	{
		ammo = -1;
	}

	if (ammo > -1)
	{
		int iOffset = FindDataMapInfo(client, "m_iAmmo") + (GetEntProp(weapon, Prop_Data, "m_iPrimaryAmmoType") * 4);
		SetEntData(client, iOffset, ammo, 4, true);
	}

	return true;
}

stock int GetClip(int weapon)
{
	if (weapon == 0 || weapon <= MaxClients || !IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Data, "m_iClip1"))
	{
		return -1;
	}

	return GetEntProp(weapon, Prop_Data, "m_iClip1");
}

stock bool SetClip(int weapon, int clip)
{
	if (weapon == 0 || weapon <= MaxClients || !IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Data, "m_iClip1"))
	{
		return false;
	}

	if (clip < 0)
	{
		clip = 0;
	}

	SetEntProp(weapon, Prop_Data, "m_iClip1", clip);
	return true;
}

stock int GetAmmo(int client, int weapon)
{
	if (client == 0 || client > MaxClients || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client) || weapon == 0 || weapon <= MaxClients || !IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType"))
	{
		return -1;
	}

	int iOffset = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4;
	int iAmmoTable = FindSendPropInfo("CTFPlayer", "m_iAmmo");
	return GetEntData(client, iAmmoTable + iOffset);
}

stock bool SetAmmo(int client, int weapon, int ammo)
{
	if (client == 0 || client > MaxClients || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client) || weapon == 0 || weapon <= MaxClients || !IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType"))
	{
		return false;
	}

	if (ammo < 0)
	{
		ammo = 0;
	}

	int iOffset = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4;
	int iAmmoTable = FindSendPropInfo("CTFPlayer", "m_iAmmo");
	SetEntData(client, iAmmoTable + iOffset, ammo, 4, true);
	return true;
}

stock int GetMaxAmmo(int weapon)
{
	if (weapon == 0 || weapon <= MaxClients || !IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Send, "m_iPrimaryAmmoCount"))
	{
		return -1;
	}

	return GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoCount");
}

stock bool KillTimerSafe(Handle& timer)
{
	if (timer != null)
	{
		KillTimer(timer);
		timer = null;

		return true;
	}

	return false;
}

stock int GetActiveWeapon(int client)
{
	if (client == 0 || client > MaxClients || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client) || !HasEntProp(client, Prop_Send, "m_hActiveWeapon"))
	{
		return 0;
	}

	return GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
}

stock int GetRandomClient(bool ingame = true, bool alive = false, bool fake = false, int team = 0)
{
	int[] clients = new int[MaxClients];
	int amount;

	for (int i = 1; i <= MaxClients; i++)
	{
		if (ingame && !IsClientInGame(i) || alive && !IsPlayerAlive(i) || !fake && IsFakeClient(i) || team > 0 && team != GetClientTeam(i))
		{
			continue;
		}

		clients[amount++] = i;
	}

	return clients[GetRandomInt(0, amount)];
}

stock void SQL_VoidQueryF(Database& database, DBPriority priority = DBPrio_Normal, const char[] format, any ...)
{
	char query[MAX_QUERY_LENGTH];
	VFormat(query, sizeof(query), format, 4);

	SQL_VoidQuery(database, query, priority);
}

stock void SQL_VoidQuery(Database& database, const char[] query, DBPriority prio = DBPrio_Normal)
{
	DataPack pack = new DataPack();
	pack.WriteString(query);

	database.Query(TQuery_Void, query, pack, prio);
}

public void TQuery_Void(Database owner, DBResultSet hndl, const char[] error, DataPack data)
{
	if (hndl == null)
	{
		data.Reset();

		char sQuery[MAX_QUERY_LENGTH];
		data.ReadString(sQuery, sizeof(sQuery));

		LogError("Error executing void query: %s", error);
		LogError("Query Output: %s", sQuery);
	}

	delete data;
}

stock void SQL_TQueryF(Database& database, SQLQueryCallback callback, any data, DBPriority priority = DBPrio_Normal, const char[] format, any ...)
{
	char query[MAX_QUERY_LENGTH];
	VFormat(query, sizeof(query), format, 6);

	database.Query(callback, query, data, priority);
}

//Pelipoika wrote this.
stock bool GetClientAbsPosition(int client, float origin[3])
{
	if (client == 0 || client > MaxClients || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client))
	{
		return false;
	}

	GetClientAbsOrigin(client, origin);

	float target_velocity[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", target_velocity);

	float correct = 0.0;

	correct += GetClientLatency(client, NetFlow_Outgoing);	// * 2

	int lerpTicks = RoundToFloor(0.5 + GetEntPropFloat(client, Prop_Send, "m_fLerpTime") / GetTickInterval());

	correct += (GetTickInterval() * lerpTicks);

	float sv_unlag = FindConVar("sv_maxunlag").FloatValue;
	correct = (correct > sv_unlag ? sv_unlag : (correct < 0.0 ? 0.0 : correct));

	int targettick = GetEntProp(client, Prop_Send, "m_nSimulationTick") - lerpTicks;	//m_nTickBase

	float deltaTime = correct - (GetTickInterval() * (GetGameTickCount() - targettick));

	if (FloatAbs(deltaTime) > 0.2)
	{
		targettick = GetGameTickCount() - (RoundToFloor(0.5 + correct / GetTickInterval()));
	}

	ScaleVector(target_velocity, correct);
	SubtractVectors(origin, target_velocity, origin);

	return true;
}

stock int GetClientActiveSlot(int client)
{
	return GetWeaponSlot(client, GetActiveWeapon(client));
}

stock int GetWeaponSlot(int client, int weapon)
{
	if (client == 0 || client > MaxClients || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client) || weapon == 0 || weapon < MaxClients || !IsValidEntity(weapon))
	{
		return -1;
	}

	for (int i = 0; i < 5; i++)
	{
		if (GetPlayerWeaponSlot(client, i) != weapon)
		{
			continue;
		}

		return i;
	}

	return -1;
}

stock bool ShowSyncHudTextAll(Handle& sync, const char[] format, any ...)
{
	if (sync == null || strlen(format) == 0)
	{
		return false;
	}

	char buffer[255];
	VFormat(buffer, sizeof(buffer), format, 3);

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || IsFakeClient(i))
		{
			continue;
		}

		ShowSyncHudText(i, sync, buffer);
	}

	return true;
}

stock bool ShowHudTextAll(int channel, const char[] format, any ...)
{
	if (channel <= 0 || channel > 6 || strlen(format) == 0)
	{
		return false;
	}

	char buffer[255];
	VFormat(buffer, sizeof(buffer), format, 3);

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || IsFakeClient(i))
		{
			continue;
		}

		ShowHudText(i, channel, buffer);
	}

	return true;
}

stock void ClearSyncHudAll(Handle& sync)
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || IsFakeClient(i))
		{
			continue;
		}

		ClearSyncHud(i, sync);
	}
}

stock bool ChangeClientTeam_Alive(int client, int team)
{
	if (client == 0 || client > MaxClients || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client) || team < 2 || team > 3)
	{
		return false;
	}

	int lifestate = GetEntProp(client, Prop_Send, "m_lifeState");
	SetEntProp(client, Prop_Send, "m_lifeState", 2);
	ChangeClientTeam(client, team);
	SetEntProp(client, Prop_Send, "m_lifeState", lifestate);
	return true;
}

stock int GetTotalAliveCount()
{
	return (GetTeamAliveClientCount(2) + GetTeamAliveClientCount(3));
}

stock int GetTeamAliveClientCount(int team)
{
	int amount;

	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || !IsPlayerAlive(i) || GetClientTeam(i) != team)
		{
			continue;
		}

		amount++;
	}

	return amount;
}

stock int GetClientsNearEntity(int entity, float distance = 1000.0, int team = 0, bool fake = true)
{
	int count;

	float vecOrigin1[3];
	GetEntPropVector(entity, Prop_Send, "m_vecOrigin", vecOrigin1);

	float vecOrigin2[3];
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || !IsPlayerAlive(i) || (team > 0 && team != GetClientTeam(i)) || (!fake && IsFakeClient(i)))
		{
			continue;
		}

		GetClientAbsOrigin(i, vecOrigin2);

		if (GetVectorDistance(vecOrigin1, vecOrigin2) > distance)
		{
			continue;
		}

		count++;
	}

	return count;
}

stock any ClampCell(any value, any min, any max)
{
	if (value < min)
	{
		value = min;
	}

	if (value > max)
	{
		value = max;
	}

	return value;
}

stock bool RemoveEntRef(int& reference = INVALID_ENT_REFERENCE)
{
	if (reference != INVALID_ENT_REFERENCE)
	{
		int entity = EntRefToEntIndex(reference);
		bool result = AcceptEntityInput(entity, "Kill");

		reference = INVALID_ENT_REFERENCE;
		return result;
	}

	return false;
}

stock float FloatMultiplier(float value, float multiplier)
{
	return value *= 1.0 + multiplier;
}

stock float FloatDivider(float value, float multiplier)
{
	return value *= 1.0 - multiplier;
}

stock bool CalculateTime(int value, int timer)
{
	return value > 0 && (GetTime() - value) > timer;
}

stock bool CalculateFloatTime(float value, float timer)
{
	return value > 0.0 && (GetGameTime() - value) > timer;
}

stock void PrintTopTextAll(int color[4], const char[] format, any ...)
{
	char message[100];
	VFormat(message, sizeof(message), format, 3);

	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i))
		{
			PrintTopText(i, color, message);
		}
	}
}

stock void PrintTopText(int client, int color[4], const char[] format, any ...)
{
	char message[100];
	VFormat(message, sizeof(message), format, 4);

	KeyValues kv = new KeyValues("Stuff", "title", message);
	kv.SetColor("color", color[0], color[1], color[2], color[3]);
	kv.SetNum("level", 1);
	kv.SetNum("time", 10);

	CreateDialog(client, kv, DialogType_Msg);

	delete kv;
}

stock bool IsValidEntityRef(int ref)
{
	return IsValidEntity(EntRefToEntIndex(ref));
}

//WARNING: Don't call this inside of the 'player_death' event, it'll crash. (May also apply to some OnTakeDamage hooks as well)
stock void DamageArea(float origin[3], float distance = 500.0, float damage = 500.0, int& attacker = 0, int inflictor = 0, int team = 0, int damagetype = DMG_GENERIC, int weapon = -1, float damageforce[3] = NULL_VECTOR)
{
	if (distance <= 0.0 || damage <= 0.0)
	{
		return;
	}

	float vecOrigin[3];
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || !IsPlayerAlive(i) || (team > 0 && team != GetClientTeam(i)) || (attacker > 0 && i == attacker))
		{
			continue;
		}

		GetClientAbsOrigin(i, vecOrigin);

		if (GetVectorDistance(origin, vecOrigin) > distance)
		{
			continue;
		}

		SDKHooks_TakeDamage(i, inflictor, attacker, damage, damagetype, weapon, damageforce, origin);
	}

	int entity = INVALID_ENT_INDEX;
	while ((entity = FindEntityByClassname(entity, "*")) != INVALID_ENT_INDEX)
	{
		if (!HasEntProp(entity, Prop_Send, "m_vecOrigin") || attacker > 0 && attacker == entity)
		{
			continue;
		}

		GetEntPropVector(entity, Prop_Send, "m_vecOrigin", vecOrigin);

		if (GetVectorDistance(origin, vecOrigin) > distance)
		{
			continue;
		}

		SDKHooks_TakeDamage(entity, inflictor, attacker, damage, damagetype, weapon, damageforce, origin);
	}
}

stock void ScreenFadeAll(int duration = 4, int hold_time = 4, int flag = FFADE_IN, int colors[4] = {255, 255, 255, 255})
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i))
		{
			ScreenFade(i, duration, hold_time, flag, colors);
		}
	}
}

stock bool ScreenFade(int client, int duration = 4, int hold_time = 4, int flag = FFADE_IN, int colors[4] = {255, 255, 255, 255}, bool reliable = true)
{
	Handle userMessage = StartMessageOne("Fade", client, (reliable ? USERMSG_RELIABLE : 0));

	if (userMessage == null)
	{
		return false;
	}

	if (GetFeatureStatus(FeatureType_Native, "GetUserMessageType") == FeatureStatus_Available && GetUserMessageType() == UM_Protobuf)
	{
		PbSetInt(userMessage, "duration", duration);
		PbSetInt(userMessage, "hold_time", hold_time);
		PbSetInt(userMessage, "flags", flag);
		PbSetColor(userMessage, "clr", colors);
	}
	else
	{
		BfWriteShort(userMessage, duration);
		BfWriteShort(userMessage, hold_time);
		BfWriteShort(userMessage, flag);
		BfWriteByte(userMessage, colors[0]);
		BfWriteByte(userMessage, colors[1]);
		BfWriteByte(userMessage, colors[2]);
		BfWriteByte(userMessage, colors[3]);
	}

	EndMessage();

	return true;
}

stock bool ScreenShakeAll(int command = SHAKE_START, float amplitude = 50.0, float frequency = 150.0, float duration = 3.0)
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i))
		{
			ScreenShake(i, command, amplitude, frequency, duration);
		}
	}
}

stock bool ScreenShake(int client, int command = SHAKE_START, float amplitude = 50.0, float frequency = 150.0, float duration = 3.0)
{
	if (command == SHAKE_STOP)
	{
		amplitude = 0.0;
	}
	else if (amplitude <= 0.0)
	{
		return false;
	}

	Handle userMessage = StartMessageOne("Shake", client);

	if (userMessage == null)
	{
		return false;
	}

	if (GetFeatureStatus(FeatureType_Native, "GetUserMessageType") == FeatureStatus_Available && GetUserMessageType() == UM_Protobuf)
	{
		PbSetInt(userMessage,   "command",         command);
		PbSetFloat(userMessage, "local_amplitude", amplitude);
		PbSetFloat(userMessage, "frequency",       frequency);
		PbSetFloat(userMessage, "duration",        duration);
	}
	else
	{
		BfWriteByte(userMessage,	command);	// Shake Command
		BfWriteFloat(userMessage,	amplitude);	// shake magnitude/amplitude
		BfWriteFloat(userMessage,	frequency);	// shake noise frequency
		BfWriteFloat(userMessage,	duration);	// shake lasts this long
	}

	EndMessage();

	return true;
}

stock void AnglesToVelocity(float vAngles[3], float fScale, float vOut[3])
{
	float vDirection[3];
	GetAngleVectors(vAngles, vDirection, NULL_VECTOR, NULL_VECTOR);

	ScaleVector(vDirection, fScale);

	vOut = vDirection;
}

stock void String_ToLower(const char[] input, char[] output, int size)
{
	size--;

	int x = 0;
	while (input[x] != '\0' && x < size)
	{
		output[x] = CharToLower(input[x]);
		x++;
	}

	output[x] = '\0';
}

stock void String_ToUpper(const char[] input, char[] output, int size)
{
	size--;

	int x = 0;
	while (input[x] != '\0' && x < size)
	{
		output[x] = CharToUpper(input[x]);
		x++;
	}

	output[x] = '\0';
}

stock void ShowOverlay(int client, const char[] overlay, float duration = 0.0)
{
	if (client == 0)
	{
		return;
	}

	int iFlags = GetCommandFlags("r_screenoverlay");
	SetCommandFlags("r_screenoverlay", iFlags & ~FCVAR_CHEAT);
	ClientCommand(client, "r_screenoverlay \"%s\"", overlay);
	SetCommandFlags("r_screenoverlay", iFlags);

	if (duration > 0.0)
	{
		CreateTimer(duration, Timer_ResetOverlay938217591327590, GetClientUserId(client), TIMER_FLAG_NO_MAPCHANGE);
	}
}

public Action Timer_ResetOverlay938217591327590(Handle timer, any data)
{
	ShowOverlay(GetClientOfUserId(data), "0", 0.0);
}

stock void ClearOverlay(int client)
{
	if (client == 0)
	{
		return;
	}

	ShowOverlay(client, "0");
}

stock float GetVotePercent(int votes, int totalVotes)
{
	return FloatDiv(float(votes), float(totalVotes));
}

/* Found from the MOTDGD plugin. Though, I did reformat it for the new SourceMod syntax. */
stock void EncodeURL(const char[] sString, char[] sResult, int iLen)
{
	char[] sHexTable = "0123456789abcdef";
	int from, c;
	int to;

	while(from < iLen)
	{
		c = sString[from++];

		if(c == 0)
		{
			sResult[to++] = c;
			break;
		}
		else if(c == ' ')
		{
			sResult[to++] = '+';
		}
		else if((c < '0' && c != '-' && c != '.') ||
		(c < 'A' && c > '9') ||
		(c > 'Z' && c < 'a' && c != '_') ||
		(c > 'z'))
		{
			if((to + 3) > iLen)
			{
				sResult[to] = 0;
				break;
			}
			sResult[to++] = '%';
			sResult[to++] = sHexTable[c >> 4];
			sResult[to++] = sHexTable[c & 15];
		}
		else
		{
			sResult[to++] = c;
		}
	}
}

stock float GetPlayerSpeed(int client)
{
	float vecVelocity[3];
	GetEntPropVector(client, Prop_Data, "m_vecAbsVelocity", vecVelocity);
	return GetVectorLength(vecVelocity);
}

stock bool GetClientSteamID(int client, char[] buffer, int size)
{
	if (!client == 0)
	{
		return false;
	}

	if (GetClientAuthId(client, AuthId_Steam2, buffer, size))
	{
		if (GetEngineVersion() == Engine_CSGO)
		{
			ReplaceString(buffer, size, "STEAM_0", "STEAM_1");
		}

		return true;
	}

	return false;
}

stock bool PushPlayerFromPoint(int client, float point[3], float magnitude = 50.0, float radius = 0.0)
{
	if (magnitude <= 0.0)
	{
		return false;
	}

	float vecOrigin[3];
	GetClientAbsOrigin(client, vecOrigin);

	if (radius > 0.0 && GetVectorDistance(point, vecOrigin) > radius)
	{
		return false;
	}

	float vector[3];
	MakeVectorFromPoints(point, vecOrigin, vector);

	NormalizeVector(vector, vector);
	ScaleVector(vector, magnitude);

	if (GetEntityFlags(client) & FL_ONGROUND)
	{
		if (vector[2] < 251.0)
		{
			vector[2] = 251.0;
		}
	}

	TeleportEntity(client, NULL_VECTOR, NULL_VECTOR, vector);
	return true;
}

stock bool PushAllPlayersFromPoint(float point[3], float magnitude = 50.0, float radius = 0.0, int team = 0, int attacker = 0)
{
	if (magnitude <= 0.0)
	{
		return false;
	}

	float vecOrigin[3]; float vector[3];
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i) || !IsPlayerAlive(i) || (team > 0 && team != GetClientTeam(i)) || (attacker > 0 && i == attacker))
		{
			continue;
		}

		GetClientAbsOrigin(i, vecOrigin);

		if (radius > 0.0 && GetVectorDistance(point, vecOrigin) > radius)
		{
			continue;
		}

		MakeVectorFromPoints(point, vecOrigin, vector);

		NormalizeVector(vector, vector);
		ScaleVector(vector, magnitude);

		if (GetEntityFlags(i) & FL_ONGROUND && vector[2] < 251.0)
		{
			vector[2] = 251.0;
		}

		TeleportEntity(i, NULL_VECTOR, NULL_VECTOR, vector);
	}

	return true;
}

//context examples:
///domation:revenge
///domation:dominated
//class examples:
///victimclass:<classname> (ex. victimclass:scout)
stock void SpeakResponseConcept(int client, const char[] concept, const char[] context = "", const char[] class = "")
{
	bool hascontext;

	//For class specific context basically.
	if (strlen(context) > 0)
	{
		SetVariantString(context);
		AcceptEntityInput(client, "AddContext");

		hascontext = true;
	}

	//dominations require you add more context to them for certain things.
	if (strlen(class) > 0)
	{
		char sClass[64];
		FormatEx(sClass, sizeof(sClass), "victimclass:%s", class);
		SetVariantString(sClass);
		AcceptEntityInput(client, "AddContext");

		hascontext = true;
	}

	SetVariantString(concept);
	AcceptEntityInput(client, "SpeakResponseConcept");

	if (hascontext)
	{
		AcceptEntityInput(client, "ClearContext");
	}
}

stock void SpeakResponseConceptDelayed(int client, const char[] concept, float delayed = 0.0, const char[] context = "", const char[] class = "")
{
	DataPack hPack;
	CreateDataTimer(ClampCell(delayed, 0.0, 999999.0), Timer_DelayClientConcept28305820358023532, hPack, TIMER_FLAG_NO_MAPCHANGE);
	hPack.WriteCell(GetClientUserId(client));
	hPack.WriteString(concept);
	hPack.WriteString(context);
	hPack.WriteString(class);
}

public Action Timer_DelayClientConcept28305820358023532(Handle timer, DataPack data)
{
	data.Reset();
	int client = GetClientOfUserId(data.ReadCell());

	char sConcept[256];
	data.ReadString(sConcept, sizeof(sConcept));

	char sContext[64];
	data.ReadString(sContext, sizeof(sContext));

	char sClass[64];
	data.ReadString(sClass, sizeof(sClass));

	if (client > 0 && IsClientInGame(client) && IsPlayerAlive(client))
	{
		SpeakResponseConcept(client, sConcept, sContext, sClass);
	}
}

stock void SpeakResponseConceptAll(const char[] concept, const char[] context = "", const char[] class = "")
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && IsPlayerAlive(i))
		{
			SpeakResponseConcept(i, concept, context, class);
		}
	}
}

stock void SpeakResponseConceptAllDelayed(const char[] concept, float delay = 0.0, const char[] context = "", const char[] class = "")
{
	DataPack hPack;
	CreateDataTimer(ClampCell(delay, 0.0, 999999.0), Timer_DelayAllConcept28305820358023532, hPack, TIMER_FLAG_NO_MAPCHANGE);
	hPack.WriteString(concept);
	hPack.WriteString(context);
	hPack.WriteString(class);
}

public Action Timer_DelayAllConcept28305820358023532(Handle timer, DataPack data)
{
	data.Reset();

	char sConcept[256];
	data.ReadString(sConcept, sizeof(sConcept));

	char sContext[64];
	data.ReadString(sContext, sizeof(sContext));

	char sClass[64];
	data.ReadString(sClass, sizeof(sClass));

	SpeakResponseConceptAll(sConcept, sContext, sClass);
}

stock void SQL_FetchClientName(int client, Database database, char[] buffer, int size)
{
	char sName[MAX_NAME_LENGTH];
	GetClientName(client, sName, sizeof(sName));

	int size2 = 2 * strlen(sName) + 1;
	char[] sEscapedName = new char[size2 + 1];
	database.Escape(sName, sEscapedName, size2 + 1);

	strcopy(buffer, size, sEscapedName);
}

stock bool GetGroundCoordinates(int entity, float buffer[3], float distance = 0.0, float offset[3] = {0.0, 0.0, 0.0})
{
	float vecOrigin[3];
	GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", vecOrigin);

	float vecLook[3] = {90.0, 0.0, 0.0};
	Handle trace = TR_TraceRayEx(vecOrigin, vecLook, MASK_SOLID_BRUSHONLY, RayType_Infinite);

	if (TR_DidHit(trace))
	{
		TR_GetEndPosition(buffer, trace);

		if (distance > 0.0 && vecOrigin[2] - buffer[2] > distance)
		{
			buffer[0] += offset[0];
			buffer[1] += offset[1];
			buffer[2] += offset[2];

			return true;
		}

		buffer[0] += offset[0];
		buffer[1] += offset[1];
		buffer[2] += offset[2];

		return false;
	}

	delete trace;
	return false;
}

stock bool GetCeilingCoordinates(int entity, float buffer[3], float distance = 0.0, float offset[3] = {0.0, 0.0, 0.0})
{
	float vecOrigin[3];
	GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", vecOrigin);

	float vecLook[3] = {-90.0, 0.0, 0.0};
	Handle trace = TR_TraceRayEx(vecOrigin, vecLook, MASK_SOLID_BRUSHONLY, RayType_Infinite);

	if (TR_DidHit(trace))
	{
		TR_GetEndPosition(buffer, trace);

		if (distance > 0.0 && vecOrigin[2] - buffer[2] > distance)
		{
			buffer[0] += offset[0];
			buffer[1] += offset[1];
			buffer[2] += offset[2];

			return true;
		}

		buffer[0] += offset[0];
		buffer[1] += offset[1];
		buffer[2] += offset[2];

		return false;
	}

	delete trace;
	return false;
}

stock bool TeleportToDestination(int client, const char[] name, bool keep_velocity = false)
{
	int entity = -1; char buffer[256]; float vecOrigin[3]; float vecAngle[3]; float blank[3];
	while ((entity = FindEntityByClassname(entity, "info_teleport_destination")) != -1)
	{
		GetEntPropString(entity, Prop_Data, "m_iName", buffer, sizeof(buffer));

		if (StrEqual(buffer, name, false))
		{
			GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", vecOrigin);
			GetEntPropVector(entity, Prop_Data, "m_angAbsRotation", vecAngle);
			TeleportEntity(client, vecOrigin, vecAngle, keep_velocity ? NULL_VECTOR : blank);
			return true;
		}
	}

	return false;
}

stock bool IsClientNearEntityViaName(int client, const char[] name, const char[] entity_name = "*", float distance = 1000.0)
{
	float vecClientOrigin[3];
	GetClientAbsOrigin(client, vecClientOrigin);

	int entity = -1; char buffer[256]; float vecOrigin[3];
	while ((entity = FindEntityByClassname(entity, entity_name)) != -1)
	{
		GetEntPropString(entity, Prop_Data, "m_iName", buffer, sizeof(buffer));

		if (StrEqual(buffer, name, false))
		{
			GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", vecOrigin);

			if (GetVectorDistance(vecClientOrigin, vecOrigin) <= distance)
			{
				return true;
			}
		}
	}

	return false;
}

//With help from Arthurdead.
stock void VectorAddRotatedOffset(const float angle[3], float buffer[3], const float offset[3])
{
    float vecForward[3]; float vecLeft[3]; float vecUp[3];
    GetAngleVectors(angle, vecForward, vecLeft, vecUp);

    ScaleVector(vecForward, offset[0]);
    ScaleVector(vecLeft, offset[1]);
    ScaleVector(vecUp, offset[2]);

    float vecAdd[3];
    AddVectors(vecAdd, vecForward, vecAdd);
    AddVectors(vecAdd, vecLeft, vecAdd);
    AddVectors(vecAdd, vecUp, vecAdd);

    AddVectors(buffer, vecAdd, buffer);
}

/* MAH BOI POIKA */
stock void EquipWeaponSlot(int client, int slot)
{
	int iWeapon = GetPlayerWeaponSlot(client, slot);
	if(IsValidEntity(iWeapon))
		EquipWeapon(client, iWeapon);
}

stock void EquipWeapon(int client, int weapon)
{
	char class[64];
	GetEntityClassname(weapon, class, sizeof(class));

	FakeClientCommand(client, "use %s", class);
}
/* */

stock void UnsetCheatVar(ConVar convar)
{
	int flags = convar.Flags;
	flags &= ~FCVAR_CHEAT;
	convar.Flags = flags;
}

stock void SetCheatVar(ConVar convar)
{
	int flags = convar.Flags;
	flags |= FCVAR_CHEAT;
	convar.Flags = flags;
}

stock void GetMapName(char[] buffer, int size)
{
	GetCurrentMap(buffer, size);
	GetMapDisplayName(buffer, buffer, size);
}

stock bool CreateFile(const char[] buffer)
{
	if (strlen(buffer) == 0)
	{
		return false;
	}

	char path[PLATFORM_MAX_PATH];
	BuildPath(Path_SM, path, sizeof(path), buffer);

	if (!FileExists(path))
	{
		File file = OpenFile(path, "w");
		delete file;
	}

	return true;
}

stock int GetEntityName(int entity, char[] buffer, int maxlen)
{
	return GetEntPropString(entity, Prop_Data, "m_iName", buffer, maxlen);
}

stock bool HasEntName(int entity, const char[] buffer, bool case_sensitive = false)
{
	char sName[128];
	GetEntPropString(entity, Prop_Data, "m_iName", sName, sizeof(sName));

	return StrEqual(buffer, sName, case_sensitive);
}

/*---------------------------------------------------------*/
//SECTIONS
/*---------------------------------------------------------*/

/*---------------------------------------------------------*/
//Menu stocks by Kisslick
/*---------------------------------------------------------*/

stock void PushMenuString(Menu hndl, const char[] id, const char[] data)
{
	hndl.AddItem(id, data, ITEMDRAW_IGNORE);
}

stock void PushMenuCell(Menu hndl, const char[] id, int data)
{
	char DataString[64];
	IntToString(data, DataString, sizeof(DataString));
	hndl.AddItem(id, DataString, ITEMDRAW_IGNORE);
}

stock void PushMenuFloat(Menu hndl, const char[] id, float data)
{
	char DataString[64];
	FloatToString(data, DataString, sizeof(DataString));
	hndl.AddItem(id, DataString, ITEMDRAW_IGNORE);
}

stock void PushMenuBool(Menu hndl, const char[] id, bool data)
{
	char DataString[64];
	IntToString(view_as<int>(data), DataString, sizeof(DataString));
	hndl.AddItem(id, DataString, ITEMDRAW_IGNORE);
}

stock bool GetMenuString(Menu hndl, const char[] id, char[] Buffer, int size)
{
	int ItemCount = hndl.ItemCount;
	char info[64]; char data[64];

	for (int i = 0; i < ItemCount; i++) {
		if (hndl.GetItem(i, info, sizeof(info), _, data, sizeof(data)))
		{
			if (StrEqual(info, id)) {
				strcopy(Buffer, size, data);
				return true;
			}
		}
	}
	return false;
}

stock int GetMenuCell(Menu hndl, const char[] id, int DefaultValue = 0)
{
	int ItemCount = hndl.ItemCount;
	char info[64]; char data[64];

	for (int i = 0; i < ItemCount; i++) {
		if (hndl.GetItem(i, info, sizeof(info), _, data, sizeof(data)))
		{
			if (StrEqual(info, id))
			return StringToInt(data);
		}
	}
	return DefaultValue;
}

stock float GetMenuFloat(Menu hndl, const char[] id, float DefaultValue = 0.0)
{
	int ItemCount = hndl.ItemCount;
	char info[64]; char data[64];

	for (int i = 0; i < ItemCount; i++) {
		if (hndl.GetItem(i, info, sizeof(info), _, data, sizeof(data)))
		{
			if (StrEqual(info, id))
			return StringToFloat(data);
		}
	}
	return DefaultValue;
}

stock bool GetMenuBool(Menu hndl, const char[] id, bool DefaultValue = false)
{
	int ItemCount = hndl.ItemCount;
	char info[64]; char data[64];

	for (int i = 0; i < ItemCount; i++) {
		if (hndl.GetItem(i, info, sizeof(info), _, data, sizeof(data)))
		{
			if (StrEqual(info, id))
			return view_as<bool>(StringToInt(data));
		}
	}
	return DefaultValue;
}

stock void CopyMenuAny(Menu source, Menu dest, const char[] id)
{
	int ItemCount = source.ItemCount;
	char info[64]; char data[64];

	for (int i = 0; i < ItemCount; i++) {
		if (source.GetItem(i, info, sizeof(info), _, data, sizeof(data)))
		{
			if (StrEqual(info, id))
			dest.AddItem(id, data, ITEMDRAW_IGNORE);
		}
	}
}

stock bool AddMenuItemFormat(Menu& menu, const char[] info, int style = ITEMDRAW_DEFAULT, const char[] format, any ...)
{
	char display[128];
	VFormat(display, sizeof(display), format, 5);

	return menu.AddItem(info, display, style);
}

stock int Util_GetMaxHealth(int client)
{
	return GetEngineVersion() == Engine_TF2 ? TF2_GetMaxHealth(client) : GetEntProp(client, Prop_Data, "m_iMaxHealth");
}

stock void Util_SetClientHealth(int client, int health)
{
	if (GetEngineVersion() == Engine_TF2)
	{
		TF2_SetPlayerHealth(client, health);
	}
	else
	{
		SetEntityHealth(client, ClampCell(health, 1, Util_GetMaxHealth(client)));
	}
}

stock void Util_AddClientHealth(int client, int health, float overheal = 1.5, bool additive = false, bool event = true)
{
	if (GetEngineVersion() == Engine_TF2)
	{
		TF2_AddPlayerHealth(client, health, overheal, additive, event);
	}
	else
	{
		SetEntityHealth(client, ClampCell((GetClientHealth(client) + health), 1, Util_GetMaxHealth(client)));
	}
}

stock void Util_HealArea(float origin[3], float distance = 500.0, int amount = 500,  int team = 0, float overheal = 1.5, bool additive = false, bool event = true)
{
	float vecOrigin[3];
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || !IsPlayerAlive(i) || (team > 0 && team != GetClientTeam(i)))
		{
			continue;
		}

		GetClientAbsOrigin(i, vecOrigin);

		if (GetVectorDistance(origin, vecOrigin) > distance)
		{
			continue;
		}

		Util_AddClientHealth(i, amount, overheal, additive, event);
	}
}

stock int[] GetNearestPlayers(int entity, int& count, float distance, bool self = false, bool fake_clients = true)
{
	count = 0;
	int clients[MAXPLAYERS + 1];

	float vecOrigin[3];
	GetEntPropVector(entity, Prop_Data, "m_vecAbsOrigin", vecOrigin);

	float vecOrigin2[3];
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientConnected(i) || !IsClientInGame(i) || !IsPlayerAlive(i) || (!self && i == entity) || (!fake_clients && IsFakeClient(i)))
		{
			continue;
		}

		GetClientAbsOrigin(i, vecOrigin2);

		if (GetVectorDistance(vecOrigin, vecOrigin2) > distance)
		{
			continue;
		}

		clients[count++] = i;
	}

	return clients;
}

/*--------------------------------------------------------->*/
//Model Stocks
/*--------------------------------------------------------->*/

stock bool SetModel(int entity, char model[PLATFORM_MAX_PATH], bool preload = false)
{
	if (entity <= 0 || strlen(model) == 0)
	{
		return false;
	}

	if (PrepareModel(model, preload) != -1)
	{
		if (GetEngineVersion() == Engine_TF2)
		{
			SetVariantString(model);
			AcceptEntityInput(entity, "SetCustomModel");

			SetEntProp(entity, Prop_Send, "m_bCustomModelRotates", 1);
			SetEntProp(entity, Prop_Send, "m_bUseClassAnimations", 1);
		}
		else
		{
			SetEntityModel(entity, model);
		}

		return true;
	}

	return false;
}

stock bool SetModelViaConVar(int entity, ConVar &convar, bool preload = false)
{
	if (convar == null)
	{
		return false;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	convar.GetString(sBuffer, sizeof(sBuffer));

	return SetModel(entity, sBuffer, preload);
}

stock int PrepareModel(char model[PLATFORM_MAX_PATH], bool preload = false)
{
	if (strlen(model) == 0)
	{
		return 0;
	}

	if (StrContains(model, "models/") != 0)
	{
		Format(model, sizeof(model), "models/%s", model);
	}

	//ParseMDLFiles(model);

	return PrecacheModel(model, preload);
}

stock int PrepareModelConVar(ConVar &convar, bool preload = false)
{
	if (convar == null)
	{
		return -1;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	convar.GetString(sBuffer, sizeof(sBuffer));

	return PrepareModel(sBuffer, preload);
}

/*--------------------------------------------------------->*/
//Sound Stocks
/*--------------------------------------------------------->*/

stock bool EmitSoundToClientSafe(int client, const char[] sample, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_AUTO, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float volume = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	if (client == 0 || !IsClientInGame(client) || IsFakeClient(client) || strlen(sample) == 0)
	{
		return false;
	}

	char sSound[PLATFORM_MAX_PATH];
	strcopy(sSound, sizeof(sSound), sample);

	if (RequiresFakePrecache())
	{
		Format(sSound, sizeof(sSound), "*%s", sSound);
	}
	else
	{
		PrecacheSound(sSound);
	}

	if (IsSoundPrecached(sSound))
	{
		EmitSoundToClient(client, sSound, entity, channel, level, flags, volume, pitch, speakerentity, origin, dir, updatePos, soundtime);
		return true;
	}

	LogError("Error playing sound to client %N, not precached: %s", client, sSound);
	return false;
}

stock bool EmitSoundToClientViaConVar(int client, ConVar convar, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_AUTO, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float volume = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	if (convar == null)
	{
		return false;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	convar.GetString(sBuffer, sizeof(sBuffer));

	return EmitSoundToClientSafe(client, sBuffer, entity, channel, level, flags, volume, pitch, speakerentity, origin, dir, updatePos, soundtime);
}

stock bool EmitSoundToClientSafeDelayed(int client, const char[] sample, float delay = 0.0, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_AUTO, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float volume = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	DataPack hPack;
	CreateDataTimer(ClampCell(delay, 0, 9999999.9), Timer_PlayClientSound280358023580235, hPack, TIMER_FLAG_NO_MAPCHANGE);
	hPack.WriteCell(client);
	hPack.WriteString(sample);
	hPack.WriteCell(entity);
	hPack.WriteCell(channel);
	hPack.WriteCell(level);
	hPack.WriteCell(flags);
	hPack.WriteFloat(volume);
	hPack.WriteCell(pitch);
	hPack.WriteCell(speakerentity);
	hPack.WriteFloat(origin[0]);
	hPack.WriteFloat(origin[1]);
	hPack.WriteFloat(origin[2]);
	hPack.WriteFloat(dir[0]);
	hPack.WriteFloat(dir[1]);
	hPack.WriteFloat(dir[2]);
	hPack.WriteCell(updatePos);
	hPack.WriteFloat(soundtime);
}

public Action Timer_PlayClientSound280358023580235(Handle timer, DataPack data)
{
	data.Reset();

	int client = data.ReadCell();

	char sample[PLATFORM_MAX_PATH];
	data.ReadString(sample, sizeof(sample));

	int entity = data.ReadCell();
	int channel = data.ReadCell();
	int level = data.ReadCell();
	int flags = data.ReadCell();
	float volume = data.ReadFloat();
	int pitch = data.ReadCell();
	int speakerentity = data.ReadCell();

	float origin[3];
 	origin[0] = data.ReadFloat();
 	origin[1] = data.ReadFloat();
 	origin[2] = data.ReadFloat();

	float dir[3];
 	dir[0] = data.ReadFloat();
 	dir[1] = data.ReadFloat();
 	dir[2] = data.ReadFloat();

	bool updatePos = data.ReadCell();
	float soundtime = data.ReadFloat();

	EmitSoundToClientSafe(client, sample, entity, channel, level, flags, volume, pitch, speakerentity, origin, dir, updatePos, soundtime);
}

stock bool EmitSoundToAllSafe(const char[] sample, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_AUTO, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float volume = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	if (strlen(sample) == 0)
	{
		return false;
	}

	char sSound[PLATFORM_MAX_PATH];
	strcopy(sSound, sizeof(sSound), sample);

	if (RequiresFakePrecache())
	{
		Format(sSound, sizeof(sSound), "*%s", sSound);
	}
	else
	{
		PrecacheSound(sSound);
	}

	if (IsSoundPrecached(sSound))
	{
		EmitSoundToAll(sSound, entity, channel, level, flags, volume, pitch, speakerentity, origin, dir, updatePos, soundtime);
		return true;
	}

	LogError("Error playing sound to all, not precached: %s", sSound);
	return false;
}

stock bool EmitSoundToAllViaConVar(ConVar convar, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_AUTO, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float volume = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	if (convar = null)
	{
		return false;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	convar.GetString(sBuffer, sizeof(sBuffer));

	return EmitSoundToAllSafe(sBuffer, entity, channel, level, flags, volume, pitch, speakerentity, origin, dir, updatePos, soundtime);
}

stock bool EmitSoundToAllSafeDelayed(const char[] sample, float delay = 0.0, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_AUTO, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float volume = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	DataPack hPack;
	CreateDataTimer(ClampCell(delay, 0, 9999999.9), Timer_PlayAllSound280358023580235, hPack, TIMER_FLAG_NO_MAPCHANGE);
	hPack.WriteString(sample);
	hPack.WriteCell(entity);
	hPack.WriteCell(channel);
	hPack.WriteCell(level);
	hPack.WriteCell(flags);
	hPack.WriteFloat(volume);
	hPack.WriteCell(pitch);
	hPack.WriteCell(speakerentity);
	hPack.WriteFloat(origin[0]);
	hPack.WriteFloat(origin[1]);
	hPack.WriteFloat(origin[2]);
	hPack.WriteFloat(dir[0]);
	hPack.WriteFloat(dir[1]);
	hPack.WriteFloat(dir[2]);
	hPack.WriteCell(updatePos);
	hPack.WriteFloat(soundtime);
}

public Action Timer_PlayAllSound280358023580235(Handle timer, DataPack data)
{
	data.Reset();

	char sample[PLATFORM_MAX_PATH];
	data.ReadString(sample, sizeof(sample));

	int entity = data.ReadCell();
	int channel = data.ReadCell();
	int level = data.ReadCell();
	int flags = data.ReadCell();
	float volume = data.ReadFloat();
	int pitch = data.ReadCell();
	int speakerentity = data.ReadCell();

	float origin[3];
 	origin[0] = data.ReadFloat();
 	origin[1] = data.ReadFloat();
 	origin[2] = data.ReadFloat();

	float dir[3];
 	dir[0] = data.ReadFloat();
 	dir[1] = data.ReadFloat();
 	dir[2] = data.ReadFloat();

	bool updatePos = data.ReadCell();
	float soundtime = data.ReadFloat();

	EmitSoundToAllSafe(sample, entity, channel, level, flags, volume, pitch, speakerentity, origin, dir, updatePos, soundtime);
}

stock bool EmitAmbientSoundSafe(const char[] sample, const float origin[3], int entity = SOUND_FROM_WORLD, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float vol = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, float delay = 0.0)
{
	if (strlen(sample) == 0)
	{
		return false;
	}

	char sSound[PLATFORM_MAX_PATH];
	strcopy(sSound, sizeof(sSound), sample);

	if (RequiresFakePrecache())
	{
		Format(sSound, sizeof(sSound), "*%s", sSound);
	}
	else
	{
		PrecacheSound(sSound);
	}

	if (IsSoundPrecached(sSound))
	{
		EmitAmbientSound(sSound, origin, entity, level, flags, vol, pitch, delay);
		return true;
	}

	LogError("Error playing ambient sound, not precached: %s", sSound);
	return false;
}

stock bool EmitAmbientSoundViaConVar(ConVar convar, bool preload = false, const float origin[3], int entity = SOUND_FROM_WORLD, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float vol = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, float delay = 0.0)
{
	if (convar == null)
	{
		return false;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	convar.GetString(sBuffer, sizeof(sBuffer));

	return EmitAmbientSoundSafe(sBuffer, origin, entity, level, flags, vol, pitch, delay);
}

stock bool StopSoundSafe(int entity, int channel, const char[] sample)
{
	if (entity <= 0 || entity > MAX_ENTITY_LIMIT || strlen(sample) == 0)
	{
		return false;
	}

	char sSound[PLATFORM_MAX_PATH];
	strcopy(sSound, sizeof(sSound), sample);

	if (RequiresFakePrecache())
	{
		Format(sSound, sizeof(sSound), "*%s", sSound);
	}

	StopSound(entity, channel, sSound);
	return true;
}

stock void StopSoundSafeAll(int entity, const char[] sample)
{
    StopSoundSafe(entity, SNDCHAN_AUTO, sample);
    StopSoundSafe(entity, SNDCHAN_WEAPON, sample);
    StopSoundSafe(entity, SNDCHAN_VOICE, sample);
    StopSoundSafe(entity, SNDCHAN_ITEM, sample);
    StopSoundSafe(entity, SNDCHAN_BODY, sample);
    StopSoundSafe(entity, SNDCHAN_STREAM, sample);
    StopSoundSafe(entity, SNDCHAN_VOICE_BASE, sample);
    StopSoundSafe(entity, SNDCHAN_USER_BASE, sample);
}

stock bool PrepareSoundConVar(ConVar convar, bool preload = false, bool download = true)
{
	if (convar == null)
	{
		return false;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	convar.GetString(sBuffer, sizeof(sBuffer));

	return PrepareSound(sBuffer, preload, download);
}

stock bool PrepareSound(const char[] sample, bool preload = false, bool download = true)
{
	if (strlen(sample) == 0)
	{
		return false;
	}

	char sSound[PLATFORM_MAX_PATH];
	strcopy(sSound, sizeof(sSound), sample);

	if (StrContains(sSound, "sound/") == 0)
	{
		if (download &&)
		{
			AddFileToDownloadsTable(sSound);
		}

		RemoveFrontString(sSound, sizeof(sSound), 6);
	}
	else if (download)
	{
		char sBuffer[PLATFORM_MAX_PATH];
		FormatEx(sBuffer, sizeof(sBuffer), "sounds/%s", sSound);
		AddFileToDownloadsTable(sBuffer);
	}

	return RequiresFakePrecache() ? FakePrecacheSound(sSound) : PrecacheSound(sSound, preload);
}

stock bool FakePrecacheSound(const char[] sound)
{
	char sPath[PLATFORM_MAX_PATH];
	Format(sPath, sizeof(sPath), "sound/%s", sound);

	if (!FileExists(sPath))
	{
		return false;
	}

	AddFileToDownloadsTable(sPath);

	Format(sPath, sizeof(sPath), "*/%s", sound);
	AddToStringTable(FindStringTable("soundprecache"), sPath);
	return true;
}

stock bool RequiresFakePrecache()
{
	EngineVersion engine2 = GetEngineVersion();
	return engine2 == Engine_CSGO || engine2 == Engine_DOTA;
}

/*--------------------------------------------------------->*/
//From SMLIB, lets draw happy little boxes together.
/*--------------------------------------------------------->*/

stock void Effect_DrawBeamBoxToClient(int client, const float bottomCorner[3], const float upperCorner[3], int modelIndex, int haloIndex, int startFrame = 0, int frameRate = 30, float life = 5.0, float width = 5.0, float endWidth = 5.0, int fadeLength = 2, float amplitude = 1.0, const color[4] =  { 255, 0, 0, 255 }, int speed = 0)
{
	int clients[1];
	clients[0] = client;
	Effect_DrawBeamBox(clients, 1, bottomCorner, upperCorner, modelIndex, haloIndex, startFrame, frameRate, life, width, endWidth, fadeLength, amplitude, color, speed);
}

stock void Effect_DrawBeamBoxToAll(const float bottomCorner[3], const float upperCorner[3], int modelIndex, int haloIndex, int startFrame = 0, int frameRate = 30, float life = 5.0, float width = 5.0, float endWidth = 5.0, int fadeLength = 2, float amplitude = 1.0, const int color[4] = { 255, 255, 255, 255 }, int speed = 0)
{
	int[] clients = new int[MaxClients];
	int numClients;

	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i))
		{
			clients[numClients++] = i;
		}
	}

	Effect_DrawBeamBox(clients, numClients, bottomCorner, upperCorner, modelIndex, haloIndex, startFrame, frameRate, life, width, endWidth, fadeLength, amplitude, color, speed);
}

stock void Effect_DrawBeamBox(int[] clients, int numClients, const float bottomCorner[3], const float upperCorner[3], int modelIndex, int haloIndex, int startFrame = 0, int frameRate = 30, float life = 5.0, float width = 5.0, float endWidth = 5.0, int fadeLength = 2, float amplitude = 1.0, const color[4] =  { 255, 0, 0, 255 }, int speed = 0)
{
	float corners[8][3];

	for (int i = 0; i < 4; i++)
	{
		CopyArrayToArray(bottomCorner, corners[i], 3);
		CopyArrayToArray(upperCorner, corners[i + 4], 3);
	}

	corners[1][0] = upperCorner[0];
	corners[2][0] = upperCorner[0];
	corners[2][1] = upperCorner[1];
	corners[3][1] = upperCorner[1];
	corners[4][0] = bottomCorner[0];
	corners[4][1] = bottomCorner[1];
	corners[5][1] = bottomCorner[1];
	corners[7][0] = bottomCorner[0];

	for (int i = 0; i < 4; i++)
	{
		int j = (i == 3 ? 0 : i + 1);
		TE_SetupBeamPoints(corners[i], corners[j], modelIndex, haloIndex, startFrame, frameRate, life, width, endWidth, fadeLength, amplitude, color, speed);
		TE_Send(clients, numClients);
	}

	for (int i = 4; i < 8; i++)
	{
		int j = (i == 7 ? 4 : i + 1);
		TE_SetupBeamPoints(corners[i], corners[j], modelIndex, haloIndex, startFrame, frameRate, life, width, endWidth, fadeLength, amplitude, color, speed);
		TE_Send(clients, numClients);
	}

	for (int i = 0; i < 4; i++)
	{
		TE_SetupBeamPoints(corners[i], corners[i + 4], modelIndex, haloIndex, startFrame, frameRate, life, width, endWidth, fadeLength, amplitude, color, speed);
		TE_Send(clients, numClients);
	}
}

stock void Effect_DrawRangedBeamBox(float origin[3], const float bottomCorner[3], const float upperCorner[3], int modelIndex, int haloIndex, int startFrame = 0, int frameRate = 30, float life = 5.0, float width = 5.0, float endWidth = 5.0, int fadeLength = 2, float amplitude = 1.0, const color[4] =  { 255, 0, 0, 255 }, int speed = 0)
{
	float corners[8][3];

	for (int i = 0; i < 4; i++)
	{
		CopyArrayToArray(bottomCorner, corners[i], 3);
		CopyArrayToArray(upperCorner, corners[i + 4], 3);
	}

	corners[1][0] = upperCorner[0];
	corners[2][0] = upperCorner[0];
	corners[2][1] = upperCorner[1];
	corners[3][1] = upperCorner[1];
	corners[4][0] = bottomCorner[0];
	corners[4][1] = bottomCorner[1];
	corners[5][1] = bottomCorner[1];
	corners[7][0] = bottomCorner[0];

	for (int i = 0; i < 4; i++)
	{
		int j = (i == 3 ? 0 : i + 1);
		TE_SetupBeamPoints(corners[i], corners[j], modelIndex, haloIndex, startFrame, frameRate, life, width, endWidth, fadeLength, amplitude, color, speed);
		TE_SendToAllInRange(origin, RangeType_Audibility);
	}

	for (int i = 4; i < 8; i++)
	{
		int j = (i == 7 ? 4 : i + 1);
		TE_SetupBeamPoints(corners[i], corners[j], modelIndex, haloIndex, startFrame, frameRate, life, width, endWidth, fadeLength, amplitude, color, speed);
		TE_SendToAllInRange(origin, RangeType_Audibility);
	}

	for (int i = 0; i < 4; i++)
	{
		TE_SetupBeamPoints(corners[i], corners[i + 4], modelIndex, haloIndex, startFrame, frameRate, life, width, endWidth, fadeLength, amplitude, color, speed);
		TE_SendToAllInRange(origin, RangeType_Audibility);
	}
}

//SM9(); has molested this stock at this point. <3
stock void GetAbsBoundingBox(int ent, float mins[3], float maxs[3], bool half = false)
{
    float origin[3];

    GetEntPropVector(ent, Prop_Data, "m_vecAbsOrigin", origin);
    GetEntPropVector(ent, Prop_Data, "m_vecMins", mins);
    GetEntPropVector(ent, Prop_Data, "m_vecMaxs", maxs);

    mins[0] += origin[0];
    mins[1] += origin[1];
    mins[2] += origin[2];
    maxs[0] += origin[0];
    maxs[1] += origin[1];

    if (!half) {
        maxs[2] += origin[2];
    } else {
        maxs[2] = mins[2];
    }
}

stock void CreateHudConVars(const char[] prefix, ConVar& position = null, ConVar& holdtime = null, ConVar& colors = null, ConVar& effect = null, ConVar& fxtime = null, ConVar& fades = null)
{
	char sName[512];

	FormatEx(sName, sizeof(sName), "%sposition", prefix);
	position = CreateConVar(sName, "-1.0, -1.0", "Position of this hud element.", FCVAR_NOTIFY);

	FormatEx(sName, sizeof(sName), "%sholdtime", prefix);
	holdtime = CreateConVar(sName, "1.0", "Holdtime for this hud element.", FCVAR_NOTIFY);

	FormatEx(sName, sizeof(sName), "%scolor", prefix);
	colors = CreateConVar(sName, "255, 255, 255, 255", "Color of this hud element.", FCVAR_NOTIFY);

	FormatEx(sName, sizeof(sName), "%seffect", prefix);
	effect = CreateConVar(sName, "0", "Effect for this hud element.", FCVAR_NOTIFY);

	FormatEx(sName, sizeof(sName), "%sfxtime", prefix);
	fxtime = CreateConVar(sName, "0.0", "Fxtime for this hud element.", FCVAR_NOTIFY);

	FormatEx(sName, sizeof(sName), "%sfades", prefix);
	fades = CreateConVar(sName, "-1.0, -1.0", "Fxtime for this hud element.", FCVAR_NOTIFY);
}

stock void SetHudTextParamsViaConVars(ConVar& position, ConVar& holdtime, ConVar& color, ConVar& effect, ConVar& fxtime, ConVar& fades)
{
	float vecPosition[2]; vecPosition = GetConVar2DVector(position);
	int iColor[4]; iColor = GetConVarColor(color);
	float fFades[2]; fFades = GetConVar2DVector(fades);

	SetHudTextParams(vecPosition[0], vecPosition[1], holdtime.FloatValue, iColor[0], iColor[1], iColor[2], iColor[3], effect.IntValue, fxtime.FloatValue, fFades[0], fFades[1]);
}

/*--------------------------------------------------------->*/
//Force look stocks by Pelipoika
/*--------------------------------------------------------->*/

stock void LookAtPosition(int client, float flGoal[3], float flAimSpeed = 0.05)
{
    float flPos[3];
    GetClientEyePosition(client, flPos);

    float flAng[3];
    GetClientEyeAngles(client, flAng);

    // get normalised direction from target to client
    float desired_dir[3];
    MakeVectorFromPoints(flPos, flGoal, desired_dir);
    GetVectorAngles(desired_dir, desired_dir);

    // ease the current direction to the target direction
    flAng[0] += AngleNormalize(desired_dir[0] - flAng[0]) * flAimSpeed;
    flAng[1] += AngleNormalize(desired_dir[1] - flAng[1]) * flAimSpeed;

    TeleportEntity(client, NULL_VECTOR, flAng, NULL_VECTOR);
}

stock float AngleNormalize(float angle)
{
    angle = fmodf(angle, 360.0);
    if (angle > 180)
    {
        angle -= 360;
    }
    if (angle < -180)
    {
        angle += 360;
    }

    return angle;
}

stock float fmodf(float number, float denom)
{
    return number - RoundToFloor(number / denom) * denom;
}

/*--------------------------------------------------------->*/
//TheXeon made methodmaps that should exist already but don't for some reason. (:
//Got permission to post them here.
/*--------------------------------------------------------->*/

// TIMER METHODMAPS
methodmap SMTimer < Handle
{
	/**
	 * Creates a basic timer.  Calling .Kill() on a timer will kill the timer. Do not .Kill(), .Close(),
	 * or delete in a callback! Make sure to null timer in callback if global!
	 *
	 * @param interval			Interval from the current game time to execute the given function.
	 * @param func				Function to execute once the given interval has elapsed.
	 * @param data				Handle or value to pass through to the timer callback function.
	 * @param flags				Flags to set (such as repeatability or auto-Handle closing).
	 * @return					Handle to the timer object.  You do not need to call CloseHandle().
	 *							If the timer could not be created, null will be returned.
	 */
	public SMTimer(float interval, Timer func, any data=INVALID_HANDLE, int flags=0)
	{
		return view_as<SMTimer>(CreateTimer(interval, func, data, flags));
	}

	/**
	 * Kills a timer.  Use this instead of delete if you need more options. Do not use in a timer's callback!
	 *
	 * @param autoClose			If autoClose is true, the data that was passed to CreateTimer() will
	 *							be closed as a handle if TIMER_DATA_HNDL_CLOSE was not specified.
	 * @error					Invalid handles will cause a run time error.
	 */
	public void Kill(bool autoClose=false)
	{
		KillTimer(this, autoClose);
		delete this;
	}

	/**
	 * Closes a timer. A clone of .Kill() if autoClose is true. Do not use in a timer's callback!
	 *
	 * @param autoClose			If autoClose is true, the data that was passed to new SMTimer() will
	 *							be deleted as a handle if TIMER_DATA_HNDL_CLOSE was not specified.
	 * @error					Invalid handles will cause a run time error.
	 */
	public void Close(bool autoClose=false)
	{
		if (autoClose)
			this.Kill(autoClose);
		else
			delete this;
	}

	/**
	 * Manually triggers a timer so its function will be called.
	 *
	 * @param reset				If reset is true, the elapsed time counter is reset
	 *							so the full interval must pass again.
	 */
	public void Trigger(bool reset=false)
	{
		TriggerTimer(this, reset);
	}
}

methodmap SMDataTimer < SMTimer
{
	/**
	 * Creates a timer associated with a new datapack if needed.
	 * @note The datapack is automatically freed when the timer ends.
	 * @note The position of the datapack is not reset or changed for the timer function.
	 *
	 * @param interval			Interval from the current game time to execute the given function.
	 * @param func				Function to execute once the given interval has elapsed.
	 * @param datapack			The newly created datapack is passed through this by-reference
	 *							parameter to the timer callback function.
	 * @param flags				Timer flags.
	 * @return					Handle to the timer object.  You do not need to call CloseHandle().
	 */
	public SMDataTimer(float interval, Timer func, DataPack &datapack, int flags=0)
	{
		if (datapack == null)
		{
			datapack = new DataPack();
		}
		flags |= TIMER_DATA_HNDL_CLOSE;
		return view_as<SMDataTimer>(new SMTimer(interval, func, datapack, flags));
	}
}

#if defined _clientprefs_included
// Cookie Methodmaps!
methodmap Cookie < Handle
{
	/**
	 * Creates a new Client preference cookie.
	 *
	 * Cookies can be closed via delete when
	 * no longer needed.
	 *
	 * @param name			Name of the new preference cookie.
	 * @param description	Optional description of the preference cookie.
	 * @param access		What CookieAccess level to assign to this cookie.
	 * @return				A handle to the newly created cookie. If the cookie already
	 *                      exists, a handle to it will still be returned.
	 * @error				Cookie name is blank.
	 */
	public Cookie(const char[] name, const char[] description, CookieAccess access)
	{
		return view_as<Cookie>(RegClientCookie(name, description, access));
	}

	/**
	 * Searches for a Client preference cookie. Use as Cookie.Find
	 *
	 * Cookies can be closed via delete when
	 * no longer needed.
	 *
	 * @param name			Name of cookie to find.
	 * @return				A cookie if it is found. null otherwise.
	 */
	public static Cookie Find(const char[] name)
	{
		return view_as<Cookie>(FindClientCookie(name));
	}

	/**
	 * Set the value of a Client preference cookie.
	 *
	 * @param client		Client index.
	 * @param value			String value to set.
	 * @error				Invalid cookie handle or invalid client index.
	 */
	public void SetValue(int client, const char[] value)
	{
		SetClientCookie(client, this, value);
	}

	/**
	 * Retrieve the value of a Client preference cookie.
	 *
	 * @param client		Client index.
	 * @param buffer		Copyback buffer for value.
	 * @param maxlen		Maximum length of the buffer.
	 * @error				Invalid cookie handle or invalid client index.
	 */
	public void GetValue(int client, char[] buffer, int maxlen)
	{
		GetClientCookie(client, this, buffer, maxlen);
	}

	/**
	 * Sets the value of a Client preference cookie based on an authID string.
	 *
	 * @param authID		String Auth/STEAM ID of player to set.
	 * @param value			String value to set.
	 * @error			Invalid cookie handle.
	 */
	public void SetByAuth(const char[] authID, const char[] value)
	{
		SetAuthIdCookie(authID, this, value);
	}

	/**
	 * Add a new prefab item to the client cookie settings menu.
	 *
	 * Note: This handles everything automatically and does not require a callback
	 *
	 * @param type			A CookieMenu prefab menu type.
	 * @param display		Text to show on the menu.
	 * @param handler		Optional handler callback for translations and output on selection
	 * @param info			Info data to pass to the callback.
	 * @error				Invalid cookie handle.
	 */
	public void SetMenu(CookieMenu type, const char[] display, CookieMenuHandler handler=INVALID_FUNCTION, any info=0)
	{
		SetCookiePrefabMenu(this, type, display, handler, info);
	}

	/**
	 * Returns the access level of a cookie
	 *
	 * @return				CookieAccess access level.
	 * @error				Invalid cookie handle.
	 */
	property CookieAccess Access
	{
		public get() {return GetCookieAccess(this);}
	}

	/**
	 * Returns the last updated timestamp for a client cookie
	 *
	 * @param client		Client index.
	 * @return			Last updated timestamp.
	 */
	public int GetTime(int client)
	{
		return GetClientCookieTime(client, this);
	}
}

methodmap CookieIterator < Handle
{
	/**
	 * Gets a cookie iterator.  Must be freed with delete.
	 *
	 * @return				A new cookie iterator.
	 */
	 public CookieIterator()
	 {
	 	return view_as<CookieIterator>(GetCookieIterator());
	 }

	 /**
	 * Reads a cookie iterator, then advances to the next cookie if any.
	 *
	 * @param name			Name buffer.
	 * @param nameLen		Name buffer size.
	 * @param access		Access level of the cookie.
	 * @param desc			Cookie description buffer.
	 * @param descLen		Cookie description buffer size.
	 * @return				True on success, false if there are no more commands.
	 */
	 public bool Read(char[] name, int nameLen, CookieAccess &access, char[] desc="", int descLen=0)
	 {
	 	return ReadCookieIterator(this, name, nameLen, access, desc, descLen);
	 }
}
#endif

/*methodmap SMMenu < Menu
{
	public void PushString(const char[] id, const char[] data)
	{
		AddMenuItem(this, id, data, ITEMDRAW_IGNORE);
	}

	public void PushCell(const char[] id, int data)
	{
		char DataString[64];
		IntToString(data, DataString, sizeof(DataString));
		AddMenuItem(this, id, DataString, ITEMDRAW_IGNORE);
	}

	public void PushFloat(const char[] id, float data)
	{
		char DataString[64];
		FloatToString(data, DataString, sizeof(DataString));
		AddMenuItem(this, id, DataString, ITEMDRAW_IGNORE);
	}

	public void PushBool(const char[] id, bool data)
	{
		char DataString[64];
		IntToString(view_as<int>(data), DataString, sizeof(DataString));
		AddMenuItem(this, id, DataString, ITEMDRAW_IGNORE);
	}

	public bool GetString(const char[] id, char[] buffer, int size)
	{
		char info[64]; char data[64];

		for (int i = 0; i < GetMenuItemCount(this); i++)
		{
			if (GetMenuItem(this, i, info, sizeof(info), _, data, sizeof(data)))
			{
				if (StrEqual(info, id))
				{
					strcopy(buffer, size, data);
					return true;
				}
			}
		}

		return false;
	}

	public int GetCell(const char[] id, int DefaultValue = 0)
	{
		char info[64]; char data[64];

		for (int i = 0; i < GetMenuItemCount(this); i++)
		{
			if (GetMenuItem(this, i, info, sizeof(info), _, data, sizeof(data)))
			{
				if (StrEqual(info, id))
				{
					return StringToInt(data);
				}
			}
		}

		return DefaultValue;
	}

	public float GetFloat(const char[] id, float DefaultValue = 0.0)
	{
		char info[64]; char data[64];

		for (int i = 0; i < GetMenuItemCount(this); i++)
		{
			if (GetMenuItem(this, i, info, sizeof(info), _, data, sizeof(data)))
			{
				if (StrEqual(info, id))
				{
					return StringToFloat(data);
				}
			}
		}

		return DefaultValue;
	}

	public bool GetBool(const char[] id, bool DefaultValue = false)
	{
		char info[64]; char data[64];

		for (int i = 0; i < GetMenuItemCount(this); i++)
		{
			if (GetMenuItem(this, i, info, sizeof(info), _, data, sizeof(data)))
			{
				if (StrEqual(info, id))
				{
					return view_as<bool>(StringToInt(data));
				}
			}
		}

		return DefaultValue;
	}

	public float Copy(Menu dest, const char[] id)
	{
		char info[64]; char data[64];

		for (int i = 0; i < GetMenuItemCount(this); i++)
		{
			if (GetMenuItem(this, i, info, sizeof(info), _, data, sizeof(data)))
			{
				if (StrEqual(info, id))
				{
					dest.AddItem(id, data, ITEMDRAW_IGNORE);
				}
			}
		}
	}

	public bool AddItemFormat(const char[] info, int style = ITEMDRAW_DEFAULT, const char[] format, any ...)
	{
		char display[128];
		VFormat(display, sizeof(display), format, 4);

		return AddMenuItem(this, info, display, style);
	}
}*/

/*--------------------------------------------------------->
.-----.                      .---.         .-.                          .---.
`-. .-'                      : .--'       .' `.                         `--. :
  : : .--.  .--.  ,-.,-.,-.  : `;.--. .--.`. .'.--.  .--.  .--.  .--.     ,','
  : :' '_.'' .; ; : ,. ,. :  : :' .; :: ..': : : ..'' '_.'`._-.'`._-.'  .'.'_
  :_;`.__.'`.__,_;:_;:_;:_;  :_;`.__.':_;  :_; :_;  `.__.'`.__.'`.__.'  :____;

- Team Fortress 2
- TF2

- Credits: Pelipoika, 11530, Chdata, Tylerst, aIM
--------------------------------------------------------->*/

#define TF2_MAX_CLASSES 10
#define TF_MAX_CLASSES TF2_MAX_CLASSES
#define TF2_MAX_SLOTS 8
#define TF_MAX_SLOTS TF2_MAX_SLOTS

#define PICKUP_TYPE_HEALTHKIT	1
#define PICKUP_TYPE_AMMOBOX		2

#define PICKUP_SMALL	0
#define PICKUP_MEDIUM	1
#define PICKUP_FULL		2

enum TFClassRole
{
	TFRole_Unknown,
	TFRole_Offense,
	TFRole_Defense,
	TFRole_Support
}

#define SPELL_FIREBALL			0
#define SPELL_BALLOBATS			1
#define SPELL_HEALINGAURA		2
#define SPELL_PUMPKINMIRV		3
#define SPELL_SUPERJUMP			4
#define SPELL_INVISIBILITY		5
#define SPELL_TELEPORT			6
#define SPELL_TELSABOLT			7
#define SPELL_MINIFY			8
#define SPELL_SUMMONMONOCULUS	9
#define SPELL_METEORSHOWER		10
#define SPELL_SUMMONSKELETONS	11

#define POWERUP_STRENGTH	0
#define POWERUP_RESIST		1
#define POWERUP_VAMPIRE		2
#define POWERUP_WARLOCK		3
#define POWERUP_HASTE		4
#define POWERUP_REGEN		5
#define POWERUP_PRECISION	6
#define POWERUP_AGILITY		7
#define POWERUP_KNOCKOUT	8
#define POWERUP_KING		9
#define POWERUP_PLAGUE		10
#define POWERUP_SUPERNOVA	11

enum TFGod
{
	TFGod_God,
	TFGod_Buddha,
	TFGod_Mortal
}

/**
*	Create a glow entity and attach it to a client.
*	This seems to have an unintended side effect for animations for models for some reason.
*
*	name	Name to give the entity.
*	target	Target entity index to glow.
*	colors	Color of the glow.
*
*	return	Entity index of the glow.
**/
stock int TF2_CreateGlow(const char[] name, int target, int color[4] = {255, 255, 255, 255})
{
	char sClassname[64];
	GetEntityClassname(target, sClassname, sizeof(sClassname));

	char sTarget[128];
	Format(sTarget, sizeof(sTarget), "%s%i", sClassname, target);
	DispatchKeyValue(target, "targetname", sTarget);

	int glow = CreateEntityByName("tf_glow");

	if (IsValidEntity(glow))
	{
		char sGlow[64];
		Format(sGlow, sizeof(sGlow), "%i %i %i %i", color[0], color[1], color[2], color[3]);

		DispatchKeyValue(glow, "targetname", name);
		DispatchKeyValue(glow, "target", sTarget);
		DispatchKeyValue(glow, "Mode", "1");
		DispatchKeyValue(glow, "GlowColor", sGlow);
		DispatchSpawn(glow);

		//SetVariantString("!activator");
		//AcceptEntityInput(glow, "SetParent", target, glow, 0);

		SetParent(target, glow);

		AcceptEntityInput(glow, "Enable");
	}

	return glow;
}

/**
*	Retrieves the name of the class a client is playing as.
*
*	client	Client index.
*	name	String buffer to save it to.
*	size	Size of the buffer.
*	capitalize	Whether to capitalize the first letter or not.
*
*	return	void
**/
stock void TF2_GetClientClassName(int client, char[] buffer, int size, bool capitalize = false)
{
	TF2_GetClassName(TF2_GetPlayerClass(client), buffer, size, capitalize);
}

/**
*	Retrieves the name of a certain class based on the type given.
*
*	class	Class.
*	name	String buffer to save it to.
*	size	Size of the buffer.
*	capitalize	Whether to capitalize the first letter or not.
*
*	return	void
**/
stock void TF2_GetClassName(TFClassType class, char[] name, int size, bool capitalize = false)
{
	switch (class)
	{
		case TFClass_Unknown: strcopy(name, size, "unknown");
		case TFClass_Scout: strcopy(name, size, "scout");
		case TFClass_Sniper: strcopy(name, size, "sniper");
		case TFClass_Soldier: strcopy(name, size, "soldier");
		case TFClass_DemoMan: strcopy(name, size, "demoman");
		case TFClass_Medic: strcopy(name, size, "medic");
		case TFClass_Heavy: strcopy(name, size, "heavy");
		case TFClass_Pyro: strcopy(name, size, "pyro");
		case TFClass_Spy: strcopy(name, size, "spy");
		case TFClass_Engineer: strcopy(name, size, "engineer");
	}

	if (capitalize) name[0] = CharToUpper(name[0]);
}

/**
*	Strips a player to melee only and equips their melee.
*
*	client	Client index.
*
*	return	void
**/
stock void TF2_StripToMelee(int client)
{
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Primary);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Secondary);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Grenade);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Building);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_PDA);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Item1);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Item2);

	int melee;
	if ((melee = GetPlayerWeaponSlot(client, TFWeaponSlot_Melee)) != -1 && IsValidEntity(melee))
	{
		EquipPlayerWeapon(client, melee);
	}
}

/**
*	Sets the color of a player including all of their weapons and cosmetics.
*
*	client	Client index.
*	red		Amount of Red. (0-255)
*	green	Amount of Green. (0-255)
*	blue	Amount of Blue. (0-255)
*	alpha	Amount of Alpha. (0-255)
*
*	return	void
**/
stock void TF2_SetPlayerColor(int client, int red, int green, int blue, int alpha)
{
	SetEntityRenderMode(client, RENDER_TRANSCOLOR);
	SetEntityRenderColor(client, red, green, blue, alpha);

	int entity = INVALID_ENT_INDEX;
	while((entity = FindEntityByClassname(entity, "tf_weapon_*")) != INVALID_ENT_INDEX)
	{
		if ((HasEntProp(entity, Prop_Send, "m_hOwner") && GetEntPropEnt(entity, Prop_Send, "m_hOwner") == client) ||
		 (HasEntProp(entity, Prop_Send, "m_hOwnerEntity") && GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity") == client))
		{
			SetEntityRenderMode(client, RENDER_TRANSCOLOR);
			SetEntityRenderColor(entity, red, green, blue, alpha);
		}
	}

	entity = INVALID_ENT_INDEX;
	while((entity = FindEntityByClassname(entity, "tf_wearable_*")) != INVALID_ENT_INDEX)
	{
		if ((HasEntProp(entity, Prop_Send, "m_hOwner") && GetEntPropEnt(entity, Prop_Send, "m_hOwner") == client) ||
		 (HasEntProp(entity, Prop_Send, "m_hOwnerEntity") && GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity") == client))
		{
			SetEntityRenderMode(client, RENDER_TRANSCOLOR);
			SetEntityRenderColor(entity, red, green, blue, alpha);
		}
	}
}

/**
*	Force the round to win and give the win to a certain team or draw.
*	You should code a boolean in your plugin to automatically pass to this callback so that it doesn't create the entity if the map isn't running.
*
*	team	Team index to have the win. (unassigned = draw)
*	map_valid	Passive check to use in your plugins to make sure the map is running.
*
*	return	void
**/
stock void TF2_ForceRoundWin(TFTeam team, bool map_valid = true)
{
	int entity = FindEntityByClassname(INVALID_ENT_INDEX, "team_control_point_master");

	if (!IsValidEntity(entity))
	{
		entity = CreateEntityByName("team_control_point_master");
		DispatchSpawn(entity);
		AcceptEntityInput(entity, "Enable");
	}

	SetVariantInt(view_as<int>(team));
	AcceptEntityInput(entity, "SetWinner");
}

/**
*	Retrieves the current uber level of a Medic.
*
*	client	Client index.
*
*	return	Amount of current uber.
**/
stock float TF2_GetUberLevel(int client)
{
	int weapon = GetPlayerWeaponSlot(client, 1);
	return (IsValidEntity(weapon) && HasEntProp(weapon, Prop_Send, "m_flChargeLevel")) ? GetEntPropFloat(weapon, Prop_Send, "m_flChargeLevel") : -1.0;
}

/**
*	Set the uber level on a Medic.
*
*	client	Client index.
*	amount	Amount of uber to set.
*
*	return	void
**/
stock void TF2_SetUberLevel(int client, float amount, bool cap = false)
{
	int weapon = GetPlayerWeaponSlot(client, 1);

	if (IsValidEntity(weapon) && HasEntProp(weapon, Prop_Send, "m_flChargeLevel"))
	{
		SetEntPropFloat(weapon, Prop_Send, "m_flChargeLevel", amount);

		if (cap && TF2_GetUberLevel(client) > 1.00)
		{
			SetEntPropFloat(weapon, Prop_Send, "m_flChargeLevel", 1.00);
		}
	}
}

/**
*	Add uber level to a Medic.
*
*	client	Client index.
*	amount	Amount of uber to add.
*
*	return	void
**/
stock void TF2_AddUberLevel(int client, float amount, bool cap = false)
{
	TF2_SetUberLevel(client, (TF2_GetUberLevel(client) + amount), cap);
}

/**
*	Remove uber level from a Medic.
*
*	client	Client index.
*	amount	Amount of uber to remove.
*
*	return	void
**/
stock void TF2_RemoveUberLevel(int client, float amount)
{
	TF2_SetUberLevel(client, (TF2_GetUberLevel(client) - amount));
}

/**
*	Retrieves the current metal of an Engineer.
*
*	client	Client index.
*
*	return	Amount of current metal.
**/
stock int TF2_GetMetal(int client)
{
	return GetEntProp(client, Prop_Data, "m_iAmmo", 4, 3);
}

/**
*	Set the metal of an Engineer.
*
*	client	Client index.
*	amount	Amount of metal to set.
*
*	return	void
**/
stock void TF2_SetMetal(int client, int amount)
{
	SetEntProp(client, Prop_Data, "m_iAmmo", amount, 4, 3);
}

/**
*	Add metal to a Engineer.
*
*	client	Client index.
*	amount	Amount of metal to add.
*
*	return	void
**/
stock void TF2_AddMetal(int client, int amount)
{
	TF2_SetMetal(client, (TF2_GetMetal(client) + amount));
}

/**
*	Remove metal from a Engineer.
*
*	client	Client index.
*	amount	Amount of metal to remove.
*
*	return	void
**/
stock void TF2_RemoveMetal(int client, int amount)
{
	TF2_SetMetal(client, (TF2_GetMetal(client) - amount));
}

/**
*	Sets the current godmode status for the client.
*
*	client	Client index.
*	status	Status for godmode.
*
*	return	void
**/
stock void TF2_SetGodmode(int client, TFGod status = TFGod_God)
{
	SetEntProp(client, Prop_Data, "m_takedamage", view_as<int>(status), 1);
}

/**
*	Gets the current godmode status from the client.
*
*	client	Client index.
*
*	return	void
**/
stock TFGod TF2_GetGodmode(int client)
{
	return view_as<TFGod>(GetEntProp(client, Prop_Data, "m_takedamage", 1));
}

/**
*	Retrieves the current healing target of a Medic.
*
*	client	Client index.
*
*	return	Client index the Medic is healing.
**/
stock int TF2_GetHealingTarget(int client)
{
	int weapon = GetPlayerWeaponSlot(client, 1);

	if (!IsValidEntity(weapon) || weapon != GetActiveWeapon(client))
	{
		return -1;
	}

	char sClassname[32];
	GetEdictClassname(weapon, sClassname, sizeof(sClassname));

	if (StrContains(sClassname, "tf_weapon_med") == -1)
	{
		return -1;
	}

	return GetEntProp(weapon, Prop_Send, "m_bHealing") ? GetEntPropEnt(weapon, Prop_Send, "m_hHealingTarget") : -1;
}

/**
*	Simple check if the mode is arena or not.
*
*	return	Is Arena or not.
**/
stock bool IsArenaActive()
{
	return FindEntityByClassname(-1, "tf_logic_arena") != -1;
}

/**
*	Simple stock to create an explosion.
*
*	origin	Vector coordinate to spawn the explosion.
*	damage	Damage to cause in the area.
*	radius	Radius of the blast damage.
*	magnitude	Magnitude to push players away from the point.
*	attacker	Attacker index if there is one. (0 = world)
*	inflictor	Inflictor index if there is one. (0 = world)
*	team	Team to cause damage to. (0 = everyone)
*	particle	Particle effect to use. (default is cinefx_goldrush which is fairly large)
*	sound	Sound to play for the explosion.
*	amplitude	Amplitude for the shaking effect.
*	frequency	Frequency for the shaking effect.
*	duration	Duration for the shaking effect.
*
*	return	void
**/
stock void TF2_CreateExplosion(float origin[3], float damage = 99999.0, float radius = 250.0, float magnitude = 500.0, int attacker = 0, int inflictor = 0, int team = 0, const char[] particle = "cinefx_goldrush", const char[] sound = "items/cart_explode.wav", float amplitude = 50.0, float frequency = 150.0, float duration = 3.0, int damagetype = DMG_BLAST, int weapon = -1)
{
	CreateParticle(particle, 10.0, origin);
	EmitSoundToAllSafe(sound);
	ScreenShakeAll(SHAKE_START, amplitude, frequency, duration);
	PushAllPlayersFromPoint(origin, magnitude, radius, team);
	DamageArea(origin, radius, damage, attacker, inflictor, team, damagetype, weapon);
}

/**
*	A simple stock to remove every wearable on a client.
*
*	client	Client index.
*
*	return	void
**/
stock void TF2_RemoveAllWearables(int client)
{
	int entity;
	while ((entity = FindEntityByClassname(entity, "tf_wearable*")) != -1)
	{
		if (HasEntProp(entity, Prop_Send, "m_hOwnerEntity") && GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity") == client)
		{
			TF2_RemoveWearable(client, entity);
		}
	}
}

/**
*	Sets the size of a player and scales accordingly.
*	https://forums.alliedmods.net/showthread.php?t=193255
*
*	client	Client index.
*	amount	Scale to set to.
*
*	return	void
**/
stock void TF2_ResizePlayer(int client, float amount)
{
	SetEntPropFloat(client, Prop_Send, "m_flModelScale", amount);
	SetEntPropFloat(client, Prop_Send, "m_flStepSize", 18.0 * amount);
}

/**
*	Spawn an item that players can pickup.
*
*	origin	Vector to spawn the pickup at.
*	type	Type of pickup.
*	size	Size of the pickup.
*	team	Which team it belongs to.
*	persistent	Whether the pickup is persistent. (will list until the round ends, map changes, etc)
*	z_offset	How far above the origin to actually spawn the pickup.
*	model	Custom model for the pickup. (can be blank)
*
*	return	Entity index of the pickup.
**/
stock int TF2_SpawnPickup(float origin[3], int type = PICKUP_TYPE_HEALTHKIT, int size = PICKUP_FULL, int team = 0, bool persistent = false, float z_offset = 5.0, const char[] model = "")
{
	char sTeam[6];
	IntToString(team, sTeam, sizeof(sTeam));

	char sEntity[64];

	switch (type)
	{
		case PICKUP_TYPE_HEALTHKIT: strcopy(sEntity, sizeof(sEntity), "item_healthkit_");
		case PICKUP_TYPE_AMMOBOX: strcopy(sEntity, sizeof(sEntity), "item_ammopack_");
	}

	switch (size)
	{
		case PICKUP_SMALL: StrCat(sEntity, sizeof(sEntity), "small");
		case PICKUP_MEDIUM: StrCat(sEntity, sizeof(sEntity), "medium");
		case PICKUP_FULL: StrCat(sEntity, sizeof(sEntity), "full");
	}

	int entity = CreateEntityByName(sEntity);

	if (IsValidEntity(entity))
	{
		origin[2] += z_offset;
		DispatchKeyValueVector(entity, "origin", origin);
		DispatchKeyValue(entity, "TeamNum", sTeam);
		if (!persistent) DispatchKeyValue(entity, "OnPlayerTouch", "!self,Kill,,0,-1");	//Make sure it's killed after 1st pickup.
		if (strlen(model) > 0) DispatchKeyValue(entity, "powerup_model", model);
		DispatchSpawn(entity);

		EmitSoundToAll("items/spawn_item.wav", entity, _, _, _, 0.75);
	}

	return entity;
}

/**
*	Retrieves the clients current class role.
*
*	client	Client index.
*
*	return	Role for the client.
**/
stock TFClassRole TF2_GetClientRole(int client)
{
	return TF2_GetClassRole(TF2_GetPlayerClass(client));
}

/**
*	Retrieves the class role from the class it's given.
*
*	class	Class index.
*
*	return	Role for the class.
**/
stock TFClassRole TF2_GetClassRole(TFClassType class)
{
	switch (class)
	{
		case TFClass_Unknown: return TFRole_Unknown;
		case TFClass_Scout: return TFRole_Offense;
		case TFClass_Sniper: return TFRole_Support;
		case TFClass_Soldier: return TFRole_Offense;
		case TFClass_DemoMan: return TFRole_Defense;
		case TFClass_Medic: return TFRole_Support;
		case TFClass_Heavy: return TFRole_Defense;
		case TFClass_Pyro: return TFRole_Offense;
		case TFClass_Spy: return TFRole_Support;
		case TFClass_Engineer: return TFRole_Defense;
	}

	return TFRole_Unknown;
}

/**
*	Sets a players health while applying TF2 health logic.
*
*	client	Client index.
*	amount	Amount of health to set.
*	overheal	Overheal amount to offset.
*	additive	Whether to account for overheal.
*	event	Fire the health gain event.
*
*	return	void
**/
stock void TF2_SetPlayerHealth(int client, int amount, float overheal = 1.5, bool additive = false, bool event = true)
{
	int iHealth = GetClientHealth(client);
	int iNewHealth = amount;
	int iMax = additive ? (TF2_GetMaxHealth(client) + RoundFloat(overheal)) : TF2_GetMaxOverHeal(client, overheal);

	if (iHealth < iMax)
	{
		iNewHealth = ClampCell(iNewHealth, 1, iMax);

		if (event)
		{
			TF2_ShowHealthGain(client, iNewHealth - iHealth);
		}

		SetEntityHealth(client, iNewHealth);
	}
}

/**
*	Add to a players health while applying TF2 health logic.
*
*	client	Client index.
*	amount	Amount of health to add.
*	overheal	Overheal amount to offset.
*	additive	Whether to account for overheal.
*	event	Fire the health gain event.
*
*	return	void
**/
stock void TF2_AddPlayerHealth(int client, int amount, float overheal = 1.5, bool additive = false, bool event = true)
{
	int iHealth = GetClientHealth(client);
	int iNewHealth = iHealth + amount;
	int iMax = additive ? (TF2_GetMaxHealth(client) + RoundFloat(overheal)) : TF2_GetMaxOverHeal(client, overheal);

	if (iHealth < iMax)
	{
		iNewHealth = ClampCell(iNewHealth, 1, iMax);

		if (event)
		{
			TF2_ShowHealthGain(client, iNewHealth - iHealth);
		}

		SetEntityHealth(client, iNewHealth);
	}
}

/**
*	Removes health from players while applying TF2 health logic.
*
*	client	Client index.
*	amount	Amount of health to remove.
*	overheal	Overheal amount to offset.
*	additive	Whether to account for overheal.
*	event	Fire the health gain event.
*
*	return	void
**/
stock void TF2_RemovePlayerHealth(int client, int amount, float overheal = 1.5, bool additive = false, bool event = true)
{
	int iHealth = GetClientHealth(client);
	int iNewHealth = iHealth - amount;
	int iMax = additive ? (TF2_GetMaxHealth(client) + RoundFloat(overheal)) : TF2_GetMaxOverHeal(client, overheal);

	if (iHealth < iMax)
	{
		iNewHealth = ClampCell(iNewHealth, 1, iMax);

		if (event)
		{
			TF2_ShowHealthGain(client, iNewHealth - iHealth);
		}

		SetEntityHealth(client, iNewHealth);
	}
}

/**
*	Fires an event in TF2 to apply proper logic like for health meter sprites.
*
*	client	Client index.
*	health	Health to pass.
*	healer	Healer index.
*
*	return	void
**/
stock void TF2_ShowHealthGain(int client, int health, int healer = -1)
{
	int iUserId = GetClientUserId(client);

	Event hEvent = CreateEvent("player_healed", true);
	hEvent.SetBool("sourcemod", true);
	hEvent.SetInt("patient", iUserId);
	hEvent.SetInt("healer", healer > 0 && IsClientInGame(healer) ? GetClientUserId(healer) : iUserId);
	hEvent.SetInt("amount", health);
	hEvent.Fire();

	hEvent = CreateEvent("player_healonhit", true);
	hEvent.SetBool("sourcemod", true);
	hEvent.SetInt("amount", health);
	hEvent.SetInt("entindex", client);
	hEvent.Fire();
}

/**
*	Retrieves the maximum health for a client in TF2.
*
*	client	Client index.
*
*	return	Max health of the client.
**/
stock int TF2_GetMaxHealth(int client)
{
	int maxhealth = GetEntProp(GetPlayerResourceEntity(), Prop_Send, "m_iMaxHealth", _, client);
	return ((maxhealth == -1 || maxhealth == 80896) ? GetEntProp(client, Prop_Data, "m_iMaxHealth") : maxhealth);
}

/**
*	Retrieves the maximum overheal for a client in TF2.
*	Quick-Fix would be 1.25.
*
*	client	Client index.
*
*	return	Max overheal of the client.
**/
stock int TF2_GetMaxOverHeal(int client, float overheal = 1.5)
{
	return RoundFloat(float(TF2_GetMaxHealth(client)) * overheal);
}

/**
*	Retrieves the current overheal for a client in TF2.
*
*	client	Client index.
*
*	return	Current overheal of the client.
**/
stock int TF2_GetOverHeal(int client, float overheal = 1.5)
{
	return RoundFloat(float(TF2_GetMaxHealth(client)) * (overheal - 1.0));
}

/**
*	Retrieves whether the client is ready to overheal in TF2.
*
*	client	Client index.
*	add		Buffer to add.
*
*	return	Whether they're ready to overheal or not.
**/
stock bool TF2_ReadyToOverheal(int client, int add = 0)
{
	return (add > 0) ? ((TF2_GetMaxHealth(client) - GetClientHealth(client)) < add) : (GetClientHealth(client) >= TF2_GetMaxHealth(client));
}

/**
*	A simple stock to allow for attributes to be applied to weapons on a client.
*
*	client	Client index.
*	weapon	Entity index for the weapon. (-1 = all weapons)
*	attrib	Attribute to apply.
*	value	Value for the attribute.
*	invert	Whether to invert the value for inverted valued attributes.
*
*	return	void
**/
stock void TF2Attrib_SetByName_Weapons(int client, int weapon = -1, char[] attrib, float value, bool invert = false)
{
	if (weapon != -1 && IsValidEntity(weapon))
	{
		TF2Attrib_SetByName(weapon, attrib, value);
		return;
	}

	if (invert)
	{
		value = 1.0 + value;
	}
	else
	{
		value = 1.0 - value;
	}

	int weapon2;
	for (int i = 0; i < 6; i++)
	{
		weapon2 = GetPlayerWeaponSlot(client, i);

		if (IsValidEntity(weapon2))
		{
			TF2Attrib_SetByName(weapon2, attrib, value);
		}
	}
}

/**
*	A simple stock to remove attributes from the weapons on a client.
*
*	client	Client index.
*	weapon	Entity index for the weapon. (-1 = all weapons)
*	attrib	Attribute to apply.
*
*	return	void
**/
stock void TF2Attrib_RemoveByName_Weapons(int client, int weapon = -1, char[] attrib)
{
	if (weapon != -1 && IsValidEntity(weapon))
	{
		TF2Attrib_RemoveByName(weapon, attrib);
		return;
	}

	int weapon2;
	for (int i = 0; i < 6; i++)
	{
		weapon2 = GetPlayerWeaponSlot(client, i);

		if (IsValidEntity(weapon2))
		{
			TF2Attrib_RemoveByName(weapon2, attrib);
		}
	}
}

/**
*	A simple stock to apply a bonus movement speed effect to the client.
*
*	client	Client index.
*	value	Amount of movement speed to apply.
*
*	return	The original speed they were at to cache later if need be.
**/
stock float TF2Attrib_ApplyMoveSpeedBonus(int client, float value)
{
	Address addr = TF2Attrib_GetByName(client, "move speed bonus");
	float old;

	if (addr != Address_Null)
	{
		old = TF2Attrib_GetValue(addr);
	}

	TF2Attrib_SetByName(client, "move speed bonus", 1.0 + value);
	TF2_AddCondition(client, TFCond_SpeedBuffAlly, 0.0);

	return old;
}

/**
*	A simple stock to remove a bonus movement speed effect from the client.
*
*	client	Client index.
*
*	return	void
**/
stock void TF2Attrib_RemoveMoveSpeedBonus(int client)
{
	TF2Attrib_RemoveByName(client, "move speed bonus");
	TF2_AddCondition(client, TFCond_SpeedBuffAlly, 0.0);
}

/**
*	A simple stock to apply a penalty movement speed effect to the client.
*
*	client	Client index.
*	value	Amount of movement speed to apply.
*
*	return	The original speed they were at to cache later if need be.
**/
stock float TF2Attrib_ApplyMoveSpeedPenalty(int client, float value)
{
	Address addr = TF2Attrib_GetByName(client, "move speed bonus");
	float old;

	if (addr != Address_Null)
	{
		old = TF2Attrib_GetValue(addr);
	}

	TF2Attrib_SetByName(client, "move speed penalty", 1.0 - value);
	TF2_AddCondition(client, TFCond_SpeedBuffAlly, 0.0);

	return old;
}

/**
*	A simple stock to remove a penalty movement speed effect from the client.
*
*	client	Client index.
*
*	return	void
**/
stock void TF2Attrib_RemoveMoveSpeedPenalty(int client)
{
	TF2Attrib_RemoveByName(client, "move speed penalty");
	TF2_AddCondition(client, TFCond_SpeedBuffAlly, 0.0);
}

/**
*	Sets the spell plus charges on a client in TF2.
*
*	client	Client index.
*	spell	Spell index.
*	uses	Amount of uses to apply.
*
*	return	void
**/
stock void TF2_SetSpell(int client, int spell, int uses)
{
	int entity = TF2_GetSpellBook(client);

	if (IsValidEntity(entity))
	{
		SetEntProp(entity, Prop_Send, "m_iSelectedSpellIndex", spell);
		SetEntProp(entity, Prop_Send, "m_iSpellCharges", uses);
	}
}

/**
*	Sets the spell uses for their current spell on a client in TF2.
*
*	client	Client index.
*	uses	Amount of uses to apply.
*
*	return	void
**/
stock void TF2_SetSpellUses(int client, int uses)
{
	int entity = TF2_GetSpellBook(client);

	if (IsValidEntity(entity))
	{
		SetEntProp(entity, Prop_Send, "m_iSpellCharges", uses);
	}
}

/**
*	Retrieves the current spell on a client in TF2.
*
*	client	Client index.
*
*	return	Spell index.
**/
stock int TF2_GetSpell(int client)
{
	int entity = TF2_GetSpellBook(client);

	if (IsValidEntity(entity))
	{
		return GetEntProp(ent, Prop_Send, "m_iSelectedSpellIndex");
	}

	return INVALID_ENT_INDEX;
}

/**
*	Retrieves the currently available uses of their current spell on a client in TF2.
*
*	client	Client index.
*
*	return	Amount of uses.
**/
stock int TF2_GetSpellUses(int client)
{
	int entity = TF2_GetSpellBook(client);

	if (IsValidEntity(entity))
	{
		return GetEntProp(entity, Prop_Send, "m_iSpellCharges");
	}
}

/**
*	Retrieves the entity index for the clients spellbook.
*
*	client	Client index.
*
*	return	Spellbook index.
**/
stock int TF2_GetSpellBook(int client)
{
	int entity = -1;
	while((entity = FindEntityByClassname(entity, "tf_weapon_spellbook")) != INVALID_ENT_REFERENCE)
	{
		if (HasEntProp(entity, Prop_Send, "m_hOwnerEntity") && GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity") == client)
		{
			return entity;
		}
	}

	return -1;
}

/**
*	A simple stock to check whether a spell is rare or not.
*
*	spell	Spell index.
*
*	return	Whether the spell is considered rare or not.
**/
stock bool TF2_SpellIsRare(int spell)
{
	switch(spell)
	{
		case SPELL_TELSABOLT, SPELL_MINIFY, SPELL_SUMMONMONOCULUS, SPELL_METEORSHOWER, SPELL_SUMMONSKELETONS: return true;
		default: return false;
	}
}

//TODO: Update it so that it properly capitalizes every word properly.
/**
*	Retrieves the name of a certain spell.
*
*	spell	Spell index.
*	buffer	Buffer to store the name into.
*	size	Size of the buffer.
*	capitalize	Whether to capitalize the name.
*
*	return	void
**/
stock void TF2_GetSpellName(int spell, char[] buffer, int size, bool capitalize = true)
{
	switch(spell)
	{
		case SPELL_FIREBALL: strcopy(buffer, size, "fireball");
		case SPELL_BALLOBATS: strcopy(buffer, size, "ball O' Bats");
		case SPELL_HEALINGAURA: strcopy(buffer, size, "healing Aura");
		case SPELL_PUMPKINMIRV: strcopy(buffer, size, "pumpkin MIRV");
		case SPELL_SUPERJUMP: strcopy(buffer, size, "superjump");
		case SPELL_INVISIBILITY: strcopy(buffer, size, "invisibility");
		case SPELL_TELEPORT: strcopy(buffer, size, "teleport");
		case SPELL_TELSABOLT: strcopy(buffer, size, "tesla Bolt");
		case SPELL_MINIFY: strcopy(buffer, size, "minify");
		case SPELL_SUMMONMONOCULUS: strcopy(buffer, size, "summon Monoculus");
		case SPELL_METEORSHOWER: strcopy(buffer, size, "meteor Shower");
		case SPELL_SUMMONSKELETONS: strcopy(buffer, size, "summon Skeletons");
		default: strcopy(buffer, size, "none");
	}

	if (capitalize) name[0] = CharToUpper(name[0]);
}

/**
*	Sets the current powerup on a client.
*
*	client	Client index.
*	powerup	Powerup to apply.
*
*	return	void
**/
stock void TF2_SetPowerup(int client, int powerup)
{
	switch (powerup)
	{
		case POWERUP_STRENGTH:
		{
			TF2_AddCondition(client, TFCond_RuneStrength);
		}

		case POWERUP_RESIST:
		{
			TF2_AddCondition(client, TFCond_RuneResist);
		}

		case POWERUP_VAMPIRE:
		{
			TF2_AddCondition(client, TFCond_RuneVampire);
		}

		case POWERUP_WARLOCK:
		{
			TF2_AddCondition(client, TFCond_RuneWarlock);
		}

		case POWERUP_HASTE:
		{
			TF2_AddCondition(client, TFCond_RuneHaste);
		}

		case POWERUP_REGEN:
		{
			TF2_AddCondition(client, TFCond_RuneRegen);
		}

		case POWERUP_PRECISION:
		{
			TF2_AddCondition(client, TFCond_RunePrecision);
		}

		case POWERUP_AGILITY:
		{
			TF2_AddCondition(client, TFCond_RuneAgility);
		}

		case POWERUP_KNOCKOUT:
		{
			TF2_AddCondition(client, TFCond_RuneKnockout);
		}

		case POWERUP_KING:
		{
			TF2_AddCondition(client, TFCond_KingRune);
			TF2_AddCondition(client, TFCond_KingAura);
		}

		case POWERUP_PLAGUE:
		{
			TF2_AddCondition(client, TFCond_Plague);
		}

		case POWERUP_SUPERNOVA:
		{
			TF2_AddCondition(client, TFCond_SupernovaRune);
		}
	}

	//This has to be here.
	TF2_AddCondition(client, TFCond_HasRune);
}

/**
*	Retrieves the default weapon class for a client based on slot.
*
*	class	Class index.
*	slot	Slot to retrieve.
*	buffer	Buffer to store the classname in.
*	size	Size of the buffer.
*
*	return	Whether it found a default or not based on the slot.
**/
stock bool TF2_GetDefaultWeaponClass(TFClassType class, int slot, char[] buffer, int size)
{
	bool found;

	switch(class)
	{
		case TFClass_Scout:
		{
			switch(slot)
			{
				case 0: Format(buffer, size, "tf_weapon_scattergun"), found = true;
				case 1: Format(buffer, size, "tf_weapon_pistol_scout"), found = true;
				case 2: Format(buffer, size, "tf_weapon_bat"), found = true;
			}
		}
		case TFClass_Sniper:
		{
			switch(slot)
			{
				case 0: Format(buffer, size, "tf_weapon_sniperrifle"), found = true;
				case 1: Format(buffer, size, "tf_weapon_smg"), found = true;
				case 2: Format(buffer, size, "tf_weapon_club"), found = true;
			}
		}
		case TFClass_Soldier:
		{
			switch(slot)
			{
				case 0: Format(buffer, size, "tf_weapon_rocketlauncher"), found = true;
				case 1: Format(buffer, size, "tf_weapon_shotgun_soldier"), found = true;
				case 2: Format(buffer, size, "tf_weapon_shovel"), found = true;
			}
		}
		case TFClass_DemoMan:
		{
			switch(slot)
			{
				case 0: Format(buffer, size, "tf_weapon_grenadelauncher"), found = true;
				case 1: Format(buffer, size, "tf_weapon_pipebomblauncher"), found = true;
				case 2: Format(buffer, size, "tf_weapon_bottle"), found = true;
			}
		}
		case TFClass_Medic:
		{
			switch(slot)
			{
				case 0: Format(buffer, size, "tf_weapon_syringegun_medic"), found = true;
				case 1: Format(buffer, size, "tf_weapon_medigun"), found = true;
				case 2: Format(buffer, size, "tf_weapon_bonesaw"), found = true;
			}
		}
		case TFClass_Heavy:
		{
			switch(slot)
			{
				case 0: Format(buffer, size, "tf_weapon_minigun"), found = true;
				case 1: Format(buffer, size, "tf_weapon_shotgun_hwg"), found = true;
				case 2: Format(buffer, size, "tf_weapon_fists"), found = true;
			}
		}
		case TFClass_Pyro:
		{
			switch(slot)
			{
				case 0: Format(buffer, size, "tf_weapon_flamethrower"), found = true;
				case 1: Format(buffer, size, "tf_weapon_shotgun_pyro"), found = true;
				case 2: Format(buffer, size, "tf_weapon_fireaxe"), found = true;
			}
		}
		case TFClass_Spy:
		{
			switch(slot)
			{
				case 0: Format(buffer, size, "tf_weapon_revolver"), found = true;
				case 1: Format(buffer, size, "tf_weapon_builder"), found = true;
				case 2: Format(buffer, size, "tf_weapon_knife"), found = true;
				case 4: Format(buffer, size, "tf_weapon_invis"), found = true;
			}
		}
		case TFClass_Engineer:
		{
			switch(slot)
			{
				case 0: Format(buffer, size, "tf_weapon_shotgun_primary"), found = true;
				case 1: Format(buffer, size, "tf_weapon_pistol"), found = true;
				case 2: Format(buffer, size, "tf_weapon_wrench"), found = true;
				case 3: Format(buffer, size, "tf_weapon_pda_engineer_build"), found = true;
			}
		}
	}

	return found;
}

/**
*	Retrieves the default weapon id for a client based on slot.
*
*	class	Class index.
*	slot	Slot to retrieve.
*
*	return	The id of the weapon slot.
**/
stock int TF2_GetDefaultWeaponID(TFClassType class, int slot)
{
	switch(class)
	{
		case TFClass_Scout:
		{
			switch(slot)
			{
				case 0: return 13;
				case 1: return 23;
				case 2: return 0;
			}
		}
		case TFClass_Sniper:
		{
			switch(slot)
			{
				case 0: return 14;
				case 1: return 16;
				case 2: return 3;
			}
		}
		case TFClass_Soldier:
		{
			switch(slot)
			{
				case 0: return 18;
				case 1: return 10;
				case 2: return 6;
			}
		}
		case TFClass_DemoMan:
		{
			switch(slot)
			{
				case 0: return 19;
				case 1: return 20;
				case 2: return 1;
			}
		}
		case TFClass_Medic:
		{
			switch(slot)
			{
				case 0: return 17;
				case 1: return 29;
				case 2: return 8;
			}
		}
		case TFClass_Heavy:
		{
			switch(slot)
			{
				case 0: return 15;
				case 1: return 11;
				case 2: return 5;
			}
		}
		case TFClass_Pyro:
		{
			switch(slot)
			{
				case 0: return 21;
				case 1: return 12;
				case 2: return 2;
			}
		}
		case TFClass_Spy:
		{
			switch(slot)
			{
				case 0: return 24;
				case 1: return 735;
				case 2: return 4;
				case 4: return 30;
			}
		}
		case TFClass_Engineer:
		{
			switch(slot)
			{
				case 0: return 9;
				case 1: return 22;
				case 2: return 7;
				case 3: return 25;
			}
		}
	}

	return -1;
}

/**
*	Retrieves a certain healer for the client by index.
*	Credits: Pelipoika
*
*	client	Client index.
*	index	Slot for the healer inside of 'm_nNumHealers' Netprop.
*
*	return	Client index for the healer.
**/
stock int TF2_GetHealerByIndex(int client, int index)
{
	if (!IsPlayerIndex(client) || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client))
	{
		return 0;
	}

	int m_aHealers = FindSendPropInfo("CTFPlayer", "m_nNumHealers") + 12;

	Address m_Shared = GetEntityAddress(client) + view_as<Address>(m_aHealers);
	Address aHealers = view_as<Address>(LoadFromAddress(m_Shared, NumberType_Int32));

	return (LoadFromAddress(aHealers + view_as<Address>(index * 0x24), NumberType_Int32) & 0xFFF);
}

/**
*	Retrieves the healer count for the client.
*
*	client	Client index.
*
*	return	Amount of healers the client has.
**/
stock int TF2_GetHealerCount(int client)
{
	if (!IsPlayerIndex(client) || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client) || !HasEntProp(client, Prop_Send, "m_nNumHealers"))
	{
		return 0;
	}

	return GetEntProp(client, Prop_Send, "m_nNumHealers");
}

/**
*	Checks whether or not a certain client is healing another client.
*
*	client	Client index.
*	healer	Healer index.
*
*	return	Amount of healers the client has.
**/
stock bool TF2_IsCurrentHealer(int client, int healer)
{
	if (!IsPlayerIndex(client) || !IsClientConnected(client) || !IsClientInGame(client) || !IsPlayerAlive(client) || !IsPlayerIndex(healer) || !IsClientConnected(healer) || !IsClientInGame(healer) || !IsPlayerAlive(healer) || TF2_GetPlayerClass(healer) != TFClass_Medic)
	{
		return false;
	}

	int healers;

	if ((healers = TF2_GetHealerCount(client)) == 0)
	{
		return false;
	}

	for (int i = 0; i < healers; i++)
	{
		if (TF2_GetHealerByIndex(client, i) == healer)
		{
			return true;
		}
	}

	return false;
}

stock TFTeam TF2_GetTeam(const char[] team)
{
	if (StrEqual(team, "1") || StrEqual(team, "spec", false) || StrEqual(team, "spectate", false) || StrEqual(team, "spectator", false) || StrEqual(team, "spectators", false))
	{
		return TFTeam_Spectator;
	}
	else if (StrEqual(team, "2") || StrEqual(team, "red", false))
	{
		return TFTeam_Red;
	}
	else if (StrEqual(team, "3") || StrEqual(team, "blu", false) || StrEqual(team, "blue", false))
	{
		return TFTeam_Blue;
	}

	return TFTeam_Unassigned;
}

stock int TF2_CreateRagdoll(int client, float destruct = 0.0, bool gold_ragdoll = false, bool ice_ragdoll = false)
{
	int ragdoll = CreateEntityByName("tf_ragdoll");

	if (IsValidEntity(ragdoll))
	{
		float vecOrigin[3];
		GetClientAbsOrigin(client, vecOrigin);

		float vecAngles[3];
		GetClientAbsAngles(client, vecAngles);

		TeleportEntity(ragdoll, vecOrigin, vecAngles, NULL_VECTOR);

		SetEntProp(ragdoll, Prop_Send, "m_iPlayerIndex", client);
		SetEntProp(ragdoll, Prop_Send, "m_bGoldRagdoll", gold_ragdoll);
		SetEntProp(ragdoll, Prop_Send, "m_bIceRagdoll", ice_ragdoll);
		SetEntProp(ragdoll, Prop_Send, "m_iTeam", GetClientTeam(client));
		SetEntProp(ragdoll, Prop_Send, "m_iClass", view_as<int>(TF2_GetPlayerClass(client)));
		SetEntProp(ragdoll, Prop_Send, "m_bOnGround", 1);

		SetEntityMoveType(ragdoll, MOVETYPE_NONE);

		DispatchSpawn(ragdoll);
		ActivateEntity(ragdoll);

		SetEntitySelfDestruct(ragdoll, destruct);

		return ragdoll;
	}

	return -1;
}

stock bool TF2_RemoveRagdoll(int client)
{
	int ragdoll = GetEntPropEnt(client, Prop_Send, "m_hRagdoll");

	if (IsValidEdict(ragdoll))
	{
		char classname[64];
		GetEdictClassname(ragdoll, classname, sizeof(classname));

		if (StrEqual(classname, "tf_ragdoll", false))
		{
			RemoveEdict(ragdoll);
		}
	}
}

/*--------------------------------------------------------->
.--.                    .-.                  .--.  .-.       _ .-.
: .--'                  .' `.                : .--'.' `.     :_;: :.-.       _
: :    .--. .-..-.,-.,-.`. .'.--. .--.  _____`. `. `. .'.--. .-.: `'.' .--. :_:
: :__ ' .; :: :; :: ,. : : :' '_.': ..':_____:_`, : : : : ..': :: . `.' '_.' _
`.__.'`.__.'`.__.':_;:_; :_;`.__.':_;        `.__.' :_; :_;  :_;:_;:_;`.__.':_;
.--. .-.        .-.          .-.     .--.  .--. .--.                 _
: .--': :        : :          : :    : ,. :: .-': .-'                :_;
: : _ : :   .--. : `-.  .--.  : :    : :: :: `; : `;.--. ,-.,-. .--. .-..-..-. .--.
: :; :: :_ ' .; :' .; :' .; ; : :_   : :; :: :  : :' '_.': ,. :`._-.': :: `; :' '_.'
`.__.'`.__;`.__.'`.__.'`.__,_;`.__;  `.__.':_;  :_;`.__.':_;:_;`.__.':_;`.__.'`.__.'

- Counter-Strike: Global Offensive
- CSGO

- Credits: SM9();
--------------------------------------------------------->*/

/**
*	A simple stock to show hud text to all in CSGO.
*
*	message	String to set on the hud text.
*	x	X coordinate to apply.
*	y	Y coordinate to apply.
*	channel	Channel to apply.
*	color	Color to apply.
*	color2	Color2 to apply.
*	effect	Effect to apply.
*	fadein	Fadein amount to apply.
*	fadeout	Fadeout amount to apply.
*	fxtime	Fxtime amount to apply.
*	holdtime	Holdtime amount to apply.
*	spawnflags	Spawn Flags to apply.
*
*	return	void
**/
stock void CSGO_ShowHudTextAll(char[] message, char[] x = "-1.0", char[] y = "-1.0", char[] channel = "0", char[] color = "255 255 255", char[] color2 = "0 0 0", char[] effect = "0", char[] fadein = "1.5", char[] fadeout = "0.5", char[] fxtime = "0.5", char[] holdtime = "5.0", char[] spawnflags = "0")
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i))
		{
			CSGO_ShowHudText(i, message, x, y, channel, color, color2, effect, fadein, fadeout, fxtime, holdtime, spawnflags);
		}
	}
}

/**
*	A simple stock to show hud text to a certain client in CSGO.
*
*	client	Client index.
*	message	String to set on the hud text.
*	x	X coordinate to apply.
*	y	Y coordinate to apply.
*	channel	Channel to apply.
*	color	Color to apply.
*	color2	Color2 to apply.
*	effect	Effect to apply.
*	fadein	Fadein amount to apply.
*	fadeout	Fadeout amount to apply.
*	fxtime	Fxtime amount to apply.
*	holdtime	Holdtime amount to apply.
*	spawnflags	Spawn Flags to apply.
*
*	return	void
**/
stock void CSGO_ShowHudText(int client, char[] message, char[] x = "-1.0", char[] y = "-1.0", char[] channel = "0", char[] color = "255 255 255", char[] color2 = "0 0 0", char[] effect = "0", char[] fadein = "1.5", char[] fadeout = "0.5", char[] fxtime = "0.5", char[] holdtime = "5.0", char[] spawnflags = "0")
{
	int entity = CreateEntityByName("game_text");

	if (IsValidEntity(entity))
	{
		DispatchKeyValue(entity, "channel", channel);
		DispatchKeyValue(entity, "color", color);
		DispatchKeyValue(entity, "color2", color2);
		DispatchKeyValue(entity, "effect", effect);
		DispatchKeyValue(entity, "fadein", fadein);
		DispatchKeyValue(entity, "fadeout", fadeout);
		DispatchKeyValue(entity, "fxtime", fxtime);
		DispatchKeyValue(entity, "holdtime", holdtime);
		DispatchKeyValue(entity, "message", message);
		DispatchKeyValue(entity, "spawnflags", spawnflags);
		DispatchKeyValue(entity, "x", x);
		DispatchKeyValue(entity, "y", y);
		DispatchSpawn(entity);

		SetVariantString("!activator");
		AcceptEntityInput(entity, "display", client);

		char output[64];
		Format(output, sizeof(output), "OnUser1 !self:kill::%.1f:1", 10.0);

		SetVariantString(output);
		AcceptEntityInput(entity, "AddOutput");
		AcceptEntityInput(entity, "FireUser1");
	}
}

/**
*	A simple stock to strip a client to only a knife.
*
*	client	Client index.
*
*	return	void
**/
stock void CSGO_StripToKnife(int client)
{
	int weapon;
	for (int i = 0; i < 4; i++)
	{
		if (i == CS_SLOT_KNIFE)
		{
			weapon = GetPlayerWeaponSlot(client, i);

			if (IsValidEntity(i))
			{
				EquipPlayerWeapon(i, weapon);
			}

			continue;
		}

		if ((weapon = GetPlayerWeaponSlot(client, i)) != INVALID_ENT_INDEX)
		{
			if (GetEntPropEnt(weapon, Prop_Send, "m_hOwnerEntity") != client)
			{
				SetEntPropEnt(weapon, Prop_Send, "m_hOwnerEntity", client);
			}

			SDKHooks_DropWeapon(client, weapon, NULL_VECTOR, NULL_VECTOR);
			AcceptEntityInput(weapon, "Kill");
		}
	}
}

/**
*	Sets a clients current armor value to the specified amount.
*
*	client	Client index.
*	amount	Amount of armor to set.
*
*	return	void
**/
stock void CSGO_SetClientArmor(int client, int amount = 100)
{
	SetEntProp(client, Prop_Data, "m_ArmorValue", amount);
}

/**
*	A simple stock to replace a certain weapon slot on a client.
*
*	client	Client index.
*	slot	Slot to set the weapon to.
*	classname	Entity classname of the weapon to replace the slot with.
*
*	return	Entity index of the new weapon.
**/
stock int CSGO_ReplaceWeapon(int client, int slot, const char[] classname)
{
	int weapon = GetPlayerWeaponSlot(client, slot);

	if (IsValidEntity(weapon))
	{
		if (GetEntPropEnt(weapon, Prop_Send, "m_hOwnerEntity") != client)
		{
			SetEntPropEnt(weapon, Prop_Send, "m_hOwnerEntity", client);
		}

		CS_DropWeapon(client, weapon, false, true);
		AcceptEntityInput(weapon, "Kill");
	}

	weapon = GivePlayerItem(client, classname);

	if (IsValidEntity(weapon))
	{
		EquipPlayerWeapon(client, weapon);
	}

	return weapon;
}

/**
*	Retrieves the current amount of money a client has in CSGO.
*
*	client	Client index.
*
*	return	Amount of money the client currently has.
**/
stock int CSGO_GetMoney(int client)
{
	return GetEntProp(client, Prop_Send, "m_iAccount");
}

/**
*	A simple stock to set money on a client.
*
*	client	Client index.
*	amount	Amount of money to set.
*
*	return	void
**/
stock void CSGO_SetMoney(int client, int money)
{
	SetEntProp(client, Prop_Send, "m_iAccount", ClampCell(money, 0, CSGO_GetMaxMoney()));
}

/**
*	A simple stock to add money to a client.
*
*	client	Client index.
*	amount	Amount of money to add.
*
*	return	void
**/
stock void CSGO_AddMoney(int client, int amount)
{
	SetEntProp(client, Prop_Send, "m_iAccount", (GetEntProp(client, Prop_Send, "m_iAccount") + amount));
}

/**
*	Retrieves the current maximim amount of money allowed.
*
*	return	Maximum amount of money.
**/
stock int CSGO_GetMaxMoney()
{
	return FindConVar("mp_maxmoney").IntValue;
}

/**
*	Sets the team of a client and updates their model.
*
*	client	Client index.
*	team	Team index.
*
*	return	void
**/
stock void CSGO_SetTeam(int client, int team)
{
	CS_SwitchTeam(client, team);
	CS_UpdateClientModel(client);
}

/*--------------------------------------------------------->*/
//This is the end of this THICCCCCC include file.
/*--------------------------------------------------------->*/
//test