#include <GLFW/glfw3.h> // for window management and incidental vk include
#include <stdio.h>   // for printf and fam
#include <stdlib.h>  // for malloc/free
#include <string.h>  // for memcpy/strcmp
#include "util.h"    // for file loading
#include "renderer.h"// for interface matching

// This is limiting, but it simplifies things
#define MAX_FRAMES 3

// Vulkan includes various elements of a single view that are needed
// either for various uses or deletion
struct ImageObj {
    VkImage image;
    VkImageView view;
    VkDeviceMemory mem;
    VkExtent2D extent;
    VkSampler sampler; // Only used for textures
};

// Buffer object includes the buffer, the backing mem and the size
struct BufferObj {
    VkBuffer buffer;
    VkDeviceMemory mem;
    VkDeviceSize size;
};

// Uniforms include per frame descriptor sets and ubo buffers, and the single common layout
struct UniformObj {
    VkDescriptorSetLayout layout;
    VkDescriptorSet sets[MAX_FRAMES];
    Buffer ubos[MAX_FRAMES];
};

// Pipelines are just the pipeline and the layout
struct PipelineObj {
    VkPipeline pipeline;
    VkPipelineLayout layout;
};

// Collection of all per-frame global data. The color buffer and framebuffer of course
// Command buffers are helpful to prevent overwriting. and fence, and two semaphores for control
typedef struct {
    Image color_buf;
    VkFramebuffer fb;
    VkFence fence;
    VkSemaphore ready;
    VkSemaphore done;
} Frame;

// Global data regarding the vulkan renderer. All are created once and used throughout.
typedef struct {
    VkInstance instance;
    VkSurfaceKHR surface;
    VkPhysicalDevice phys_device;
    VkDevice device;
    uint32_t qfams[2];
    VkQueue gfx_q, pres_q;
    VkSwapchainKHR swapchain;
    Frame frames[MAX_FRAMES];
    Image depth_buf;
    VkRenderPass render_pass;
    VkCommandPool cmd_pool;
    VkCommandBuffer cmd_bufs[MAX_FRAMES + 1];
    VkCommandBuffer scratch_cmdbuf;
    VkDescriptorPool descriptor_pool;
    uint32_t cur_frame;
    uint32_t frame_ct;
} Renderer;

static Renderer renderer;
static Renderer *vk = &renderer;

// Debug callback used by the validation layer. Most of the information is ignored, but the prototype is preset
// Only the callback data is used to print the message.
static VKAPI_ATTR VkBool32 VKAPI_CALL debug_cb(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                                        VkDebugUtilsMessageTypeFlagsEXT messageType,
                                        const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                                        void* pUserData) {
    fprintf(stderr, "validation layer: %s\n", pCallbackData->pMessage);

    return VK_FALSE;

} // debug_cb()

// Create the instance, the main reference to the API itself.
// if <validate> is true, a validation layer is included.
static VkInstance create_instance(VkBool32 validate) {
    VkInstance instance = VK_NULL_HANDLE;
    VkApplicationInfo app_info = {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pApplicationName = "craft",
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "crafty",
        .engineVersion  = VK_MAKE_VERSION(1, 0, 0),
        .apiVersion = VK_API_VERSION_1_0,
    };
    VkInstanceCreateInfo instance_info = {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pApplicationInfo = &app_info,
    };
    uint32_t ext_ct = 0;
    const char **req_extensions = glfwGetRequiredInstanceExtensions(&ext_ct);
    VkResult result;
    if (validate) {
        const char *layer_name = "VK_LAYER_KHRONOS_validation";
        instance_info.enabledLayerCount = 1;
        instance_info.ppEnabledLayerNames = &layer_name;

        // add debug utils extension
        ext_ct++;
        const char **extensions = (const char**)malloc(ext_ct*sizeof(char*));
        int i;
        for (i = 0; i < ext_ct - 1; i++) {
            extensions[i] = req_extensions[i];
        }
        extensions[i++] = VK_EXT_DEBUG_UTILS_EXTENSION_NAME;

        instance_info.enabledExtensionCount = i;
        instance_info.ppEnabledExtensionNames = extensions;

        VkDebugUtilsMessengerCreateInfoEXT messenger_info = {
            .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
            .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        messenger_info.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
            .pfnUserCallback = debug_cb,
        };
        instance_info.pNext = &messenger_info;

        result = vkCreateInstance(&instance_info, NULL, &instance);
        free(extensions);
    } else {
        instance_info.enabledLayerCount = 0;
        instance_info.pNext = NULL;

        instance_info.enabledExtensionCount = ext_ct;
        instance_info.ppEnabledExtensionNames = req_extensions;
        result = vkCreateInstance(&instance_info, NULL, &instance);
    }

    if (result) {
        glfwTerminate();
        fprintf(stderr, "Instance creation failed!");
        return VK_NULL_HANDLE;
    }

    return instance;

} // create_instance()

