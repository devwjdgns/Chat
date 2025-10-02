#pragma once
#include <string>
#include <vector>

class __declspec(dllexport) IData
{
public:
	virtual ~IData() = default;

	template<typename T>
	static T* GetData(IData* data) { return dynamic_cast<T*>(data); }

	template<typename T>
	static const T* GetData(const IData* data) { return dynamic_cast<const T*>(data); }
};

class __declspec(dllexport) StringData : public IData
{
public:
	StringData(const std::string& str) :str(str) {}
	std::string str;
};

class __declspec(dllexport) ListData : public IData
{
public:
	ListData(const std::vector<std::string>& list) :list(list) {}
	std::vector<std::string> list;
};

class __declspec(dllexport) MessageData : public IData
{
public:
	MessageData() :name(""), message(""), timestamp("") {}
	MessageData(const std::string& name, const std::string& message, const std::string& timestamp) 
		:name(name), message(message), timestamp(timestamp) {}
	std::string name;
	std::string message;
	std::string timestamp;
};