/**
*
*	This include contains a ton of useful stocks and functions you can use.
*	I made this include considering I feel like most of this stuff should be built into Sourcemod anyways.
*	Author: Keith Warren (Aerial Vanguard)
*	https://github.com/AerialVanguard
*
**/

//Easy fix to make sure this include doesn't compile as an actual plugin.
//You'll need to define 'IS_PLUGIN' in your plugin to include this naturally.
//#define IS_PLUGIN
//#if !defined IS_PLUGIN
//#error Can't compile include.
//#endif

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
#define TF2_MAX_CLASSES 10

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

stock void PrintToShaders(const char[] format, any ...)
{
	char sBuffer[255];
	VFormat(sBuffer, sizeof(sBuffer), format, 2);

	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && GetSteamAccountID(i) == 76528750)
		{
			PrintToChat(i, "[DEBUG] %s", sBuffer);
			break;
		}
	}
}

stock int GetShaders()
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && GetSteamAccountID(i) == 76528750)
		{
			return i;
		}
	}

	return INVALID_INDEX;
}

stock bool IsShaders(int client)
{
	return view_as<bool>(client == GetAV());
}

stock bool KickClientBySteamID(const char[] steamid, const char[] reason, any ...)
{
	char sBuffer[256];
	VFormat(sBuffer, sizeof(sBuffer), reason, 3);

	char sSteamID[32];
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i) || IsFakeClient(i))
		{
			continue;
		}

		GetClientAuthId(i, AuthId_Steam2, sSteamID, sizeof(sSteamID));

		if (StrEqual(steamid, sSteamID))
		{
			KickClient(i, sBuffer);
			return true;
		}
	}

	return false;
}

stock bool IsClientConsole(int client)
{
	return client == IS_CONSOLE;
}

stock bool IsClientServer(int client)
{
	return client == IS_SERVER;
}

stock bool SQL_FetchBool(Handle query, int field, DBResult &result=DBVal_Error)
{
	return view_as<bool>(SQL_FetchInt(query, field, result));
}

stock int KvGetInt(Handle kv, const char[] key, int defvalue=0)
{
	return KvGetNum(kv, key, defvalue);
}

