#include <opencv2/opencv.hpp>
#include "texturize.h"

int main(int argc, char** argv) {
    if (argc != 5) {
        std::cerr << "Usage: " << argv[0] << " <input_image> <output_image> <output_width> <output_height>" << std::endl;
        return -1;
    }

    std::string input_image_path = argv[1];
    std::string output_image_path = argv[2];
    int output_width = std::stoi(argv[3]);
    int output_height = std::stoi(argv[4]);

    cv::Mat input_image = cv::imread(input_image_path, cv::IMREAD_COLOR);
    if (input_image.empty()) {
        std::cerr << "Error: Could not open or find the image!" << std::endl;
        return -1;
    }

    cv::Mat output_image(output_height, output_width, input_image.type());

    // Initialize the filled matrix
    unsigned char** filled = init_guchar_tab_2d(output_width, output_height);

    // Initialize the patch and image buffers
    int channels = input_image.channels();
    unsigned char* patch = input_image.data;
    unsigned char* image = output_image.data;

    // Initialize the coupe buffers
    unsigned char* coupe_h_here = new unsigned char[output_width * output_height * channels];
    unsigned char* coupe_h_west = new unsigned char[output_width * output_height * channels];
    unsigned char* coupe_v_here = new unsigned char[output_width * output_height * channels];
    unsigned char* coupe_v_north = new unsigned char[output_width * output_height * channels];

    // Initialize the first patch
    for (int x = 0; x < input_image.cols; ++x) {
        for (int y = 0; y < input_image.rows; ++y) {
            for (int c = 0; c < channels; ++c) {
                image[(y * output_width + x) * channels + c] = patch[(y * input_image.cols + x) * channels + c];
            }
            filled[x][y] = 1;
        }
    }

    // Main texture synthesis loop
    int cur_posn[2] = {0, 0};
    int patch_posn[2];
    while (count_filled_pixels(filled, output_width, output_height) < output_width * output_height) {
        if (pixel_to_fill(filled, output_width, output_height, cur_posn) == NULL) {
            std::cerr << "Error: Could not find the next pixel to fill!" << std::endl;
            return -1;
        }

        offset_optimal(patch_posn, image, patch, input_image.cols, input_image.rows, output_width, output_height,
                       cur_posn[0] - 10, cur_posn[1] - 10, cur_posn[0] + 10, cur_posn[1] + 10, channels, filled, false);

        decoupe_graphe(patch_posn, output_width, output_height, input_image.cols, input_image.rows, channels, filled,
                       image, patch, coupe_h_here, coupe_h_west, coupe_v_here, coupe_v_north, false, false);
    }

    // Save the output image
    cv::imwrite(output_image_path, output_image);

    // Clean up
    delete[] coupe_h_here;
    delete[] coupe_h_west;
    delete[] coupe_v_here;
    delete[] coupe_v_north;
    for (int i = 0; i < output_width; ++i) {
        delete[] filled[i];
    }
    delete[] filled;

    return 0;
}