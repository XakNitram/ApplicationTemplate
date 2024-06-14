#include "pch.hpp"
import Window;
import SizedArray;
import Array;
import Tether;

// Learn data structures

namespace sc {
    using namespace std::chrono;
    using steady_point = time_point<steady_clock>;
}

using Tick = uint64_t;


struct Vertex {
    float x = 0.0f;
    float y = 0.0f;

    //float u = 0.0f;
    //float v = 0.0f;

    //float r = 0.0f;
    //float g = 0.39215686274509803f;
    //float b = 0.0f;
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


constexpr ptrdiff_t MINER_COUNT = 4;
constexpr ptrdiff_t BOARD_SIZE = 16;
static_assert(MINER_COUNT <= BOARD_SIZE * BOARD_SIZE);  // Remove if we implement board tiling.
constexpr ptrdiff_t GRID_SIZE = BOARD_SIZE + 1;
using BoardCell = uint32_t;
using Mat4 = StaticArray<float, 16>;
using Board = SizedArray<BoardCell, BOARD_SIZE * BOARD_SIZE>;
using GridModel = StaticArray<Cell, GRID_SIZE * GRID_SIZE>;
using GridIndices = StaticArray<uint32_t, GRID_SIZE * GRID_SIZE * 6>;
// Index buffer matches or cuts down on the upload size of a vertex draw for any vertex dimension greater than 2.
using GridTexCoords = SizedArray<TexCoordCell, GRID_SIZE * GRID_SIZE>;
using TextureMap = StaticArray<TexCoordCell, 16>;

struct Miner {
    enum class Animation {
        None = 0,
        Move,
        Mine
    };

    uint32_t id = 0;
    int64_t index = 0;
    Tick next_move = 200;
    bool in_animation {false};
    Animation animation {};
};


struct MoveAnimation {
    float start_x {0.0f};
    float start_y {0.0f};
    float end_x {0.0f};
    float end_y {0.0f};
    sc::steady_point start_time { sc::steady_clock::now() };
    sc::steady_point end_time { sc::steady_clock::now() };
};


struct MineAnimation {
    std::ptrdiff_t target {0};
    sc::steady_point start_time { sc::steady_clock::now() };
    sc::steady_point end_time { sc::steady_clock::now() };
};

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
    TexCoordCell{{0.0f, 0.0f}, {0.2f, 0.0f}, {0.2f, 0.2f}, {0.0f, 0.2f}},
    TexCoordCell{{0.4f, 0.4f}, {0.6f, 0.4f}, {0.6f, 0.6f}, {0.4f, 0.6f}},
    TexCoordCell{{0.2f, 0.4f}, {0.4f, 0.4f}, {0.4f, 0.6f}, {0.2f, 0.6f}},
    TexCoordCell{{0.0f, 0.2f}, {0.2f, 0.2f}, {0.2f, 0.4f}, {0.0f, 0.4f}},
    TexCoordCell{{0.4f, 0.2f}, {0.6f, 0.2f}, {0.6f, 0.4f}, {0.4f, 0.4f}},
    TexCoordCell{{0.0f, 0.6f}, {0.2f, 0.6f}, {0.2f, 0.8f}, {0.0f, 0.8f}},
    TexCoordCell{{0.6f, 0.4f}, {0.8f, 0.4f}, {0.8f, 0.6f}, {0.6f, 0.6f}},
    TexCoordCell{{0.4f, 0.6f}, {0.6f, 0.6f}, {0.6f, 0.8f}, {0.4f, 0.8f}},
    TexCoordCell{{0.2f, 0.2f}, {0.4f, 0.2f}, {0.4f, 0.4f}, {0.2f, 0.4f}},
    TexCoordCell{{0.0f, 0.4f}, {0.2f, 0.4f}, {0.2f, 0.6f}, {0.0f, 0.6f}},
    TexCoordCell{{0.6f, 0.6f}, {0.8f, 0.6f}, {0.8f, 0.8f}, {0.6f, 0.8f}},
    TexCoordCell{{0.2f, 0.6f}, {0.4f, 0.6f}, {0.4f, 0.8f}, {0.2f, 0.8f}},
    TexCoordCell{{0.6f, 0.2f}, {0.8f, 0.2f}, {0.8f, 0.4f}, {0.6f, 0.4f}},
    TexCoordCell{{0.4f, 0.0f}, {0.6f, 0.0f}, {0.6f, 0.2f}, {0.4f, 0.2f}},
    TexCoordCell{{0.2f, 0.0f}, {0.4f, 0.0f}, {0.4f, 0.2f}, {0.2f, 0.2f}},
    TexCoordCell{{0.6f, 0.0f}, {0.8f, 0.0f}, {0.8f, 0.2f}, {0.6f, 0.2f}},
};