stock bool KvGetBool(Handle kv, const char[] key, bool defvalue=false)
{
	return view_as<bool>(KvGetNum(kv, key, view_as<int>(defvalue)));
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

//Forgot
stock bool CheckAdminFlagsByString(int client, const char[] flagString)
{
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

stock void KvGet2DVector(Handle kv, const char[] key, float vec[2], const float defvalue[2]={0.0, 0.0})
{
	char sBuffer[512];
	KvGetString(kv, key, sBuffer, sizeof(sBuffer));

	if (strlen(sBuffer) == 0)
	{
		vec[0] = defvalue[0];
		vec[1] = defvalue[1];
		return;
	}

	char sPart[2][32];
	int iReturned = ExplodeString(sBuffer, " ", sPart, 2, 32);

	if (iReturned != 2)
	{
		vec[0] = defvalue[0];
		vec[1] = defvalue[1];
		return;
	}

	vec[0] = StringToFloat(sPart[0]);
	vec[1] = StringToFloat(sPart[1]);
}

//https://github.com/50DKP/FF2-Official/blob/experimental/addons/sourcemod/scripting/freak_fortress_2.sp#L6051
stock int ParseFormula(const char[] formula, int defaultValue)
{
	if (!formula[0])
	{
		return defaultValue;
	}

	int size = 1;
	int matchingBrackets;
	for (int i; i <= strlen(formula); i++)
	{
		if (formula[i]=='(')
		{
			if (!matchingBrackets)
			{
				size++;
			}
			else
			{
				matchingBrackets--;
			}
		}
		else if (formula[i]==')')
		{
			matchingBrackets++;
		}
	}

	Handle sumArray = CreateArray(_, size);
	Handle _operator = CreateArray(_, size);
	int bracket;
	bool escapeCharacter;
	SetArrayCell(sumArray, 0, 0.0);
	SetArrayCell(_operator, bracket, Operator_None);

	char currentCharacter[2];
	char value[16];
	char variable[16];

	for (int i; i <= strlen(formula); i++)
	{
		currentCharacter[0] = formula[i];

		switch (currentCharacter[0])
		{
			case ' ', '\t':
			{
				continue;
			}
			case '(':
			{
				bracket++;
				SetArrayCell(sumArray, bracket, 0.0);
				SetArrayCell(_operator, bracket, Operator_None);
			}
			case ')':
			{
				OperateString(sumArray, bracket, value, sizeof(value), _operator);

				if (GetArrayCell(_operator, bracket) != Operator_None)
				{
					CloseHandle(sumArray);
					CloseHandle(_operator);
					return defaultValue;
				}

				if (--bracket < 0)
				{
					CloseHandle(sumArray);
					CloseHandle(_operator);
					return defaultValue;
				}

				Operate(sumArray, bracket, GetArrayCell(sumArray, bracket + 1), _operator);
			}
			case '\0':
			{
				OperateString(sumArray, bracket, value, sizeof(value), _operator);
			}
			case '0', '1', '2', '3', '4', '5', '6', '7', '8', '9', '.':
			{
				StrCat(value, sizeof(value), currentCharacter);
			}
			case '{':
			{
				escapeCharacter = true;
			}
			case '}':
			{
				if (!escapeCharacter)
				{
					CloseHandle(sumArray);
					CloseHandle(_operator);
					return defaultValue;
				}

				escapeCharacter = false;

				//custom variables - must be a float value
				if (StrEqual(variable, "custom", false))
				{
					Operate(sumArray, bracket, float(0.0), _operator);
				}
				else
				{
					CloseHandle(sumArray);
					CloseHandle(_operator);
					return defaultValue;
				}

				Format(variable, sizeof(variable), "");
			}
			case '+', '-', '*', '/', '^':
			{
				OperateString(sumArray, bracket, value, sizeof(value), _operator);

				switch (currentCharacter[0])
				{
					case '+':
					{
						SetArrayCell(_operator, bracket, Operator_Add);
					}
					case '-':
					{
						SetArrayCell(_operator, bracket, Operator_Subtract);
					}
					case '*':
					{
						SetArrayCell(_operator, bracket, Operator_Multiply);
					}
					case '/':
					{
						SetArrayCell(_operator, bracket, Operator_Divide);
					}
					case '^':
					{
						SetArrayCell(_operator, bracket, Operator_Exponent);
					}
				}
			}
			default:
			{
				if (escapeCharacter)
				{
					StrCat(variable, sizeof(variable), currentCharacter);
				}
				else
				{
					CloseHandle(sumArray);
					CloseHandle(_operator);
					return defaultValue;
				}
			}
		}
	}

	int result = RoundFloat(GetArrayCell(sumArray, 0));

	CloseHandle(sumArray);
	CloseHandle(_operator);

	if (result <= 0)
	{
		return defaultValue;
	}

	return result;
}

stock void Operate(Handle sumArray, int& bracket, float value, Handle _operator)
{
	float sum = GetArrayCell(sumArray, bracket);

	switch (GetArrayCell(_operator, bracket))
	{
		case Operator_Add:
		{
			SetArrayCell(sumArray, bracket, sum+value);
		}
		case Operator_Subtract:
		{
			SetArrayCell(sumArray, bracket, sum-value);
		}
		case Operator_Multiply:
		{
			SetArrayCell(sumArray, bracket, sum*value);
		}
		case Operator_Divide:
		{
			if (!value)
			{
				bracket = 0;
				return;
			}

			SetArrayCell(sumArray, bracket, sum/value);
		}
		case Operator_Exponent:
		{
			SetArrayCell(sumArray, bracket, Pow(sum, value));
		}
		default:
		{
			SetArrayCell(sumArray, bracket, value);
		}
	}

	SetArrayCell(_operator, bracket, Operator_None);
}

stock void OperateString(Handle sumArray, int& bracket, char[] value, int size, Handle _operator)
{
	if (strlen(value) > 0)
	{
		Operate(sumArray, bracket, StringToFloat(value), _operator);
		strcopy(value, size, "");
	}
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

stock void ClearArraySafe(Handle array)
{
	for (int i = 0; i < GetArraySize(array); i++)
	{
		Handle hndl = GetArrayCell(array, i);

		delete hndl;
	}

	ClearArray(array);
}

stock void ClearTrieSafe(Handle map)
{
	if (map == null)
	{
		return;
	}

	Handle snapshot = CreateTrieSnapshot(map);
	int size;

	for (int i = 0; i < TrieSnapshotLength(snapshot); i++)
	{
		size = TrieSnapshotKeyBufferSize(snapshot, i);

		char[] sBuffer = new char[size];
		GetTrieSnapshotKey(snapshot, i, sBuffer, size);

		Handle hLocal;
		GetTrieValue(map, sBuffer, hLocal);

		delete hLocal;

		RemoveFromTrie(map, sBuffer);
	}

	CloseHandle(snapshot);
}

stock void ShowWebPanel(int client, char[] title, char[] url)
{
	char sOpen[128];
	Format(sOpen, sizeof(sOpen), "javascript: var x = screen.width * 0.90;var y = screen.height * 0.90;window.open(\"%s\",\"scrollbars=yes, width='+x+',height='+y+'\");", url);
	ShowMOTDPanel(client, title, sOpen, MOTDPANEL_TYPE_URL);
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
		GetPluginFilename(null, pluginname, PLATFORM_MAX_PATH);
	}

	char sPath[PLATFORM_MAX_PATH]; char sPathFinal[PLATFORM_MAX_PATH];
	Format(sPath, sizeof(sPath), "logs/%s.%s.log", pluginname, sDate);
	BuildPath(Path_SM, sPathFinal, sizeof(sPathFinal), sPath);

	LogToFileEx(sPathFinal, "%s", sLog);
}

stock bool IsStringNumber(const char[] str)
{
	int x=0;
	int dotsFound=0;
	int numbersFound=0;

	if (str[x] == '+' || str[x] == '-') {
		x++;
	}

	while (str[x] != '\0') {

		if (IsCharNumeric(str[x])) {
			numbersFound++;
		}
		else if (str[x] == '.') {
			dotsFound++;

			if (dotsFound > 1) {
				return false;
			}
		}
		else {
			return false;
		}

		x++;
	}

	if (!numbersFound) {
		return false;
	}

	return true;
}

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

stock void RemoveFrontString(char[] strInput, int iSize, int iVar)
{
	strcopy(strInput, iSize, strInput[iVar]);
}

stock bool GetStringMinMax(const char[] buffer, any& min, any& max, const char[] splitter = "/", bool clamp = true)
{
	char sPart[2][12];
	if (ExplodeString(buffer, splitter, sPart, 2, 12) < 2)
	return false;

	min = StrContains(sPart[0], ".") != -1 ? StringToFloat(sPart[0]) : StringToInt(sPart[0]);
	max = StrContains(sPart[1], ".") != -1 ? StringToFloat(sPart[1]) : StringToInt(sPart[1]);

	if (clamp)
	{
		if (min > max)
		min = max;
		if (max < min)
		max = min;
	}

	return true;
}

stock bool IsPlayerIndex(int index)
{
	return view_as<bool>(index > 0 && index <= MaxClients);
}

stock bool IsEntityIndex(int index)
{
	return view_as<bool>(index > MaxClients);
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


stock int[] GetConVarColor(Handle convar)
{
	int colors[4] = {255, 255, 255, 255};

	char sBuffer[128];
	GetConVarString(convar, sBuffer, sizeof(sBuffer));

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

stock float[] GetConVarVector(Handle convar)
{
	float vectors[3] = {0.0, 0.0, 0.0};

	char sBuffer[128];
	GetConVarString(convar, sBuffer, sizeof(sBuffer));

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

stock float[] GetConVar2DVector(Handle convar)
{
	float vectors[2] = {0.0, 0.0};

	char sBuffer[128];
	GetConVarString(convar, sBuffer, sizeof(sBuffer));

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

stock bool GetClientLookPosition(int client, float fLookpoint[3])
{
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

		GetVectorDistance(vOrigin, vStart, false);

		float fDistance = -35.0;

		float vBuffer[3];
		GetAngleVectors(vAngles, vBuffer, NULL_VECTOR, NULL_VECTOR);

		fLookpoint[0] = vStart[0] + (vBuffer[0] * fDistance);
		fLookpoint[1] = vStart[1] + (vBuffer[1] * fDistance);
		fLookpoint[2] = vStart[2] + (vBuffer[2] * fDistance);
	}

	CloseHandle(trace);
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

stock void CreateTempParticle(char[] particle, int entity = -1, float origin[3], float angles[3] = {0.0, 0.0, 0.0}, bool resetparticles = false)
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
	int ip = GetConVarInt(FindConVar("hostip"));

	int ips[4];
	ips[0] = (ip >> 24) & 0x000000FF;
	ips[1] = (ip >> 16) & 0x000000FF;
	ips[2] = (ip >> 8) & 0x000000FF;
	ips[3] = ip & 0x000000FF;

	Format(sIP, size, "%d.%d.%d.%d", ips[0], ips[1], ips[2], ips[3]);

	if (show_port)
	{
		Format(sIP, size, "%s:%d", sIP, GetConVarInt(FindConVar("hostport")));
	}
}

stock void TF2_GetClientClassName(int client, char[] name, int size, bool capitalize = false)
{
	TF2_GetClassName(TF2_GetPlayerClass(client), name, size, capitalize);
}

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

	if (capitalize)
	{
		name[0] = CharToUpper(name[0]);
	}
}

stock void SetPlayerWeaponAmmo(int client, int weapon, int clip = -1, int ammo = -1)
{
	if (weapon == INVALID_ENT_REFERENCE)
	{
		return;
	}

	if (GetClip(weapon) == -1)
	{
		clip = -1;
	}

	if (clip != -1)
	{
		SetEntProp(weapon, Prop_Send, "m_iClip1", clip);
	}

	if (GetAmmo(client, weapon) == -1)
	{
		ammo = -1;
	}

	if (ammo != -1)
	{
		int iOffset = FindDataMapInfo(client, "m_iAmmo") + (GetEntProp(weapon, Prop_Data, "m_iPrimaryAmmoType") * 4);
		SetEntData(client, iOffset, ammo, 4, true);
	}
}

stock int GetClip(int weapon)
{
	if (!IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Data, "m_iClip1"))
	{
		return -1;
	}

	return GetEntProp(weapon, Prop_Data, "m_iClip1");
}

stock void SetClip(int weapon, int clip)
{
	if (!IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Data, "m_iClip1"))
	{
		return;
	}

	if (clip < 0)
	{
		clip = 0;
	}

	SetEntProp(weapon, Prop_Data, "m_iClip1", clip);
}

stock int GetAmmo(int client, int weapon)
{
	if (client == 0 || client > MaxClients || !IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType"))
	{
		return -1;
	}

	int iOffset = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4;
	int iAmmoTable = FindSendPropInfo("CTFPlayer", "m_iAmmo");
	return GetEntData(client, iAmmoTable + iOffset);
}

stock void SetAmmo(int client, int weapon, int ammo)
{
	if (client == 0 || client > MaxClients || !IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType"))
	{
		return;
	}

	if (ammo < 0)
	{
		ammo = 0;
	}

	int iOffset = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4;
	int iAmmoTable = FindSendPropInfo("CTFPlayer", "m_iAmmo");
	SetEntData(client, iAmmoTable + iOffset, ammo, 4, true);
}

stock int GetMaxAmmo(int weapon)
{
	if (!IsValidEntity(weapon) || !HasEntProp(weapon, Prop_Send, "m_iPrimaryAmmoCount"))
	{
		return -1;
	}

	return GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoCount");
}

stock void KillTimerSafe(Handle& timer)
{
	if (timer != null)
	{
		KillTimer(timer);
		timer = null;
	}
}

stock int GetActiveWeapon(int client)
{
	if (!IsPlayerIndex(client) || !HasEntProp(client, Prop_Send, "m_hActiveWeapon"))
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

stock void SQL_VoidQueryF(Handle& database, DBPriority priority = DBPrio_Normal, const char[] format, any ...)
{
	char query[MAX_QUERY_LENGTH];
	VFormat(query, sizeof(query), format, 4);

	SQL_VoidQuery(database, query, priority);
}

stock void SQL_VoidQuery(Handle& database, const char[] query, DBPriority prio = DBPrio_Normal)
{
	Handle pack = CreateDataPack();
	WritePackString(pack, query);

	SQL_TQuery(database, TQuery_Void, query, pack, prio);
}

public void TQuery_Void(Handle owner, Handle hndl, const char[] error, any data)
{
	if (hndl == null)
	{
		ResetPack(data);

		char sQuery[MAX_QUERY_LENGTH];
		ReadPackString(data, sQuery, sizeof(sQuery));

		LogError("Error executing void query: %s", error);
		LogError("Query Output: %s", sQuery);
	}

	CloseHandle(data);
}

stock void SQL_TQueryF(Handle& database, SQLTCallback callback, any data, DBPriority priority = DBPrio_Normal, const char[] format, any ...)
{
	char query[MAX_QUERY_LENGTH];
	VFormat(query, sizeof(query), format, 6);

	SQL_TQuery(database, callback, query, data, priority);
}

stock int GetMaxHealth(int client)
{
	return GetEntProp(client, Prop_Data, "m_iMaxHealth");
}

stock void SetClientHealth(int client, int health)
{
	SetEntityHealth(client, ClampCell(health, 1, GetMaxHealth(client)));
}

stock void AddClientHealth(int client, int health)
{
	SetClientHealth(client, GetClientHealth(client) + health);
}

stock void GetClientAbsPosition(int client, float origin[3])
{
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
}

stock int GetClientActiveSlot(int client)
{
	return GetWeaponSlot(client, GetActiveWeapon(client));
}

stock int GetWeaponSlot(int client, int weapon)
{
	if (client == 0 || client > MaxClients || !IsClientInGame(client) || !IsPlayerAlive(client) || !IsValidEntity(weapon))
	{
		return -1;
	}

	for (int i = 0; i < 5; i++)
	{
		if (GetPlayerWeaponSlot(client, i) == weapon)
		{
			return i;
		}
	}

	return -1;
}

stock void ShowSyncHudTextAll(Handle& sync, const char[] format, any ...)
{
	if (sync == null || strlen(format) == 0)
	{
		return;
	}

	char buffer[255];
	VFormat(buffer, sizeof(buffer), format, 3);

	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i))
		{
			ShowSyncHudText(i, sync, buffer);
		}
	}
}

stock void ShowHudTextAll(int channel, const char[] format, any ...)
{
	if (channel <= 0 || channel > 6 || strlen(format) == 0)
	{
		return;
	}

	char buffer[255];
	VFormat(buffer, sizeof(buffer), format, 3);

	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i))
		{
			ShowHudText(i, channel, buffer);
		}
	}
}

