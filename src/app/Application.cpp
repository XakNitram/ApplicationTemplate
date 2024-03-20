#include "pch.hpp"
import Window;
import Structures;

// 3D space for lighting calculations. For calculating the light on the vertical wall textures,
//   calculate the floor light at 1/4 the vertical resolution, and the wall light at 3/4 the vertical resolution.

// We are attempting to find new ways to lay out our code that provide the greatest flexibility.
// Code layout concepts:
// . Callbacks
// . Information hiding
// . Encapsulation

// GLFW has callbacks.
// . Pull the events out of a general callback
// . Pass a global state object to a customized callback.
// . Basically, how do we subvert callbacks?
//   . Push events to Lua, handle them there.
//   . What is a callback, fundamentally?
//     . A way for libraries to allow arbitrary code execution given a standard interface.

// Parameters have to be added to every function, but passing structs instead of parameters means only one place has to change.
// . However, are there downsides to passing structs around?
//   . Members of a struct have to initialized same as parameters.
// . Structs can accept arbitrary data that isn't necessarily needed in the function.
//   . Good way to abstract across Vulkan/DX12/OpenGL?
//     . Perhaps for function calls, but doesn't answer how we would handle objects and memory there.

// I'm not sure ECS is the optimal way to go. Having all the transforms in a scene being together is beneficial for iteration, but we don't need an array of damage value components.
//   Instead of trying to find a way to relate values to justify a component, random values should be in some kind of lru cache.
// . However, great care needs to be taken here to ensure gameplay development is easy. ECS is a workflow unity and unreal devs are familiar with.
//   . What's the problem?
//   . Too much focus on deconstructing game objects into individual variables will probably lead to us being unable to reason about the system or how to change it.

// std::expected is nice. Simplifies our error returns and allows for situations without simple invalid values. For those cases right now we can use std::optional, which has the same funky value-extraction syntax.

// Good idea: You can send your own messages in Stalker chat, maybe someone can respond.


struct Vertex {
    float x = 0.0f;
    float y = 0.0f;

    //float u = 0.0f;
    //float v = 0.0f;

    //float r = 0.0;
    //float g = 0.39215686274509803f;
    //float b = 0.0;
};


struct Cell {
    Vertex bs{};
    Vertex be{};
    Vertex te{};
    Vertex ts{};
};


struct TexCoord {
    float u = 0.0f;
    float v = 0.0f;
};


struct TexCoordCell {
    TexCoord bs{};
    TexCoord be{};
    TexCoord te{};
    TexCoord ts{};
};


struct SpriteVertex {
    float x{0.0f};
    float y{0.0f};

    float u{0.0f};
    float v{0.0f};
};


struct SpriteCell {
    SpriteVertex bs{};
    SpriteVertex be{};
    SpriteVertex te{};
    SpriteVertex ts{};
};


struct SquareIndices {
    uint32_t a{0};
    uint32_t b{0};
    uint32_t c{0};
    uint32_t d{0};
    uint32_t e{0};
    uint32_t f{0};
};


struct Miner {
    uint32_t id = 0;
    uint32_t index = 0;
    float x{0.0f};
    float y{0.0f};
};


struct MinerCell {
    SpriteVertex bs{};
    SpriteVertex be{};
    SpriteVertex te{};
    SpriteVertex te2{};
    SpriteVertex ts{};
    SpriteVertex bs2{};

    MinerCell(const SpriteVertex n_bs, const SpriteVertex n_be, const SpriteVertex n_te, const SpriteVertex n_ts) :
        bs(n_bs), be(n_be), te(n_te), te2(n_te), ts(n_ts), bs2(n_bs) {}

    MinerCell() = default;
};


constexpr ptrdiff_t BOARD_SIZE = 16;
constexpr ptrdiff_t GRID_SIZE = BOARD_SIZE + 1;
using Mat4 = std::array<float, 16>;
using Board = std::array<uint32_t, BOARD_SIZE * BOARD_SIZE>;
using GridModel = std::array<Cell, GRID_SIZE * GRID_SIZE>;
using GridIndices = std::array<uint32_t, GRID_SIZE * GRID_SIZE * 6>;
// Index buffer matches or cuts down on the upload size of a vertex draw for any vertex dimension greater than 2.
using GridTexCoords = std::array<TexCoordCell, GRID_SIZE * GRID_SIZE>;
using TextureMap = std::array<TexCoordCell, 16>;