// Create a logical device by selecting a physical device with the necessary
// extensions and queue types. If <validate> is set, validation extensions are enabled
// If any of the vaiours API calls fails, returns NULL_HANDLE
static VkDevice create_device(VkBool32 validate) {
    uint32_t dev_ct = 0;
    VkPhysicalDevice phys_device;
    vkEnumeratePhysicalDevices(vk->instance, &dev_ct, NULL);
    if (!dev_ct) {
        fprintf(stderr, "no physical devices found!");
        return VK_NULL_HANDLE;
    }
    VkPhysicalDevice *phys_devices = malloc(sizeof(VkPhysicalDevice)*dev_ct);
    vkEnumeratePhysicalDevices(vk->instance, &dev_ct, phys_devices);
    int32_t gfx_ix = -1, pres_ix = -1;
    for (int i = 0; i < dev_ct; i++) {
        VkPhysicalDeviceProperties device_properties;
        phys_device = phys_devices[i];
        vkGetPhysicalDeviceProperties(phys_device, &device_properties);

        // find gfx and present queue indices.
        uint32_t qfam_ct = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &qfam_ct, NULL);
        VkQueueFamilyProperties *qfams = malloc(sizeof(VkQueueFamilyProperties)*qfam_ct);
        vkGetPhysicalDeviceQueueFamilyProperties(phys_device, &qfam_ct, qfams);
        for (int j = 0; j < qfam_ct; j++) {
            if (!qfams[j].queueCount)
                continue; // no queues. no point
            if (gfx_ix < 0 && qfams[j].queueFlags & VK_QUEUE_GRAPHICS_BIT)
                gfx_ix = j;
            VkBool32 pres_ok = VK_FALSE;
            vkGetPhysicalDeviceSurfaceSupportKHR(phys_device, j, vk->surface, &pres_ok);
            if (pres_ix < 0 && pres_ok)
                pres_ix = j;
        }
        free(qfams);

        // Check for needed extension
        uint32_t ext_ct;
        vkEnumerateDeviceExtensionProperties(phys_device, NULL, &ext_ct, NULL);
        VkExtensionProperties *extensions = malloc(sizeof(VkExtensionProperties)*ext_ct);
        vkEnumerateDeviceExtensionProperties(phys_device, NULL, &ext_ct, extensions);
        uint32_t ext_found = 0;

        for (int j = 0; j < ext_ct; j++) {
            if (!strcmp(extensions[j].extensionName, VK_KHR_SWAPCHAIN_EXTENSION_NAME) ||
                !strcmp(extensions[j].extensionName, VK_KHR_MAINTENANCE1_EXTENSION_NAME) ||
                !strcmp(extensions[j].extensionName, VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME))
                ext_found++;
        }
        free(extensions);

        // Check for at least one surface format and present mode
        uint32_t fmt_ct = 0;
        vkGetPhysicalDeviceSurfaceFormatsKHR(phys_device, vk->surface, &fmt_ct, NULL);

        uint32_t mode_ct = 0;
        vkGetPhysicalDeviceSurfacePresentModesKHR(phys_device, vk->surface, &mode_ct, NULL);

        // If everything needed is found, we're done
        if (gfx_ix > -1 && pres_ix > -1 && ext_found == 3 && fmt_ct && mode_ct)
            break;
        else
            phys_device = VK_NULL_HANDLE;

    } // for each device
    free(phys_devices);

    if (phys_device == VK_NULL_HANDLE) {
        fputs("No acceptable physical device found\n", stderr);
        return VK_NULL_HANDLE;
    }

    // Valid physical device in hand, now create the logical device.

    // setup gfx queue request
    uint32_t qcreate_ct = 1;
    VkDeviceQueueCreateInfo qcreate_infos[2] = {};
    float qpriority = 1.0f;

    qcreate_infos[0].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
    qcreate_infos[0].queueFamilyIndex = gfx_ix;
    qcreate_infos[0].queueCount = 1;
    qcreate_infos[0].pQueuePriorities = &qpriority;

    // If present queue is separate, create it too
    if (gfx_ix != pres_ix) {
        qcreate_ct++;
        qcreate_infos[1].sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
        qcreate_infos[1].queueFamilyIndex = pres_ix;
        qcreate_infos[1].queueCount = 1;
        qcreate_infos[1].pQueuePriorities = &qpriority;
    }

    const char *ext_names[3] = {VK_KHR_SWAPCHAIN_EXTENSION_NAME, VK_KHR_MAINTENANCE1_EXTENSION_NAME,
                                VK_EXT_DEPTH_RANGE_UNRESTRICTED_EXTENSION_NAME};
    const char *layer_name = "VK_LAYER_KHRONOS_validation";
    VkPhysicalDeviceFeatures dev_features = {.logicOp = VK_TRUE, .wideLines = VK_TRUE, .depthClamp = VK_TRUE};

    VkDeviceCreateInfo dev_info = {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .queueCreateInfoCount = qcreate_ct,
        .pQueueCreateInfos = qcreate_infos,
        .enabledExtensionCount = 3,
        .ppEnabledExtensionNames = ext_names,
        .enabledLayerCount = validate,
        .ppEnabledLayerNames = validate?&layer_name:NULL,
        .pEnabledFeatures = &dev_features,
    };

    VkDevice device;
    if (vkCreateDevice(phys_device, &dev_info, NULL, &device)) {
        fputs("Failed to create logical device!\n", stderr);
        glfwTerminate();
        return VK_NULL_HANDLE;
    }

    VkCommandPoolCreateInfo pool_info = {
             .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
             .queueFamilyIndex = gfx_ix,
             .flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
        };

    VkCommandPool cmd_pool;
    if (vkCreateCommandPool(device, &pool_info, NULL, &cmd_pool)) {
        fputs("graphics command pool creation failed!\n", stderr);
        glfwTerminate();
        return VK_NULL_HANDLE;
    }

    // Save off ancillary products to global struct
    vk->phys_device = phys_device;
    vk->qfams[0] = gfx_ix;
    vk->qfams[1] = pres_ix;
    vkGetDeviceQueue(device, gfx_ix, 0, &vk->gfx_q);
    vkGetDeviceQueue(device, pres_ix, 0, &vk->pres_q);
    vk->cmd_pool = cmd_pool;

    return device;
            
} // create_device()

// Allocate memory from the device according to the requirements in <mem_reqs>
// and possessing the <properties> specified.
// If no appropriate memory is found or allocation fails, null handle is returned
static VkDeviceMemory alloc_device_mem(VkMemoryRequirements mem_reqs, VkMemoryPropertyFlags properties) {
    VkPhysicalDeviceMemoryProperties mem_properties;
    vkGetPhysicalDeviceMemoryProperties(vk->phys_device, &mem_properties);
    uint32_t memIx;
    for (memIx = 0; memIx < mem_properties.memoryTypeCount; memIx++)
        if ((mem_reqs.memoryTypeBits & (1 << memIx)) &&
            (mem_properties.memoryTypes[memIx].propertyFlags & properties) == properties)
            break;

    if (memIx == mem_properties.memoryTypeCount) {
        fputs("no acceptable device memory found\n", stderr);
        return VK_NULL_HANDLE;
    }

    VkMemoryAllocateInfo alloc_info = {
        .sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
        .allocationSize = mem_reqs.size,
        .memoryTypeIndex = memIx,
    };
    VkDeviceMemory mem;
    if (vkAllocateMemory(vk->device, &alloc_info, NULL, &mem)) {
        fprintf(stderr, "Failed to allocate %ld bytes of device memory\n", mem_reqs.size);
        return VK_NULL_HANDLE;
    }

    return mem;

} // alloc_device_mem()

// Create a view of the <image> that exposes the <format>
// the image view handle is returned on success. null handle on failure.
static VkImageView create_image_view(VkImage image, VkFormat format) {

    VkImageView image_view;
    VkImageViewCreateInfo view_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = format,
        .subresourceRange.aspectMask = (format == VK_FORMAT_D24_UNORM_S8_UINT?VK_IMAGE_ASPECT_DEPTH_BIT:VK_IMAGE_ASPECT_COLOR_BIT),
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };
    if (vkCreateImageView(vk->device, &view_info, NULL, &image_view)) {
        fputs("Image view creation failed\n", stderr);
        return VK_NULL_HANDLE;
    }

    return image_view;

} // create_image_view()

// Transition <image> from its existing layout to <new_layout>. <src_mask> and <dst_mask>
// indicate the scope of the access and <src_stage> and <dst_stage> the stage ownership transfer
static void convert_image_layout(VkImage image, VkImageLayout new_layout,
                                 VkAccessFlags src_mask, VkAccessFlags dst_mask,
                                 VkPipelineStageFlags src_stage, VkPipelineStageFlags dst_stage) {

    VkImageMemoryBarrier barrier = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
        .srcAccessMask = src_mask,
        .dstAccessMask = dst_mask,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = new_layout,
        .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
        .image = image,
        .subresourceRange.baseMipLevel = 0,
        .subresourceRange.levelCount = 1,
        .subresourceRange.baseArrayLayer = 0,
        .subresourceRange.layerCount = 1,
    };

    if (new_layout == VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL) {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT | VK_IMAGE_ASPECT_STENCIL_BIT;
    } else {
        barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
    }

    vkCmdPipelineBarrier(vk->scratch_cmdbuf, src_stage, dst_stage, 0, 0, NULL, 0, NULL, 1, &barrier);

} // convert_image_layout()

