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

stock void OpenMOTD(int client, const char[] base, const char[] title, const char[] url)
{
	char sOpen[512];
	Format(sOpen, sizeof(sOpen), base, url);
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

stock bool String_IsNumeric(const char[] str)
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

stock int String_GetRandom(char[] buffer, int size, int length = 32, const char[] chrs = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ01234556789")
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

stock void Array_Fill(any[] array, int size, any value, int start = 0)
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

stock void Array_Copy(const any[] array, any[] newArray, int size)
{
	for (int i = 0; i < size; i++)
	{
		newArray[i] = array[i];
	}
}