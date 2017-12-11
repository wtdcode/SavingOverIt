#include<Windows.h>
#include<iostream>
#include<fstream>
#include<string>
#include<ctime>
#include<json\json.h>

#define WIN32_LEAN_AND_MEAN
#define NOCOMM

#define GOI_REG_PATH "Software\\Bennett Foddy\\Getting Over It"

using Json::Value;
using std::ofstream;
using std::ifstream;
using std::ios;
using std::string;
using std::to_string;
using std::cout;

bool isExcluded(string name) {
	// These values in my registry are inavailabe, so I can't get proper values through RegEnumValue.
	// So I decide to exclude them from the saved values.
	// In fact, some other values should be also excluded, but I am too lazy :).
	static string exclude[] = { "Curvy_ControlPointSize",
								"Curvy_OrientationLength",
								"Curvy_SceneViewResolution",
								"MouseSensitivity",
								"MusicVolume",
								"SFXVolume",
								"VoiceVolume" };
	int len = 7;
	for (int i = 0; i < len; i++)
		if (name.find(exclude[i]) != string::npos)
			return 1;
	return 0;
}

bool getSaveData(HKEY key, Value &save_data) {
	DWORD values_num;
	DWORD maxnamelen;
	DWORD maxvaluelen;
	PCHAR name;
	DWORD name_len;
	PCHAR buffer;
	DWORD buffer_len;
	DWORD value_type;
	if (RegQueryInfoKey(key, NULL, NULL, NULL, NULL, NULL, NULL, &values_num, &maxnamelen, &maxvaluelen, NULL, NULL) != ERROR_SUCCESS)
		return 0;
	name = new CHAR[maxnamelen + 1];
	buffer = new CHAR[maxvaluelen + 1];
	for (DWORD i = 0; i < values_num; i++) {
		name_len = maxnamelen + 1;
		buffer_len = maxvaluelen + 1;
		if (RegEnumValue(key, i, name, &name_len, NULL, &value_type, (BYTE*)buffer, &buffer_len) != ERROR_SUCCESS)
			return 0;
		if (isExcluded(string(name)))
			continue;
		switch (value_type) {
		case REG_DWORD:
			save_data[name]["data"] = *((DWORD32*)buffer);
			break;
		case REG_BINARY:
			buffer[buffer_len] = 0;
			save_data[name]["data"] = buffer;
			break;
		}
		save_data[name]["type"] = (unsigned int)value_type;
		save_data[name]["length"] = (unsigned int)buffer_len;
	}
	return 1;
}

bool getRegKey(HKEY &key) {
	// Maybe RegOpenCurrentUser
	if (RegOpenKeyEx(HKEY_CURRENT_USER, GOI_REG_PATH, NULL, KEY_ALL_ACCESS, &key) != ERROR_SUCCESS)
		return 0;
	else
		return 1;
}

bool writeSaveData(int index, const Value& save_data) {
	string file_name = "save" + to_string(index) + ".json";
	ofstream out(file_name,ios::out|ios::trunc);
	out << save_data;
	out.close();
	return 1;
}

bool readSaveData(int index, Value& save_data) {
	string file_name = "save" + to_string(index) + ".json";
	ifstream in(file_name);
	if (!in.is_open())
		return 0;
	in >> save_data;
	return 1;
}

// maybe add exception handle next time
bool setSaveData(HKEY key, const Value& save_data) {
	DWORD type;
	BYTE* data;
	DWORD dword;
	DWORD length;
	string temp;
	for (auto it = save_data.begin(); it != save_data.end(); it++) {
		type = it->get("type", REG_BINARY).asUInt();
		length = it->get("length", 0).asUInt();
		switch (type) {
		case REG_DWORD:
			dword = it->get("data", "").asUInt();
			data = (BYTE*)&dword;// maybe cause endian problem. but who cares :)
			break;
		case REG_BINARY:
			temp = it->get("data", "").asString();
			data = (BYTE*)temp.c_str();
			break;
		default:
			data = nullptr;
			break;
		}
		if (RegSetValueEx(key, it.name().c_str(),NULL,type,data, length) != ERROR_SUCCESS)
			return 0;
	}
	return 1;
}

bool registerKey() {
	if (RegisterHotKey(NULL, 1, MOD_NOREPEAT, VK_F7) && RegisterHotKey(NULL, 2, MOD_NOREPEAT, VK_F8))
		return 1;
	else
		return 0;
}

string getTime() {
	SYSTEMTIME stime;
	GetLocalTime(&stime);
	return string(to_string(stime.wYear) + "/" + to_string(stime.wMonth) + "/" + to_string(stime.wDay) + " " + to_string(stime.wHour) + ":" + to_string(stime.wMinute));
}

bool writeConfig(Value &config) {
	const static string file_name = "config.json";
	ofstream out(file_name, ios::out | ios::trunc);
	out << config;
	return 1;
}

bool readConfig(Value& config) {
	const static string file_name = "config.json";
	ifstream in(file_name, ios::in);
	if (!in.is_open()) {
		config["index"] = 0;
		writeConfig(config);
	}
	else {
		in >> config;
	}
	return 1;
}

int main() {
	HKEY GOI_key;
	Value save_data;
	Value config;
	int index;
	MSG m;
	if (!getRegKey(GOI_key)) {
		cout << "Please run the game at least one time.\n";
		cout << "[Error Code]" << GetLastError() << "\n";
		return 0;
	}
	readConfig(config);
	if (!registerKey()) {
		cout << "Hotkey registration failed.\n";
		cout << "[Error Code]" << GetLastError() << "\n";
		return 0;
	}
	index = config.get("index",0).asInt();
	while (GetMessage(&m, NULL, NULL, NULL) != 0) {
		switch (m.message) {
		case WM_HOTKEY:
			if (m.wParam == 1) {
				index++;
				getSaveData(GOI_key, save_data);
				writeSaveData(index, save_data);
				cout <<"[" << getTime() << "]\n";
				cout << "save" + to_string(index) + " saved\n";
			}
			else if(m.wParam == 2) {
				readSaveData(index, save_data);
				setSaveData(GOI_key, save_data);
				cout << "[" << getTime() << "]\n";
				cout << "load latest save\n";
			}
		}
	}
}