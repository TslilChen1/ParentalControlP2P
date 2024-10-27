#pragma once
#include "IRequestHandler.h"
#include "Responses.h"
#include "Requsests.h" 

class RemoteControlingRequestHandler : public IRequestHandler
{
private:
    KeyBoardReadingRequest deserializeRequest(const std::vector<char>& buffer);
    std::vector<char> convertToVector(const TurnOfComputerResponse& response);
    IRequestHandler::RequestResult turnOffComputer();
    IRequestHandler::RequestResult readKeyboard(int timeInSeconds);
public:
    RemoteControlingRequestHandler() = default;
    IRequestHandler::RequestResult handleRequest(const RequestInfo& request) override;
    bool isRequestRelevant(const RequestInfo& info) const override;
};
