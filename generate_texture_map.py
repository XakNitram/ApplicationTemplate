from time import perf_counter
from dataclasses import dataclass
from sys import stdout


@dataclass
class TexCell:
    start_x: float
    start_y: float
    end_x: float
    end_y: float


def run(name: str, width: int, height: int, offset_x: int, offset_y: int):
    # We're mapping 4 bits to 16 textures
    # Standard layout for the grid textures
    # Accept offset and atlas dimensions
    # constexpr TextureMap STONE_TEXTURE_MAP{
    #     TexCoordCell{{0.0f, TRS * 3}, {TCS, TRS * 3}, {TCS, TRS * 4}, {0.0f, TRS * 4}}, // 0b0000
    #     TexCoordCell{{TCS * 2, TRS * 2}, {TCS * 3, TRS * 2}, {TCS * 3, TRS * 3}, {TCS * 2, TRS * 3}}, // 0b0001
    #     TexCoordCell{{0.0f, TRS * 2}, {TCS, TRS * 2}, {TCS, TRS * 3}, {0.0f, TRS * 3}}, // 0b0010
    #     TexCoordCell{{TCS, TRS * 2}, {TCS * 2, TRS * 2}, {TCS * 2, TRS * 3}, {TCS, TRS * 3}}, // 0b0011
    #     TexCoordCell{{0.0f, TRS}, {TCS, TRS}, {TCS, TRS * 1}, {0.0f, TRS * 1}}, // 0b0100
    #     TexCoordCell{{TCS * 3, TRS}, {TCS * 4, TRS}, {TCS * 4, TRS * 1}, {TCS * 3, TRS * 1}}, // 0b0101
    #     TexCoordCell{{0.0f, TRS * 1}, {TCS, TRS * 1}, {TCS, TRS * 2}, {0.0f, TRS * 2}}, // 0b0110
    #     TexCoordCell{{TCS * 4, TRS * 1}, {TCS * 5, TRS * 1}, {TCS * 5, TRS * 2}, {TCS * 4, TRS * 2}}, // 0b0111
    #     TexCoordCell{{TCS * 2, TRS}, {TCS * 3, TRS}, {TCS * 3, TRS * 1}, {TCS * 2, TRS * 1}}, // 0b1000
    #     TexCoordCell{{TCS * 2, TRS * 1}, {TCS * 3, TRS * 1}, {TCS * 3, TRS * 2}, {TCS * 2, TRS * 2}}, // 0b1001
    #     TexCoordCell{{TCS * 4, TRS}, {TCS * 5, TRS}, {TCS * 5, TRS * 1}, {TCS * 4, TRS * 1}}, // 0b1010
    #     TexCoordCell{{TCS * 3, TRS * 1}, {TCS * 4, TRS * 1}, {TCS * 4, TRS * 2}, {TCS * 3, TRS * 2}}, // 0b1011
    #     TexCoordCell{{TCS, 0.0f}, {TCS * 2, 0.0f}, {TCS * 2, TRS}, {TCS, TRS}}, // 0b1100
    #     TexCoordCell{{TCS * 3, TRS * 2}, {TCS * 4, TRS * 2}, {TCS * 4, TRS * 3}, {TCS * 3, TRS * 3}}, // 0b1101
    #     TexCoordCell{{TCS * 4, TRS * 2}, {TCS * 5, TRS * 2}, {TCS * 5, TRS * 3}, {TCS * 4, TRS * 3}}, // 0b1110
    #     TexCoordCell{{TCS, TRS * 1}, {TCS * 2, TRS * 1}, {TCS * 2, TRS * 2}, {TCS, TRS * 2}}, // 0b1111
    # };
    # The determinant is the order in which we search the board.
    # 4 3   3 4   2 3
    # 1 2   1 2   1 4
    # We'll stick with
    # 3 4
    # 1 2

    # Hilbert curve
    textures = [
        TexCell(float(i), float(j), float(i + 1), float(j + 1)) for j in range(4) for i in range(4)
    ]

    # 0b0000 -  0 0b0001 -  1 0b0010 -  2 0b0011 -  3
    # 0b0100 -  4 0b0101 -  5 0b0110 -  6 0b0111 -  7
    # 0b1000 -  8 0b1001 -  9 0b1010 - 10 0b1011 - 11
    # 0b1100 - 12 0b1101 - 13 0b1110 - 14 0b1111 - 15
    mapping = [
        0, 10, 9, 4,
        6, 12, 11, 14,
        5, 8, 15, 13,
        7, 2, 1, 3
    ]

    # hilbert curve
    # mapping = [
    #     0, 8, 7, 3,
    #     13, 5, 11, 9,
    #     2, 4, 10, 6,
    #     12, 14, 1, 15
    # ]

    print(f"constexpr TextureMap {name}{{")
    for i in range(16):
        texture = textures[mapping[i]]
        x1 = round((texture.start_x + offset_x) / width, 5)
        x2 = round((texture.end_x + offset_x) / width, 5)
        y1 = round((texture.start_y + offset_y) / height, 5)
        y2 = round((texture.end_y + offset_y) / height, 5)
        print(f"    TexCoordCell{{{{{x1}f, {y1}f}}, {{{x2}f, {y1}f}}, {{{x2}f, {y2}f}}, {{{x1}f, {y2}f}}}},")
    print("};")

    return 0


if __name__ == '__main__':
    exit(run("STONE_TEXTURE_MAP", 5, 5, 0, 0))