// Create an image object of <format> for the givan <usage> of size <width>x<height>
// This creates the image, the view, the device memory and saves the format and size.
static Image create_image(VkFormat format, VkImageUsageFlags usage, uint32_t width, uint32_t height) {

    VkImageCreateInfo image_info = {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .extent.width = width,
        .extent.height = height,
        .extent.depth = 1,
        .mipLevels = 1,
        .arrayLayers = 1,
        .format = format,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .usage = usage,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkImage image;
    if (vkCreateImage(vk->device, &image_info, NULL, &image)) {
        fputs("failed to create image\n", stderr);
        return NULL;
    }

    VkMemoryRequirements mem_reqs;
    vkGetImageMemoryRequirements(vk->device, image, &mem_reqs);
    VkDeviceMemory image_mem = alloc_device_mem(mem_reqs, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
    vkBindImageMemory(vk->device, image, image_mem, 0);

    VkImageView image_view = create_image_view(image, format);
    if (image_view == VK_NULL_HANDLE)
        return NULL;

    Image image_obj = malloc(sizeof(struct ImageObj));
    image_obj->image = image;
    image_obj->view = image_view;
    image_obj->mem = image_mem;
    image_obj->extent.width = width;
    image_obj->extent.height = height;
    image_obj->sampler = VK_NULL_HANDLE;

    return image_obj;

} // create_image()

// Create sufficient per-frame elements to go with the <swapchain> and store in vk->frames
// Returns VK_SUCCESS on success, VK_INCOMPLETE on failure.
static int32_t create_frames(VkSwapchainKHR swapchain) {

    // First figure out how many frames in swapchain
    uint32_t frame_ct;
    vkGetSwapchainImagesKHR(vk->device, swapchain, &frame_ct, NULL);
    VkImage *swap_imgs = malloc(sizeof(VkImage) * frame_ct);
    vkGetSwapchainImagesKHR(vk->device, swapchain, &frame_ct, swap_imgs);

    // Framebuffer and sync info structs to be used for each
    VkExtent2D extent = vk->depth_buf->extent; // crib off depth bufer for size
    VkImageView attachments[2] = {VK_NULL_HANDLE, vk->depth_buf->view};
    VkFramebufferCreateInfo fb_info = {
        .sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
        .renderPass = vk->render_pass,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .width = extent.width,
        .height = extent.height,
        .layers = 1,
    };

    VkSemaphoreCreateInfo semaphoreInfo = {.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO};
    VkFenceCreateInfo fenceInfo = {
        .sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
        .flags = VK_FENCE_CREATE_SIGNALED_BIT,
    };

    for (int i = 0; i < frame_ct; i++) {
        Frame *frame = &vk->frames[i];

        // Start with the color buffers
        Image color_buf = malloc(sizeof(struct ImageObj));
        color_buf->image = swap_imgs[i];
        color_buf->view = create_image_view(swap_imgs[i], VK_FORMAT_B8G8R8A8_UNORM);
        color_buf->mem = VK_NULL_HANDLE;
        color_buf->extent = extent;
        frame->color_buf = color_buf;

        attachments[0] = color_buf->view;
        // Create framebuffers
        if (vkCreateFramebuffer(vk->device, &fb_info, NULL, &frame->fb)) {
            fprintf(stderr, "framebuffer %d creation failed\n", i);
            return VK_INCOMPLETE;
        }

        // Create semaphore and fences
        if (vkCreateSemaphore(vk->device, &semaphoreInfo, NULL, &frame->ready) ||
            vkCreateSemaphore(vk->device, &semaphoreInfo, NULL, &frame->done) ||
            vkCreateFence(vk->device, &fenceInfo, NULL, &frame->fence)) {
            fputs("semaphore/fence creation failed\n", stderr);
            return VK_INCOMPLETE;// whatever
        }
    }
    free(swap_imgs);

    return VK_SUCCESS;
} // create_frames()

// Create swapchain with images and attached framebuffers of dimensions matching <window>
// Create command buffers too since they are also per-frame.
static VkSwapchainKHR create_swapchain(GLFWwindow *window) {

    // Select image format
    uint32_t fmt_ct = 0;
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk->phys_device, vk->surface, &fmt_ct, NULL);
    VkSurfaceFormatKHR *fmts = malloc( sizeof(VkSurfaceFormatKHR) * fmt_ct);
    vkGetPhysicalDeviceSurfaceFormatsKHR(vk->phys_device, vk->surface, &fmt_ct, fmts);
    VkSurfaceFormatKHR chosen_fmt = fmts[0]; // if all else fails, just take what's first
    for (int j = 0; j < fmt_ct; j++) {
        if (fmts[j].format == VK_FORMAT_B8G8R8A8_UNORM && fmts[j].colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
            chosen_fmt = fmts[j];
    }
    if (chosen_fmt.format == VK_FORMAT_UNDEFINED) {
        chosen_fmt.format = VK_FORMAT_B8G8R8A8_UNORM;
        chosen_fmt.colorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR;
    }
    free(fmts);

    // Select present mode
    uint32_t mode_ct = 0;
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk->phys_device, vk->surface, &mode_ct, NULL);
    VkPresentModeKHR *modes = malloc (sizeof(VkPresentModeKHR) * mode_ct);
    vkGetPhysicalDeviceSurfacePresentModesKHR(vk->phys_device, vk->surface, &mode_ct, modes);
    VkPresentModeKHR chosen_mode = VK_PRESENT_MODE_FIFO_KHR;
    for (int j = 0; j < mode_ct; j++) {
        if (modes[j] == VK_PRESENT_MODE_MAILBOX_KHR)
            chosen_mode = VK_PRESENT_MODE_MAILBOX_KHR;
    }
    free(modes);

    // Select dimensions
    VkSurfaceCapabilitiesKHR surf_caps;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->phys_device, vk->surface, &surf_caps);
    VkExtent2D chosen_extent = surf_caps.currentExtent;
    if (chosen_extent.width == 0xFFFFFFFF) {
        int width, height;
        glfwGetFramebufferSize(window, &width, &height);

        chosen_extent.width = (uint32_t)width;
        chosen_extent.height = (uint32_t)height;
    }

    // Select number of frames
    uint32_t req_img_ct = MAX_FRAMES;
    if (surf_caps.maxImageCount > 0 && req_img_ct > surf_caps.maxImageCount)
        req_img_ct = surf_caps.maxImageCount;

    VkSwapchainCreateInfoKHR chain_info = {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = vk->surface,
        .minImageCount = req_img_ct,
        .imageFormat = chosen_fmt.format,
        .imageColorSpace = chosen_fmt.colorSpace,
        .imageExtent = chosen_extent,
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = chosen_mode,
        .clipped = VK_TRUE,
        .oldSwapchain = VK_NULL_HANDLE,
    };

    // Set sharing mode depending on if our graphics queue can present
    if (vk->qfams[0] != vk->qfams[1]) {
        chain_info.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        chain_info.queueFamilyIndexCount = 2;
        chain_info.pQueueFamilyIndices = vk->qfams;
    } else {
        chain_info.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
    }

    VkSwapchainKHR swapchain;
    if (vkCreateSwapchainKHR(vk->device, &chain_info, NULL, &swapchain)) {
        glfwTerminate();
        fputs("Swapchain creation failed!\n", stderr);
        return VK_NULL_HANDLE;
    }

    // Retrieve how many images in the swapchain.
    uint32_t frame_ct;
    vkGetSwapchainImagesKHR(vk->device, swapchain, &frame_ct, NULL);
    vk->frame_ct = frame_ct;

    // Create one command buffer per frame so they won't be reset before the are done
    VkCommandBufferAllocateInfo cmd_buf_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = vk->cmd_pool,
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = frame_ct + 1,
    };
    if (vkAllocateCommandBuffers(vk->device, &cmd_buf_info, vk->cmd_bufs)) {
        fputs("failed command buffer allocation\n", stderr);
        return VK_NULL_HANDLE;
    }
    vk->scratch_cmdbuf = vk->cmd_bufs[frame_ct];

    // Create depth attachment
    vk->depth_buf = create_image(VK_FORMAT_D24_UNORM_S8_UINT, VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
                                 chosen_extent.width, chosen_extent.height);

    // Convert to optimal layout
    VkCommandBuffer cmdbuf = vk->scratch_cmdbuf;

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmdbuf, &begin_info);

    convert_image_layout(vk->depth_buf->image, VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
                         0, VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_READ_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT);

    vkEndCommandBuffer(cmdbuf);
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdbuf,
    };
    vkQueueSubmit(vk->gfx_q, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(vk->gfx_q);

    return swapchain;

} // create_swapchain()

