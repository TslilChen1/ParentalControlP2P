#define _CRT_SECURE_NO_WARNINGS

#include "Decoder.h"
#include <fstream>
#include <cstring>
#include "Comunication.h"
#include <vpx/vpx_decoder.h>
#include <iostream>
#include "LockImage.h"

std::mutex Decoder::imageMutex;

// Helper function to clamp values between min and max
inline int clamp(int value, int min, int max) {
    if (value < min) return min;  // If value is less than min, return min
    if (value > max) return max;  // If value is greater than max, return max
    return value;                 // Otherwise, return the value itself
}


void Decoder::decodeImage(const RTPPacket& rtpPacket, const std::string& outputPngPath)
{
    // Setup VPX Decoder
    vpx_codec_ctx_t decoder;
    vpx_codec_dec_cfg_t cfg = { 0 };
    cfg.threads = 1;

    vpx_codec_dec_init(&decoder, vpx_codec_vp8_dx(), &cfg, 0);

    // Decode
    vpx_codec_decode(&decoder, rtpPacket.payload, rtpPacket.payloadSize, NULL, 0);

    vpx_codec_iter_t iter = NULL;
    vpx_image_t* img = vpx_codec_get_frame(&decoder, &iter);

    if (!img) {
        //std::cerr << "Failed to decode frame" << std::endl;
        return;
    }

    //std::lock_guard<std::mutex> lock(imageMutex);

    // Prepare PNG
    //LockImage::lockFile(L"output.png");
    FILE* fp = fopen(outputPngPath.c_str(), "wb");
    if (!fp) return;

    png_structp png = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    png_infop info = png_create_info_struct(png);

    png_init_io(png, fp);
    png_set_IHDR(png, info, img->d_w, img->d_h, 8, PNG_COLOR_TYPE_RGBA,
        PNG_INTERLACE_NONE, PNG_COMPRESSION_TYPE_DEFAULT, PNG_FILTER_TYPE_DEFAULT);

    // Convert YUV to RGBA
    std::vector<uint8_t> rgbaData(img->d_w * img->d_h * 4);
    std::vector<png_bytep> rows(img->d_h);

    for (unsigned int y = 0; y < img->d_h; y++) {
        for (unsigned int x = 0; x < img->d_w; x++) {
            int pixelIndex = (y * img->d_w + x) * 4;
            int yIndex = y * img->stride[0] + x;
            int uvIndex = (y / 2) * img->stride[1] + (x / 2);

            int Y = img->planes[0][yIndex];
            int U = img->planes[1][uvIndex];
            int V = img->planes[2][uvIndex];

            // YUV to RGB conversion
            int C = Y - 16;
            int D = U - 128;
            int E = V - 128;

            rgbaData[pixelIndex + 0] = clamp((298 * C + 409 * E + 128) >> 8, 0, 255); // R
            rgbaData[pixelIndex + 1] = clamp((298 * C - 100 * D - 208 * E + 128) >> 8, 0, 255); // G
            rgbaData[pixelIndex + 2] = clamp((298 * C + 516 * D + 128) >> 8, 0, 255); // B
            rgbaData[pixelIndex + 3] = 255; // Alpha
        }
        rows[y] = &rgbaData[y * img->d_w * 4];
    }

    png_write_info(png, info);
    png_write_image(png, rows.data());
    png_write_end(png, NULL);

    // Cleanup
    fclose(fp);


    //LockImage::unlockFile(L"output.png");

    png_destroy_write_struct(&png, &info);
    vpx_codec_destroy(&decoder);
}