constexpr GridModel grid_model_generate() {
    constexpr float size_fraction = 1.0f / static_cast<float>(GRID_SIZE);
    GridModel grid_model{};

    for (std::ptrdiff_t i = 0; i < GRID_SIZE; ++i) {
        for (std::ptrdiff_t j = 0; j < GRID_SIZE; ++j) {
            const std::ptrdiff_t index = (i * GRID_SIZE + j);
            auto &[bs, be, te, ts] = grid_model[index];
            bs = {
                static_cast<float>(j) * size_fraction - 0.5f, static_cast<float>(i) * size_fraction - 0.5f,
                //0.0f, 0.0f
            };

            be = {
                static_cast<float>(j + 1) * size_fraction - 0.5f, static_cast<float>(i) * size_fraction - 0.5f,
                //1.0f, 0.0f
            };

            te = {
                static_cast<float>(j + 1) * size_fraction - 0.5f, static_cast<float>(i + 1) * size_fraction - 0.5f,
                //1.0f, 1.0f
            };

            ts = {
                static_cast<float>(j) * size_fraction - 0.5f, static_cast<float>(i + 1) * size_fraction - 0.5f,
                //0.0f, 1.0f
            };
        }
    }

    return grid_model;
}

constexpr GridIndices grid_indices_generate() {
    GridIndices grid_indices{};

    for (std::ptrdiff_t i = 0; i < GRID_SIZE; ++i) {
        for (std::ptrdiff_t j = 0; j < GRID_SIZE; ++j) {
            // now we need 0, 1, 2, 2, 3, 0 instead of 0, 1, 6, 6, 5, 0
            //   then      4, 5, 6, 6, 7, 4 instead of 1, 2, 7, 7, 6, 1
            const std::ptrdiff_t cell_index = i * GRID_SIZE + j;
            const auto start_index = static_cast<uint32_t>((i * GRID_SIZE + j) * 4);
            uint32_t *cell = std::addressof(grid_indices[cell_index * 6]);
            cell[0] = start_index;
            cell[1] = start_index + 1;
            cell[2] = start_index + 2;
            cell[3] = start_index + 2;
            cell[4] = start_index + 3;
            cell[5] = start_index;
        }
    }

    return grid_indices;
}