// Create a standard render pass with a color and depth attachment and a single subpass
static VkRenderPass create_render_pass() {
    VkAttachmentDescription color_attach = {
        .format = VK_FORMAT_B8G8R8A8_UNORM,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
    };
    VkAttachmentReference color_attach_ref = {
        .attachment = 0,
        .layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
    };
    VkAttachmentDescription depth_attach = {
        .format = VK_FORMAT_D24_UNORM_S8_UINT,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE,
        .stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };

    VkAttachmentReference depth_attach_ref = {
        .attachment = 1,
        .layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
    };
    VkSubpassDescription subpass = {
        .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
        .colorAttachmentCount = 1,
        .pColorAttachments = &color_attach_ref,
        .pDepthStencilAttachment = &depth_attach_ref,
    };
    VkAttachmentDescription attachments[2] = {color_attach, depth_attach};
    VkRenderPassCreateInfo render_pass_info = {
        .sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
        .attachmentCount = 2,
        .pAttachments = attachments,
        .subpassCount = 1,
        .pSubpasses = &subpass,
    };
    VkSubpassDependency dependency = {
        .srcSubpass = VK_SUBPASS_EXTERNAL,
        .dstSubpass = 0,
        .srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = 0,
        .dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
        .dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_READ_BIT | VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
    };
    render_pass_info.dependencyCount = 1;
    render_pass_info.pDependencies = &dependency;

    VkRenderPass render_pass;
    if (vkCreateRenderPass(vk->device, &render_pass_info, NULL, &render_pass)) {
        fputs("failed to create render pass\n", stderr);
        return VK_NULL_HANDLE;
    }

    return render_pass;

} // create_render_pass()

// Create descriptor pool from which to allocate descriptor sets
// That allows at least <ubo_ct> ubo and <samp_ct> sample allocations
static VkDescriptorPool create_descriptor_pool(uint32_t ubo_ct, uint32_t samp_ct) {
    VkDescriptorPoolSize pool_sizes[2];
    pool_sizes[0].type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    pool_sizes[0].descriptorCount = ubo_ct;
    pool_sizes[1].type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
    pool_sizes[1].descriptorCount = samp_ct;
    VkDescriptorPoolCreateInfo pool_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .poolSizeCount = 2,
        .pPoolSizes = pool_sizes,
        .maxSets = ubo_ct + samp_ct,
    };
    VkDescriptorPool descriptor_pool;
    if (vkCreateDescriptorPool(vk->device, &pool_info, NULL, &descriptor_pool)) {
        fputs("create descriptor pool failed\n", stderr);
        return VK_NULL_HANDLE;
    }

    return descriptor_pool;

} // create_descriptor_pool()


// Initialize persistent global state for the renderer using API-dependent <window>
// returns 0 on success. For VK, it creates the instance, the device, the swapchain,
// renderpass, various per-frame objects, and a descriptor pool
int init_renderer(GLFWwindow *window) {
    // Initialize vulkan
    VkBool32 validate = VK_FALSE;
    vk->instance = create_instance(validate);
    if (vk->instance == VK_NULL_HANDLE)
        return -1;
    if (glfwCreateWindowSurface(vk->instance, window, NULL, &vk->surface)) {
        fprintf(stderr, "window surface creation failed!\n");
        return -1;
    }

    vk->device = create_device(validate);
    if (vk->device == VK_NULL_HANDLE)
        return -1;

    vk->swapchain = create_swapchain(window);
    if (vk->swapchain == VK_NULL_HANDLE)
        return -1;

    vk->render_pass = create_render_pass();
    if (vk->render_pass == VK_NULL_HANDLE)
        return -1;

    if (create_frames(vk->swapchain))
        return -1;

    vk->descriptor_pool = create_descriptor_pool(vk->frame_ct*11, vk->frame_ct*12);
    if (vk->descriptor_pool == VK_NULL_HANDLE)
        return -1;

    vk->cur_frame = 0;

    return 0;

} // init_renderer()

// Clear the a portion of the  framebuffer indicated by <x,y,width,height>
// for the attachments indicated by the <bitfield>
void subclear_frame(uint32_t bitfield, int32_t x, int32_t y, int32_t width, int32_t height) {

    int start = 1, end = 1;
    if (bitfield & CLEAR_COLOR_BIT)
        start = 0;
    if (bitfield & CLEAR_DEPTH_BIT)
        end = 2;
    VkClearAttachment attachments[2] = {{
        .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .clearValue = {.color = {{0.0, 0.0, 0.0, 1.0}}}
    },{
        .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
        .clearValue = {.depthStencil = {1.0, 0.0}}
    }};

    VkClearRect rect = {
        .rect = {{(float)x, vk->depth_buf->extent.height - (float)y - (float)height},
                 {(float)width, (float)height}},
        .baseArrayLayer = 0,
        .layerCount = 1
     };

    vkCmdClearAttachments(vk->cmd_bufs[vk->cur_frame], end - start, attachments + start, 1, &rect);

} // subclear_frame()

// Clear the full framebuffer for the attachments indicated by the <bitfield>
void clear_frame(uint32_t bitfield) {
    subclear_frame(bitfield, 0, 0, vk->depth_buf->extent.width, vk->depth_buf->extent.height);

} // clear_frame()

// set viewport for the framebuffer limited to <x,y,width,height>
void set_viewport(int32_t x, int32_t y, int32_t width, int32_t height) {
    VkViewport viewport = {
        .x = x,
        .y = vk->depth_buf->extent.height - y,
        .width = width,
        .height = -height,
        .minDepth = -1.0f,
        .maxDepth = 1.0f,
    };

    vkCmdSetViewport(vk->cmd_bufs[vk->cur_frame], 0, 1, &viewport);

} // set_viewport()

// Create a bufer object of <size> bytes able to be used for <usage> allocated with memory
// matching <properties>
// Create a buffer object including the vulkan buffer handle, device memory, and size.
static Buffer create_buffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties) {

    VkBufferCreateInfo buffer_info = {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = size,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
    };
    VkBuffer buffer;
    if (vkCreateBuffer(vk->device, &buffer_info, NULL, &buffer)) {
        fputs("Fail to create buffer\n", stderr);
        return NULL;
    }

    VkMemoryRequirements mem_reqs;
    vkGetBufferMemoryRequirements(vk->device, buffer, &mem_reqs);
    VkDeviceMemory buf_mem = alloc_device_mem(mem_reqs, properties);
    vkBindBufferMemory(vk->device, buffer, buf_mem, 0);

    Buffer buffer_obj = malloc(sizeof(struct BufferObj));
    buffer_obj->buffer = buffer;
    buffer_obj->mem = buf_mem;
    buffer_obj->size = size;

    return buffer_obj;

} // create_buffer()

// Copy <size> bytes of the contents of <src> buffer into <dst>
// This begins and records a command to copy the <src> to <dst> into the
// scratch command buffer and submits it to the graphics queue.
static void copy_buffer(VkBuffer src, VkBuffer dst, VkDeviceSize size) {

    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(vk->scratch_cmdbuf, &begin_info);

    VkBufferCopy copyRegion = {.size = size};
    vkCmdCopyBuffer(vk->scratch_cmdbuf, src, dst, 1, &copyRegion);

    vkEndCommandBuffer(vk->scratch_cmdbuf);

    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &vk->scratch_cmdbuf,
    };
    vkQueueSubmit(vk->gfx_q, 1, &submit_info, VK_NULL_HANDLE);
    // Inefficient, but only called during startup
    vkQueueWaitIdle(vk->gfx_q);

} // copy_buffer()

