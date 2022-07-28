# Chat-Processor

A Sourcemod plugin which allows other plugins to add and manage chat related features.

🍴 This is a fork of [Drixevel's Chat Processor](https://github.com/Drixevel-Archive/Chat-Processor) that uses MultiColors instead of ColorVariables. Use it you're having issues with the original.

## Installation

Place the .smx into the plugins folder and the .cfg into the configs folder.

## Usage

```C#
public Action CP_OnChatMessage(int& author, ArrayList recipients, char[] flagstring, char[] name, char[] message, bool& processcolors, bool& removecolors)
{
	Format(name, MAXLENGTH_NAME, "{red}%s", name);
	Format(message, MAXLENGTH_MESSAGE, "{blue}%s", message);
	return Plugin_Changed;
}
```

## Contributing
Pull requests are welcome. For major changes, please open an issue first to discuss what you would like to change.

Please make sure to update tests as appropriate.

## License
[GPL-3.0](https://github.com/Drixevel/Chat-Processor/blob/master/LICENSE.txt)
