#ifndef LOGGER_H
#define LOGGER_H

enum logger_timestamp_mode {
	LOGGER_TIMESTAMP_NONE,
	LOGGER_TIMESTAMP_SIMPLE,
	LOGGER_TIMESTAMP_COMPLEX
};

enum logger_timestamp_mode get_logger_mode();
void set_logger_mode(enum logger_timestamp_mode t);

#endif /* LOGGER_H */