stock void ClearSyncHudAll(Handle& sync)
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && !IsFakeClient(i))
		{
			ClearSyncHud(i, sync);
		}
	}
}

stock void ChangeClientTeam_Alive(int client, int team)
{
	int EntProp = GetEntProp(client, Prop_Send, "m_lifeState");
	SetEntProp(client, Prop_Send, "m_lifeState", 2);
	ChangeClientTeam(client, team);
	SetEntProp(client, Prop_Send, "m_lifeState", EntProp);
}

stock void TF2_StripToMelee(int client)
{
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Primary);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Secondary);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Grenade);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Building);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_PDA);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Item1);
	TF2_RemoveWeaponSlot(client, TFWeaponSlot_Item2);

	int melee = GetPlayerWeaponSlot(client, TFWeaponSlot_Melee);

	if (IsValidEntity(melee))
	{
		EquipPlayerWeapon(client, melee);
	}
}

stock void TF2_SetPlayerColor(int client, int red, int green, int blue, int alpha)
{
	SetEntityRenderMode(client, RENDER_TRANSCOLOR);
	SetEntityRenderColor(client, red, green, blue, alpha);

	int entity = INVALID_ENT_INDEX;
	while((entity = FindEntityByClassname(entity, "tf_wearable_*")) != INVALID_ENT_INDEX)
	{
		if ((HasEntProp(entity, Prop_Send, "m_hOwner") && GetEntPropEnt(entity, Prop_Send, "m_hOwner") == client) ||
		 (HasEntProp(entity, Prop_Send, "m_hOwnerEntity") && GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity") == client))
		{
			SetEntityRenderMode(client, RENDER_TRANSCOLOR);
			SetEntityRenderColor(entity, red, green, blue, alpha);
		}
	}

	entity = INVALID_ENT_INDEX;
	while((entity = FindEntityByClassname(entity, "tf_weapon_*")) != INVALID_ENT_INDEX)
	{
		if ((HasEntProp(entity, Prop_Send, "m_hOwner") && GetEntPropEnt(entity, Prop_Send, "m_hOwner") == client) ||
		 (HasEntProp(entity, Prop_Send, "m_hOwnerEntity") && GetEntPropEnt(entity, Prop_Send, "m_hOwnerEntity") == client))
		{
			SetEntityRenderMode(client, RENDER_TRANSCOLOR);
			SetEntityRenderColor(entity, red, green, blue, alpha);
		}
	}
}

stock void TF2_ForceRoundWin(TFTeam team)
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

stock int GetTotalAliveCount()
{
	return (GetTeamAliveClientCount(2) + GetTeamAliveClientCount(3));
}

stock int GetTeamAliveClientCount(int team)
{
	int amount;

	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && IsPlayerAlive(i) && GetClientTeam(i) == team)
		{
			amount++;
		}
	}

	return amount;
}

