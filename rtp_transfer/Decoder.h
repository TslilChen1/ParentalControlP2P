#ifndef DECODER_H
#define DECODER_H

#include <string>
#include <vector>
#include <vpx/vpx_decoder.h>
#include <vpx/vp8dx.h>
#include <png.h>
#include "Comunication.h"
#include <mutex>

class Decoder {
private:
    static std::mutex imageMutex;
public:
    static void decodeImage(const RTPPacket& rtpPacket, const std::string& outputPngPath);
};

#endif // DECODER_H
