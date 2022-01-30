#include <fstream>
#include <iostream>
#include <string>
#include <vector>

class BMPFileHeader {
   public:
    BMPFileHeader(){};
    BMPFileHeader(std::ifstream &);
    void print();

    uint8_t type[2];
    uint32_t file_size;
    uint16_t reserved1;
    uint16_t reserved2;
    uint32_t data_start;
};

BMPFileHeader::BMPFileHeader(std::ifstream &stream) {
    stream.read(reinterpret_cast<char *>(&type), 2);
    stream.read(reinterpret_cast<char *>(&file_size), 4);
    stream.read(reinterpret_cast<char *>(&reserved1), 2);
    stream.read(reinterpret_cast<char *>(&reserved2), 2);
    stream.read(reinterpret_cast<char *>(&data_start), 4);
}

void BMPFileHeader::print() {
    std::cout << "type: " << type[0] << type[1] << '\n';
    std::cout << "file size: " << file_size << '\n';
    std::cout << "reserved 1: " << reserved1 << '\n';
    std::cout << "reserved 2: " << reserved2 << '\n';
    std::cout << "data start: " << data_start << '\n';
}

class BMPImageHeader {
   public:
    BMPImageHeader(){};
    BMPImageHeader(std::ifstream &);
    void print();

    uint32_t header_size;
    uint32_t width;
    uint32_t height;
    uint16_t planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t image_size;
    uint32_t resolution_h;
    uint32_t resolution_v;
    uint32_t colors;
    uint32_t important_colors;
};

BMPImageHeader::BMPImageHeader(std::ifstream &stream) {
    stream.read(reinterpret_cast<char *>(&header_size), 4);
    stream.read(reinterpret_cast<char *>(&width), 4);
    stream.read(reinterpret_cast<char *>(&height), 4);
    stream.read(reinterpret_cast<char *>(&planes), 2);
    stream.read(reinterpret_cast<char *>(&bits_per_pixel), 2);
    stream.read(reinterpret_cast<char *>(&compression), 4);
    stream.read(reinterpret_cast<char *>(&image_size), 4);
    stream.read(reinterpret_cast<char *>(&resolution_h), 4);
    stream.read(reinterpret_cast<char *>(&resolution_v), 4);
    stream.read(reinterpret_cast<char *>(&colors), 4);
    stream.read(reinterpret_cast<char *>(&important_colors), 4);
}

void BMPImageHeader::print() {
    std::cout << "header size: " << header_size << '\n';
    std::cout << "width: " << width << '\n';
    std::cout << "height: " << height << '\n';
    std::cout << "planes: " << planes << '\n';
    std::cout << "bits per pixel: " << bits_per_pixel << '\n';
    std::cout << "compression: " << compression << '\n';
    std::cout << "image size: " << image_size << '\n';
    std::cout << "horizontal resolution: " << resolution_h << '\n';
    std::cout << "vertical resolution: " << resolution_v << '\n';
    std::cout << "colors: " << colors << '\n';
    std::cout << "important colors: " << important_colors << '\n';
}

struct Pixel {
    uint8_t b, g, r;
};

class BMPImage {
   public:
    BMPImage(std::ifstream &);
    void write(std::ofstream &);

    BMPFileHeader file_header;
    BMPImageHeader image_header;
    std::vector<std::vector<Pixel>> pixels;

   private:
    uint32_t padding;
};

BMPImage::BMPImage(std::ifstream &stream) {
    file_header = BMPFileHeader(stream);
    image_header = BMPImageHeader(stream);

    padding = (4 - ((image_header.width * sizeof(Pixel)) % 4)) % 4;

    for (uint32_t y = 0; y < image_header.height; y++) {
        pixels.emplace_back();

        for (uint32_t x = 0; x < image_header.width; x++) {
            Pixel pixel;
            stream.read(reinterpret_cast<char *>(&pixel), 3);

            pixels[y].emplace_back(pixel);
        }

        if (padding) {
            stream.seekg(padding, std::ios::cur);
        }
    }

    // TODO: Chunked load
}

void BMPImage::write(std::ofstream &stream) {
    auto fh = file_header;
    stream.write(reinterpret_cast<char *>(&fh.type), 2);
    stream.write(reinterpret_cast<char *>(&fh.file_size), 4);
    stream.write(reinterpret_cast<char *>(&fh.reserved1), 2);
    stream.write(reinterpret_cast<char *>(&fh.reserved2), 2);
    stream.write(reinterpret_cast<char *>(&fh.data_start), 4);

    auto ih = image_header;
    stream.write(reinterpret_cast<char *>(&ih.header_size), 4);
    stream.write(reinterpret_cast<char *>(&ih.width), 4);
    stream.write(reinterpret_cast<char *>(&ih.height), 4);
    stream.write(reinterpret_cast<char *>(&ih.planes), 2);
    stream.write(reinterpret_cast<char *>(&ih.bits_per_pixel), 2);
    stream.write(reinterpret_cast<char *>(&ih.compression), 4);
    stream.write(reinterpret_cast<char *>(&ih.image_size), 4);
    stream.write(reinterpret_cast<char *>(&ih.resolution_h), 4);
    stream.write(reinterpret_cast<char *>(&ih.resolution_v), 4);
    stream.write(reinterpret_cast<char *>(&ih.colors), 4);
    stream.write(reinterpret_cast<char *>(&ih.important_colors), 4);

    for (uint32_t y = 0; y < image_header.height; y++) {
        for (uint32_t x = 0; x < image_header.width; x++) {
            auto pixel = pixels[y][x];

            stream.write(reinterpret_cast<char *>(&pixel), 3);
        }

        stream.write("\0\0\0", padding);
    }
}

int main() {
    std::string file_name;
    std::cout << "Enter a file name: ";
    std::cin >> file_name;

    std::ifstream file(file_name, std::ios::binary);

    auto image = BMPImage(file);

    std::cout << "BMP File Header:\n";
    image.file_header.print();
    std::cout << "\nBMP Image Header:\n";
    image.image_header.print();

    // Modify image
    // for (uint32_t y = 0; y < image.image_header.height; y++) {
    //     for (uint32_t x = 0; x < image.image_header.width; x++) {
    //         // if (x < image.image_header.width / 2) {
    //         //     std::swap(image.pixels[y][x],
    //         //               image.pixels[y][image.image_header.width - x - 1]);
    //         // }
    //         auto pixel = &image.pixels[y][x];
    //         pixel->r = ~pixel->r;
    //         pixel->g = ~pixel->g;
    //         pixel->b = ~pixel->b;
    //     }
    // }

    // Write result
    // std::ofstream out("out.bmp", std::ios::binary);
    // image.write(out);
}