constexpr GridModel GRID_MODEL{grid_model_generate()};
constexpr GridIndices GRID_INDICES{grid_indices_generate()};
constexpr uint64_t TEXTURE_COLUMNS = 5;
constexpr uint64_t TEXTURE_ROWS = 5;
constexpr float TCS = 1.0f / static_cast<float>(TEXTURE_COLUMNS);
constexpr float TRS = 1.0f / static_cast<float>(TEXTURE_ROWS);
constexpr TextureMap STONE_TEXTURE_MAP{
    TexCoordCell{{0.0f, TRS * 3},    {TCS, TRS * 3},     {TCS, TRS * 4},     {0.0f, TRS * 4}   }, // 0b0000
    TexCoordCell{{TCS * 2, TRS * 2}, {TCS * 3, TRS * 2}, {TCS * 3, TRS * 3}, {TCS * 2, TRS * 3}}, // 0b0001
    TexCoordCell{{0.0f, TRS * 2},    {TCS, TRS * 2},     {TCS, TRS * 3},     {0.0f, TRS * 3}   }, // 0b0010
    TexCoordCell{{TCS, TRS * 2},     {TCS * 2, TRS * 2}, {TCS * 2, TRS * 3}, {TCS, TRS * 3}    }, // 0b0011
    TexCoordCell{{0.0f, TRS},        {TCS, TRS},         {TCS, TRS * 1},     {0.0f, TRS * 1}   },
 // 0b0100
    TexCoordCell{{TCS * 3, TRS},     {TCS * 4, TRS},     {TCS * 4, TRS * 1}, {TCS * 3, TRS * 1}}, // 0b0101
    TexCoordCell{{0.0f, TRS * 1},    {TCS, TRS * 1},     {TCS, TRS * 2},     {0.0f, TRS * 2}   }, // 0b0110
    TexCoordCell{{TCS * 4, TRS * 1}, {TCS * 5, TRS * 1}, {TCS * 5, TRS * 2}, {TCS * 4, TRS * 2}}, // 0b0111
    TexCoordCell{{TCS * 2, TRS},     {TCS * 3, TRS},     {TCS * 3, TRS * 1}, {TCS * 2, TRS * 1}}, // 0b1000
    TexCoordCell{{TCS * 2, TRS * 1}, {TCS * 3, TRS * 1}, {TCS * 3, TRS * 2}, {TCS * 2, TRS * 2}}, // 0b1001
    TexCoordCell{{TCS * 4, TRS},     {TCS * 5, TRS},     {TCS * 5, TRS * 1}, {TCS * 4, TRS * 1}}, // 0b1010
    TexCoordCell{{TCS * 3, TRS * 1}, {TCS * 4, TRS * 1}, {TCS * 4, TRS * 2}, {TCS * 3, TRS * 2}}, // 0b1011
    TexCoordCell{{TCS, 0.0f},        {TCS * 2, 0.0f},    {TCS * 2, TRS},     {TCS, TRS}        },
 // 0b1100
    TexCoordCell{{TCS * 3, TRS * 2}, {TCS * 4, TRS * 2}, {TCS * 4, TRS * 3}, {TCS * 3, TRS * 3}}, // 0b1101
    TexCoordCell{{TCS * 4, TRS * 2}, {TCS * 5, TRS * 2}, {TCS * 5, TRS * 3}, {TCS * 4, TRS * 3}}, // 0b1110
    TexCoordCell{{TCS, TRS * 1},     {TCS * 2, TRS * 1}, {TCS * 2, TRS * 2}, {TCS, TRS * 2}    }, // 0b1111
};

constexpr std::array<uint8_t, 256> RANDOM_ARRAY{
    56,  133, 226, 14,  244, 249, 104, 48,  248, 16,  230, 96,  202, 0,   153, 224, 240, 109, 63,  234, 173, 74,
    127, 200, 186, 8,   112, 241, 59,  110, 77,  118, 180, 141, 251, 213, 252, 164, 128, 183, 95,  169, 36,  172,
    94,  193, 19,  201, 235, 196, 145, 83,  150, 167, 79,  146, 191, 55,  35,  154, 92,  1,   211, 220, 10,  117,
    143, 136, 129, 181, 51,  120, 53,  28,  88,  242, 106, 76,  208, 195, 123, 165, 205, 149, 4,   12,  91,  237,
    184, 250, 218, 69,  45,  115, 103, 34,  81,  102, 39,  50,  68,  214, 90,  15,  5,   215, 49,  24,  84,  178,
    47,  54,  177, 229, 46,  204, 121, 107, 42,  99,  245, 219, 29,  174, 58,  111, 247, 37,  156, 217, 71,  210,
    187, 70,  119, 185, 32,  227, 179, 231, 116, 98,  78,  11,  100, 17,  168, 188, 159, 52,  131, 64,  89,  192,
    158, 2,   22,  43,  144, 232, 72,  253, 151, 87,  135, 114, 61,  23,  166, 170, 132, 85,  13,  38,  93,  65,
    18,  194, 25,  140, 134, 225, 161, 125, 21,  62,  206, 223, 41,  171, 101, 3,   57,  147, 182, 137, 152, 139,
    197, 7,   189, 209, 122, 20,  212, 105, 203, 198, 130, 97,  175, 31,  82,  246, 155, 190, 124, 86,  108, 33,
    254, 6,   73,  80,  255, 148, 160, 142, 40,  176, 216, 207, 44,  238, 9,   163, 228, 199, 30,  66,  67,  221,
    26,  126, 138, 233, 27,  113, 157, 236, 239, 162, 243, 60,  75,  222
};

