#pragma once
#include "rsa.h"
#include "KeyUse.h"
#include "PRF.h"

class Communicator;

class DTLS
{
private:
	rsa randomRsa;
	std::pair<KeyUse::Keys, KeyUse::Keys> keys;

	std::vector<uint8_t> generateFinishedHash(uint64_t clientRandom, uint64_t serverRandom);
	bool verifyFinishedMessage(const std::vector<char>& finishedMsg, uint64_t clientRandom, uint64_t serverRandom);

	std::pair<uint64_t, uint64_t> serverHandshake(Communicator& comm);// return the random number of client and server. client->first, server->second

	uint64_t sendPremasterToClient(Communicator& comm);

	std::pair< KeyUse::Keys, KeyUse::Keys> generateKeyDerivation(uint64_t premasterSecret, uint64_t clientRandom, uint64_t serverRandom);

	

	static uint32_t generateRandomUint32();

public:
	bool runChildDTLS(Communicator& comm); // run handshake and certifcate

	std::vector<char> encryptMessege(int role, std::string msg) const;
	std::string decryptMeessege(int role, std::vector<char> encryptedMsg, int bytesRead) const;
	void printKeyMaterial();

};

