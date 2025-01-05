#define _CRT_SECURE_NO_WARNINGS

#include "Encoder.h"
#include <fstream>
#include <cstring>

// Helper function to read PNG
void Encoder::readPNG(const std::string& filename, std::vector<uint8_t>& imageData, int& width, int& height) {
    FILE* fp = fopen(filename.c_str(), "rb");
    if (!fp) return;

    png_structp png = png_create_read_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);

    png_init_io(png, fp);
    png_read_info(png, info);

    width = png_get_image_width(png, info);
    height = png_get_image_height(png, info);

    imageData.resize(width * height * 4);
    std::vector<png_bytep> rows(height);

    for (int y = 0; y < height; y++) {
        rows[y] = &imageData[y * width * 4];
    }

    png_read_image(png, rows.data());

    fclose(fp);
    png_destroy_read_struct(&png, &info, NULL);
}

RTPPacket Encoder::encodeImage(const std::string& inputPngPath)
{
    // Read PNG
    std::vector<uint8_t> imageData;
    int width, height;
    readPNG(inputPngPath, imageData, width, height);

    // Setup VPX Encoder
    vpx_codec_ctx_t encoder;
    vpx_codec_enc_cfg_t cfg;
    vpx_codec_enc_config_default(vpx_codec_vp8_cx(), &cfg, 0);

    cfg.g_w = width;
    cfg.g_h = height;
    cfg.rc_target_bitrate = width * height;

    vpx_codec_enc_init(&encoder, vpx_codec_vp8_cx(), &cfg, 0);

    // Prepare image for encoding
    vpx_image_t raw;
    vpx_img_alloc(&raw, VPX_IMG_FMT_I420, width, height, 1);

    // Convert RGBA to YUV
    for (int y = 0; y < height; y++) {
        for (int x = 0; x < width; x++) {
            int rgbIndex = (y * width + x) * 4;
            int yIndex = y * raw.stride[0] + x;
            int uvIndex = (y / 2) * raw.stride[1] + (x / 2);

            uint8_t r = imageData[rgbIndex];
            uint8_t g = imageData[rgbIndex + 1];
            uint8_t b = imageData[rgbIndex + 2];

            // RGB to Y
            raw.planes[0][yIndex] = ((66 * r + 129 * g + 25 * b + 128) >> 8) + 16;

            // RGB to UV (every 2x2 block)
            if (x % 2 == 0 && y % 2 == 0) {
                raw.planes[1][uvIndex] = ((-38 * r - 74 * g + 112 * b + 128) >> 8) + 128;
                raw.planes[2][uvIndex] = ((112 * r - 94 * g - 18 * b + 128) >> 8) + 128;
            }
        }
    }

    // Encode
    vpx_codec_encode(&encoder, &raw, 0, 1, 0, VPX_DL_GOOD_QUALITY);

    vpx_codec_iter_t iter = NULL;
    const vpx_codec_cx_pkt_t* pkt;
    RTPPacket rtpPacket;

    // Set RTP header fields
    rtpPacket.version = 2;  // RTP version is 2
    rtpPacket.padding = 0;
    rtpPacket.extension = 0;
    rtpPacket.csrcCount = 0;
    rtpPacket.marker = 1;
    rtpPacket.payloadType = 96;  // Dynamic payload type for VP8 (example)
    rtpPacket.sequenceNumber = 1; // Example sequence number
    rtpPacket.timestamp = 123456; // Example timestamp
    rtpPacket.ssrc = 12345; // Example SSRC

    while ((pkt = vpx_codec_get_cx_data(&encoder, &iter))) {
        if (pkt->kind == VPX_CODEC_CX_FRAME_PKT) {
            rtpPacket.payloadSize = pkt->data.frame.sz;
            rtpPacket.payload = new uint8_t[rtpPacket.payloadSize];
            memcpy(rtpPacket.payload, pkt->data.frame.buf, rtpPacket.payloadSize);
            break;
        }
    }

    // Cleanup
    vpx_img_free(&raw);
    vpx_codec_destroy(&encoder);

    return rtpPacket;
}
