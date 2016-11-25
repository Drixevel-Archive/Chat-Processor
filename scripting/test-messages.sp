#pragma semicolon 1
#pragma newdecls required

#include <sourcemod>
#include <chat-processor>

public Plugin myinfo =
{
	name = "Test Messages",
	author = "Keith Warren (Drixevel)",
	description = "Tests the Chat-Processor plugin.",
	version = "1.0.0",
	url = "http://www.drixevel.com/"
};

public void OnPluginStart()
{
	LogMessage("ONLY RUN THIS PLUGIN IF YOU WANT TO TEST THE FORWARDS FOR CHAT-PROCESSOR!");
}

public Action OnChatMessage(int& author, ArrayList recipients, char[] flagstring, char[] name, char[] message, bool& processcolors, bool& removecolors)
{
	char sNamesBuffer[1024];

	for (int i = 0; i < GetArraySize(recipients); i++)
	{
		int client = GetArrayCell(recipients, i);

		if (i == 0)
		{
			Format(sNamesBuffer, sizeof(sNamesBuffer), "%N", client);
		}
		else
		{
			Format(sNamesBuffer, sizeof(sNamesBuffer), "%s, %N", sNamesBuffer, client);
		}
	}

	PrintToServer("---MESSAGE:\nAuthor: %i\nFlag: %s\nName: %s\nMessage: %s\nProcessColors: %s\nRemoveColors: %s\nRecipients: %s", author, flagstring, name, message, processcolors ? "true" : "false", removecolors ? "true" : "false", sNamesBuffer);

	Format(name, MAXLENGTH_NAME, "[Test] {red}%s", name);
	Format(message, MAXLENGTH_MESSAGE, "{blue}%s", message);
	return Plugin_Changed;
}

public void OnChatMessagePost(int author, ArrayList recipients, const char[] flagstring, const char[] formatstring, const char[] name, const char[] message, bool processcolors, bool removecolors)
{
	char sNamesBuffer[1024];

	for (int i = 0; i < GetArraySize(recipients); i++)
	{
		int client = GetArrayCell(recipients, i);

		if (i == 0)
		{
			Format(sNamesBuffer, sizeof(sNamesBuffer), "%N", client);
		}
		else
		{
			Format(sNamesBuffer, sizeof(sNamesBuffer), "%s, %N", sNamesBuffer, client);
		}
	}

	PrintToServer("---MESSAGE POST:\nAuthor: %i\nFlag: %s\nFormat: %s\nName: %s\nMessage: %s\nProcessColors: %s\nRemoveColors: %s\nRecipients: %s", author, flagstring, formatstring, name, message, processcolors ? "true" : "false", removecolors ? "true" : "false", sNamesBuffer);
}
