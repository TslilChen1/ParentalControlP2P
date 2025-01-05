#include <string>
#include <vector>
#include <png.h>
#include <vpx/vpx_encoder.h>
#include <vpx/vpx_codec.h>
#include <vpx/vp8cx.h>
#include "Comunication.h"

class Encoder {

public:
    static RTPPacket encodeImage(const std::string& inputPngPath);
private:
    static void readPNG(const std::string& filename, std::vector<uint8_t>& imageData, int& width, int& height);
};