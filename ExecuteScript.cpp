#include "Scripts.h"
#include "WaveSynth.h"
#include "strrpl.h"
#include "SimpleScript.h"

extern void WriteLog(const char*, double = 0);

ScriptVariable createVariable(const std::string& name, double value = 0.0, char type = TYPE_TEMPORARY)
{
	return {
		type,
		name, 
		value
	};
}

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
		char buf[256], cmd1[32] = "";
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

ScriptVariable *getScriptVariable(const std::string &variableName, std::vector<ScriptVariable> &variables)
{
	ScriptVariable *var = NULL;
	for (int i = 0; i < variables.size(); i++)
	{
		if (variables[i].name == variableName)
		{
			var = &variables[i];
			break;
		}
	}
	return var;
}

void _ExecuteScript(const std::string &filename, std::vector<ScriptVariable> &variables)
{
	FILE *script = fopen(filename.c_str(), "r");
	int execTime = (int)time(NULL);
	long whileLoopPos[100], forLoopPos[100];
	for (int i = 0; i < 100; i++) whileLoopPos[i] = forLoopPos[i] = -1;
	bool enableDebug = false;
	while (!feof(script) && (int)time(NULL) - execTime < 5) // endless loop protection
	{
		char full_cmd[256];
		long cmdPos = ftell(script);
		fgets(full_cmd, 256, script);
		if (enableDebug) WriteLog(full_cmd);
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
			auto var = getScriptVariable(cmd[1], variables);
			WriteLog(var->name.c_str(), var->value);
		}
		else if (!strcmp(cmd[0], "debugmsg"))
		{
			WriteLog(full_cmd);
			if (!strcmp(cmd[1], "enable_all"))
			{
				enableDebug = true;
				WriteLog("-- Debug output enabled");
			}
		}
		else if (!strcmp(cmd[0], "call"))
		{
			int pos = filename.find_last_of('\\');
			std::string ext = "";
			if (std::string(cmd[1]).find_first_of('.') == std::string::npos)
				ext = ".syn";
			auto filename2 = filename.substr(0, pos + 1) + cmd[1] + ext;
			_ExecuteScript(filename2, variables);
		}
		// get_selected_index #variable total_count value to #result
		else if (!strcmp(cmd[0], "get_selected_index"))
		{
			ScriptVariable *var = getScriptVariable(cmd[1], variables);
			double value;
			if (var == 0)
				sscanf(cmd[1], "%lf", &value);
			else
				value = var->value;
			int count = 0;
			sscanf(cmd[3], "%d", &count);
			var = getScriptVariable(cmd[5], variables);
			var->value = GetSelection(value, count);
		}
		// set_selected_index #index total_count value to #result
		else if (!strcmp(cmd[0], "set_selected_index"))
		{
			ScriptVariable *var = getScriptVariable(cmd[1], variables);
			double value;
			if (var == 0)
				sscanf(cmd[1], "%lf", &value);
			else
				value = var->value;
			int count = 0;
			sscanf(cmd[3], "%d", &count);
			var = getScriptVariable(cmd[5], variables);
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
			ScriptVariable *var = getScriptVariable(cmd[1], variables);
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
			for (int i = 0; i < variables.size(); i++)
			{
				if (variables[i].name == cmd[1])
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
			double op1 = 0, op2 = 0;
			char oper = '+';
			for (int i = 0; i < variables.size(); i++)
			{
				if (variables[i].name == cmd[0])
				{
					result = &variables[i];
					break;
				}
			}
			if (result == NULL)
			{
				auto isTemp = cmd[0][0] == '%' || cmd[0][0] == '$' || cmd[0][0] == '#';
				variables.push_back(createVariable(cmd[0], 0, isTemp ? TYPE_TEMPORARY : TYPE_VST_FLOAT_PARAM));
				result = &variables[variables.size() - 1];
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
				for (int i = 0; i < variables.size(); i++)
					if (variables[i].name == cmd[2])
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
					for (int i = 0; i < variables.size(); i++)
						if (variables[i].name == cmd[4])
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
}

std::vector<ParamDTO> ExecuteScript(const std::string &filename, std::vector<ScriptVariable> &initial)
{
	static bool srandDone = false;
	if (!srandDone)
	{
		srand((int)time(NULL));
		srandDone = true;
	}
	std::map<std::string, double> variables;
	std::string entryPoint;
	for (const auto& v : initial)
	{
		if (v.type == 'e')
			entryPoint = v.name;
		else
			variables[v.name] = v.value;
	}

	std::string intParams;

	const auto notify = [&variables, &intParams](const std::vector<std::string> & sv)
	{
		if (sv.size() > 1 && sv[0] == "params_are_integers")
		{
			for (const auto& s : sv)
			{
				intParams = intParams + "{" + s + "}";
			}
		}
	};

	SimpleScript script(filename);
	try
	{
		script.execute(variables, notify, entryPoint);
	}
	catch (const std::exception & e)
	{
		WriteLog(e.what());
		throw e;
	}

	std::vector<ParamDTO> dtos;
	for (const auto& v : initial)
	{
		ParamDTO dto;
		auto val = variables[v.name];
		dto.create(v.name.c_str(), (float)val);
		if (intParams.find('{' + v.name + '}') != std::string::npos)
		{
			dto.intValue = (int)val;
		}
		dtos.push_back(dto);
	}
	return dtos;
	/*std::vector<ScriptVariable> variables;
	variables.push_back(createVariable("#index"));
	for (auto i = 0; i < initial.size(); i++)
	{
		variables.push_back(initial[i]);
	}
	srand((int)time(NULL));
	rand();
	_ExecuteScript(filename, variables);
	unsigned int size = 0;
	std::vector<ParamDTO> dtos;
	for (int i = 0; i < variables.size(); i++)
	{
		ScriptVariable *var = &variables[i];
		if (var->type != TYPE_VST_FLOAT_PARAM && var->type != TYPE_VST_INTEGER_PARAM) continue;
		ParamDTO dto;
		int intVal = -1;
		if (var->type == TYPE_VST_INTEGER_PARAM)
		{
			intVal = (int)var->value;
		}
		dto.create(var->name.c_str(), (float)var->value);
		dto.intValue = intVal;
		dtos.push_back(dto);
	}
	return dtos;*/
}
