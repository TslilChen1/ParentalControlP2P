#pragma once
#include <windows.h>

#include "IRequestHandler.h"
#include "Responses.h"
#include "Requests.h" 

#include <iostream>
#include <map> 
#include <cstring> 
#include "Deserializer.h"
#include "BlockWeb.h"

class RemoteControlingRequestHandler : public IRequestHandler
{
private:
    std::vector<char> convertToVector(const TurnOfComputerResponse& response);

    IRequestHandler::RequestResult turnOffComputer();
    IRequestHandler::RequestResult readKeyboard(int timeInSeconds);
    IRequestHandler::RequestResult blockWeb(std::string webName);

public:
    RemoteControlingRequestHandler() = default;
    IRequestHandler::RequestResult handleRequest(const RequestInfo& request) override;
    bool isRequestRelevant(const RequestInfo& info) const override;
};
