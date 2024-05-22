#pragma once

#include "lwvl/lwvl.hpp"
#include "lwvl/stb_image.h"
#include "GLFW/glfw3.h"
#include "imgui/imgui.h"
#include "Platform/GLFW/imgui_impl_glfw.h"
#include "Platform/OpenGL/imgui_impl_opengl3.h"

#include <chrono>
#include <iostream>
#include <array>
template <class T, size_t Size>
using StaticArray = std::array<T, Size>;
#include <vector>
template <class T, class Alloc = std::allocator<T>>
using DynamicArray = std::vector<T, Alloc>;
#include <string>
#include <sstream>
#include <fstream>
#include <variant>