uint8_t doom_random() {
    static uint32_t index = 0;
    return RANDOM_ARRAY[index++ & 0xff];
}


void compile_shader_from_file(const lwvl::Shader &_shader, const std::string &path) {
    const GLuint shader{_shader};
    const std::string source{[](const std::string &i_path) {
        std::ifstream file(i_path);
        std::stringstream output_stream;

        std::string line;
        while (getline(file, line)) {
            output_stream << line << '\n';
        }
        return output_stream.str();
    }(path)};

    const char *src = source.c_str();
    glShaderSource(shader, 1, &src, nullptr);
    glCompileShader(shader);

    if (const int result{[](const GLuint i_shader) {
            int temp;
            glGetShaderiv(i_shader, GL_COMPILE_STATUS, &temp);
            return temp;
        }(shader)};
        result == GL_FALSE) {
        int length;
        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &length);
        const auto log = std::make_unique<char[]>(length);
        glGetShaderInfoLog(shader, length, &length, log.get());
        std::stringstream error;
        error << "Failed to compile shader:\n" << log.get() << '\n';
        throw std::invalid_argument(error.str().c_str());
    }
}


static void stbi_vertical_flip(void *image, const int64_t w, const int64_t h, const int64_t bytes_per_pixel) {
    const int64_t bytes_per_row = w * bytes_per_pixel;
    auto *bytes = static_cast<unsigned char *>(image);

    for (int64_t row = 0; row < h >> 1; row++) {
        unsigned char *row0 = bytes + row * bytes_per_row;
        unsigned char *row1 = bytes + (h - row - 1) * bytes_per_row;
        // swap row0 with row1
        size_t bytes_left = bytes_per_row;
        while (bytes_left) {
            unsigned char temp[2048];
            const size_t bytes_copy = bytes_left < sizeof(temp) ? bytes_left : sizeof(temp);
            memcpy(temp, row0, bytes_copy);
            memcpy(row0, row1, bytes_copy);
            memcpy(row1, temp, bytes_copy);
            row0 += bytes_copy;
            row1 += bytes_copy;
            bytes_left -= bytes_copy;
        }
    }
}


uint8_t select_texture(const std::ptrdiff_t i, const std::ptrdiff_t j, const uint32_t *board) {
    constexpr std::ptrdiff_t grid_limit = GRID_SIZE - 1;

    // 0b1 - bottom left, 0b10 - bottom right, 0b100 - top right, 0b1000 - top left
    // follows standard triangle vertex direction
    uint8_t texture_selector{0};

    //texture_selector |= (i == 0 || j == 0 ? 0x1 : (board[(i - 1) * BOARD_SIZE + (j - 1)] > 0));
    if (i == 0 || j == 0) {
        texture_selector |= 0x1;
    } else {
        texture_selector |= board[(i - 1) * BOARD_SIZE + (j - 1)] > 0;
    }

    //texture_selector |= (i == 0 || j == grid_limit ? 0x2 : (board[(i - 1) * BOARD_SIZE + j] > 0) << 1);
    if (i == 0 || j == grid_limit) {
        texture_selector |= 0x2;
    } else {
        texture_selector |= (board[(i - 1) * BOARD_SIZE + j] > 0) << 1;
    }

    //texture_selector |= (i == grid_limit || j == grid_limit ? 0x4 : (board[i * BOARD_SIZE + j] > 0) << 2);
    if (i == grid_limit || j == grid_limit) {
        texture_selector |= 0x4;
    } else {
        texture_selector |= (board[i * BOARD_SIZE + j] > 0) << 2;
    }

    //texture_selector |= (i == grid_limit || j == 0 ? 0x8 : (board[i * BOARD_SIZE + (j - 1)] > 0) << 3);
    if (i == grid_limit || j == 0) {
        texture_selector |= 0x8;
    } else {
        texture_selector |= (board[i * BOARD_SIZE + (j - 1)] > 0) << 3;
    }

    return texture_selector;
}