// Delete the buffer object represented by <buffer>
// Destroys the vulkan buffer, frees the device memory, and frees the object
void del_buffer(Buffer buf) {
    if (!buf) return; // it happens
    vkDestroyBuffer(vk->device, buf->buffer, NULL);
    vkFreeMemory(vk->device, buf->mem, NULL);
    free(buf);

} // del_buffer()

// Generate a buffer object of <size> bytes and initialize with <data> and return its handle
// Uses a temporary host memory accessible buffer as a temporary location to copy the information
// and then copy the <data> to the final buffer.
Buffer gen_buffer(int32_t size, float *data) {
    Buffer stage = create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    void* map_mem;
    vkMapMemory(vk->device, stage->mem, 0, size, 0, &map_mem);
    memcpy(map_mem, data, (size_t) size);
    vkUnmapMemory(vk->device, stage->mem);

    Buffer buf = create_buffer(size, VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

    copy_buffer(stage->buffer, buf->buffer, size);

    del_buffer(stage);

    return buf;

} // gen_buffer()

// Generate a buffer object of <size> bytes and initialize with <data> that
// is expected to be changed frequently
// For Vulkan, this means host-visible and coherent memory that can easily be
// mapped and written to by the host.
Buffer gen_dynamic_buffer(int32_t size, float *data) {
    Buffer buf = create_buffer(size, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
                               VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
    if (data) {
        void* map_mem;
        vkMapMemory(vk->device, buf->mem, 0, size, 0, &map_mem);
        memcpy(map_mem, data, (size_t) size);
        vkUnmapMemory(vk->device, buf->mem);
    }

    return buf;

} // gen_dynamic_buffer()

// Update the contents of <buffer> with <size> bytes of <data>
// This is done by mapping the device memory associated with <buffer>
// and writing to it with standard copy commands
void update_buffer(Buffer buffer, int32_t size, float *data) {
    if (!buffer) return;
    if (data) {
        void* map_mem;
        vkMapMemory(vk->device, buffer->mem, 0, size, 0, &map_mem);
        memcpy(map_mem, data, (size_t) size);
        vkUnmapMemory(vk->device, buffer->mem);
    }

} // update_buffer()

// Allocate and return memory for attribs consisting of <components> attribs for <faces> quads
float *malloc_faces(int components, int faces) {
    return malloc(sizeof(float) * 6 * components * faces);
}

// Generate a vertex buffer representing <faces> quads with vertex attributes
// consisting of <components> attributes using <data>  and return its handle
Buffer gen_faces(int components, int faces, float *data) {
    Buffer buffer = gen_buffer(
        sizeof(float) * 6 * components * faces, data);
    free(data);
    return buffer;
}

// Update <buffer> with <faces> quads with vertex attributes
// consisting of <components> attributes using <data>
// <data> will be freed
void update_faces(Buffer buffer, int components, int faces, float *data) {
    update_buffer(buffer,
        sizeof(float) * 6 * components * faces, data);
    free(data);
} // update_faces()

// Make a vulkan shader module of <type> using spir-v <source> of <length> bytes
static VkShaderModule make_shader(uint32_t type, const void *source, int length) {
    VkShaderModuleCreateInfo shader_info = {
         .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
         .codeSize = length,
         .pCode = (uint32_t*)source,
        };
    VkShaderModule shader;
    if (vkCreateShaderModule(vk->device, &shader_info, NULL, &shader)) {
        fputs("Failed to create shader", stderr);
        return VK_NULL_HANDLE;
    }
    return shader;
} // make_shader()

// Read spir-v code from <path> and create shader of <type> and return the vulkan handle
static VkShaderModule load_shader(uint32_t type, const char *path) {
    int len;
    void *data = load_file(path, &len);
    VkShaderModule result = make_shader(type, data, len);
    free(data);
    return result;
} // load_shader()

static void draw_verts(Buffer vbo, int vert_ct) {
    VkCommandBuffer cbuf = vk->cmd_bufs[vk->cur_frame];
    VkDeviceSize offset = 0;
    vkCmdBindVertexBuffers(cbuf, 0, 1, &vbo->buffer, &offset);
    vkCmdDraw(cbuf, vert_ct, 1, 0, 0);
} // draw_verts()

// Draw lines consisting of <count> 3D vertices
// taken from vertex buffer <buffer> at <width> pixels
void draw_lines(Buffer buffer, int count, float width) {
    vkCmdSetLineWidth(vk->cmd_bufs[vk->cur_frame], width);
    draw_verts(buffer, count);
} // draw_lines()

// Draw landscape chunk using vertex buffer <buffer> consisting of <faces> quads
void draw_chunk(Buffer buffer, int faces) {
    if (buffer)
        draw_verts(buffer, faces * 6);
} // draw_chunk()

// Draw UI placement option represented by vertex buffer <buffer>
// consisting of <count> vertices
void draw_item(Buffer buffer, int count) {
    draw_verts(buffer, count);
} // draw_item()

// Draw UI text represented by vertex buffer <buffer> of <length> characters
void draw_text(Buffer buffer, int length) {
    draw_verts(buffer, length * 6);
} // draw_text()

// Draw text placed on landscape chunks represented by vertex buffer <buffer>
// of <faces> characters
void draw_signs(Buffer buffer, int faces) {
    if (!buffer) return;
    draw_verts(buffer, faces * 6);
} // draw_signs()

// Draw text currently being applied to a chunk represented by vertex buffer <buffer>
// of <length> characters
void draw_sign(Buffer buffer, int length) {
    if (!buffer) return;
    draw_verts(buffer, length * 6);
} // draw_sign()

// Draw UI landscape chunk placement option represented by vertex buffer <buffer>
void draw_cube(Buffer buffer) {
    draw_item(buffer, 36);
} // draw_cube()

// Draw UI plant placement option represented by vertex buffer <buffer>
void draw_plant(Buffer buffer) {
    draw_item(buffer, 24);
} // draw_plant()

// Draw player cube represented by vertex buffer <buffer>
void draw_player(Buffer buffer) {
    draw_cube(buffer);
} // draw_player()

// Draw large sphere around origin represented by vertex buffer <buffer>
void draw_sky(Buffer buffer) {
    draw_verts(buffer, 512 * 3);
} // draw_sky()

// Create a descriptor layout with UBO acces in <ubo_stages> shader stages and containing <sampler_ct>
// This assumes that the sampler bindings will be enumerated first followed by a single UBO
// Returns the layout handle if successful, null handle otherwise.
static VkDescriptorSetLayout create_descriptor_layout(VkShaderStageFlags ubo_stages, uint32_t sampler_ct) {

    // Add however many samplers to the bindings
    // Never gonna need more than 3
    VkDescriptorSetLayoutBinding bindings[3] = {};
    int i;
    for (i = 0; i < sampler_ct; i++) {
        bindings[i].binding = i;
        bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        bindings[i].descriptorCount = 1;
        bindings[i].stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT;
        bindings[i].pImmutableSamplers = NULL;
    }

    // Tack the ubo on the end
    bindings[i].binding = i;
    bindings[i].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    bindings[i].descriptorCount = 1;
    bindings[i].stageFlags = ubo_stages;

    // Create and return the layout
    VkDescriptorSetLayoutCreateInfo layout_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .bindingCount = sampler_ct + 1,
        .pBindings = bindings,
    };
    VkDescriptorSetLayout layout;
    if (vkCreateDescriptorSetLayout(vk->device, &layout_info, NULL, &layout)) {
        fputs("failed to create descriptor set layout\n", stderr);
        return VK_NULL_HANDLE;
    }

    return layout;

} // create_descriptor_layout()

// Create a uniform object containing a ubo of size <ubo_size> to be used in <ubo_stages>
// and textures <texture0> and <texture1> then return the handle.
// For Vulkan, we need a ubo and consequently a descriptor set per frame.
Uniform gen_uniform(uint32_t ubo_size, uint32_t ubo_stages, Image texture0, Image texture1) {

    uint32_t frame_ct = vk->frame_ct;
    Uniform uniform = malloc(sizeof(struct UniformObj));

    // Figure out how many textures
    int tex_ct = 0;
    if (texture0) tex_ct++;
    if (texture1) tex_ct++;

    // Create a common layout for all the per-frame descriptor sets
    VkDescriptorSetLayout layout = VK_NULL_HANDLE;
    VkShaderStageFlags vk_stages = 0;
    vk_stages |= (ubo_stages&STAGE_VERT_BIT)?VK_SHADER_STAGE_VERTEX_BIT:0;
    vk_stages |= (ubo_stages&STAGE_FRAG_BIT)?VK_SHADER_STAGE_FRAGMENT_BIT:0;
    layout = create_descriptor_layout(vk_stages, tex_ct);

    VkDescriptorSetLayout layouts[MAX_FRAMES];
    for (int i = 0; i < frame_ct; i++) {
        layouts[i] = layout;
    }

    // Allocate the descriptor sets using this layout smeared across an array
    VkDescriptorSetAllocateInfo set_info = {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = vk->descriptor_pool,
        .descriptorSetCount = frame_ct,
        .pSetLayouts = layouts,
    };

    VkDescriptorSet *sets = uniform->sets;
    if (vkAllocateDescriptorSets(vk->device, &set_info, sets)) {
        fputs("failed to allocate descriptor sets\n", stderr);
        free(sets);
        return NULL;
    }

    // Allocate per-frame ubos
    Buffer *ubos = uniform->ubos;

    int j = 0;
    VkDescriptorImageInfo image_info[2] = {};
    VkWriteDescriptorSet descriptor_writes[3] = {};

    // Partially initialize info structures for the samplers and buffers attached to the descriptor set writes
    // frame-specific values are assinged inside the loop below
    // Add possible first texture to set
    if (texture0) {
        image_info[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info[j].imageView = texture0->view;
        image_info[j].sampler = texture0->sampler;

        descriptor_writes[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[j].dstSet = VK_NULL_HANDLE;// filled inside loop
        descriptor_writes[j].dstBinding = j;
        descriptor_writes[j].dstArrayElement = 0;
        descriptor_writes[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[j].descriptorCount = 1;
        descriptor_writes[j].pImageInfo = &image_info[j];

        j++;
    }

    // Add possible second texture to set
    if (texture1) {
        image_info[j].imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        image_info[j].imageView = texture1->view;
        image_info[j].sampler = texture1->sampler;

        descriptor_writes[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
        descriptor_writes[j].dstSet = VK_NULL_HANDLE; // inside loop
        descriptor_writes[j].dstBinding = j;
        descriptor_writes[j].dstArrayElement = 0;
        descriptor_writes[j].descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
        descriptor_writes[j].descriptorCount = 1;
        descriptor_writes[j].pImageInfo = &image_info[j];

        j++;
    };

        // Add definite UBO to set
    descriptor_writes[j].sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
    descriptor_writes[j].dstSet = VK_NULL_HANDLE;// inside loop
    descriptor_writes[j].dstBinding = j;
    descriptor_writes[j].dstArrayElement = 0;
    descriptor_writes[j].descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
    descriptor_writes[j].descriptorCount = 1;
    descriptor_writes[j].pBufferInfo = NULL; // inside loop

    // Update each per-frame descriptor set with the UBOs and samplers (if any) accordingly
    uint32_t desc_ct = j + 1;
    for (int i = 0; i < frame_ct; i++) {
        ubos[i] = create_buffer(ubo_size, VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
                                VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        if (texture0)
            descriptor_writes[0].dstSet = sets[i];
        if (texture1)
            descriptor_writes[1].dstSet = sets[i];

        VkDescriptorBufferInfo buffer_info = {
            .buffer = ubos[i]->buffer,
            .offset = 0,
            .range = ubo_size,
        };

        descriptor_writes[j].dstSet = sets[i];
        descriptor_writes[j].pBufferInfo = &buffer_info;

        vkUpdateDescriptorSets(vk->device, desc_ct, descriptor_writes, 0, NULL);

    } // for each frame

    uniform->layout = layout;

    return uniform;

} // gen_uniform()


// Destroy <uniform> and free any associated memory.
// Destroys the layout and deletes the UBOs. Individual descriptor sets can't be
// Freed with how the pool was setup. and they last the duration of the program. so no need.
void del_uniform(Uniform uniform) {

    // Free descriptor set layout. Can't easily free the sets themselves
    vkDestroyDescriptorSetLayout(vk->device, uniform->layout, NULL);

    // Destroy UBOs
    for (int i = 0; i < vk->frame_ct; i++)
        del_buffer(uniform->ubos[i]);

    free(uniform);

} // del_uniform()

// Create an image associated with a texture defined by <pixesl> of dimensions <width>x<height>
// data is copied first to a host-visible buffer and then transferred to the image
//  with needed layout changes using the scratch command buffer
static Image create_tex_image(uint8_t *pixels, int width, int height) {
    VkDeviceSize size = width * height * 4;

    // Create the staging buffer for loading the pixel info
    Buffer stage_buf = create_buffer(size, VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
                                     VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

    void* data;
    vkMapMemory(vk->device, stage_buf->mem, 0, size, 0, &data);
    memcpy(data, pixels, (size_t)size);
    vkUnmapMemory(vk->device, stage_buf->mem);

    // Create the image
    Image tex_image = create_image(VK_FORMAT_R8G8B8A8_UNORM,
                                   VK_IMAGE_USAGE_TRANSFER_SRC_BIT | VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
                                   width, height);

    // Begin the scratch command buffer
    VkCommandBuffer cmdbuf = vk->scratch_cmdbuf;
    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };
    vkBeginCommandBuffer(cmdbuf, &begin_info);

    // Convert to optimal destination
    convert_image_layout(tex_image->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                         0, VK_ACCESS_TRANSFER_WRITE_BIT,
                         VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT);


    // Copy the stage buffer to the image
    VkBufferImageCopy region = {
        .bufferOffset = 0,
        .bufferRowLength = 0,
        .bufferImageHeight = 0,
        .imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
        .imageSubresource.mipLevel = 0,
        .imageSubresource.baseArrayLayer = 0,
        .imageSubresource.layerCount = 1,
        .imageOffset = {0, 0, 0},
        .imageExtent = {width, height, 1},
    };
    vkCmdCopyBufferToImage(cmdbuf, stage_buf->buffer, tex_image->image,
                           VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

    // Convert to optimal reading from shader
    convert_image_layout(tex_image->image, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
                         VK_ACCESS_TRANSFER_WRITE_BIT, VK_ACCESS_SHADER_READ_BIT,
                         VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT);


    // End command buffer and submit
    vkEndCommandBuffer(cmdbuf);
    VkSubmitInfo submit_info = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &cmdbuf,
    };
    vkQueueSubmit(vk->gfx_q, 1, &submit_info, VK_NULL_HANDLE);
    vkQueueWaitIdle(vk->gfx_q);

    // destroy temp stage buffer
    del_buffer(stage_buf);

    return tex_image;

} // create_tex_image()

// Create a sampler object for a texture image with <filter> filtering and <address_mode> wrap rule
// Min and max filters are made the same. No mipmaps are expected
static VkSampler create_sampler(VkFilter filter, VkSamplerAddressMode address_mode) {

    VkSamplerCreateInfo sampler_info = {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = filter,
        .minFilter = filter,
        .addressModeU = address_mode,
        .addressModeV = address_mode,
        .addressModeW = address_mode,
        .borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK,
        .unnormalizedCoordinates = VK_FALSE,
        .compareEnable = VK_FALSE,
        .compareOp = VK_COMPARE_OP_ALWAYS,
        .mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR,
        .mipLodBias = 0.0f,
        .minLod = 0,
        .maxLod = 1,
    };
    VkSampler sampler;
    if (vkCreateSampler(vk->device, &sampler_info, NULL, &sampler)) {
        fputs("Failed to create sampler\n", stderr);
    }
    return sampler;

} // create_sampler()

// Load the image stored at <path> and use it to create a texture image
// Then create a sampler with filtering and address mode according to <linear> and <clamp>
// Returns the image handle.
Image load_tex_image(const char *path, int linear, int clamp) {
    uint32_t width, height;
    uint8_t *data = load_png_texture(path, &width, &height);
    Image ret_image = create_tex_image(data, width, height);

    VkFilter filter = VK_FILTER_NEAREST;
    VkSamplerAddressMode address_mode = VK_SAMPLER_ADDRESS_MODE_REPEAT;
    if (linear)
        filter = VK_FILTER_LINEAR;
    if (clamp)
        address_mode = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;

    ret_image->sampler = create_sampler(filter, address_mode);

    free(data);

    return ret_image;

} // load_tex_image()

// Destroy <image> and free any associated resources
// Destroys the image, its view, the device memory, the sampler if it exists and frees the obj
void del_image(Image image) {

    if (image->sampler != VK_NULL_HANDLE)
        vkDestroySampler(vk->device, image->sampler, NULL);
    vkDestroyImageView(vk->device, image->view, NULL);
    vkDestroyImage(vk->device, image->image, NULL);
    vkFreeMemory(vk->device, image->mem, NULL);
    free(image);

} // del_image()

// Create pipeline object containing shaders as extracted from files <path1> and <path2>
// useable with <uniform> with <attrib_ct> attribs containing <components> enabling <feature_bits>
// Reads and "specializes" the spirv found in the given files. Creates a pipeline compatible with
// the descriptor layout contained in <uniform>. <featureBits> consists of blending, line render,
// and depth bias bits that are enabled or disabled accordingly.
Pipeline gen_pipeline(const char *path1, const char *path2, Uniform uniform,
                      uint32_t attrib_ct, const uint32_t *components, uint32_t featureBits) {
    // Convert feature bits into more convenient booleans
    VkBool32 blend = !!(featureBits & FEATURE_BLEND_BIT);
    VkBool32 line = !!(featureBits & FEATURE_LINE_BIT);
    VkBool32 depth_bias = !!(featureBits & FEATURE_DEPTH_BIAS_BIT);

    // Load and create the shaders
    VkShaderModule vert_shader = load_shader(VK_SHADER_STAGE_VERTEX_BIT, path1);
    VkShaderModule frag_shader = load_shader(VK_SHADER_STAGE_FRAGMENT_BIT, path2);

    VkPipelineShaderStageCreateInfo vert_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_VERTEX_BIT,
        .module = vert_shader,
        .pName = "main",
    };


    VkPipelineShaderStageCreateInfo frag_stage_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
        .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
        .module = frag_shader,
        .pName = "main",
    };

    VkPipelineShaderStageCreateInfo stage_infos[] = {vert_stage_info, frag_stage_info};

    // Create attrib descriptions specifying format, offset, and locations.
    VkVertexInputAttributeDescription *attrib_descriptions = malloc(sizeof(VkVertexInputAttributeDescription) * attrib_ct);
    VkFormat color_fmts[4] = {VK_FORMAT_R32_SFLOAT, VK_FORMAT_R32G32_SFLOAT, VK_FORMAT_R32G32B32_SFLOAT, VK_FORMAT_R32G32B32A32_SFLOAT};
    uint32_t offset = 0;
    for (int i = 0; i < attrib_ct; i++) {
        attrib_descriptions[i].binding = 0;
        attrib_descriptions[i].location = i;
        attrib_descriptions[i].format = color_fmts[components[i] - 1];
        attrib_descriptions[i].offset = offset;
        offset += sizeof(float)*components[i];
    }

    VkVertexInputBindingDescription bind_description = {
        .binding = 0,
        .stride = offset,
        .inputRate = VK_VERTEX_INPUT_RATE_VERTEX,
    };

    VkPipelineVertexInputStateCreateInfo vert_input_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
        .vertexBindingDescriptionCount = 1,
        .pVertexBindingDescriptions = &bind_description,
        .vertexAttributeDescriptionCount = attrib_ct,
        .pVertexAttributeDescriptions = attrib_descriptions,
    };

    // If line rendering is enabled, input assmbly makes lines. Triangles otherwise
    VkPipelineInputAssemblyStateCreateInfo assembly_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = line?VK_PRIMITIVE_TOPOLOGY_LINE_LIST:VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
        .primitiveRestartEnable = VK_FALSE,
    };

    VkRect2D scissor = {
        .offset = {0, 0},
        .extent = vk->depth_buf->extent,
    };

    VkPipelineViewportStateCreateInfo viewport_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = NULL,
        .scissorCount = 1,
        .pScissors = &scissor,
    };

    // Rasterization always fills polygons when present and culls CW triangles
    // depth clamp is needed to allow -1 to 1 range. if depth bias is enabled, the
    // values are taken into effect and ignored otherwise.
    VkPipelineRasterizationStateCreateInfo rasterizer_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
        .depthClampEnable = VK_TRUE,
        .rasterizerDiscardEnable = VK_FALSE,
        .polygonMode = VK_POLYGON_MODE_FILL,
        .lineWidth = 1.0f,
        .cullMode = VK_CULL_MODE_BACK_BIT,
        .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
        .depthBiasEnable = depth_bias,
        .depthBiasSlopeFactor = -8,
        .depthBiasConstantFactor = -1024,
    };

    // No MSAA
    VkPipelineMultisampleStateCreateInfo msaa_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .sampleShadingEnable = VK_FALSE,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
    };
    // Standard blending settings. Only enabled if indicated in the bitfield
    VkPipelineColorBlendAttachmentState blend_attachment = {
        .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
        .blendEnable = blend,
        .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .colorBlendOp = VK_BLEND_OP_ADD,
        .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
        .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
        .alphaBlendOp = VK_BLEND_OP_ADD,
    };
    // Logic op is enabled to invert colors if we are doing lines. Disabled otherwise
    VkPipelineColorBlendStateCreateInfo blend_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = line,
        .logicOp = VK_LOGIC_OP_INVERT,
        .attachmentCount = 1,
        .pAttachments = &blend_attachment,
    };
    // Standard depth enabled settings. no stencil test
    VkPipelineDepthStencilStateCreateInfo depth_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
        .depthTestEnable = VK_TRUE,
        .depthWriteEnable = VK_TRUE,
        .depthCompareOp = VK_COMPARE_OP_LESS,
        .depthBoundsTestEnable = VK_FALSE,
        .stencilTestEnable = VK_FALSE,
    };
    // descriptor set layout is set to match the provided uniform
    VkPipelineLayoutCreateInfo pipeline_layout_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = 1,
        .pSetLayouts = &uniform->layout,
    };
    VkPipelineLayout layout;
    if (vkCreatePipelineLayout(vk->device, &pipeline_layout_info, NULL, &layout)) {
        fputs("pipeline layout creation failed\n", stderr);
        return NULL;
    }

    // The only definitely required dynamic state is viewport
    // line width too if line rendering is enabled
    VkDynamicState dynamic_states[2] = {VK_DYNAMIC_STATE_VIEWPORT,
                                        VK_DYNAMIC_STATE_LINE_WIDTH};

    VkPipelineDynamicStateCreateInfo dynamic_info = {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = 1 + line,
        .pDynamicStates = dynamic_states,
    };
    // This is where it all gets packaged up and submitted to the Vulkan bureaucracy
    VkGraphicsPipelineCreateInfo pipeline_info = {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .stageCount = 2, // frag and vertex
        .pStages = stage_infos,
        .pVertexInputState = &vert_input_info,
        .pInputAssemblyState = &assembly_info,
        .pViewportState = &viewport_info,
        .pRasterizationState = &rasterizer_info,
        .pMultisampleState = &msaa_info,
        .pDepthStencilState = &depth_info,
        .pColorBlendState = &blend_info,
        .pDynamicState = &dynamic_info,
        .layout = layout,
        .renderPass = vk->render_pass,
        .subpass = 0,
    };

    VkPipeline pipeline;
    if (vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, &pipeline_info, NULL, &pipeline)) {
        fputs("Pipeline creation failed\n", stderr);
        return NULL;
    }

    // Free shaders and temporary array
    free(attrib_descriptions);
    vkDestroyShaderModule(vk->device, vert_shader, NULL);
    vkDestroyShaderModule(vk->device, frag_shader, NULL);

    // Wrap in an object and return
    Pipeline ret = malloc(sizeof(struct PipelineObj));
    ret->pipeline = pipeline;
    ret->layout = layout;

    return ret;

} // create_pipeline_obj()

