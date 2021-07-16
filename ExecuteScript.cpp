#include "Scripts.h"
#include "WaveSynth.h"
#include "strrpl.h"

extern void WriteLog(char*, double = 0);

int GetSelection(double value, int displayCount)
{
	return (int)(value * 0.99 * displayCount);
}


double SetSelection(int index, int displayCount)
{
	if (displayCount == 0) return 0;
	double value = (double)(index + 0.5) / displayCount;
	if (value > 1) value = 1;
	else if (value < 0) value = 0;
	GetSelection(value, displayCount);
	return value;
}

void readUntil(FILE *script, char *what, int id = 0)
{
	while (!feof(script))
	{
		char buf[256], cmd1[32];
		int readId;
		fgets(buf, 256, script);
		sscanf(buf, "%s %d", cmd1, &readId);
		if (!strcmp(cmd1, what))
		{
			if (cmd1[0] != '@' || id == readId)
				break;
		}
	}
}

int variableCompare(ScriptVariable *variable, double value, char greaterTest, char eqTest, char invert)
{
	int result = 0;
	if (variable->type == TYPE_VST_INTEGER_PARAM)
	{
		int iVarValue = (int)variable->value;
		int iValue = (int)value;

		result = (iVarValue == iValue && eqTest) || (iVarValue > iValue && greaterTest);
	}
	else
	{
		result = (variable->value == value && eqTest) || (variable->value > value && greaterTest);
	}
	if (invert)
		result = !result;
	return result;
}

ScriptVariable *getScriptVariable(char * variableName, ScriptVariable *variables, int usedVariables)
{
	ScriptVariable *var = NULL;
	for (int i = 0; i <= usedVariables; i++)
		if (!strcmp(variables[i].name, variableName))
		{
			var = &variables[i];
			break;
		}
	return var;
}