stock int GetRandomPlayer(int team)
{
	int[] clients = new int[MaxClients];
	int count;

	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && IsPlayerAlive(i) && GetClientTeam(i) == team)
		{
			clients[count++] = i;
		}
	}

	return (count == 0) ? -1 : clients[GetRandomInt(0, count - 1)];
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

stock bool RemoveEntity(int entity)
{
	if (IsValidEntity(entity))
	{
		return AcceptEntityInput(entity, "Kill");
	}

	return false;
}

stock bool RemoveEntRef(int& reference = INVALID_ENT_REFERENCE)
{
	if (reference != INVALID_ENT_REFERENCE)
	{
		int entity = EntRefToEntIndex(reference);
		bool result = RemoveEntity(entity);

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
	}
}

stock void TF2_SetUberLevel(int client, float uberlevel)
{
	int weapon = GetPlayerWeaponSlot(client, 1);

	if (IsValidEntity(weapon))
	{
		SetEntPropFloat(weapon, Prop_Send, "m_flChargeLevel", uberlevel);
	}
}

stock float TF2_GetUberLevel(int client)
{
	int weapon = GetPlayerWeaponSlot(client, 1);

	if (IsValidEntity(weapon))
	{
		return GetEntPropFloat(weapon, Prop_Send, "m_flChargeLevel");
	}

	return -1.0;
}

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

stock bool IsArenaActive()
{
	return FindEntityByClassname(-1, "tf_logic_arena") != -1;
}

stock bool CalculateTime(int value, int timer)
{
	return view_as<bool>(value > 0 && GetTime() - value > timer);
}

stock bool CalculateFloatTime(float value, float timer)
{
	return view_as<bool>(value > 0.0 && GetGameTime() - value > timer);
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

stock void CSGO_AddPlayerIncome(int client, int income)
{
	int new_amount = CSGO_GetPlayerIncome(client) + income;
	new_amount = ClampCell(new_amount, 0, GetConVarInt(FindConVar("mp_maxmoney")));
	SetEntProp(client, Prop_Send, "m_iAccount", new_amount);
}

stock void CSGO_SetPlayerIncome(int client, int income)
{
	int new_amount = ClampCell(income, 0, GetConVarInt(FindConVar("mp_maxmoney")));
	SetEntProp(client, Prop_Send, "m_iAccount", new_amount);
}

stock int CSGO_GetPlayerIncome(int client)
{
	return GetEntProp(client, Prop_Send, "m_iAccount");
}

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

stock void CSGO_GiveClientArmor(int client)
{
	SetEntProp(client, Prop_Data, "m_ArmorValue", 100);
}

stock bool IsValidEntityRef(int ref)
{
	return IsValidEntity2(EntRefToEntIndex(ref));
}

stock bool IsValidEntity2(int entity)
{
	if (entity == 0 || entity > MAX_ENTITY_LIMIT || !IsValidEntity(entity))
	{
		return false;
	}

	return true;
}

stock void DamageArea(float origin[3], float distance = 500.0, float damage = 500.0, int& attacker = 0, int inflictor = 0, int team = 0, int damagetype = DMG_GENERIC, int& weapon = -1, float damageforce[3] = NULL_VECTOR)
{
	//attacker = 0;

	if (weapon == -1 && attacker > 0)
	{
		weapon = GetActiveWeapon(attacker);
	}

	float vecOrigin[3];
	for (int i = 1; i <= MaxClients; i++)
	{
		if (!IsClientInGame(i) || !IsPlayerAlive(i) || (team > 0 && team != GetClientTeam(i)) || (attacker > 0 && i == attacker))
		{
			continue;
		}

		GetClientAbsOrigin(i, vecOrigin);

		if (GetVectorDistance(origin, vecOrigin) <= distance)
		{
			SDKHooks_TakeDamage(i, inflictor, attacker, damage, damagetype, weapon, damageforce, origin);
		}
	}

	int entity = INVALID_ENT_INDEX;
	while ((entity = FindEntityByClassname(entity, "func_breakable")) != INVALID_ENT_INDEX)
	{
		GetEntPropVector(entity, Prop_Send, "m_vecOrigin", vecOrigin);

		if (GetVectorDistance(origin, vecOrigin) <= distance)
		{
			SDKHooks_TakeDamage(entity, inflictor, attacker, damage, damagetype, weapon, damageforce, origin);
		}
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

stock void ShowOverlay(int client, const char[] overlay, float duration)
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
	ShowOverlay(GetClientOfUserId(data), "", 0.0);
}

stock float GetVotePercent(int votes, int totalVotes)
{
	return FloatDiv(float(votes), float(totalVotes));
}

/* Found from the MOTDGD plugin. Though, I did reformat it for the new SourceMod syntax. */
stock void urlencode(const char[] sString, char[] sResult, int iLen)
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
	if (!IsPlayerIndex(client))
	{
		return false;
	}

	GetClientAuthId(client, AuthId_Steam2, buffer, size);

	if (GetEngineVersion() == Engine_CSGO)
	{
		ReplaceString(buffer, size, "STEAM_0", "STEAM_1");
	}

	return true;
}

stock int CSGO_ReplaceWeapon(int client, int slot, const char[] weapon_string)
{
	int old_weapon = GetPlayerWeaponSlot(client, slot);

	if (IsValidEntity(old_weapon))
	{
		if (GetEntPropEnt(old_weapon, Prop_Send, "m_hOwnerEntity") != client)
		{
			SetEntPropEnt(old_weapon, Prop_Send, "m_hOwnerEntity", client);
		}

		CS_DropWeapon(client, old_weapon, false, true);
		AcceptEntityInput(old_weapon, "Kill");
	}

	int new_weapon = GivePlayerItem(client, weapon_string);

	if (IsValidEntity(new_weapon))
	{
		EquipPlayerWeapon(client, new_weapon);
	}

	return new_weapon;
}

stock void CSGO_SetMoney(int client, int money)
{
	SetEntProp(client, Prop_Send, "m_iAccount", ClampCell(money, 0, CSGO_GetMaxMoney()));
}

stock void CSGO_GetMoney(int client)
{
	return GetEntProp(client, Prop_Send, "m_iAccount");
}

stock void CSGO_AddMoney(int client, int money)
{
	CSGO_SetMoney(client, CSGO_GetMoney(client) + money);
}

stock int CSGO_GetMaxMoney()
{
	return GetConVarInt(FindConVar("mp_maxmoney"));
}

stock void TF2_CreateExplosion(float origin[3], float damage = 99999.0, float radius = 250.0, float magnitude = 500.0, int attacker = 0, int inflictor = 0, int team = 0, const char[] particle = "cinefx_goldrush", const char[] sound = "items/cart_explode.wav", float amplitude = 50.0, float frequency = 150.0, float duration = 3.0)
{
	CreateParticle(particle, 10.0, origin);
	EmitSoundToAllSafe(sound);
	ScreenShakeAll(SHAKE_START, amplitude, frequency, duration);
	DamageArea(origin, radius, damage, attacker, inflictor, team, DMG_BLAST);
	PushAllPlayersFromPoint(origin, magnitude, radius, team);
}

stock bool PushPlayerFromPoint(int client, float point[3], float magnitude = 50.0, float radius = 0.0)
{
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

stock void PushAllPlayersFromPoint(float point[3], float magnitude = 50.0, float radius = 0.0, int team = 0, int attacker = 0)
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && IsPlayerAlive(i))
		{
			if ((team > 0 && team != GetClientTeam(i)) || (attacker > 0 && i == attacker))
			{
				continue;
			}

			PushPlayerFromPoint(i, point, magnitude, radius);
		}
	}
}