// Bind <pipeline> in the current command buffer and bind descriptor set in <uniform>
// Update UBO associated with that descriptor set with <size> bytes from <data>
// On vulkan this is just binding the pipeline and descriptor set in the current frame
// command buffer which should have been begun before this and updating the UBO
void bind_pipeline(Pipeline pipeline, Uniform uniform, int size, void *data) {
    uint32_t cur = vk->cur_frame;
    vkCmdBindPipeline(vk->cmd_bufs[cur], VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline->pipeline);
    vkCmdBindDescriptorSets(vk->cmd_bufs[cur], VK_PIPELINE_BIND_POINT_GRAPHICS,
                            pipeline->layout, 0, 1, &uniform->sets[cur], 0, NULL);
    // Update UBO
    update_buffer(uniform->ubos[cur], size, data);

} // bind_pipeline()

// Destroy pipeline object <pipeline> including its layout and free the object memory
// Destroys the layout, the pipeline, and the containing object
void del_pipeline(Pipeline pipeline) {
    vkDestroyPipelineLayout(vk->device, pipeline->layout, NULL);
    vkDestroyPipeline(vk->device, pipeline->pipeline, NULL);
    free(pipeline);

} // del_pipeline()

// Perform any initialization or setup required at the start of the frame rendering
// For VK this involves beginning command buffer recording and beginning the render pass.
void start_frame() {
    VkCommandBuffer cbuf = vk->cmd_bufs[vk->cur_frame];


    VkCommandBufferBeginInfo begin_info = {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT
    };

    vkWaitForFences(vk->device, 1, &vk->frames[vk->cur_frame].fence, VK_TRUE, ~0UL);
    if (vkBeginCommandBuffer(cbuf, &begin_info)) {
        fputs("Command buffer Begin Failed\n",stderr);
        return;
    }

    uint32_t cur = vk->cur_frame;
    VkClearValue clear_color = {.color = {{0.0,0.0,0.0,1.0}}};
    VkClearValue clear_depth = {.depthStencil = {.depth = 1.0, .stencil = 0}};
    VkClearValue clears[2] = {clear_color, clear_depth};
    VkRenderPassBeginInfo rp_begin_info = {
         .sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
         .renderPass = vk->render_pass,
         .framebuffer = vk->frames[cur].fb,
         .renderArea.offset = {0, 0},
         .renderArea.extent = vk->frames[cur].color_buf->extent,
         .clearValueCount = 2,
         .pClearValues = clears,
        };
    vkCmdBeginRenderPass(cbuf, &rp_begin_info, VK_SUBPASS_CONTENTS_INLINE);

} // start_frame()