int _ExecuteScript(char *filename, ScriptVariable *variables, int usedVariables = 0)
{
	FILE *script = fopen(filename, "r");
	int execTime = (int)time(NULL);
	long whileLoopPos[100], forLoopPos[100];
	for (int i = 0; i < 100; i++) whileLoopPos[i] = forLoopPos[i] = -1;
	while (!feof(script) && (int)time(NULL) - execTime < 5) // endless loop protection
	{
		char full_cmd[256];
		long cmdPos = ftell(script);
		fgets(full_cmd, 256, script);
		if (feof(script)) break;
		char numAsStr[10];
		sprintf(numAsStr, "%d", (int)variables[0].value);
		strrpl(full_cmd, full_cmd, 256, "?", numAsStr, 1);
		strrpl(full_cmd, full_cmd, 256, "_true", "1", 1);
		strrpl(full_cmd, full_cmd, 256, "_false", "0", 1);
		if (full_cmd[0] == ':' || full_cmd[0] == '\'') continue;
		char cmd[6][32];
		int numCmds = sscanf(full_cmd, "%s %s %s %s %s %s", cmd[0], cmd[1], cmd[2], cmd[3], cmd[4], cmd[5]);
		if (numCmds == 1 && !strcmp(cmd[0], "wend"))
			fseek(script, whileLoopPos[0], SEEK_SET);
		else if (numCmds == 1 && !strcmp(cmd[0], "loop"))
			fseek(script, forLoopPos[0], SEEK_SET);
		else if (numCmds == 1 && !strcmp(cmd[0], "else"))
		{
			int ifLevel = 1;
			while (!feof(script) && ifLevel > 0)
			{
				char buf[256], cmd1[32] = "";
				fgets(buf, 256, script);
				sscanf(buf, "%s", cmd1);
				if (!strcmp(cmd1, "if"))
					ifLevel++;
				else if (!strcmp(cmd1, "endif"))
					ifLevel--;
			}
		}
		if (numCmds <= 1) continue;
		if (!strcmp(cmd[0], "debug"))
		{
			auto var = getScriptVariable(cmd[1], variables, usedVariables);
			WriteLog(var->name, var->value);
		}
		else if (!strcmp(cmd[0], "debugmsg"))
		{
			WriteLog(full_cmd);
		}
		else if (!strcmp(cmd[0], "call"))
		{
			int pos = strlen(filename);
			while (filename[--pos] != '\\');
			char filename2[1024] = "";
			strncpy(filename2, filename, pos + 1);
			strcat(filename2, cmd[1]);
			int hasExtension = 0;
			for (char *p = filename2; *p && !hasExtension; p++) hasExtension = *p == '.';
			if (!hasExtension)
				strcat(filename2, ".syn");
			usedVariables = _ExecuteScript(filename2, variables, usedVariables);
		}
		// get_selected_index #variable total_count value to #result
		else if (!strcmp(cmd[0], "get_selected_index"))
		{
			ScriptVariable *var = getScriptVariable(cmd[1], variables, usedVariables);
			double value;
			if (var == 0)
				sscanf(cmd[1], "%lf", &value);
			else
				value = var->value;
			int count = 0;
			sscanf(cmd[3], "%d", &count);
			var = getScriptVariable(cmd[5], variables, usedVariables);
			var->value = GetSelection(value, count);
		}
		// set_selected_index #index total_count value to #result
		else if (!strcmp(cmd[0], "set_selected_index"))
		{
			ScriptVariable *var = getScriptVariable(cmd[1], variables, usedVariables);
			double value;
			if (var == 0)
				sscanf(cmd[1], "%lf", &value);
			else
				value = var->value;
			int count = 0;
			sscanf(cmd[3], "%d", &count);
			var = getScriptVariable(cmd[5], variables, usedVariables);
			var->value = SetSelection((int)value, count);
		}
		else if (!strcmp(cmd[0], "@wend"))
		{
			int loopId = 0;
			sscanf(cmd[1], "%d", &loopId);
			fseek(script, whileLoopPos[loopId], SEEK_SET);
		}
		else if (!strcmp(cmd[0], "@loop"))
		{
			int loopId = 0;
			sscanf(cmd[1], "%d", &loopId);
			fseek(script, forLoopPos[loopId], SEEK_SET);
		}
		else if (!strcmp(cmd[0], "for") || !strcmp(cmd[0], "@for"))
		{
			int start = 0, end = 0;
			sscanf(cmd[2], "%d", &start);
			sscanf(cmd[4], "%d", &end);
			if (start > end) end = start;
			/*ScriptVariable *var = NULL;
			for (int i = 0; i <= usedVariables; i++)
				if (!strcmp(variables[i].name, cmd[1]))
				{
					var = &variables[i];
					break;
				}*/
			ScriptVariable *var = getScriptVariable(cmd[1], variables, usedVariables);
			int loopId = 0;
			if (!strcmp(cmd[0], "@for"))
				sscanf(cmd[5], "%d", &loopId);
			if (var == NULL)
				continue;
			if (forLoopPos[loopId] != cmdPos)
			{
				var->value = start;
				forLoopPos[loopId] = cmdPos;
			}
			else if ((int)(var->value) < end)
				var->value += 1;
			else
			{
				forLoopPos[loopId] = -1;
				if (!strcmp(cmd[0], "@for"))
					readUntil(script, "@loop", loopId);
				else
					readUntil(script, "loop");
			}
		}
		else if (!strcmp(cmd[0], "if") || !strcmp(cmd[0], "while") || !strcmp(cmd[0], "@while"))
		{
			int ifResult = 0;
			for (int i = 0; i <= usedVariables; i++)
				if (!strcmp(variables[i].name, cmd[1]))
				{
					double compareTo = 0;
					sscanf(cmd[3], "%lf", &compareTo);
					if (!strcmp(cmd[2], ">"))
						ifResult = variableCompare(&variables[i], compareTo, 1, 0, 0);
					else if (!strcmp(cmd[2], "<"))
						ifResult = variableCompare(&variables[i], compareTo, 1, 1, 1);
					else if (!strcmp(cmd[2], ">="))
						ifResult = variableCompare(&variables[i], compareTo, 1, 1, 0);
					else if (!strcmp(cmd[2], "<="))
						ifResult = variableCompare(&variables[i], compareTo, 1, 0, 1);
					else if (!strcmp(cmd[2], "="))
						ifResult = variableCompare(&variables[i], compareTo, 0, 1, 0);
					else if (!strcmp(cmd[2], "!="))
						ifResult = variableCompare(&variables[i], compareTo, 0, 1, 1);
					else if (!strcmp(cmd[2], "<>"))
						ifResult = variableCompare(&variables[i], compareTo, 0, 1, 1);
				}
			if (!strcmp(cmd[0], "while") || !strcmp(cmd[0], "@while"))
			{
				int loopId = 0;
				if (!strcmp(cmd[0], "@for"))
					sscanf(cmd[5], "%d", &loopId);
				whileLoopPos[loopId] = cmdPos;
				if (!ifResult)
				{
					if (!strcmp(cmd[0], "@while"))
						readUntil(script, "@wend", loopId);
					else
						readUntil(script, "wend");
				}

			}
			else if (!ifResult)
			{
				int ifLevel = 1;
				while (!feof(script) && ifLevel > 0)
				{
					char buf[256], cmd1[32] = "";
					fgets(buf, 256, script);
					sscanf(buf, "%s", cmd1);
					if (!strcmp(cmd1, "if"))
						ifLevel++;
					else if (!strcmp(cmd1, "endif"))
						ifLevel--;
					else if (!strcmp(cmd1, "else") && ifLevel == 1)
						ifLevel--;
				}
			}
		}
		else if (!strcmp(cmd[0], "goto"))
		{
			fseek(script, 0, SEEK_SET);
			while (!feof(script))
			{
				char buf[256], cmd1[32];
				fgets(buf, 256, script);
				if (buf[0] == ':')
				{
					sscanf(buf, "%s", cmd1);
					if (!strcmp(cmd1, cmd[1]))
						break;
				}
			}
		}
		else // equation
		{
			ScriptVariable *result = NULL;
			double op1, op2 = 0;
			char oper = '+';
			for (int i = 0; i <= usedVariables; i++)
			{
				if (!strcmp(variables[i].name, cmd[0]))
				{
					result = &variables[i];
					break;
				}
			}
			if (result == NULL)
			{
				result = &variables[++usedVariables];
				strcpy(result->name, cmd[0]);
				if (cmd[0][0] == '%' || cmd[0][0] == '$' || cmd[0][0] == '#')
					result->type = TYPE_TEMPORARY;
				else
					result->type = TYPE_VST_FLOAT_PARAM;
			}
			if (!strcmp(cmd[1], "as_integer"))
			{
				result->type = TYPE_VST_INTEGER_PARAM;
				continue;
			}
			if (!strcmp(cmd[1], "truncate"))
			{
				int iVal = (int)result->value;
				result->value = (double)iVal;
				continue;
			}
			char c = cmd[2][0];
			if (c == '-' || c == '.' || (c >= '0' && c <= '9'))
				sscanf(cmd[2], "%lf", &op1);
			else if (!strcmp("random", cmd[2]))
				op1 = (double)(rand() % 30000) / 30000.0;
			else
				for (int i = 0; i <= usedVariables; i++)
					if (!strcmp(variables[i].name, cmd[2]))
					{
						op1 = variables[i].value;
						break;
					}
			if (numCmds > 3)
			{
				oper = cmd[3][0];
				c = cmd[4][0];
				if (c == '-' || c == '.' || (c >= '0' && c <= '9'))
					sscanf(cmd[4], "%lf", &op2);
				else if (!strcmp("random", cmd[4]))
					op2 = (double)(rand() % 30000) / 30000.0;
				else
					for (int i = 0; i <= usedVariables; i++)
						if (!strcmp(variables[i].name, cmd[4]))
						{
							op2 = variables[i].value;
							break;
						}
			}
			if (oper == '+')
				result->value = op1 + op2;
			else if (oper == '-')
				result->value = op1 - op2;
			else if (oper == '/')
				result->value = op1 / op2;
			else if (oper == '*')
				result->value = op1 * op2;
			else if (oper == '%')
				result->value = (double)((int)op1 % (int)op2);
		}
	}
	fclose(script);
	return usedVariables;
}

