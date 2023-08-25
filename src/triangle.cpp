#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <vulkan/vulkan_core.h>

#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <vector>
#include <cstring>
#include <optional>
#include <set>

// constants for the window width and height
const uint32_t WIDTH = 800;
const uint32_t HEIGHT = 600;

const std::vector<const char *> validationLayers = {
    "VK_LAYER_KHRONOS_validation"};

const std::vector<const char *> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME};

// check if the program is in debug mode or not to enable validation layers
#ifdef NDEBUG
const bool enableValidationLayers = false;
#else
const bool enableValidationLayers = true;
#endif

struct QueueFamilyIndices
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> presentFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value() && presentFamily.has_value();
    }
};

struct SwapChainSupportDetails
{
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

/**
 * proxy function for vkCreateDebugUtilsMessengerEXT
 * This function is used to load the vkCreateDebugUtilsMessengerEXT function from the vulkan library
 * This function is used to create a debug messenger
 *
 * A debug messenger triggers a debug callback with a debug message when an event of interest occurs.
 */
VkResult CreateDebugUtilsMessengerEXT(VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT *pCreateInfo, const VkAllocationCallbacks *pAllocator, VkDebugUtilsMessengerEXT *pDebugMessenger)
{
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    }
    else
    {
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

/**
 * proxy function for vkDestroyDebugUtilsMessengerEXT
 * This function is used to load the vkDestroyDebugUtilsMessengerEXT function from the vulkan library
 * This function is used to destroy a debug messenger
 */
void DestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT debugMessenger, const VkAllocationCallbacks *pAllocator)
{
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr)
    {
        func(instance, debugMessenger, pAllocator);
    }
}

/**
 * The main application class
 * This class is responsible for creating the window, creating the vulkan instance, and running the main loop
 *
 * @class TriangleApplication
 *
 * @property {GLFWwindow*} window - the window to draw to
 * @property {VkInstance} instance - the vulkan instance
 *
 */
class TriangleApplication
{
public:
    // run the application
    void run()
    {
        initWindow();
        initVulkan();
        mainLoop();
        cleanup();
    }

private:
    // the window to draw to
    GLFWwindow *window;

    // vk instance
    VkInstance instance;

    // debug messenger
    VkDebugUtilsMessengerEXT debugMessenger;

    // physical device
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE;

    // logical device
    VkDevice device;

    // create the queue family indices
    VkQueue graphicsQueue;

    // create the queue family indices
    VkQueue presentQueue;

    // surface to draw to
    VkSurfaceKHR surface;

    void initWindow()
    {
        // initialize glfw
        glfwInit();

        // tell glfw not to create an opengl context
        glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

        // create a window
        window = glfwCreateWindow(WIDTH, HEIGHT, "Vulkan", nullptr, nullptr);
    }


    // create a surface to draw to
    void createSurface()
    {
        if (glfwCreateWindowSurface(instance, window, nullptr, &surface) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create window surface!");
        }
    }

    // create a logical device
    void createLogicalDevice()
    {
        // to create a logical device we need to
        // 1. specify the queue family to create
        // 2. and the device features we will be using
        // 3. and the device extensions we will be using
        // 4. and the validation layers we will be using
        // 5. and the swap chain we will be using

        // specify the queue family to create
        QueueFamilyIndices indices = findQueueFamilies(physicalDevice);

        std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
        std::set<uint32_t> uniqueQueueFamilies = {indices.graphicsFamily.value(), indices.presentFamily.value()};

        float queuePriority = 1.0f;

        // loop through the queue families uniqueQueueFamilies and create a queue for each
        for (uint32_t queueFamily : uniqueQueueFamilies)
        {
            VkDeviceQueueCreateInfo queueCreateInfo{};
            queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
            queueCreateInfo.queueFamilyIndex = queueFamily;
            queueCreateInfo.queueCount = 1;
            queueCreateInfo.pQueuePriorities = &queuePriority;
            queueCreateInfos.push_back(queueCreateInfo);
        }

        // specify the device features
        VkPhysicalDeviceFeatures deviceFeatures{};

        VkDeviceCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

        createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());