// Perform any shutdown or submission required at the end of the frame rendering to <window>
// For VK, this ends the render pass and command buffer recording, gets the next image, and when
// fences indicate it's time, submitting the command buffer to the queue and presenting the rendered image
void end_frame(GLFWwindow *window) {

    (void)window; // VK has no use for this.

    uint32_t cur = vk->cur_frame;
    VkCommandBuffer cbuf = vk->cmd_bufs[cur];
    Frame *frame = &vk->frames[cur];
    vkCmdEndRenderPass(cbuf);

    if (vkEndCommandBuffer(cbuf)) {
        fputs("end command buf failed\n", stderr);
        return;
    }

    // get image
    uint32_t img_ix;

    if (vkAcquireNextImageKHR(vk->device, vk->swapchain, ~0UL, frame->ready, VK_NULL_HANDLE, &img_ix)) {
        fputs("image acquire failed\n", stderr);
        return;
    }

    // Submit command buffer
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    VkSubmitInfo submitInfo = {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame->ready,
        .pWaitDstStageMask = waitStages,
        .commandBufferCount = 1,
        .pCommandBuffers = &vk->cmd_bufs[cur],
        .signalSemaphoreCount = 1,
        .pSignalSemaphores = &frame->done,
    };

    vkResetFences(vk->device, 1, &frame->fence);

    if (vkQueueSubmit(vk->gfx_q, 1, &submitInfo, frame->fence)) {
        fputs("queue submit failed\n", stderr);
        return;
    }

    // present image
    VkPresentInfoKHR presentInfo = {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &frame->done,
        .swapchainCount = 1,
        .pSwapchains = &vk->swapchain,
        .pImageIndices = &img_ix,
    };
    if(vkQueuePresentKHR(vk->pres_q, &presentInfo))
        fputs("present failed\n", stderr);

    vk->cur_frame = (vk->cur_frame + 1) % vk->frame_ct;

} // end_frame()