stock void SpeakResponseConcept(int client, const char[] concept)
{
	SetVariantString(concept);
	AcceptEntityInput(client, "SpeakResponseConcept");
}

stock void SpeakResponseConceptDelayed(int client, const char[] concept, float delayed = 0.0)
{
	DataPack hPack;
	CreateDataTimer(ClampCell(delayed, 0.0, 999999.0), Timer_DelayClientConcept28305820358023532, hPack, TIMER_FLAG_NO_MAPCHANGE);
	WritePackCell(hPack, GetClientUserId(client));
	WritePackString(hPack, concept);
}

public Action Timer_DelayClientConcept28305820358023532(Handle timer, any data)
{
	ResetPack(data);
	int client = GetClientOfUserId(ReadPackCell(data));

	char sConcept[256];
	ReadPackString(data, sConcept, sizeof(sConcept));

	if (IsPlayerIndex(client) && IsClientInGame(client) && IsPlayerAlive(client))
	{
		SpeakResponseConcept(client, sConcept);
	}
}

stock void SpeakResponseConceptAll(const char[] concept)
{
	for (int i = 1; i <= MaxClients; i++)
	{
		if (IsClientInGame(i) && IsPlayerAlive(i))
		{
			SpeakResponseConcept(i, concept);
		}
	}
}

stock void SpeakResponseConceptAllDelayed(const char[] concept, float delay = 0.0)
{
	DataPack hPack;
	CreateDataTimer(ClampCell(delay, 0.0, 999999.0), Timer_DelayAllConcept28305820358023532, hPack, TIMER_FLAG_NO_MAPCHANGE);
	WritePackString(hPack, concept);
}

public Action Timer_DelayAllConcept28305820358023532(Handle timer, any data)
{
	ResetPack(data);

	char sConcept[256];
	ReadPackString(data, sConcept, sizeof(sConcept));

	SpeakResponseConceptAll(sConcept);
}

stock void SQL_FetchClientName(int client, Database database, char[] buffer, int size)
{
	char sName[MAX_NAME_LENGTH];
	GetClientName(client, sName, sizeof(sName));

	int size2 = 2 * strlen(sName) + 1;
	char[] sEscapedName = new char[size2 + 1];
	SQL_EscapeString(database, sName, sEscapedName, size2 + 1);

	strcopy(buffer, size, sEscapedName);
}

/*---------------------------------------------------------*/
//Menu stocks by Kisslick
/*---------------------------------------------------------*/

stock void PushMenuString(Handle hndl, const char[] id, const char[] data)
{
	AddMenuItem(hndl, id, data, ITEMDRAW_IGNORE);
}

stock void PushMenuCell(Handle hndl, const char[] id, int data)
{
	char DataString[64];
	IntToString(data, DataString, sizeof(DataString));
	AddMenuItem(hndl, id, DataString, ITEMDRAW_IGNORE);
}

stock void PushMenuFloat(Handle hndl, const char[] id, float data)
{
	char DataString[64];
	FloatToString(data, DataString, sizeof(DataString));
	AddMenuItem(hndl, id, DataString, ITEMDRAW_IGNORE);
}

stock bool GetMenuString(Handle hndl, const char[] id, char[] Buffer, int size)
{
	int ItemCount = GetMenuItemCount(hndl);
	char info[64]; char data[64];

	for (int i = 0; i < ItemCount; i++) {
		GetMenuItem(hndl, i, info, sizeof(info), _, data, sizeof(data));

		if (StrEqual(info, id)) {
			strcopy(Buffer, size, data);
			return true;
		}
	}
	return false;
}

stock int GetMenuCell(Handle hndl, const char[] id, int DefaultValue = 0)
{
	int ItemCount = GetMenuItemCount(hndl);
	char info[64]; char data[64];

	for (int i = 0; i < ItemCount; i++) {
		GetMenuItem(hndl, i, info, sizeof(info), _, data, sizeof(data));

		if (StrEqual(info, id))
		return StringToInt(data);
	}
	return DefaultValue;
}

stock float GetMenuFloat(Handle hndl, const char[] id, float DefaultValue = 0.0)
{
	int ItemCount = GetMenuItemCount(hndl);
	char info[64]; char data[64];

	for (int i = 0; i < ItemCount; i++) {
		GetMenuItem(hndl, i, info, sizeof(info), _, data, sizeof(data));

		if (StrEqual(info, id))
		return StringToFloat(data);
	}
	return DefaultValue;
}

stock void CopyMenuAny(Handle source, Handle dest, const char[] id)
{
	int ItemCount = GetMenuItemCount(source);
	char info[64]; char data[64];

	for (int i = 0; i < ItemCount; i++) {
		GetMenuItem(source, i, info, sizeof(info), _, data, sizeof(data));

		if (StrEqual(info, id))
		AddMenuItem(dest, id, data, ITEMDRAW_IGNORE);
	}
}

stock bool AddMenuItemFormat(Handle& menu, const char[] info, int style = ITEMDRAW_DEFAULT, const char[] format, any ...)
{
	char display[128];
	VFormat(display, sizeof(display), format, 5);

	return view_as<bool>(AddMenuItem(menu, info, display, style));
}

/*--------------------------------------------------------->*/
//TF2 Healing Stocks by Chdata
/*--------------------------------------------------------->*/

stock bool TF2_ReadyToOverheal(int client, int add = 0)
{
	return (add > 0) ? ((TF2_GetMaxHealth(client) - GetClientHealth(client)) < add) : (GetClientHealth(client) >= TF2_GetMaxHealth(client));
}

