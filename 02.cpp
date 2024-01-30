#include <iostream>
#include <fstream>
#include <vector>

class LogCommand {
public:
	virtual ~LogCommand() = default;
	virtual void print(const std::string& message) = 0;
};

class FileLog : public LogCommand
{
public:
	FileLog(const std::string& f_path, std::ofstream* f_str) : file_path{ f_path }, f_ostream{ f_str } {}
	void print(const std::string& message) override
	{
		f_ostream->open(file_path, std::ios::app);
		if (f_ostream->is_open())
		{
			*f_ostream << message << std::endl;
		}
	}
	~FileLog() { f_ostream->close(); }

private:
	const std::string file_path;
	std::ofstream* f_ostream;
};

class ConsoleLog : public LogCommand
{
public:
	ConsoleLog(std::ostream* p_ostr) : p_ostream{ std::move(p_ostr) } {}
	void print(const std::string& message) override
	{
		*p_ostream << message << std::endl;
	}
	~ConsoleLog() {};

private:
	std::ostream* p_ostream;
};

class Observer {
public:
	virtual void onWarning(const std::string& message) = 0;
	virtual void onError(const std::string& message) = 0;
	virtual void onFatalError(const std::string& message) = 0;
	virtual ~Observer() {};
};

class LogObserver : public Observer //бывш. ConsoleLogObserver
{
public:
	LogObserver(LogCommand* lc) : logcomm{std::move(lc)} {};
	void onWarning(const std::string& message) override
	{
		logcomm->print(message);
	}
	void onError(const std::string& message) override
	{
		logcomm->print(message);
	}
	void onFatalError(const std::string& message) override
	{
		logcomm->print(message);
	}
	~LogObserver() {};

private:
	LogCommand* logcomm;
};
//DRY
//class FileLogObserver : public Observer
//{
//public:
//	FileLogObserver(LogCommand* file) : file{ std::move(file) }{};
//	void onWarning(const std::string& message) override
//	{
//		file->print(message);
//	}
//	void onError(const std::string& message) override
//	{
//		file->print(message);
//	}
//	void onFatalError(const std::string& message) override
//	{
//		file->print(message);
//	}
//	~FileLogObserver() {};
//
//private:
//	LogCommand* file;
//};

class AllLogObserver : public Observer
{
public:
	AllLogObserver(LogCommand* console, LogCommand* file) : cons{ std::move(console) }, file{ std::move(file) }{};
	void onWarning(const std::string& message) override
	{
		cons->print(message);
		file->print(message);
	}
	void onError(const std::string& message) override
	{
		cons->print(message);
		file->print(message);
	}
	void onFatalError(const std::string& message) override
	{
		cons->print(message);
		file->print(message);
	}
	~AllLogObserver() {};

private:
	LogCommand* cons;
	LogCommand* file;
};

class Observed
{
public:
	Observed() {};
	void AddObserver(Observer* observer)
	{
		observers_.push_back(observer);
	}
	void RemoveObserver(Observer* observer)
	{
		auto it = std::remove(observers_.begin(), observers_.end(), observer);
		observers_.erase(it, observers_.end());
	}
	void warning(const std::string& message) const
	{
		for (const auto& observer : observers_)
		{
			observer->onWarning(message);
		}
	};
	void error(const std::string& message) const 
	{
		for (const auto& observer : observers_)
		{
			observer->onError(message);
		}
	};
	void fatalError(const std::string& message) const 
	{
		for (const auto& observer : observers_)
		{
			observer->onFatalError(message);
		}
	};
	~Observed() {};

private:
	std::vector<Observer*> observers_;
};

int main()
{
	ConsoleLog cl(&std::cout);
	LogObserver clo(&cl);
	std::ofstream ofs;
	FileLog fl("file_path", &ofs);
	LogObserver flo(&fl);
	
	Observed obs;
	obs.AddObserver(&clo);
	obs.AddObserver(&flo);
	obs.AddObserver(new AllLogObserver(&cl, &fl));
	obs.warning("warning");
	obs.RemoveObserver(&clo);

	return 0;
}