constexpr StaticArray<uint8_t, 256> RANDOM_ARRAY {
    56, 133, 226, 14, 244, 249, 104, 48, 248, 16, 230, 96, 202, 0, 153, 224, 240, 109, 63, 234, 173, 74,
    127, 200, 186, 8, 112, 241, 59, 110, 77, 118, 180, 141, 251, 213, 252, 164, 128, 183, 95, 169, 36, 172,
    94, 193, 19, 201, 235, 196, 145, 83, 150, 167, 79, 146, 191, 55, 35, 154, 92, 1, 211, 220, 10, 117,
    143, 136, 129, 181, 51, 120, 53, 28, 88, 242, 106, 76, 208, 195, 123, 165, 205, 149, 4, 12, 91, 237,
    184, 250, 218, 69, 45, 115, 103, 34, 81, 102, 39, 50, 68, 214, 90, 15, 5, 215, 49, 24, 84, 178,
    47, 54, 177, 229, 46, 204, 121, 107, 42, 99, 245, 219, 29, 174, 58, 111, 247, 37, 156, 217, 71, 210,
    187, 70, 119, 185, 32, 227, 179, 231, 116, 98, 78, 11, 100, 17, 168, 188, 159, 52, 131, 64, 89, 192,
    158, 2, 22, 43, 144, 232, 72, 253, 151, 87, 135, 114, 61, 23, 166, 170, 132, 85, 13, 38, 93, 65,
    18, 194, 25, 140, 134, 225, 161, 125, 21, 62, 206, 223, 41, 171, 101, 3, 57, 147, 182, 137, 152, 139,
    197, 7, 189, 209, 122, 20, 212, 105, 203, 198, 130, 97, 175, 31, 82, 246, 155, 190, 124, 86, 108, 33,
    254, 6, 73, 80, 255, 148, 160, 142, 40, 176, 216, 207, 44, 238, 9, 163, 228, 199, 30, 66, 67, 221,
    26, 126, 138, 233, 27, 113, 157, 236, 239, 162, 243, 60, 75, 222
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
        while (getline(file, line)) { output_stream << line << '\n'; }
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


uint8_t select_texture(const std::ptrdiff_t i, const std::ptrdiff_t j, const Tether<BoardCell> board) {
    constexpr std::ptrdiff_t grid_limit = GRID_SIZE - 1;

    // 0b1 - bottom left, 0b10 - bottom right, 0b100 - top left, 0b1000 - top right
    // follows standard triangle vertex direction
    const auto x = (i == 0 || j == 0 ? 0x8 : (board[(i - 1) * BOARD_SIZE + (j - 1)] > 0) << 3)
        + (i == 0 || j == grid_limit ? 0x4 : (board[(i - 1) * BOARD_SIZE + j] > 0) << 2)
        + (i == grid_limit || j == 0 ? 0x2 : (board[i * BOARD_SIZE + (j - 1)] > 0) << 1)
        + (i == grid_limit || j == grid_limit ? 0x1 : (board[i * BOARD_SIZE + j] > 0));
    // std::cout << x << '\n';
    return x;
}


inline double delta(const sc::time_point<sc::steady_clock> start) {
    const auto diff = static_cast<double>(
        sc::duration_cast<sc::nanoseconds>(sc::high_resolution_clock::now() - start).count()
    );
    return 0.000000001 * diff;
}


inline double delta(const sc::time_point<sc::steady_clock> start, const sc::time_point<sc::steady_clock> end) {
    const auto diff = static_cast<double>(
        sc::duration_cast<sc::nanoseconds>(end - start).count()
    );
    return 0.000000001 * diff;
}


inline float lerp(const float s, const float e, const float t) {
    return s + t * (e - s);
}


inline double easeInOut(const double t) {
    return t < 0.5 ? 2.0 * t * t : -1.0 + (4 - 2 * t) * t;
}


struct GridRenderPipeline {
    lwvl::VertexArray vao{};
    lwvl::Buffer vbo{};
    lwvl::Buffer vtcbo{};
    lwvl::Buffer veo{};

    void construct(const GridTexCoords &grid_tex_coords) const {
        lwvl::Buffer::const_fill(vbo, GRID_MODEL.begin(), GRID_MODEL.end());
        lwvl::Buffer::const_fill(veo, GRID_INDICES.begin(), GRID_INDICES.end());
        lwvl::Buffer::const_fill(vtcbo, grid_tex_coords.begin(), grid_tex_coords.end(), GL_CLIENT_STORAGE_BIT | GL_DYNAMIC_STORAGE_BIT);

        lwvl::VertexArray::add_buffer(vao, vbo, 0, sizeof(Vertex));
        lwvl::VertexArray::add_element_buffer(vao, veo);
        lwvl::VertexArray::add_attribute(vao, 0, 2, GL_FLOAT, offsetof(Vertex, x));
        lwvl::VertexArray::use_binding(vao, 0, 0);
        //lwvl::VertexArray::add_attribute(vao, 1, 3, GL_FLOAT, offsetof(Vertex, r));
        //lwvl::VertexArray::add_attribute(vao, 1, 2, GL_FLOAT, offsetof(Vertex, u));
        lwvl::VertexArray::add_buffer(vao, vtcbo, 1, sizeof(TexCoord));
        lwvl::VertexArray::add_attribute(vao, 1, 2, GL_FLOAT, offsetof(TexCoord, u));
        lwvl::VertexArray::use_binding(vao, 1, 1);
    }
};


struct SpriteRenderPipeline {
    lwvl::VertexArray vao{};
    lwvl::Buffer vbo{};
    lwvl::Buffer veo{};

    void construct(const std::vector<SpriteCell> &sprite_vertices, const std::vector<SquareIndices> &sprite_indices) const {
        lwvl::Buffer::const_fill(vbo, sprite_vertices.begin(), sprite_vertices.end(), GL_CLIENT_STORAGE_BIT | GL_DYNAMIC_STORAGE_BIT);
        lwvl::Buffer::const_fill(veo, sprite_indices.begin(), sprite_indices.end(), GL_CLIENT_STORAGE_BIT | GL_DYNAMIC_STORAGE_BIT);

        lwvl::VertexArray::add_buffer(vao, vbo, 0, sizeof(SpriteVertex));
        lwvl::VertexArray::add_element_buffer(vao, veo);
        lwvl::VertexArray::add_attribute(vao, 0, 2, GL_FLOAT, offsetof(SpriteVertex, x));
        lwvl::VertexArray::add_attribute(vao, 1, 2, GL_FLOAT, offsetof(SpriteVertex, u));
        lwvl::VertexArray::use_binding(vao, 0, 0);
        lwvl::VertexArray::use_binding(vao, 0, 1);
    }
};


struct MinerRenderPipeline {
    lwvl::VertexArray vao{};
    lwvl::Buffer vbo{};

    void construct(const std::vector<MinerCell> &miner_vertices) const {
        lwvl::Buffer::const_fill(vbo, miner_vertices.begin(), miner_vertices.end(), GL_CLIENT_STORAGE_BIT | GL_DYNAMIC_STORAGE_BIT);

        lwvl::VertexArray::add_buffer(vao, vbo, 0, sizeof(SpriteVertex));
        lwvl::VertexArray::add_attribute(vao, 0, 2, GL_FLOAT, offsetof(SpriteVertex, x));
        lwvl::VertexArray::add_attribute(vao, 1, 2, GL_FLOAT, offsetof(SpriteVertex, u));
        lwvl::VertexArray::use_binding(vao, 0, 0);
        lwvl::VertexArray::use_binding(vao, 0, 1);
    }
};


int run() {
    core::Window::Status window_create_status;
    constexpr int window_width{1920};
    constexpr int window_height{1080};
    const core::Window window{window_create_status, core::Hints{"Miners", window_width, window_height}};
    {
        using enum core::Window::Status;
        if (window_create_status != Success) {
            if (window_create_status == GLFWUninitialized) { std::cerr << "Unable to initialize GLFW.\n"; } else if (
                window_create_status ==
                ContextCreationFailed) { std::cerr << "Unable to create OpenGL context.\n"; } else {
                std::cerr << "Unable to create window.\n";
            }
            return 1;
        }
    }

    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO &io {ImGui::GetIO()};
    (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;     // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;      // Enable Gamepad Controls

    ImGui::StyleColorsDark();
    // ImGuiStyle &style {ImGui::GetStyle()};
    ImGui_ImplGlfw_InitForOpenGL(static_cast<GLFWwindow*>(window), true);
    ImGui_ImplOpenGL3_Init("#version 460 core");

    bool show_demo_window = false;

    Board board {};
    for (std::ptrdiff_t i = 0; i < BOARD_SIZE * BOARD_SIZE; ++i) { board[i] = 1; }

    DynamicArray<Miner> miners{MINER_COUNT};
    for (std::ptrdiff_t i = 0; i < MINER_COUNT;) {
        constexpr float random_div {1.0f / 255.0f};  // Because 255 is the maximum value. 256 here would never give a value of 1.0.
        const auto index {
            static_cast<int64_t>(
                std::round(
                    static_cast<float>(doom_random())
                    * random_div
                    * static_cast<float>((BOARD_SIZE - 1) * (BOARD_SIZE - 1)))
            )
        };

        // Check other miners so none share a starting location.
        // Need a different method if MINER_COUNT increases too much.
        bool valid_index {true};
        for (std::ptrdiff_t j = 0; j < MINER_COUNT; ++j) {
            if (index == miners[j].index) {
                valid_index = false;
            }
        }

        if (!valid_index) {
            continue;
        }

        board[index] = 0;

        miners[i].id = i;
        miners[i].index = index;
        miners[i].next_move = doom_random();
        ++i;
    }

    DynamicArray<MinerCell> miner_vertices{};
    miner_vertices.reserve(MINER_COUNT);
    constexpr float cell_size{1.0f / static_cast<float>(GRID_SIZE)};
    for (std::ptrdiff_t i = 0; i < MINER_COUNT; ++i) {
        const auto &miner{miners[i]};
        std::ptrdiff_t n = miner.index % BOARD_SIZE;
        std::ptrdiff_t m = miner.index / BOARD_SIZE;

        const float base_x = (static_cast<float>(n) + 0.5f) * cell_size - 0.5f;
        const float base_y = (static_cast<float>(m) + 0.5f) * cell_size - 0.5f;
        miner_vertices.emplace_back(MinerCell{
            {base_x, base_y, 0.0f, 0.0f},
            {base_x + cell_size, base_y, 1.0f, 0.0f},
            {base_x + cell_size, base_y + cell_size, 1.0f, 1.0f},
            {base_x, base_y + cell_size, 0.0f, 1.0f}
        });
    }

    // auto grid_tex_coords{std::make_unique<GridTexCoords>()};
    GridTexCoords grid_tex_coords{};
    for (std::ptrdiff_t i = 0; i < GRID_SIZE; ++i) {
        for (std::ptrdiff_t j = 0; j < GRID_SIZE; ++j) {
            const uint8_t texture_selector = select_texture(i, j, board.tether());
            grid_tex_coords[i * GRID_SIZE + j] = STONE_TEXTURE_MAP[texture_selector];
        }
    }

    DynamicArray<SpriteCell> sprite_vertices{};
    sprite_vertices.reserve(BOARD_SIZE * BOARD_SIZE);
    DynamicArray<SquareIndices> sprite_indices{};
    sprite_indices.reserve(BOARD_SIZE * BOARD_SIZE);
    for (std::ptrdiff_t i = 0; i < BOARD_SIZE; ++i) {
        for (std::ptrdiff_t j = 0; j < BOARD_SIZE; ++j) {
            if (board.at(i * BOARD_SIZE + j) > 0) { continue; }

            if (i == BOARD_SIZE - 1 || board.at((i + 1) * BOARD_SIZE + j) > 0) {
                const float base_x = (static_cast<float>(j) + 0.5f) * cell_size - 0.5f;
                const float base_y = (static_cast<float>(i) + 0.5f) * cell_size - 0.5f;
                sprite_vertices.emplace_back(SpriteCell{
                    {base_x, base_y, TCS * 3, TRS * 4},
                    {base_x + cell_size, base_y, TCS * 4, TRS * 4},
                    {base_x + cell_size, base_y + cell_size, TCS * 4, TRS * 5},
                    {base_x, base_y + cell_size, TCS * 3, TRS * 5}
                });

                const uint32_t base_index{static_cast<uint32_t>(sprite_indices.size()) * 4};
                sprite_indices.emplace_back(SquareIndices{
                    base_index, base_index + 1, base_index + 2, base_index + 2, base_index + 3, base_index
                });
            }
        }
    }

    glDebugMessageCallback(
        [](
            [[maybe_unused]] const GLenum source, [[maybe_unused]] const GLenum type, [[maybe_unused]] GLuint id,
            [[maybe_unused]] const GLenum severity,
            [[maybe_unused]] const GLsizei length,
            [[maybe_unused]] const GLchar *message,
            [[maybe_unused]] const void *userParam
        ) {
            if (severity == GL_DEBUG_SEVERITY_NOTIFICATION) { return; }
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
    grid_pipeline.construct(grid_tex_coords);

    const SpriteRenderPipeline sprite_pipeline{};
    sprite_pipeline.construct(sprite_vertices, sprite_indices);

    const MinerRenderPipeline miner_pipeline{};
    miner_pipeline.construct(miner_vertices);

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

        glTextureParameteri(GLuint{vto}, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(GLuint{vto}, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(GLuint{vto}, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(GLuint{vto}, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureStorage2D(GLuint{vto}, 1, GL_RGBA8, width, height);
        glTextureSubImage2D(GLuint{vto}, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data);
        stbi_image_free(data);
    }

    lwvl::Texture miner_texture{GL_TEXTURE_2D};
    {
        // GLFW expects the image to be loaded upside down.
        stbi_set_flip_vertically_on_load(false);
        int width, height, nr_channels;
        unsigned char *data = stbi_load("Data/Textures/diamond_pickaxe.png", &width, &height, &nr_channels, 0);
        if (!data) {
            std::cout << "Failed to load Data/Textures/diamond_pickaxe.png." << '\n';
            glfwSetWindowIcon(static_cast<GLFWwindow *>(window), 0, nullptr);
            return 1;
        }

        GLFWimage icon{width, height, data};
        glfwSetWindowIcon(static_cast<GLFWwindow *>(window), 1, &icon);

        // Flip for OpenGL
        stbi_vertical_flip(data, width, height, nr_channels);

        glTextureParameteri(GLuint{miner_texture}, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTextureParameteri(GLuint{miner_texture}, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTextureParameteri(GLuint{miner_texture}, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTextureParameteri(GLuint{miner_texture}, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTextureStorage2D(GLuint{miner_texture}, 1, GL_RGBA8, width, height);
        glTextureSubImage2D(
            GLuint{miner_texture}, 0, 0, 0, width, height, GL_RGBA, GL_UNSIGNED_BYTE, data
        );

        stbi_image_free(data);
    }

    constexpr float aspect{static_cast<float>(window_width) / static_cast<float>(window_height)};
    constexpr Mat4 projection{
        1.0f / aspect, 0.0f, 0.0f, 0.0f,
        0.0f,          1.0f, 0.0f, 0.0f,
        0.0f,          0.0f, 1.0f, 0.0f,
        0.0f,          0.0f, 0.0f, 1.0f
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

        lwvl::Program::LinkStatus status{};
        lwvl::Program::link(vso, stages, stage_count, status);
        if (status != lwvl::Program::LinkStatus::Success) {
            if (status == lwvl::Program::LinkStatus::ValidationFailure) {
                std::cerr << "Failed to validate program.\n";
            } else { std::cerr << "Failed to link program.\n"; }
            return 1;
        }
    }

    //lwvl::program_activate(vso);
    const GLint u_projection{glGetUniformLocation(GLuint{vso}, "u_projection")};
    if (u_projection < 0) {
        std::cerr << "Unable to find Projection uniform.\n";
        return 1;
    }
    glProgramUniformMatrix4fv(GLuint{vso}, u_projection, 1, GL_FALSE, projection.data());

    const GLint u_scale{glGetUniformLocation(GLuint{vso}, "u_scale")};
    if (u_scale < 0) {
        std::cerr << "Unable to find Scale uniform.\n";
        return 1;
    }
    glProgramUniform1f(GLuint{vso}, u_scale, 2);

    const GLint u_texture{lwvl::Program::uniform_location(vso, "u_texture")};
    if (u_texture < 0) {
        std::cerr << "Unable to find texture sampler uniform.\n";
        return 1;
    }
    glProgramUniform1i(GLuint{vso}, u_texture, 0);

    const GLint u_use_texture{lwvl::Program::uniform_location(vso, "u_use_texture")};
    if (u_use_texture < 0) {
        std::cerr << "Unable to find texture sampler uniform." << '\n';
        return 1;
    }
    glProgramUniform1i(GLuint{vso}, u_use_texture, 1);

    // const GLint u_time { lwvl::Program::uniform_location(vso, "u_Time") };
    // if (u_time < 0) {
    //     std::cerr << "Unable to find Time uniform." << '\n';
    //     return 1;
    // }
    // glProgramUniform1f(GLuint{vso}, u_time, 0.0f);

    // const GLint u_resolution { lwvl::Program::uniform_location(vso, "u_Resolution") };
    // if (u_resolution < 0) {
    //     std::cerr << "Unable to find Resolution uniform." << '\n';
    //     return 1;
    // }
    // glProgramUniform2d(GLuint{vso}, u_resolution, 800.0, 600.0);

    //glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );

    glBindTextureUnit(0, GLuint{vto});
    glBindTextureUnit(1, GLuint{miner_texture});

    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glClearColor(0.24314f, 0.25490f, 0.30588f, 1.0f);

    int display_w, display_h;
    glfwGetFramebufferSize(static_cast<GLFWwindow*>(window), &display_w, &display_h);

    {
        // Some claim clock resolutions can be as coarse as 10ms. 2ms is 500fps so I think that should be workable.
        using Period = sc::steady_clock::period;
        if constexpr (constexpr auto resolution_ns = 1'000'000'000 * Period::num / Period::den; resolution_ns > 8'333'333) {
            std::cerr << "System clock resolution is too coarse for accurate tick calculations.\n";
        } else if constexpr (resolution_ns > 2'000'000) {
            std::cerr << "System clock resolution may be too coarse for accurate tick calculations.\n";
        }
    }

    MoveAnimation move_animations[MINER_COUNT] {};
    MineAnimation mine_animations[MINER_COUNT] {};
    bool board_needs_update {false};

    // Ticks to limit how often the miner can act.
    constexpr uint64_t tick_nanoseconds {50'000'000};
    // 64-bits is over 29,247,120,867 years worth of 50ms intervals
    uint64_t tick_count {0};
    uint64_t tick_overshoot {0};  // We may catch the frame at 0.001ms past tick_start. This keeps track of the nanoseconds lost this way.
    auto tick_start {sc::steady_clock::now()};  // steady clock guarantees t0<t1
    while (!window.should_close()) {
        const auto new_tick_start {sc::steady_clock::now()};
        const int64_t frame_time {sc::duration_cast<sc::nanoseconds>(
            new_tick_start - tick_start
        ).count()};
        assert(frame_time >= 0);

        if (const uint64_t adj_time {static_cast<uint64_t>(frame_time) + tick_overshoot}; adj_time >= tick_nanoseconds) {
            tick_start = new_tick_start;
            tick_count += adj_time / tick_nanoseconds;
            tick_overshoot = adj_time % tick_nanoseconds;  // mod should be free with above div.
        }

        // move miners
        for (auto &miner : miners) {
            // Check if miner needs to move.
            if (miner.in_animation) {
                switch (miner.animation) {
                    case Miner::Animation::Move: {
                        // Update the miner with the next frame of the animation.
                        const MoveAnimation &move_animation {move_animations[miner.id]};
                        const bool finished_animation {move_animation.end_time <= new_tick_start};

                        if (finished_animation) {
                            miner.next_move = tick_count + 20;
                            miner.in_animation = false;
                            miner.animation = Miner::Animation::None;
                        }

                        const auto current_time_point {finished_animation ? move_animation.end_time : new_tick_start};
                        // Calculate lerp'd position.
                        const auto t = static_cast<float>(easeInOut(delta(move_animation.start_time, current_time_point) / delta(move_animation.start_time, move_animation.end_time)));
                        const float base_x = lerp(move_animation.start_x, move_animation.end_x, t);
                        const float base_y = lerp(move_animation.start_y, move_animation.end_y, t);

                        miner_vertices[miner.id] = MinerCell{
                                {base_x, base_y, 0.0f, 0.0f},
                                {base_x + cell_size, base_y, 1.0f, 0.0f},
                                {base_x + cell_size, base_y + cell_size, 1.0f, 1.0f},
                                {base_x, base_y + cell_size, 0.0f, 1.0f}
                        };

                        // miner.next_move = tick_count + 20;
                        continue;
                    }
                    case Miner::Animation::Mine: {
                        const MineAnimation &mine_animation {mine_animations[miner.id]};
                        const bool finished_animation {mine_animation.end_time <= new_tick_start};

                        if (finished_animation) {
                            // miner.next_move = tick_count + 20; // idk which choice here.

                            // Triggers a move animation right now. Would prefer this to be the miner's choice.
                            board[mine_animation.target] = 0;
                            board_needs_update = true;

                            const std::ptrdiff_t current_m {miner.index / BOARD_SIZE};
                            const std::ptrdiff_t current_n {miner.index % BOARD_SIZE};
                            const std::ptrdiff_t m {mine_animation.target / BOARD_SIZE};
                            const std::ptrdiff_t n {mine_animation.target % BOARD_SIZE};

                            const float c_base_x = (static_cast<float>(current_n) + 0.5f) * cell_size - 0.5f;
                            const float c_base_y = (static_cast<float>(current_m) + 0.5f) * cell_size - 0.5f;

                            const float base_x = (static_cast<float>(n) + 0.5f) * cell_size - 0.5f;
                            const float base_y = (static_cast<float>(m) + 0.5f) * cell_size - 0.5f;

                            move_animations[miner.id] = MoveAnimation{
                                c_base_x, c_base_y, base_x, base_y,
                                new_tick_start, new_tick_start + sc::milliseconds(500)
                            };

                            miner.animation = Miner::Animation::Move;
                            miner.index = mine_animation.target;

                            std::cout << "Miner " << miner.id << " finished mining the cell (" << n << ", " << m << ")\n";
                        }

                        continue;
                    }
                    case Miner::Animation::None: {
                        miner.in_animation = false;
                        break;
                    }
                }
            }

            if (miner.next_move > tick_count) {
                continue;
            }

            // Queue the move animation

            // Find a new spot for the miner to move.
            const ptrdiff_t direction = doom_random() % 5;
            if (direction == 4) {
                miner.next_move = tick_count + 20;
                continue;
            }

            constexpr ptrdiff_t direction_board[] {-BOARD_SIZE, 1, BOARD_SIZE, -1};
            constexpr ptrdiff_t direction_m[] {-1, +0, +1, +0};
            constexpr ptrdiff_t direction_n[] {+0, +1, +0, -1};
            assert(direction >= 0);
            assert(direction < 4);

            const std::ptrdiff_t current_m {miner.index / BOARD_SIZE};
            const std::ptrdiff_t current_n {miner.index % BOARD_SIZE};
            const std::ptrdiff_t m {current_m + direction_m[direction]};
            const std::ptrdiff_t n {current_n + direction_n[direction]};

            if (m < 0 || m >= BOARD_SIZE) { continue; }
            if (n < 0 || n >= BOARD_SIZE) { continue; }
            assert(current_m >= 0 && current_m < BOARD_SIZE);
            assert(current_n >= 0 && current_n < BOARD_SIZE);

            const std::ptrdiff_t board_index {miner.index + direction_board[direction]};
            if (board[board_index] > 0) {
                miner.in_animation = true;
                miner.animation = Miner::Animation::Mine;
                mine_animations[miner.id] = MineAnimation {
                    board_index, new_tick_start, new_tick_start + sc::milliseconds(1000)
                };
                continue;
            }

            bool spot_taken {false};
            for (const auto &other : miners) {
                if (other.index == board_index) {
                    spot_taken = true;
                    break;
                }
            }

            if (spot_taken) {
                continue;
            }

            const float c_base_x = (static_cast<float>(current_n) + 0.5f) * cell_size - 0.5f;
            const float c_base_y = (static_cast<float>(current_m) + 0.5f) * cell_size - 0.5f;

            const float base_x = (static_cast<float>(n) + 0.5f) * cell_size - 0.5f;
            const float base_y = (static_cast<float>(m) + 0.5f) * cell_size - 0.5f;

            move_animations[miner.id] = MoveAnimation{
                c_base_x, c_base_y, base_x, base_y,
                new_tick_start, new_tick_start + sc::milliseconds(500)
            };
            miner.in_animation = true;
            miner.animation = Miner::Animation::Move;
            miner.index = board_index;
            // miner.next_move = tick_count + 20;
        }
        lwvl::Buffer::fill_slice(miner_pipeline.vbo, miner_vertices.begin(), miner_vertices.end());
        std::cout.flush();

        if (board_needs_update) {
            // do stuff.
            for (std::ptrdiff_t i = 0; i < GRID_SIZE; ++i) {
                for (std::ptrdiff_t j = 0; j < GRID_SIZE; ++j) {
                    grid_tex_coords[i * GRID_SIZE + j] = STONE_TEXTURE_MAP[select_texture(i, j, board.tether())];
                }
            }
            lwvl::Buffer::fill_slice(grid_pipeline.vtcbo, grid_tex_coords.begin(), grid_tex_coords.end());
        }

        core::Window::poll_events();
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplGlfw_NewFrame();
        ImGui::NewFrame();

        // ImGui::ShowMetricsWindow();

        if (show_demo_window) { ImGui::ShowDemoWindow(&show_demo_window); }

        {
            ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.
            ImGui::Checkbox("Demo Window", &show_demo_window);      // Edit bools storing our window open/close state

            ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / io.Framerate, io.Framerate);
            ImGui::Text("Tick: %d", tick_count);
            ImGui::Text("Tick overshoot: %d", tick_overshoot);
            ImGui::End();
        }

        ImGui::Render();
        int new_display_w, new_display_h;
        glfwGetFramebufferSize(static_cast<GLFWwindow*>(window), &new_display_w, &new_display_h);
        if (new_display_w != display_w || new_display_h != display_h) {
            glViewport(0, 0, display_w, display_h);
            display_w = new_display_w;
            display_h = new_display_h;
        }

        lwvl::clear();

        lwvl::Program::activate(vso);
        //glProgramUniform1f(static_cast<GLuint>(grid_pipeline.vso), u_time, float(delta(time_point)));

        glProgramUniform1i(GLuint{vso}, u_use_texture, 1);
        glProgramUniform1i(GLuint{vso}, u_texture, 0);
        lwvl::VertexArray::activate(grid_pipeline.vao);
        lwvl::draw_elements(GL_TRIANGLES, GRID_SIZE * GRID_SIZE * 6, GL_UNSIGNED_INT);

        // lwvl::VertexArray::activate(sprite_pipeline.vao);
        // lwvl::draw_elements(GL_TRIANGLES, static_cast<GLsizei>(sprite_indices.size() * 6), GL_UNSIGNED_INT);

        glProgramUniform1i(GLuint{vso}, u_texture, 1);
        lwvl::VertexArray::activate(miner_pipeline.vao);
        lwvl::draw_arrays(GL_TRIANGLES, static_cast<GLsizei>(miner_vertices.size() * 6));

        // glPolygonMode( GL_FRONT_AND_BACK, GL_LINE );
        // glProgramUniform1i(GLuint{vso}, u_use_texture, 0);
        // lwvl::VertexArray::activate(grid_pipeline.vao);
        // lwvl::draw_elements(GL_TRIANGLES, GRID_SIZE * GRID_SIZE * 6, GL_UNSIGNED_INT);
        // glPolygonMode( GL_FRONT_AND_BACK, GL_FILL );

        lwvl::Program::clear();

        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
        window.swap();
    }

    return 0;
}


int main() {
    try {
        return run();
    } catch(const std::runtime_error &e) {
        std::cerr << e.what() << '\n';
        return 1;
    } catch (const std::exception &) {
        std::cerr << "Caught unknown exception.\n";
        return 1;
    }
}