// Conclude any rendering by the renderer in preparation for deletion
// Not much to this. Just needed to wait for things to complete before I deleted everything
void shutdown_renderer() {
    vkDeviceWaitIdle(vk->device);
} // shutdown_renderer()

// Destroy all renderer resources and free any memory
// This is where every vulkan object created should be destroyed.
void del_renderer() {
    del_image(vk->depth_buf);

    // Destroy per-frame objects
    for (int i = 0; i < vk->frame_ct; i++) {
        Frame *frame = &vk->frames[i];
        vkDestroyFramebuffer(vk->device, frame->fb, NULL);

        vkDestroyImageView(vk->device, frame->color_buf->view, NULL);

        vkDestroySemaphore(vk->device, frame->ready, NULL);
        vkDestroySemaphore(vk->device, frame->done, NULL);
        vkDestroyFence(vk->device, frame->fence, NULL);
    }
    vkDestroyCommandPool(vk->device, vk->cmd_pool, NULL);
    vkDestroyRenderPass(vk->device, vk->render_pass, NULL);

    vkDestroySwapchainKHR(vk->device, vk->swapchain, NULL);
    vkDestroyDescriptorPool(vk->device, vk->descriptor_pool, NULL);

    vkDestroyDevice(vk->device, NULL);
    vkDestroySurfaceKHR(vk->instance, vk->surface, NULL);
    vkDestroyInstance(vk->instance, NULL);
    glfwTerminate();

} // del_renderer()
