/**
*
*	This include contains a ton of useful stocks and functions you can use.
*	I made this include considering I feel like most of this stuff should be built into Sourcemod anyways.
*	Author: Keith Warren (Drixevel)
*	http://www.drixevel.com/
*
**/

#if defined _sourcemod_misc_included
#endinput
#endif
#define _sourcemod_misc_included

#include <sdktools>

#undef REQUIRE_EXTENSIONS
#include <tf2>
#define REQUIRE_EXTENSIONS

#define MAX_ENTITY_LIMIT 4096 + 1
#define MAX_QUERY_LENGTH 4096
#define MAX_BUTTONS 25

stock bool IsClientConsole(int client)
{
	return client == 0;
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
	case true: Format(sSeconds, sizeof(sSeconds), "%06.3f", float(t) + seconds - RoundToFloor(seconds));
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
	VFormat(sLog, sizeof(sLog), format, 2);

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

#define NO_ATTACH 0
#define ATTACH_NORMAL 1
#define ATTACH_HEAD 2

stock int CreateParticle(const char[] particle, float time = 0.0, int parent = 0, int attach = NO_ATTACH, float xOffs = 0.0, float yOffs = 0.0, float zOffs = 0.0)
{
	int entity = CreateEntityByName("info_particle_system");

	if (IsValidEntity(entity))
	{
		float pos[3];
		GetEntPropVector(parent, Prop_Send, "m_vecOrigin", pos);

		pos[0] += xOffs;
		pos[1] += yOffs;
		pos[2] += zOffs;

		TeleportEntity(entity, pos, NULL_VECTOR, NULL_VECTOR);
		DispatchKeyValue(entity, "effect_name", particle);

		if (attach != NO_ATTACH)
		{
			SetVariantString("!activator");
			AcceptEntityInput(entity, "SetParent", parent, entity, 0);

			if (attach == ATTACH_HEAD)
			{
				SetVariantString("head");
				AcceptEntityInput(entity, "SetParentAttachmentMaintainOffset", entity, entity, 0);
			}
		}

		DispatchSpawn(entity);
		ActivateEntity(entity);
		AcceptEntityInput(entity, "Start");

		if (time > 0.0)
		{
			CreateTimer(time, DeleteParticle, EntIndexToEntRef(entity));
		}
	}

	return entity;
}

public Action DeleteParticle(Handle timer, any data)
{
	int entity = EntRefToEntIndex(data);

	if (entity > 0 && IsValidEntity(entity))
	{
		char classname[64];
		GetEntityClassname(entity, classname, sizeof(classname));

		if (StrEqual(classname, "info_particle_system"))
		{
			AcceptEntityInput(entity, "Kill");
		}
	}
}

stock void AttachParticleToClient(int client, const char[] particle, const char[] attachment, float life = 0.0, float vOffsets[3] = {0.0, 0.0, 0.0})
{
	float vecPosition[3];
	GetClientAbsOrigin(client, vecPosition);

	int entity = CreateEntityParticle(particle, vecPosition, life);

	if (IsValidEntity(entity))
	{
		SetParent(client, entity, attachment, vOffsets);
	}
}

stock int CreateEntityParticle(const char[] sParticle, const float[3] pos, float life = 0.0)
{
	int entity = CreateEntityByName("info_particle_system");

	if (IsValidEntity(entity))
	{
		TeleportEntity(entity, pos, NULL_VECTOR, NULL_VECTOR);
		DispatchKeyValue(entity, "effect_name", sParticle);
		DispatchSpawn(entity);
		ActivateEntity(entity);
		AcceptEntityInput(entity, "start");

		if (life > 0.0)
		{
			SetEntitySelfDestruct(entity, life);
		}
	}

	return entity;
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
			decl Float:vPos[3];
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

stock void TF2_GetClassName(TFClassType class, char[] name, int size, bool capitalize)
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
	if (!IsValidEntity(weapon))
	{
		return 0;
	}

	return GetEntProp(weapon, Prop_Data, "m_iClip1");
}

stock void SetClip(int weapon, int clip)
{
	if (!IsValidEntity(weapon))
	{
		return;
	}

	SetEntProp(weapon, Prop_Data, "m_iClip1", clip);
}

stock int GetAmmo(int client, int weapon)
{
	if (client == 0 || client > MaxClients || !IsValidEntity(weapon))
	{
		return 0;
	}

	int iOffset = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4;
	int iAmmoTable = FindSendPropInfo("CTFPlayer", "m_iAmmo");
	return GetEntData(client, iAmmoTable + iOffset);
}

stock void SetAmmo(int client, int weapon, int ammo)
{
	if (client == 0 || client > MaxClients || !IsValidEntity(weapon))
	{
		return;
	}

	int iOffset = GetEntProp(weapon, Prop_Send, "m_iPrimaryAmmoType", 1) * 4;
	int iAmmoTable = FindSendPropInfo("CTFPlayer", "m_iAmmo");
	SetEntData(client, iAmmoTable + iOffset, ammo, 4, true);
}

//#Kisslick
stock void AutoExecConfigAppend( const char[] name = "", const char[] folder = "sourcemod", char AppendNote[ 512 ] = "", bool AlwaysAppend = false )
{
	char CommandOutPut[ 4096 ];
	char PluginFileName[ 256 ];
	char PluginVersion[ 16 ];
	char ConfigName[ PLATFORM_MAX_PATH ];
	Handle FileHandle = INVALID_HANDLE;
	char FileLine[ 256 ];
	GetPluginFilename( INVALID_HANDLE , PluginFileName, sizeof( PluginFileName ) );
	GetPluginInfo( INVALID_HANDLE, PlInfo_Version, PluginVersion, sizeof( PluginVersion ) );

	strcopy( ConfigName, FindCharInString( PluginFileName, '.' ) + 1, PluginFileName );
	if( strlen( name ) == 0 )
	{
		Format( ConfigName, sizeof( ConfigName ), "plugin.%s", ConfigName );
	}
	else
	{
		strcopy( ConfigName, sizeof( ConfigName ), name );
	}

	AutoExecConfig( false, ConfigName, folder );

	// Build path
	Format( ConfigName, sizeof( ConfigName ), "cfg/%s/%s.cfg", folder, ConfigName );

	// Check plugin version
	bool VersionAlreadyAppended = false;
	FileHandle = OpenFile( ConfigName, "r" );
	if( FileHandle != INVALID_HANDLE )
	{
		while( !IsEndOfFile( FileHandle ) && ReadFileLine( FileHandle, FileLine, sizeof( FileLine ) ) )
		{
			TrimString( FileLine );

			if( strlen( FileLine ) == 0 )
				continue;

			if( FileLine[ 0 ] != '/' || FileLine[ 1 ] != '/' )
				continue;

			if( ReplaceStringEx( FileLine, sizeof( FileLine ), "// @version:", "" ) != -1 )
			{
				TrimString( FileLine );

				if( StrEqual( FileLine, PluginVersion ) )
					VersionAlreadyAppended = true;
			}
		}
		CloseHandle( FileHandle );
	}

	if( VersionAlreadyAppended && !AlwaysAppend )
		return;

	// Append info about new version
	if( !VersionAlreadyAppended )
	{
		FileHandle = OpenFile( ConfigName, "a" );
		WriteFileLine( FileHandle, "// Append new ConVars for plugin %s", PluginFileName );
		WriteFileLine( FileHandle, "// @version: %s", PluginVersion );
		if( strlen( AppendNote ) != 0 )
		{
			ReplaceString( AppendNote, sizeof( AppendNote ), "\n", "\n// " );
			WriteFileLine( FileHandle, "// @note: %s", AppendNote );
		}
		WriteFileLine( FileHandle, "" );
		LogMessage( "ConVars appended to file \"%s\" (version: \"%s\")", ConfigName, PluginVersion );
		CloseHandle( FileHandle );
	}

	// Load ConVar list
	ServerCommandEx( CommandOutPut, sizeof( CommandOutPut ), "sm cvars %s", PluginFileName );
	strcopy( CommandOutPut, sizeof( CommandOutPut ), CommandOutPut[  FindCharInString( CommandOutPut, '\n' ) + 1 ] );
	strcopy( CommandOutPut, sizeof( CommandOutPut ), CommandOutPut[  FindCharInString( CommandOutPut, '\n' ) + 1 ] );
	TrimString( CommandOutPut );

	char cvar[ 256 ];
	Handle ConVarsList = INVALID_HANDLE;
	int index = SplitString( CommandOutPut, " ", cvar, sizeof( cvar ) );
	ConVarsList = CreateArray( 256 );

	while( index != -1 )
	{
		PushArrayString( ConVarsList, cvar );

		if( FindCharInString( CommandOutPut, '\n' ) + 1 == EOS )
			break;

		strcopy( CommandOutPut, sizeof( CommandOutPut ), CommandOutPut[ FindCharInString( CommandOutPut, '\n' ) + 1 ] );
		TrimString( CommandOutPut );
		index = SplitString( CommandOutPut, " ", cvar, sizeof( cvar ) );
	}

	// Filter ConVars
	FileHandle = OpenFile( ConfigName, "r" );
	if( FileHandle != INVALID_HANDLE )
	{
		while( !IsEndOfFile( FileHandle ) && ReadFileLine( FileHandle, FileLine, sizeof( FileLine ) ) )
		{
			TrimString( FileLine );

			if( FileLine[ 0 ] == '/' && FileLine[ 1 ] == '/' )
				continue;

			if( strlen( FileLine ) == 0 )
				continue;

			index = SplitString( FileLine, " ", cvar, sizeof( cvar ) );

			if( index != -1 )
			{
				index = FindStringInArray( ConVarsList, cvar );
				if( index != -1 )
				{
					RemoveFromArray( ConVarsList, index );
				}
			}
		}
		CloseHandle( FileHandle );
	}


	if( GetArraySize( ConVarsList ) < 1 )
		return;

	// Append new ConVars
	Handle ConVarHandle = INVALID_HANDLE;
	Handle ConVarIterator = INVALID_HANDLE;
	float ConVarBound;
	char ConVarDescription[ 512 ];
	char ConVarDefault[ 256 ];
	char ConVarValue[ 256 ];
	char Name[64];
	int flags; bool isCommand;

	FileHandle = OpenFile( ConfigName, "a" );
	for( int i = 0; i < GetArraySize( ConVarsList ); i++ )
	{
		GetArrayString( ConVarsList, i, cvar, sizeof( cvar ) );

		ConVarHandle = FindConVar( cvar );

		if( ConVarHandle == INVALID_HANDLE )
			continue;

		ConVarIterator = FindFirstConCommand( Name, sizeof( Name ), isCommand, flags, ConVarDescription, sizeof( ConVarDescription ) );
		if( ConVarIterator == INVALID_HANDLE )
			continue;

		do
		{
			if( isCommand || !StrEqual( Name, cvar ) )
				continue;

			break;
		}while( FindNextConCommand( ConVarIterator, Name, sizeof( Name ), isCommand, flags, ConVarDescription, sizeof( ConVarDescription ) ) );
		CloseHandle( ConVarIterator );

		ReplaceString( ConVarDescription, sizeof( ConVarDescription ), "\n", "\n// " );
		WriteFileLine( FileHandle, "// %s", ConVarDescription );
		WriteFileLine( FileHandle, "// -" );

		GetConVarDefault( ConVarHandle, ConVarDefault, sizeof( ConVarDefault ) );
		WriteFileLine( FileHandle, "// Default: \"%s\"", ConVarDefault );

		if( GetConVarBounds( ConVarHandle, ConVarBound_Lower, ConVarBound ) )
			WriteFileLine( FileHandle, "// Minimum: \"%f\"", ConVarBound );

		if( GetConVarBounds( ConVarHandle, ConVarBound_Upper, ConVarBound ) )
			WriteFileLine( FileHandle, "// Maximum: \"%f\"", ConVarBound );

		GetConVarString( ConVarHandle, ConVarValue, sizeof( ConVarValue ) );
		WriteFileLine( FileHandle, "%s \"%s\"", cvar, ConVarValue );
		WriteFileLine( FileHandle, "" );

	}

	CloseHandle( ConVarsList );
	CloseHandle( FileHandle );
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
	return GetEntPropEnt(client, Prop_Send, "m_hActiveWeapon");
}