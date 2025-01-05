#pragma once
#include <vector>
#include <memory>

struct RequestInfo
{
	uint8_t id; // RequestId as a single byte   
	time_t receivalTime;
	std::vector<char> buffer;
};

class IRequestHandler
{
public:
	virtual ~IRequestHandler() = default;

	struct RequestResult
	{
		std::vector<char> response;
		std::unique_ptr<IRequestHandler> newHandler;
	};

	virtual bool isRequestRelevant(const RequestInfo& info) const = 0;
	virtual RequestResult handleRequest(const RequestInfo& requestInfo) = 0;
};