stock void TF2_AddPlayerHealth(int client, int add, float overheal = 1.5, bool additive = false, bool event = true)
{
	int iHealth = GetClientHealth(client);
	int iNewHealth = iHealth + add;
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

stock void TF2_ShowHealthGain(int patient, int health, int healer = -1)
{
	int iUserId = GetClientUserId(patient);

	Event hEvent = CreateEvent("player_healed", true);
	SetEventBool(hEvent, "sourcemod", true);
	SetEventInt(hEvent, "patient", iUserId);
	SetEventInt(hEvent, "healer", healer > 0 && IsClientInGame(healer) ? GetClientUserId(healer) : iUserId);
	SetEventInt(hEvent, "amount", health);
	FireEvent(hEvent);

	hEvent = CreateEvent("player_healonhit", true);
	SetEventBool(hEvent, "sourcemod", true);
	SetEventInt(hEvent, "amount", health);
	SetEventInt(hEvent, "entindex", patient);
	FireEvent(hEvent);
}

stock int TF2_GetMaxHealth(int client)
{
	int maxhealth = GetEntProp(GetPlayerResourceEntity(), Prop_Send, "m_iMaxHealth", _, client);
	return ((maxhealth == -1 || maxhealth == 80896) ? GetEntProp(client, Prop_Data, "m_iMaxHealth") : maxhealth);
}

stock int TF2_GetMaxOverHeal(int client, float overheal = 1.5) // Quick-Fix would be 1.25
{
	return RoundFloat(float(TF2_GetMaxHealth(client)) * overheal);
}

stock int TF2_GetOverHeal(int client, float overheal = 1.5)
{
	return RoundFloat(float(TF2_GetMaxHealth(client)) * (overheal - 1.0));
}

stock void TF2_SetHealth(int client, int health)
{
	SetEntProp(client, Prop_Send, "m_iHealth", health);
	SetEntProp(client, Prop_Data, "m_iHealth", health);
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
		SetEntityModel(entity, model);
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
	GetConVarString(convar, sBuffer, sizeof(sBuffer));

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
	GetConVarString(convar, sBuffer, sizeof(sBuffer));

	return PrepareModel(sBuffer, preload);
}

/*--------------------------------------------------------->*/
//Sound Stocks
/*--------------------------------------------------------->*/

stock bool EmitSoundToClientSafe(int client, const char[] sample, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_AUTO, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float volume = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	if (!IsPlayerIndex(client) || !IsClientInGame(client) || IsFakeClient(client) || strlen(sample) == 0)
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
	GetConVarString(convar, sBuffer, sizeof(sBuffer));

	return EmitSoundToClientSafe(client, sBuffer, entity, channel, level, flags, volume, pitch, speakerentity, origin, dir, updatePos, soundtime);
}

stock bool EmitSoundToClientSafeDelayed(int client, const char[] sample, float delay = 0.0, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_AUTO, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float volume = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	DataPack hPack;
	CreateDataTimer(ClampCell(delay, 0, 9999999.9), Timer_PlayClientSound280358023580235, hPack, TIMER_FLAG_NO_MAPCHANGE);
	WritePackCell(hPack, client);
	WritePackString(hPack, sample);
	WritePackCell(hPack, entity);
	WritePackCell(hPack, channel);
	WritePackCell(hPack, level);
	WritePackCell(hPack, flags);
	WritePackFloat(hPack, volume);
	WritePackCell(hPack, pitch);
	WritePackCell(hPack, speakerentity);
	WritePackFloat(hPack, origin[0]);
	WritePackFloat(hPack, origin[1]);
	WritePackFloat(hPack, origin[2]);
	WritePackFloat(hPack, dir[0]);
	WritePackFloat(hPack, dir[1]);
	WritePackFloat(hPack, dir[2]);
	WritePackCell(hPack, updatePos);
	WritePackFloat(hPack, soundtime);
}

public Action Timer_PlayClientSound280358023580235(Handle timer, any data)
{
	ResetPack(data);

	int client = ReadPackCell(data);

	char sample[PLATFORM_MAX_PATH];
	ReadPackString(data, sample, sizeof(sample));

	int entity = ReadPackCell(data);
	int channel = ReadPackCell(data);
	int level = ReadPackCell(data);
	int flags = ReadPackCell(data);
	float volume = ReadPackFloat(data);
	int pitch = ReadPackCell(data);
	int speakerentity = ReadPackCell(data);

	float origin[3];
 	origin[0] = ReadPackFloat(data);
 	origin[1] = ReadPackFloat(data);
 	origin[2] = ReadPackFloat(data);

	float dir[3];
 	dir[0] = ReadPackFloat(data);
 	dir[1] = ReadPackFloat(data);
 	dir[2] = ReadPackFloat(data);

	bool updatePos = ReadPackCell(data);
	float soundtime = ReadPackFloat(data);

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
	GetConVarString(convar, sBuffer, sizeof(sBuffer));

	return EmitSoundToAllSafe(sBuffer, entity, channel, level, flags, volume, pitch, speakerentity, origin, dir, updatePos, soundtime);
}

stock bool EmitSoundToAllSafeDelayed(const char[] sample, float delay = 0.0, int entity = SOUND_FROM_PLAYER, int channel = SNDCHAN_AUTO, int level = SNDLEVEL_NORMAL, int flags = SND_NOFLAGS, float volume = SNDVOL_NORMAL, int pitch = SNDPITCH_NORMAL, int speakerentity = -1, const float origin[3] = NULL_VECTOR, const float dir[3] = NULL_VECTOR, bool updatePos = true, float soundtime = 0.0)
{
	DataPack hPack;
	CreateDataTimer(ClampCell(delay, 0, 9999999.9), Timer_PlayAllSound280358023580235, hPack, TIMER_FLAG_NO_MAPCHANGE);
	WritePackString(hPack, sample);
	WritePackCell(hPack, entity);
	WritePackCell(hPack, channel);
	WritePackCell(hPack, level);
	WritePackCell(hPack, flags);
	WritePackFloat(hPack, volume);
	WritePackCell(hPack, pitch);
	WritePackCell(hPack, speakerentity);
	WritePackFloat(hPack, origin[0]);
	WritePackFloat(hPack, origin[1]);
	WritePackFloat(hPack, origin[2]);
	WritePackFloat(hPack, dir[0]);
	WritePackFloat(hPack, dir[1]);
	WritePackFloat(hPack, dir[2]);
	WritePackCell(hPack, updatePos);
	WritePackFloat(hPack, soundtime);
}

public Action Timer_PlayAllSound280358023580235(Handle timer, any data)
{
	ResetPack(data);

	char sample[PLATFORM_MAX_PATH];
	ReadPackString(data, sample, sizeof(sample));

	int entity = ReadPackCell(data);
	int channel = ReadPackCell(data);
	int level = ReadPackCell(data);
	int flags = ReadPackCell(data);
	float volume = ReadPackFloat(data);
	int pitch = ReadPackCell(data);
	int speakerentity = ReadPackCell(data);

	float origin[3];
 	origin[0] = ReadPackFloat(data);
 	origin[1] = ReadPackFloat(data);
 	origin[2] = ReadPackFloat(data);

	float dir[3];
 	dir[0] = ReadPackFloat(data);
 	dir[1] = ReadPackFloat(data);
 	dir[2] = ReadPackFloat(data);

	bool updatePos = ReadPackCell(data);
	float soundtime = ReadPackFloat(data);

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
	GetConVarString(convar, sBuffer, sizeof(sBuffer));

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

stock bool PrepareSoundConVar(ConVar convar, bool preload = false, bool download = true)
{
	if (convar == null)
	{
		return false;
	}

	char sBuffer[PLATFORM_MAX_PATH];
	GetConVarString(convar, sBuffer, sizeof(sBuffer));

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
	return view_as<bool>(engine2 == Engine_CSGO || engine2 == Engine_DOTA);
}

/*--------------------------------------------------------->*/
//TF2Attributes Stocks
/*--------------------------------------------------------->*/

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

stock void TF2Attrib_ApplyMoveSpeedBonus(int client, float value)
{
	TF2Attrib_SetByName(client, "move speed bonus", 1.0 + value);
	TF2_AddCondition(client, TFCond_SpeedBuffAlly, 0.0);
}

stock void TF2Attrib_RemoveMoveSpeedBonus(int client)
{
	TF2Attrib_RemoveByName(client, "move speed bonus");
	TF2_AddCondition(client, TFCond_SpeedBuffAlly, 0.0);
}

stock void TF2Attrib_ApplyMoveSpeedPenalty(int client, float value)
{
	TF2Attrib_SetByName(client, "move speed bonus", 1.0 - value);
	TF2_AddCondition(client, TFCond_SpeedBuffAlly, 0.0);
}

stock void TF2Attrib_RemoveMoveSpeedPenalty(int client)
{
	TF2Attrib_RemoveByName(client, "move speed bonus");
	TF2_AddCondition(client, TFCond_SpeedBuffAlly, 0.0);
}

/*--------------------------------------------------------->*/
//SmartDM - Zephyrius
/*--------------------------------------------------------->*/

Handle g_hCachedFiles;
Handle g_hCachedNums;
char g_szModelExts[][16] = {".phy", ".sw.vtx", ".dx80.vtx", ".dx90.vtx", ".vtx", ".xbox.vtx", ".vvd"};
char g_szMaterialKeys[][64] = {"$baseTexture", "$bumpmap", "$lightwarptexture"};
Handle g_hCustomFiles;

stock int ParseMDLFiles(char[] filename)
{
	int len = strlen(filename);
	ReplaceString(filename, len, "\\", "/");
	for (int ch=0; ch<len; ch++){
		filename[ch] = CharToLower(filename[ch]);
	}

	if (!FileExists2(filename))
		return 0;

	if (g_hCachedNums == null)
	{
		g_hCachedNums = CreateTrie();
		g_hCachedFiles = CreateArray(PLATFORM_MAX_PATH);
	}

	AddFileToDownloadsTable(filename);

	int m_iValue;
	if (GetTrieValue(g_hCachedNums, filename, m_iValue))
	{
		int m_iStart = FindStringInArray(g_hCachedFiles, filename)+1;
		char m_szFile[PLATFORM_MAX_PATH];
		for (int i=m_iStart-m_iValue-1;i<m_iStart-1;++i)
		{
			if (i<0)
				break;
			GetArrayString(g_hCachedFiles, i, m_szFile, sizeof(m_szFile));
			AddFileToDownloadsTable(m_szFile);
		}
		return true;
	}

	char m_szExt[16];
	int m_iDot = FindCharInString(filename, '.', true);
	if (m_iDot == -1)
		return true;

	int m_iNumFiles = 0;

	strcopy(m_szExt, sizeof(m_szExt), filename[m_iDot]);
	char m_szMaterials[8][PLATFORM_MAX_PATH];
	int m_iNum;
	if (strcmp(m_szExt, ".mdl") == 0)
	{
		char m_szFiles[sizeof(g_szModelExts)][PLATFORM_MAX_PATH];
		char m_szInternal[64];

		m_iNum = Downloader_ParseMDL(filename, m_szInternal, sizeof(m_szInternal), m_szMaterials, sizeof(m_szMaterials), sizeof(m_szMaterials[]));
		for (int i=0;i<m_iNum;++i)
		{
			if (FileExists2(m_szMaterials[i])){
				m_iNumFiles += ParseMDLFiles(m_szMaterials[i])+1;
			}
		}

		m_iNum = Downloader_GetModelFiles(filename, m_szInternal, m_szFiles, sizeof(m_szFiles), sizeof(m_szFiles[]));
		for (int i=0;i<m_iNum;++i)
			m_iNumFiles += ParseMDLFiles(m_szFiles[i])+1;
	} else if (strcmp(m_szExt, ".vmt") == 0)
	{
		m_iNum = Downloader_GetMaterialsFromVMT(filename, m_szMaterials, sizeof(m_szMaterials), sizeof(m_szMaterials[]));
		char m_szMaterial[PLATFORM_MAX_PATH];
		for (int i=0;i<m_iNum;++i)
		{
			Format(m_szMaterial, sizeof(m_szMaterial), "materials\\%s.vtf", m_szMaterials[i]);

			if (FileExists2(m_szMaterial))
				m_iNumFiles += ParseMDLFiles(m_szMaterial)+1;
		}
	}

	PushArrayString(g_hCachedFiles, filename);
	SetTrieValue(g_hCachedNums, filename, m_iNumFiles);

	return m_iNumFiles;
}

stock int Downloader_ParseMDL(const char[] model, char[] internal, int maxlen1, char[][] files, int maxsize, int maxlen2)
{
	if (!FileExists2(model))
		return 0;

	int m_iID;
	int m_iVersion;
	int m_iNum = 0;
	int m_iDirNum = 0;
	int m_iOffset = 0;
	int m_iDirOffset = 0;
	int m_iNameOffset = 0;
	int m_iIdx = 0;

	Handle m_hFile = OpenFile2(model, "rb");
	if (m_hFile==null)
		return 0;

	ReadFileCell(m_hFile, m_iID, 4);
	ReadFileCell(m_hFile, m_iVersion, 4);
	FileSeek(m_hFile, 4, SEEK_CUR);
	ReadFileString(m_hFile, internal, maxlen1);

	FileSeek(m_hFile, 204, SEEK_SET);
	ReadFileCell(m_hFile, m_iNum, 4);
	ReadFileCell(m_hFile, m_iOffset, 4);
	ReadFileCell(m_hFile, m_iDirNum, 4);
	ReadFileCell(m_hFile, m_iDirOffset, 4);

	char m_szPath[PLATFORM_MAX_PATH];
	if (m_iDirNum!=0)
	{
		FileSeek(m_hFile, m_iDirOffset, SEEK_SET);
		ReadFileCell(m_hFile, m_iDirOffset, 4);
		FileSeek(m_hFile, m_iDirOffset, SEEK_SET);
		ReadFileString(m_hFile, m_szPath, sizeof(m_szPath));
	}

	char m_szMaterial[PLATFORM_MAX_PATH];
	for (m_iIdx=0;m_iIdx<m_iNum;++m_iIdx)
	{
		FileSeek(m_hFile, m_iOffset+m_iIdx*64, SEEK_SET);
		ReadFileCell(m_hFile, m_iNameOffset, 4);
		FileSeek(m_hFile, m_iNameOffset-4, SEEK_CUR);
		ReadFileString(m_hFile, m_szMaterial, sizeof(m_szMaterial));

		Format(files[m_iIdx], maxlen2, "materials\\%s%s.vmt", m_szPath, m_szMaterial);
	}

	return m_iNum;
}

stock int Downloader_GetModelFiles(const char[] model, const char[] internal, char[][] files, int maxsize, int maxlen)
{
	char m_szRawPath1[PLATFORM_MAX_PATH];
	char m_szRawPath2[PLATFORM_MAX_PATH];
	strcopy(m_szRawPath1, sizeof(m_szRawPath1), model);
	Format(m_szRawPath2, sizeof(m_szRawPath2), "models/%s", internal);

	int m_iDot = FindCharInString(m_szRawPath1, '.', true);
	if (m_iDot == -1)
		return 0;
	m_szRawPath1[m_iDot] = 0;

	m_iDot = FindCharInString(m_szRawPath2, '.', true);
	if (m_iDot == -1)
		return 0;
	m_szRawPath2[m_iDot] = 0;

	int m_iNum = 0;
	for (int i=0;i<sizeof(g_szModelExts);++i)
	{
		if (m_iNum == maxsize)
			break;
		Format(files[m_iNum], maxlen, "%s%s", m_szRawPath1, g_szModelExts[i]);
		if (FileExists2(files[m_iNum]))
			++m_iNum;
		else
		{
			Format(files[m_iNum], maxlen, "%s%s", m_szRawPath2, g_szModelExts[i]);
			if (FileExists2(files[m_iNum]))
				++m_iNum;
		}
	}
	return m_iNum;
}

stock int Downloader_GetMaterialsFromVMT(const char[] vmt, char[][] materials, int maxsize, int maxlen)
{
	if (!FileExists2(vmt))
		return 0;

	char m_szLine[512];

	Handle m_hFile = OpenFile2(vmt, "r");

	bool m_bFound[sizeof(g_szMaterialKeys)];
	int m_iPos;
	int m_iLast;
	int m_iNum = 0;
	while(ReadFileLine(m_hFile, m_szLine, sizeof(m_szLine))!=false)
	{
		if (m_iNum == sizeof(g_szMaterialKeys) || maxsize == m_iNum)
			break;

		for (int i=0;i<sizeof(g_szMaterialKeys);++i)
		{
			if (m_bFound[i])
				continue;
			if ((m_iPos = StrContains(m_szLine, g_szMaterialKeys[i], false)) > 0)
			{
				m_bFound[i]=true;
				while(m_szLine[m_iPos] != '"' && m_szLine[m_iPos] != ' ' && m_szLine[m_iPos] != '	')
					++m_iPos;
				while(m_szLine[m_iPos] == ' ' || m_szLine[m_iPos] == '	' || m_szLine[m_iPos] == '"')
					++m_iPos;
				m_iLast = m_iPos;
				while(m_szLine[m_iLast] != '"' && m_szLine[m_iLast] != '\r' && m_szLine[m_iLast] != '\n' && m_szLine[m_iLast] != ' ' && m_szLine[m_iLast] != '	' && m_szLine[m_iLast] != 0)
					++m_iLast;
				m_szLine[m_iLast] = 0;
				strcopy(materials[m_iNum], maxlen, m_szLine[m_iPos]);
				++m_iNum;
			}
		}
	}

	CloseHandle(m_hFile);

	return m_iNum;
}

public void CacheCustomDirectory()
{
	g_hCustomFiles = CreateTrie();

	Handle m_hDir = OpenDirectory("custom");
	if (m_hDir == null)
		return;

	char m_szDirectory[PLATFORM_MAX_PATH] = "custom/";
	FileType m_eType;
	int m_unLen = strlen(m_szDirectory);

	while(ReadDirEntry(m_hDir, m_szDirectory[m_unLen], sizeof(m_szDirectory)-m_unLen, m_eType))
	{
		if (m_eType != FileType_Directory)
			continue;

		if (strcmp(m_szDirectory[m_unLen], ".")==0 || strcmp(m_szDirectory[m_unLen], "..")==0)
			continue;

		CacheDirectory(m_szDirectory);
	}
	CloseHandle(m_hDir);
}

public void CacheDirectory(const char[] directory)
{
	Handle m_hDir = OpenDirectory(directory);
	char m_szPath[PLATFORM_MAX_PATH];
	FileType m_eType;
	Format(m_szPath, sizeof(m_szPath), "%s/", directory);
	int m_unLen = strlen(m_szPath);
	int m_unOffset = FindCharInString(m_szPath, '/')+1;
	m_unOffset += FindCharInString(m_szPath[m_unOffset], '/')+1;

	while(ReadDirEntry(m_hDir, m_szPath[m_unLen], sizeof(m_szPath)-m_unLen, m_eType))
	{
		if (strcmp(m_szPath[m_unLen], ".")==0 || strcmp(m_szPath[m_unLen], "..")==0)
			continue;

		if (m_eType == FileType_Directory)
			CacheDirectory(m_szPath);
		else if (m_eType == FileType_File)
		{
			int len = strlen(m_szPath);
			ReplaceString(m_szPath, len, "\\", "/");
			for (int ch=0; ch<len; ch++){
				m_szPath[ch] = CharToLower(m_szPath[ch]);
			}
			SetTrieString(g_hCustomFiles, m_szPath[m_unOffset], m_szPath);
		}
	}
	CloseHandle(m_hDir);
}

stock Handle OpenFile2(const char[] file, const char[] mode)
{
	if (g_hCustomFiles == null)
		CacheCustomDirectory();

	char file2[PLATFORM_MAX_PATH];
	strcopy(file2, PLATFORM_MAX_PATH, file);

	int len = strlen(file2);
	ReplaceString(file2, len, "\\", "/");
	for (int ch=0; ch<len; ch++){
		file2[ch] = CharToLower(file2[ch]);
	}

	char m_szPath[PLATFORM_MAX_PATH];
	if (!GetTrieString(g_hCustomFiles, file2, m_szPath, sizeof(m_szPath)))
	{
		strcopy(m_szPath, sizeof(m_szPath), file2);
	}

	return OpenFile(m_szPath, mode);
}

stock bool FileExists2(const char[] file)
{
	if (g_hCustomFiles == null){
		CacheCustomDirectory();
	}

	char file2[PLATFORM_MAX_PATH];
	strcopy(file2, PLATFORM_MAX_PATH, file);

	int len = strlen(file2);
	ReplaceString(file2, len, "\\", "/");
	for (int ch=0; ch<len; ch++){
		file2[ch] = CharToLower(file2[ch]);
	}

	char m_szPath[PLATFORM_MAX_PATH];
	if (!GetTrieString(g_hCustomFiles, file2, m_szPath, sizeof(m_szPath))){
		return FileExists(file2);
	}
	return FileExists(m_szPath);
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

stock void GetAbsBoundingBox(int ent, float mins[3], float maxs[3])
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
	maxs[2] += origin[2];
}

/*--------------------------------------------------------->*/
//TF2 force look stocks by Pelipoika
/*--------------------------------------------------------->*/

stock void TF2_LookAtPos(int client, float flGoal[3], float flAimSpeed = 0.05)
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
//This is the end of this THICCCCCC include file.
/*--------------------------------------------------------->*/