        createInfo.pQueueCreateInfos = queueCreateInfos.data();

        createInfo.pEnabledFeatures = &deviceFeatures;

        // specify the device extensions
        createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
        createInfo.ppEnabledExtensionNames = deviceExtensions.data();

        // specify the validation layers
        if (enableValidationLayers)
        {
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();
        }
        else
        {
            createInfo.enabledLayerCount = 0;
        }

        if (vkCreateDevice(physicalDevice, &createInfo, nullptr, &device) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to create logical device!");
        }

        vkGetDeviceQueue(device, indices.graphicsFamily.value(), 0, &graphicsQueue);
        vkGetDeviceQueue(device, indices.presentFamily.value(), 0, &presentQueue);
    }

    // pick a physical device
    void pickPhysicalDevice()
    {
        // get the number of physical devices
        uint32_t deviceCount = 0;
        vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

        // check if there are no devices with vulkan support
        if (deviceCount == 0)
        {
            throw std::runtime_error("Failed to find GPUs with Vulkan support");
        }

        // get the physical devices
        std::vector<VkPhysicalDevice> devices(deviceCount);
        vkEnumeratePhysicalDevices(instance, &deviceCount, devices.data());

        // check if the physical devices are suitable
        for (const auto &device : devices)
        {
            if (isDeviceSuitable(device))
            {
                physicalDevice = device;
                break;
            }
        }

        // check if a suitable physical device was found
        if (physicalDevice == VK_NULL_HANDLE)
        {
            throw std::runtime_error("Failed to find a suitable GPU");
        }
    }

    /**
     * check if the device is a discrete gpu,
     * has geometry shader support
     * has a graphics queue family
     * has a present queue family
     * has the required extensions
     */
    bool isDeviceSuitable(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices = findQueueFamilies(device);
        bool extensionsSupported = checkDeviceExtensionSupport(device);

        // get device properties and features
        VkPhysicalDeviceProperties deviceProperties;
        VkPhysicalDeviceFeatures deviceFeatures;

        vkGetPhysicalDeviceProperties(device, &deviceProperties);
        vkGetPhysicalDeviceFeatures(device, &deviceFeatures);

        return (deviceProperties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU &&
                deviceFeatures.geometryShader) &&
               indices.isComplete() && extensionsSupported;
    }

    bool checkDeviceExtensionSupport(VkPhysicalDevice device)
    {
        // get the number of extensions
        uint32_t extensionCount;
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);

        // get the extensions
        std::vector<VkExtensionProperties> availableExtensions(extensionCount);
        vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());

        std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

        for (const auto &extension : availableExtensions)
        {
            requiredExtensions.erase(extension.extensionName);
        }

        return requiredExtensions.empty();
    }

    void createInstance()
    {
        // create a struct to hold information about the application
        VkApplicationInfo appInfo = {};
        appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
        appInfo.pApplicationName = "Triangle";
        appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.pEngineName = "No Engine";
        appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
        appInfo.apiVersion = VK_API_VERSION_1_0; // vulkan 1.0

        // create a struct to hold information about the instance
        VkInstanceCreateInfo createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
        createInfo.pApplicationInfo = &appInfo; // point to the app info struct

        // get the required extensions
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        auto extensions = getRequiredExtensions();

        // set the required extensions
        createInfo.enabledExtensionCount = static_cast<uint32_t>(extensions.size());
        createInfo.ppEnabledExtensionNames = extensions.data();

        // check if validation layers are enabled
        VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
        if (enableValidationLayers)
        {
            // check if the validation layers are supported
            if (!checkValidationLayerSupport())
            {
                throw std::runtime_error("Validation layers requested, but not available");
            }

            // set the validation layers
            createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
            createInfo.ppEnabledLayerNames = validationLayers.data();

            // create an additional debug messenger to log validation layer messages before the instance is created, and after it is destroyed
            populateDebugMessengerCreateInfo(debugCreateInfo);
            createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT *)&debugCreateInfo;
        }
        else
        {
            // set the validation layers
            createInfo.enabledLayerCount = 0;
            createInfo.pNext = nullptr;
        }

        // create the instance
        VkResult result = vkCreateInstance(&createInfo, nullptr, &instance);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create instance");
        }
    }

    QueueFamilyIndices findQueueFamilies(VkPhysicalDevice device)
    {
        QueueFamilyIndices indices;

        uint32_t queueFamilyCount = 0;
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);

        std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
        vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());

        // find a queue family that supports graphics
        int i = 0;
        for (const auto queueFamily : queueFamilies)
        {
            if (queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
            {
                indices.graphicsFamily = i;
            }
            VkBool32 presentSupport = false;
            vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport);
            if (presentSupport)
            {
                indices.presentFamily = i;
            }

            if (indices.isComplete())
            {
                break;
            }

            i++;
        }

        return indices;
    }

    void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT &createInfo)
    {
        createInfo = {};
        createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
        createInfo.messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
        createInfo.messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
        createInfo.pfnUserCallback = debugCallback;
    }

    /**
     * setup debug messenger
     */
    void setupDebugMessenger()
    {
        if (!enableValidationLayers)
            return;

        VkDebugUtilsMessengerCreateInfoEXT createInfo{};
        populateDebugMessengerCreateInfo(createInfo);

        // create the debug messenger and check if it was created successfully
        // an object is returned to allow for cleanup
        if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr, &debugMessenger) != VK_SUCCESS)
        {
            throw std::runtime_error("failed to set up debug messenger!");
        }
    }

    void cleanup()
    {
        // destory the logical device
        vkDestroyDevice(device, nullptr);

        // destroy the debug messenger if validation layers are enabled
        if (enableValidationLayers)
        {
            DestroyDebugUtilsMessengerEXT(instance, debugMessenger, nullptr);
        }

        // destroy the surface
        vkDestroySurfaceKHR(instance, surface, nullptr);

        // destroy the instance
        vkDestroyInstance(instance, nullptr);

        // destroy the window
        glfwDestroyWindow(window);

        // terminate glfw
        glfwTerminate();
    }

    /**
     * Check if the validation layers are supported
     * @return true if the validation layers are supported, false otherwise
     */
    bool checkValidationLayerSupport()
    {
        uint32_t layerCount;
        vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

        std::vector<VkLayerProperties> availableLayers(layerCount);
        vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

        // check if the layers in validationLayers exist in availableLayers
        for (const char *layerName : validationLayers)
        {
            bool layerFound = false;

            // check if the layer exists in availableLayers
            for (const auto &layerProperties : availableLayers)
            {
                if (strcmp(layerName, layerProperties.layerName) == 0)
                {
                    // layer found
                    layerFound = true;
                    break;
                }
            }

            // if the layer was not found, return false
            if (!layerFound)
            {
                return false;
            }
        }
        return true;
    }

    /**
     * Get the required extensions for the application
     * @return a vector of the required extensions
     */
    std::vector<const char *> getRequiredExtensions()
    {
        uint32_t glfwExtensionCount = 0;
        const char **glfwExtensions;
        glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

        std::vector<const char *> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

        if (enableValidationLayers)
        {
            // add the debug utils extension if validation layers are enabled to get debug messages
            extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
        }

        return extensions;
    }

    // debug callback function
    static VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
        VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
        VkDebugUtilsMessageTypeFlagsEXT messageType,
        const VkDebugUtilsMessengerCallbackDataEXT *pCallbackData,
        void *pUserData)
    {

        // check the message severity
        if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
        {
            // message is important enough to show
            std::cerr << "validation layer: " << pCallbackData->pMessage << std::endl;
        }

        return VK_FALSE;
    }

    void initVulkan()
    {
        // create a vulkan instance
        createInstance();

        // setup debug messenger
        setupDebugMessenger();

        // create a surface to draw to
        createSurface();

        // pick a physical device
        pickPhysicalDevice();

        // create a logical device
        createLogicalDevice();
    }

    void mainLoop()
    {
        // keep the window open until the user closes it
        while (!glfwWindowShouldClose(window))
        {
            glfwPollEvents();
        }
    }
};

int main()
{
    TriangleApplication app;

    try
    {
        app.run();
    }
    catch (const std::exception &e)
    {
        std::cerr << e.what() << std::endl;
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}