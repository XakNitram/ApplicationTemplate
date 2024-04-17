module;
#include "pch.hpp"
export module Board;


// Some kind of Atlas object injected in? Used to map textures to coordinates?
// . Perhaps this is better done with bit-packed integers uploaded to the GPU and the coordinates selected there.
// . Would still need some kind of map to between block type and texture index in the atlas.
// . Uniform block to provide atlas information like w/h.

export class Board {

};
