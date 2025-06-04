//
// Created by NBT22 on 5/19/25.
//

#include <luna/core/CommandPool.hpp>
#include <luna/core/Instance.hpp>

VkResult lunaCreateCommandPool(const LunaCommandPoolCreationInfo *creationInfo, LunaCommandPool *commandPool)
{
    assert(creationInfo);
    CHECK_RESULT_RETURN(luna::core::device.addApplicationCommandPool(*creationInfo, commandPool));
    return VK_SUCCESS;
}

// VkResult lunaResetCommandPool(const LunaCommandPool commandPool, const VkCommandPoolResetFlagBits flags)
// {
//     switch (reinterpret_cast<uintptr_t>(commandPool))
//     {
//         case LUNA_INTERNAL_GRAPHICS_COMMAND_POOL:
//             CHECK_RESULT_RETURN(luna::core::device.commandPools().graphics.reset(luna::core::device, flags));
//             return VK_SUCCESS;
//         default:
//             return VK_ERROR_UNKNOWN;
//     }
// }
