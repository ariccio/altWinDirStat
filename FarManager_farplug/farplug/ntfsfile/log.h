#pragma once

struct LogRecord {
	std::string object;
	std::string message;
	};

struct Log : public std::vector < LogRecord > {
	void add( const std::string& object, const std::string& message ) {
		LogRecord log_rec;
		log_rec.object = object;
		log_rec.message = message;
		std::vector<LogRecord>::add( log_rec );
		}
	void show( );
	};
