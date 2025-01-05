#pragma once
#include <string>

struct AuthResponse {
	int status;
	std::string message;
};

enum success_and_failure_codes
{
	TURN_OF_COMPUTER_SUCCESS = 1,
	TURN_OF_COMPUTER_FAILED = 2,
	KEY_BOARD_READING_SUCCESS = 3,
	KEY_BOARD_READING_FAILED = 4,
	KEY_BOARD_CONTROLING_SUCCESS = 5,
	KEY_BOARD_CONTROLING_FAILED = 6,
	BLOCKING_WEB_FAILED = 7,
	BLOCKING_WEB_SUCCESS = 8
};


struct TurnOfComputerResponse
{
	unsigned int status;
};

struct KeyBoardReadingResponse
{
	unsigned int status;

};

struct KeyBoardControlingResponse
{
	unsigned int status;

};

struct blockingWebResponse
{
	unsigned int status;

};
