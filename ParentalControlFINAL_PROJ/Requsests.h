#pragma once

enum resquests
{
	TURN_OFF_COMPUTER_REQUEST = 1,
	KEY_BOARD_READING_REQUEST = 2,
	KEY_BOARD_CONTROLING_REQUEST = 3,
	BLOCK_WEB_REQUEST = 4
};

struct KeyBoardReadingRequest
{
	int timeInSecondToRead;
};

struct KeyBoardControlingRequest
{
	char ch;
	int timeInSecondToControl;
};

struct BlockingWebRequest
{
	std::string webName;
};