inline double delta(const std::chrono::time_point<std::chrono::steady_clock> start) {
    const auto diff = static_cast<double>(
        std::chrono::duration_cast<std::chrono::microseconds>(std::chrono::high_resolution_clock::now() - start).count()
    );
    return 0.000001 * diff;
}


struct GridRenderPipeline {
    lwvl::VertexArray vao{};
    lwvl::Buffer vbo{};
    lwvl::Buffer vtcbo{};
    lwvl::Buffer veo{};
};


struct SpriteRenderPipeline {
    lwvl::VertexArray vao{};
    lwvl::Buffer vbo{};
    lwvl::Buffer veo{};
};


struct MinerRenderPipeline {
    lwvl::VertexArray vao{};
    lwvl::Buffer vbo{};
};


int run() {
    glfwSetErrorCallback([](int code, const char *message) { std::cerr << message << '\n'; });

    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW." << '\n';
        return 1;
    }

    core::WindowStatus window_create_status;
    constexpr int window_width{1920};
    constexpr int window_height{1080};
    const core::Window window{core::window_create(window_width, window_height, "Miners", window_create_status)};
    if (window_create_status != core::WindowStatus::Success) {
        std::cerr << "Unable to create GLFW window." << '\n';
        return 1;
    }

    auto board{std::make_unique<Board>()};
    for (std::ptrdiff_t i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) {
        (*board)[i] = 0;
    }

    std::vector<Miner> miners{4};
    for (std::ptrdiff_t i = 0; i < 4; ++i) {
        miners[i].id = i;
        miners[i].index = i;
    }

    std::vector<MinerCell> miner_vertices{};
    miner_vertices.reserve(4);
    constexpr float cell_size{1.0f / static_cast<float>(GRID_SIZE)};
    for (std::ptrdiff_t i = 0; i < 4; ++i) {
        const auto &[id, index, x, y]{miners[i]};
        std::ptrdiff_t n = index % BOARD_SIZE;
        std::ptrdiff_t m = index / BOARD_SIZE;

        const float base_x = (static_cast<float>(n) + 0.5f) * cell_size - 0.5f;
        const float base_y = (static_cast<float>(m) + 0.5f) * cell_size - 0.5f;
        miner_vertices.emplace_back(MinerCell{
            {base_x,             base_y,             0.0f, 0.0f},
            {base_x + cell_size, base_y,             1.0f, 0.0f},
            {base_x + cell_size, base_y + cell_size, 1.0f, 1.0f},
            {base_x,             base_y + cell_size, 0.0f, 1.0f}
        });
    }

    auto grid_tex_coords{std::make_unique<GridTexCoords>()};
    for (std::ptrdiff_t i = 0; i < GRID_SIZE; ++i) {
        for (std::ptrdiff_t j = 0; j < GRID_SIZE; ++j) {
            const uint8_t texture_selector = select_texture(i, j, board->data());
            (*grid_tex_coords)[i * GRID_SIZE + j] = STONE_TEXTURE_MAP[texture_selector];
        }
    }

    std::vector<SpriteCell> sprite_vertices{};
    sprite_vertices.reserve(BOARD_SIZE * BOARD_SIZE);
    std::vector<SquareIndices> sprite_indices{};
    sprite_indices.reserve(BOARD_SIZE * BOARD_SIZE);
    for (std::ptrdiff_t i = 0; i < BOARD_SIZE; ++i) {
        for (std::ptrdiff_t j = 0; j < BOARD_SIZE; ++j) {
            if ((*board)[i * BOARD_SIZE + j] > 0) { continue; }

            if (i == BOARD_SIZE - 1 || (*board)[(i + 1) * BOARD_SIZE + j] > 0) {
                const float base_x = (static_cast<float>(j) + 0.5f) * cell_size - 0.5f;
                const float base_y = (static_cast<float>(i) + 0.5f) * cell_size - 0.5f;
                sprite_vertices.emplace_back(SpriteCell{
                    {base_x,             base_y,             TCS * 3, TRS * 4},
                    {base_x + cell_size, base_y,             TCS * 4, TRS * 4},
                    {base_x + cell_size, base_y + cell_size, TCS * 4, TRS * 5},
                    {base_x,             base_y + cell_size, TCS * 3, TRS * 5}
                });

                const uint32_t base_index{static_cast<uint32_t>(sprite_indices.size()) * 4};
                sprite_indices.emplace_back(SquareIndices{
                    base_index, base_index + 1, base_index + 2, base_index + 2, base_index + 3, base_index
                });
            }
        }
    }

    glDebugMessageCallback(
        []([[maybe_unused]] const GLenum source, [[maybe_unused]] const GLenum type, [[maybe_unused]] GLuint id,
           [[maybe_unused]] GLenum severity, [[maybe_unused]] GLsizei length, [[maybe_unused]] const GLchar *message,
           [[maybe_unused]] const void *userParam) {
            auto *stream = &std::cout;
            if (type == GL_DEBUG_TYPE_ERROR) { stream = &std::cerr; }
            *stream << "[OpenGL]" << lwvl::source_to_string(source) << ' ' << message << '\n';
        },
        nullptr
    );
    glEnable(GL_DEBUG_OUTPUT);
    glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

    GLuint unused_ids;
    glDebugMessageControl(GL_DONT_CARE, GL_DONT_CARE, GL_DONT_CARE, 0, &unused_ids, true);


    const GridRenderPipeline grid_pipeline{};

    lwvl::Buffer::const_fill(grid_pipeline.vbo, GRID_MODEL.begin(), GRID_MODEL.end());
    lwvl::Buffer::const_fill(grid_pipeline.veo, GRID_INDICES.begin(), GRID_INDICES.end());
    lwvl::Buffer::const_fill(grid_pipeline.vtcbo, grid_tex_coords->begin(), grid_tex_coords->end());

    lwvl::VertexArray::add_buffer(grid_pipeline.vao, grid_pipeline.vbo, 0, sizeof(Vertex));
    lwvl::VertexArray::add_element_buffer(grid_pipeline.vao, grid_pipeline.veo);
    lwvl::VertexArray::add_attribute(grid_pipeline.vao, 0, 2, GL_FLOAT, offsetof(Vertex, x));
    lwvl::VertexArray::use_binding(grid_pipeline.vao, 0, 0);
    //lwvl::VertexArray::add_attribute(vao, 1, 3, GL_FLOAT, offsetof(Vertex, r));
    //lwvl::VertexArray::add_attribute(vao, 1, 2, GL_FLOAT, offsetof(Vertex, u));
    lwvl::VertexArray::add_buffer(grid_pipeline.vao, grid_pipeline.vtcbo, 1, sizeof(TexCoord));
    lwvl::VertexArray::add_attribute(grid_pipeline.vao, 1, 2, GL_FLOAT, offsetof(TexCoord, u));
    lwvl::VertexArray::use_binding(grid_pipeline.vao, 1, 1);


    const SpriteRenderPipeline sprite_pipeline{};

    lwvl::Buffer::const_fill(sprite_pipeline.vbo, sprite_vertices.begin(), sprite_vertices.end());
    lwvl::Buffer::const_fill(sprite_pipeline.veo, sprite_indices.begin(), sprite_indices.end());

    lwvl::VertexArray::add_buffer(sprite_pipeline.vao, sprite_pipeline.vbo, 0, sizeof(SpriteVertex));
    lwvl::VertexArray::add_element_buffer(sprite_pipeline.vao, sprite_pipeline.veo);
    lwvl::VertexArray::add_attribute(sprite_pipeline.vao, 0, 2, GL_FLOAT, offsetof(SpriteVertex, x));
    lwvl::VertexArray::add_attribute(sprite_pipeline.vao, 1, 2, GL_FLOAT, offsetof(SpriteVertex, u));
    lwvl::VertexArray::use_binding(sprite_pipeline.vao, 0, 0);
    lwvl::VertexArray::use_binding(sprite_pipeline.vao, 0, 1);


    const MinerRenderPipeline miner_pipeline{};

    lwvl::Buffer::const_fill(miner_pipeline.vbo, miner_vertices.begin(), miner_vertices.end());

    lwvl::VertexArray::add_buffer(miner_pipeline.vao, miner_pipeline.vbo, 0, sizeof(SpriteVertex));
    lwvl::VertexArray::add_attribute(miner_pipeline.vao, 0, 2, GL_FLOAT, offsetof(SpriteVertex, x));
    lwvl::VertexArray::add_attribute(miner_pipeline.vao, 1, 2, GL_FLOAT, offsetof(SpriteVertex, u));
    lwvl::VertexArray::use_binding(miner_pipeline.vao, 0, 0);
    lwvl::VertexArray::use_binding(miner_pipeline.vao, 0, 1);

    //const MinerRenderPipeline miner_pipeline {};
    //lwvl::buffer_const_fill(miner_pipeline.vbo);

    lwvl::Texture vto{GL_TEXTURE_2D};
    {
        stbi_set_flip_vertically_on_load(true);
        int width, height, nr_channels;
        unsigned char *data = stbi_load("Data/Textures/dungeon-tiles.png", &width, &height, &nr_channels, 0);
        if (!data) {
            std::cout << "Failed to load Data/Textures/dungeon-tiles.png." << '\n';
            stbi_image_free(data);
            return 1;
        }

        glTextureParameteri(static_cast<GLuint>(vto), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(static_cast<GLuint>(vto), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(static_cast<GLuint>(vto), GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(static_cast<GLuint>(vto), GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureStorage2D(static_cast<GLuint>(vto), 1, GL_RGBA8, width, height);
        glTextureSubImage2D(static_cast<GLuint>(vto), 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    lwvl::Texture miner_texture{GL_TEXTURE_2D};
    {
        // GLFW expects the image to be loaded upside down.
        stbi_set_flip_vertically_on_load(false);
        int width, height, nr_channels;
        unsigned char *data = stbi_load("Data/Textures/pickaxe.png", &width, &height, &nr_channels, 0);
        if (!data) {
            std::cout << "Failed to load Data/Textures/pickaxe.png." << '\n';
            glfwSetWindowIcon(static_cast<GLFWwindow *>(window), 0, nullptr);
            return 1;
        }

        GLFWimage icon{width, height, data};
        glfwSetWindowIcon(static_cast<GLFWwindow *>(window), 1, &icon);

        // Flip for OpenGL
        stbi_vertical_flip(data, width, height, nr_channels);

        glTextureParameteri(static_cast<GLuint>(miner_texture), GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(static_cast<GLuint>(miner_texture), GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(static_cast<GLuint>(miner_texture), GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(static_cast<GLuint>(miner_texture), GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureStorage2D(static_cast<GLuint>(miner_texture), 1, GL_RGBA8, width, height);
        glTextureSubImage2D(
            static_cast<GLuint>(miner_texture), 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data
        );

        stbi_image_free(data);
    }

    constexpr float aspect{static_cast<float>(window_width) / static_cast<float>(window_height)};
    constexpr Mat4 projection{
        1.0f / aspect, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f
    };

    lwvl::Program vso{};
    {
        constexpr GLsizei stage_count{2};
        const lwvl::Shader stages[stage_count]{lwvl::Shader(GL_VERTEX_SHADER), lwvl::Shader(GL_FRAGMENT_SHADER)};

        try {
            compile_shader_from_file(stages[0], "Data/Shaders/default.vert");
            compile_shader_from_file(stages[1], "Data/Shaders/default.frag");
        } catch (const std::exception &e) {
            std::cerr << e.what() << '\n';
            return -1;
        }

        lwvl::Program::LinkStatus status;
        lwvl::Program::link(vso, stages, stage_count, status);
    }

    //lwvl::program_activate(vso);
    const GLint u_projection{glGetUniformLocation(static_cast<GLuint>(vso), "u_projection")};
    if (u_projection < 0) {
        std::cerr << "Unable to find Projection uniform." << '\n';
        return 1;
    }
    glProgramUniformMatrix4fv(static_cast<GLuint>(vso), u_projection, 1, GL_FALSE, projection.data());

    const GLint u_scale{glGetUniformLocation(static_cast<GLuint>(vso), "u_scale")};
    if (u_scale < 0) {
        std::cerr << "Unable to find Scale uniform." << '\n';
        return 1;
    }
    glProgramUniform1f(static_cast<GLuint>(vso), u_scale, 2);

    const GLint u_texture{lwvl::Program::uniform_location(vso, "u_texture")};
    if (u_texture < 0) {
        std::cerr << "Unable to find texture sampler uniform." << '\n';
        return 1;
    }
    glProgramUniform1i(static_cast<GLuint>(vso), u_texture, 0);

    const GLint u_use_texture{lwvl::Program::uniform_location(vso, "u_use_texture")};
    if (u_use_texture < 0) {
        std::cerr << "Unable to find texture sampler uniform." << '\n';
        return 1;
    }
    glProgramUniform1i(static_cast<GLuint>(vso), u_use_texture, 1);

    //const GLint u_time { glGetUniformLocation(static_cast<GLuint>(grid_pipeline.vso), "u_Time") };
    //if (u_time < 0) {
    //    std::cerr << "Unable to find Time uniform." << '\n';
    //    return 1;
    //}
    //glProgramUniform1f(static_cast<GLuint>(grid_pipeline.vso), u_time, 0.0f);

    //const GLint u_resolution { glGetUniformLocation(static_cast<GLuint>(grid_pipeline.vso), "u_Resolution") };
    //if (u_resolution < 0) {
    //    std::cerr << "Unable to find Resolution uniform." << '\n';
    //    return 1;
    //}
    //glProgramUniform2d(static_cast<GLuint>(grid_pipeline.vso), u_resolution, 800.0, 600.0);

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
    glClearColor(0.24314f, 0.25490f, 0.30588f, 1.0f);

    glBindTextureUnit(0, static_cast<GLuint>(vto));
    glBindTextureUnit(1, static_cast<GLuint>(miner_texture));

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    //const auto time_point = std::chrono::high_resolution_clock::now();
    while (!core::window_should_close(window)) {
        core::glfw_update();
        core::window_clear();
        lwvl::Program::activate(vso);
        //glProgramUniform1f(static_cast<GLuint>(grid_pipeline.vso), u_time, float(delta(time_point)));

        glProgramUniform1i(static_cast<GLuint>(vso), u_texture, 0);
        lwvl::VertexArray::activate(grid_pipeline.vao);
        lwvl::draw_elements(GL_TRIANGLES, GRID_SIZE * GRID_SIZE * 6, GL_UNSIGNED_INT);

        lwvl::VertexArray::activate(sprite_pipeline.vao);
        lwvl::draw_elements(GL_TRIANGLES, static_cast<GLsizei>(sprite_indices.size() * 6), GL_UNSIGNED_INT);

        glProgramUniform1i(static_cast<GLuint>(vso), u_texture, 1);
        lwvl::VertexArray::activate(miner_pipeline.vao);
        lwvl::draw_arrays(GL_TRIANGLES, static_cast<GLsizei>(miner_vertices.size() * 2));

        // lwvl::VertexArray::activate(grid_pipeline.vao);
        // glProgramUniform1i(static_cast<GLuint>(vso), u_use_texture, 0);
        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        // lwvl::draw_elements(GL_TRIANGLES, GRID_SIZE * GRID_SIZE * 6, GL_UNSIGNED_INT);
        // glProgramUniform1i(static_cast<GLuint>(vso), u_use_texture, 1);
        // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

        //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        //glProgramUniform1i(static_cast<GLuint>(grid_pipeline.vso), u_use_texture, 0);
        //lwvl::draw_elements(GL_TRIANGLES, GRID_SIZE * GRID_SIZE * 6, GL_UNSIGNED_INT);
        //glProgramUniform1i(static_cast<GLuint>(grid_pipeline.vso), u_use_texture, 1);
        //glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );
        lwvl::Program::clear();
        core::window_swap_buffers(window);
    }

    return 0;
}


int main() { return run(); }