/*char * evaluate(char *src, char *dst, int size, char openTag, char closeTag) {
	char *ptr = src, *temp = (char*)malloc(size);
	char *ptr_temp = temp;
	if (*ptr != openTag)
		*(ptr_temp++) = *ptr;
	while (*ptr && ptr - src < size && ptr_temp - temp < size) {
		ptr++;
		if (*ptr == openTag) {
			char *evalResult = (char*)malloc(size);
			ptr = evaluate(ptr, evalResult, size, openTag, closeTag);
			*(ptr_temp++) = 0;
			strcat(temp, evalResult);
			ptr_temp = &temp[strlen(temp)];
			free(evalResult);
		}
		else if (*ptr == closeTag) {
			*(ptr_temp++) = 0;
			_evaluate(temp, dst);
			break;
		}
		else {
			*(ptr_temp++) = *ptr;
		}
		if (*ptr == 0)
		{
			strcpy(dst, temp);
		}
	}
	free(temp);
	return ptr;
}*/

unsigned int ExecuteScript(char *filename, char **dtoStream, char *initial)
{
	ScriptVariable variables[1000];
	strcpy(variables[0].name, "#index");
	variables[0].type = TYPE_TEMPORARY;
	variables[0].value = 0;
	int usedVariables = 0;
	srand((int)time(NULL));
	rand();
	char *ptr = initial;
	char buf[32];
	int bpos = 0;
	while (*ptr != 0)
	{
		if (*ptr == '>')
		{
			buf[bpos] = 0;
			sscanf(buf, "%lf", &variables[usedVariables].value);
			variables[usedVariables].type = TYPE_VST_FLOAT_PARAM;
			bpos = 0;
		}
		else if (*ptr == '<')
		{
			buf[bpos] = 0;
			strcpy(variables[++usedVariables].name, buf);
			bpos = 0;
		}
		else
		{
			buf[bpos++] = *ptr;
		}
		ptr++;
	}
	usedVariables = _ExecuteScript(filename, variables, usedVariables);

	unsigned int size = 0;
	char *mem = (char *)malloc(1);
	for (int i = 0; i <= usedVariables; i++)
	{
		ScriptVariable *var = &variables[i];
		if (var->type != TYPE_VST_FLOAT_PARAM && var->type != TYPE_VST_INTEGER_PARAM) continue;
		ParamDTO dto;
		int len = strlen(var->name), intVal = -1;
		if (var->type == TYPE_VST_INTEGER_PARAM)
		{
			intVal = (int)var->value;
		}
		dto.create(var->name, (float)var->value);
		dto.intValue = intVal;
		size = dto.toMem(&mem, size);
	}
	*dtoStream = mem;
	return size;
}
