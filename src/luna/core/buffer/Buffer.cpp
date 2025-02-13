//
// Created by NBT22 on 2/12/25.
//

#include <luna/core/buffer/Buffer.hpp>

namespace luna::core::buffer
{
Buffer::Buffer(size_t bufferSize, VkBufferUsageFlags bufferUsageFlags, VkMemoryPropertyFlags memoryFlags) {}
Buffer::Buffer(size_t bufferSize, VkBufferUsageFlags bufferUsageFlags, std::unique_ptr<size_t> &memoryIndex) {}
} // namespace luna::core::buffer
