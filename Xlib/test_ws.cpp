//
// Created by af on 10.08.21.
//

#include "vulkan_render.h"
#include "ws_xlib.h"

#define SET_RGB(R, G, B) (R << 16 | G << 8 | B << 0)

#include <cstdio>
#include <cstdlib>
#include <unistd.h>

#include <cstring>
#include <memory_resource>

std::string msg;

struct {
	X::X_Display *display = nullptr;
	X::X_Screen screen{};
	X::X_Window window;
} state;

struct {
	VkDevice device{};
	VkSurfaceKHR surface{};
	VkSwapchainKHR swapchain = VK_NULL_HANDLE;
	uint32_t imagesInSwapchain = 0;
	VkSurfaceFormatKHR format{};
	VkSurfaceFormatKHR other_format{};
	VkPhysicalDevice physicalDevice{};
	VkPipelineLayout pipelineLayout{};
	VkRenderPass renderPass{};
	VkPipeline pipeline{};
	VkPipeline dynamic_pipeline{};
	std::vector<VkFramebuffer> framebuffers{};
	std::vector<VK::CommandBuffer<VK_QUEUE_GRAPHICS_BIT>> commandBuffers{};
	VkSemaphore imageAvailable{};
	VkSemaphore renderingDone{};
	VkQueue queue{};
	VK::CommandPool<VK_QUEUE_GRAPHICS_BIT> *commandPool;
	std::vector<VkImageView> swapchainImageViews{};
	std::vector<VkImage> swapchainImages{};
	VkBuffer vertexBuffer{};
	VkDeviceMemory vertexBufferMemory;
	VkBuffer indexBuffer{};
	VkDeviceMemory indexBufferMemory;
	VkBuffer uniformBuffer{};
	VkDeviceMemory uniformBufferMemory;
	VkDescriptorSetLayout uniformDescriptorLayout;
	VkDescriptorSet uniformDescriptor;
	VkDescriptorPool uniformDescriptorPool;
} vulkan_state;

struct {
	VkShaderModule vertShaderModule{};
	VkShaderModule fragShaderModule{};
	//std::array<VK::Vertex, 4> vertices{VK::Vertex(glm::vec2(-1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 0.0f)),
	//								   VK::Vertex(glm::vec2(-1.0f, 1.0f), glm::vec3(1.0f, 0.0f, 1.0f)),
	//								   VK::Vertex(glm::vec2(1.0f, -1.0f), glm::vec3(1.0f, 0.0f, 1.0f)),
	//								   VK::Vertex(glm::vec2(1.0f, 1.0f), glm::vec3(0.0f, 1.0f, 1.0f))};
	std::array<VK::Vertex3D, 3> vertices{
			VK::Vertex3D(glm::vec3(-1.0f, -1.0f, 0.0f),
						 glm::vec3(1.0f, 0.0f, 0.0f)),
			VK::Vertex3D(glm::vec3(-1.0f, 1.0f, 0.0f),
						 glm::vec3(1.0f, 0.0f, 1.0f)),
			VK::Vertex3D(glm::vec3(1.0f, -1.0f, 0.0f),
						 glm::vec3(1.0f, 0.0f, 1.0f))};
	std::array<int, 6> indices{0, 1, 2, 1, 2, 3};
	std::array<int, 6> indices2{0, 1, 3, 3, 2, 0};
	std::vector<int> indicesCube;
	std::vector<VK::Vertex3D> verticesCube;
	VK::MVP mvp;
	glm::mat4 model = glm::identity<glm::mat4>();
	glm::mat4 pro = glm::perspective(glm::radians(90.0f), 1.0f, 0.001f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(4.0f, 4.0f, 0.0f),
								 glm::vec3(0.0f, 0.0f, 0.0f),
								 glm::vec3(0.0f, 1.0f, 0.0f));
	//std::vector<VK::Vertex> vertices{VK::Vertex(glm::vec2(0.0f, -0.8f), glm::vec3(1.0f, 0.0f, 0.0f)),
	//								 VK::Vertex(glm::vec2(-0.8f, 0.8f), glm::vec3(1.0f, 0.0f, 1.0f)),
	//								 VK::Vertex(glm::vec2(0.8f, 0.8f), glm::vec3(1.0f, 0.0f, 1.0f))};
	VK::PushConstant push;
	VK::PushConstantVariable<VK::Vertex3DPosData> pushVar;
} vulkan_resource;

void updateMVP() {
	static float f = 0.0;
	f += 1.0f;
	glm::mat4 pro = glm::perspective(glm::radians(60.0f), 800.0f / 600.0f,
									 0.001f, 100.0f);
	glm::mat4 view = glm::lookAt(glm::vec3(-2.0f, -2.0f, -2.0f),
								 glm::vec3(0.0f, 0.0f, 0.0f),
								 glm::vec3(0.0f, 1.0f, 0.0f));
	vulkan_resource.model = glm::rotate(glm::mat4(1.0f),
										glm::radians(20.0f) * f,
										glm::vec3(0.0f, 1.0f, 0.0f));
	vulkan_resource.mvp = VK::MVP(pro * view * vulkan_resource.model);
	//vulkan_resource.mvp.mvp[1][1] *= -1;
	void *vBufferMem = VK::mapMemory(vulkan_state.device,
									 vulkan_state.uniformBufferMemory);
	memcpy(vBufferMem, &vulkan_resource.mvp, sizeof(VK::MVP));
	VK::unmapMemory(vulkan_state.device, vulkan_state.uniformBufferMemory);
}

void generateCube() {
	std::vector<VK::Vertex3D> cube;
	for (int x = 0; x < 2; x++) {
		for (int y = 0; y < 2; y++) {
			for (int z = 0; z < 2; z++) {
				cube.push_back({{(static_cast<float>(x) -
								  0.5f),    (static_cast<float>(y) -
											 0.5f),    (static_cast<float>(z) -
														0.5f)},
								{(float) x, (float) y, (float) z}});
			}
		}
	}
	//cube.push_back({{0.5f, -0.5f, 0.0f}, {1.0f, 0.0f, 0.0f}});
	//cube.push_back({{0.5f, 0.5f, 0.0f}, {0.0f, 1.0f, 0.0f}});
	//cube.push_back({{-0.5f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}});
	std::vector<int> indices;
	for (int a = 2; a >= 0; a--) {
		for (int b = 1; b >= 0; b--) {
			int con = b << a;
			int first = (a != 0) ? 0 : 1;
			int second = (a != 2) ? 2 : 1;
			std::cout << " " << a << "/" << first << "/" << second << " ";
			indices.push_back(con | (0 << first) | (0 << second));
			indices.push_back(con | (1 << first) | (0 << second));
			indices.push_back(con | (0 << first) | (1 << second));
			indices.push_back(con | (1 << first) | (0 << second));
			indices.push_back(con | (0 << first) | (1 << second));
			indices.push_back(con | (1 << first) | (1 << second));
		}
	}
	//indices = {};
	//indices.push_back(1);
	//indices.push_back(2);
	//indices.push_back(0);
	for (int i: indices) {
		std::cout << cube[i].position.z << " ";
	}
	std::cout << std::endl;
	for (auto i: cube) {
		std::cout << i.position.z << " ";
	}
	vulkan_resource.verticesCube = cube;
	vulkan_resource.indicesCube = indices;
}


void allocateVertexBuffer() {

	vkDeviceWaitIdle(vulkan_state.device);// TODO Sync properly

	VK::freeMemory(vulkan_state.device, vulkan_state.vertexBufferMemory);
	VK::destroyBuffer(vulkan_state.device, vulkan_state.vertexBuffer);
	VK::freeMemory(vulkan_state.device, vulkan_state.indexBufferMemory);
	VK::destroyBuffer(vulkan_state.device, vulkan_state.indexBuffer);

	vulkan_state.vertexBuffer = VK::createBuffer<VK_BUFFER_USAGE_VERTEX_BUFFER_BIT>(
			vulkan_state.device,
			vulkan_resource.verticesCube.size() * sizeof(VK::Vertex3D));
	vulkan_state.indexBuffer = VK::createBuffer<VK_BUFFER_USAGE_INDEX_BUFFER_BIT>(
			vulkan_state.device,
			vulkan_resource.indicesCube.size() * sizeof(int));

	auto memoryType = VK::findMemory(vulkan_state.physicalDevice,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
									 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


	VkMemoryRequirements memoryRequirements = VK::getMemoryRequirement(
			vulkan_state.device, vulkan_state.vertexBuffer);
	VkMemoryRequirements memoryIndexRequirements = VK::getMemoryRequirement(
			vulkan_state.device, vulkan_state.indexBuffer);

	vulkan_state.vertexBufferMemory = VK::allocateMemory(vulkan_state.device,
														 memoryRequirements.size,
														 memoryType.index);
	vulkan_state.indexBufferMemory = VK::allocateMemory(vulkan_state.device,
														memoryIndexRequirements.size,
														memoryType.index);

	VK::bind(vulkan_state.device, vulkan_state.vertexBuffer,
			 {0, vulkan_state.vertexBufferMemory, 0});
	void *vBufferMem = VK::mapMemory(vulkan_state.device,
									 vulkan_state.vertexBufferMemory);
	for (auto vertex: vulkan_resource.verticesCube) {
		std::cout << "[[" << vertex.position.x << ", " << vertex.position.y
				  << ", " << vertex.position.y << "], [" << vertex.color.x
				  << ", " << vertex.color.y << ", " << vertex.color.y << "]"
				  << std::endl;
	}
	std::cout << vulkan_resource.indicesCube.size() << std::endl;
	memcpy(vBufferMem, vulkan_resource.verticesCube.data(),
		   vulkan_resource.verticesCube.size() * sizeof(VK::Vertex3D));
	VK::unmapMemory(vulkan_state.device, vulkan_state.vertexBufferMemory);

	VK::bind(vulkan_state.device, vulkan_state.indexBuffer,
			 {0, vulkan_state.indexBufferMemory, 0});
	vBufferMem = VK::mapMemory(vulkan_state.device,
							   vulkan_state.indexBufferMemory);
	memcpy(vBufferMem, vulkan_resource.indicesCube.data(),
		   vulkan_resource.indicesCube.size() * sizeof(int));
	VK::unmapMemory(vulkan_state.device, vulkan_state.indexBufferMemory);
};

void allocateUniformBuffer() {

	vkDeviceWaitIdle(vulkan_state.device);// TODO Sync properly

	vulkan_state.uniformBuffer = VK::createBuffer<VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT>(
			vulkan_state.device, sizeof(VK::MVP));

	auto memoryType = VK::findMemory(vulkan_state.physicalDevice,
									 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
									 VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);


	VkMemoryRequirements memoryRequirements = VK::getMemoryRequirement(
			vulkan_state.device, vulkan_state.uniformBuffer);

	vulkan_state.uniformBufferMemory = VK::allocateMemory(vulkan_state.device,
														  memoryRequirements.size,
														  memoryType.index);
	VK::bind(vulkan_state.device, vulkan_state.uniformBuffer,
			 {0, vulkan_state.uniformBufferMemory, 0});

	updateMVP();
};

void recordCommandBuffersDynamicPipeline(const auto FRAME_SIZE) {
	for (size_t i = 0; i < vulkan_state.framebuffers.size(); i++) {
		const auto &commandBuffer = vulkan_state.commandBuffers[i];
		const auto framebuffer = vulkan_state.framebuffers[i];
		VK::recordCommandBuffer(commandBuffer, [&]() {
			commandBuffer.beginRenderPass(vulkan_state.renderPass, framebuffer,
										  FRAME_SIZE)
					.setScissor(FRAME_SIZE)
					.setViewport(FRAME_SIZE)
					.bindVertexBuffer(vulkan_state.vertexBuffer)
					.bindIndexBuffer(vulkan_state.indexBuffer)
					.template bindDescriptorSets<VK_PIPELINE_BIND_POINT_GRAPHICS>(
							vulkan_state.pipelineLayout,
							SPAN_E(vulkan_state.uniformDescriptor))
					.bindGraphicsPipeline(vulkan_state.pipeline);
			for (int j = -5; j < 5; j++) {
				for (int k = -5; k < 5; k++) {
					glm::vec3 vec = {float(j) * 0.2f,
									 float(abs(j) > abs(k) ? abs(j) : abs(k)) *
									 0.2f, float(k) * 0.2f};
					vulkan_resource.pushVar.write(vulkan_resource.push, {vec});
					commandBuffer.pushConstants(vulkan_state.pipelineLayout,
												vulkan_resource.push)
							.drawIndexed(vulkan_resource.indicesCube.size());
				}
			}
			commandBuffer.endRenderPass();
		});
	}
}


void initDynamic() {

	VkDevice &device = vulkan_state.device;
	vulkan_state.pipelineLayout = VK::createPipelineLayout(device,
														   SPAN_E(vulkan_state.uniformDescriptorLayout),
														   SPAN_E(vulkan_resource.push.crateRange()));

	VkAttachmentDescription attachmentDescription = VK::generateAttachmentDescriptionToPresent(
			vulkan_state.format.format);
	std::vector<VkAttachmentReference> attachmentReferences = {
			{0, VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL}};
	VkSubpassDescription subpassDescription = VK::generateSubpass<VK_PIPELINE_BIND_POINT_GRAPHICS>(
			{},
			attachmentReferences);


	VkSubpassDependency subpassDependency{
			.srcSubpass = VK_SUBPASS_EXTERNAL,
			.dstSubpass = 0,
			.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0,
			.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT |
							 VK_ACCESS_COLOR_ATTACHMENT_READ_BIT,
			.dependencyFlags = 0,
	};


	std::vector<VkSubpassDescription> subpasses = {subpassDescription};
	std::vector<VkSubpassDependency> subpassDependencies = {subpassDependency};

	vulkan_state.renderPass = VK::createRenderPass(device,
												   {attachmentDescription},
												   subpasses,
												   subpassDependencies);


	vulkan_state.pipeline = VK::createGraphicsPipeline(device,
													   {vulkan_resource.vertShaderModule,
														"main"},
													   {vulkan_resource.fragShaderModule,
														"main"},
													   {VK::Vertex3D::getVertexBindingDescription()},
													   VK::Vertex3D::getAttributeDescriptions(),
													   vulkan_state.pipelineLayout,
													   vulkan_state.renderPass,
													   0,
													   {VK_DYNAMIC_STATE_VIEWPORT,
														VK_DYNAMIC_STATE_SCISSOR},
													   {0, 0});
}

void createSwapchainDynamicPipeline() {
	VkDevice &device = vulkan_state.device;
	VkSurfaceCapabilitiesKHR capabilities = VK::getSurfaceCapabilities(
			vulkan_state.physicalDevice,
			vulkan_state.surface);
	const auto FRAME_SIZE = capabilities.currentExtent;
	vulkan_state.swapchain = VK::createSwapchain(device, vulkan_state.surface,
												 vulkan_state.imagesInSwapchain,
												 vulkan_state.format,
												 capabilities.currentExtent, 0,
												 VK_PRESENT_MODE_FIFO_KHR,
												 vulkan_state.swapchain);

	vulkan_state.swapchainImages = VK::getSwapchainImage(device,
														 vulkan_state.swapchain);

	VK::destroyImageView(device, vulkan_state.swapchainImageViews);
	vulkan_state.swapchainImageViews = VK::createImageViews(device,
															vulkan_state.swapchainImages,
															vulkan_state.format.format);


	VK::destroyFramebuffers(device, vulkan_state.framebuffers);
	vulkan_state.framebuffers = VK::createFramebuffers(device,
													   vulkan_state.renderPass,
													   vulkan_state.swapchainImageViews,
													   FRAME_SIZE);

	vkDeviceWaitIdle(device);
	VK::freeCommandBuffers<VK_QUEUE_GRAPHICS_BIT>(vulkan_state.commandPool,
												  SPAN(vulkan_state.commandBuffers));

	vulkan_state.commandBuffers = VK::allocateCommandBuffers(
			vulkan_state.commandPool,
			vulkan_state.framebuffers.size());

	recordCommandBuffersDynamicPipeline(FRAME_SIZE);
}


void drawVulkan() {

	uint32_t image = VK::acquireSwapchainImage(vulkan_state.device,
											   vulkan_state.swapchain,
											   vulkan_state.imageAvailable);

	VK::queueSubmit(vulkan_state.queue, {vulkan_state.imageAvailable},
					{VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT},
					{vulkan_state.renderingDone},
					{vulkan_state.commandBuffers[image].commandBuffer});

	VK::presentImage(vulkan_state.queue, {vulkan_state.renderingDone},
					 vulkan_state.swapchain, image);
}

bool draw(X::X_ExposeEvent) {
	/*
	auto &display = state.display;
	auto &screen = state.screen;
	auto &window = state.window;
	X::X_Rectangle rect = {{20, 20,},
						   {10, 10}};
	display->drawRectangle<true>(window, display->getDefaultGraphicsContext(screen), rect);
	auto stringLocation = X::X_Point(50, 50);
	display->drawString(window, display->getDefaultGraphicsContext(screen), stringLocation, msg);
	display->flush();
	 */
	createSwapchainDynamicPipeline();
	drawVulkan();
	return true;
}

bool changeColorspace(X::X_KeyEvent) {
	updateMVP();
	((void (*)()) draw)();

	return true;
}


bool quit(X::X_MouseButtonEvent) {
	state.display->quit();
	return true;
}

bool change(X::X_ConfigureEvent) {
	auto &display = state.display;
	//auto &screen = state.screen;
	auto &window = state.window;
	display->showWindow(window);
	return true;
}

/*
int main() {
	msg = "Hello, World!";

	X::X_Display display;
	state.display = &display;
	if (!display.check()) {
		exit(1);
	}

	state.screen = display.getScreen();

	X::X_Rectangle windowDimensions = {{10,  10},
									   {200, 200}};
	auto border = X::X_Border(state.screen.getColor<X::X_ColorBlack>(), 2);
	state.window = display.createSimpleWindow(state.screen, windowDimensions, border,
											  state.screen.getColor<X::X_ColorWhite>(), ExposureMask | KeyPressMask);


	display.showWindow(display.createSimpleWindow(state.window, {{60, 60},
																 {60, 60}}, border,
												  state.screen.getColor<X::X_ColorWhite>(), ExposureMask | KeyPressMask));


	display.showWindow(state.window);
	display.addExposeEvent(state.window, draw);
	display.addKeyPressedEvent(state.window, quit);

	display.runEventLoop();

	return 0;
}
*/

#include <fstream>

std::vector<unsigned int> readShader(const std::string &path) {
	auto file = std::ifstream(path, std::ios::binary | std::ios::ate);
	if (!file.is_open()) {
		std::cout << "no such file" << std::endl;
		exit(-1);
	}


	auto fileSize = file.tellg();
	std::vector<unsigned int> buffer(fileSize / 4);

	file.seekg(0);
	file.read((char *) buffer.data(), fileSize);

	file.close();

	return buffer;
}


int main() {
	vulkan_resource.pushVar.submit(vulkan_resource.push,
								   VK_SHADER_STAGE_VERTEX_BIT);
	std::cout << sizeof(VK::Vertex3D) << std::endl;
	generateCube();
	msg = "Hello, World!";
	X::X_Display display;
	state.display = &display;
	state.screen = display.getScreen();
	state.window = display.createSimpleWindow(state.screen, {{0,   0},
															 {800, 600}},
											  X::X_Border(
													  state.screen.getColor<X::X_ColorBlack>(),
													  2),
											  state.screen.getColor<X::X_ColorWhite>(),
											  ExposureMask | ButtonPressMask |
											  KeyPressMask);

	VK::printInstanceExtensions();
	VK::printInstanceLayers();
	VkInstance instance = VK::createInstance();
	VK::printDeviceInfo(instance);
	vulkan_state.physicalDevice = VK::getBestDevice(instance);
	VK::printQueues(vulkan_state.physicalDevice);


	vulkan_state.surface = display.createSurface(instance, state.window);
	if (!VK::queueCanPresent(vulkan_state.physicalDevice, 0,
							 vulkan_state.surface)) {
		std::cout << "cant present" << std::endl;
		exit(-1);
	}

	VK::printMemoryInfo(vulkan_state.physicalDevice);

	VK::printFormats(VK::getSurfaceFormats(vulkan_state.physicalDevice,
										   vulkan_state.surface));
	VK::printPresentModes(VK::getPresentModes(vulkan_state.physicalDevice,
											  vulkan_state.surface));

	vulkan_state.format = VK::getSurfaceFormats(vulkan_state.physicalDevice,
												vulkan_state.surface)[0];
	vulkan_state.other_format = VK::getSurfaceFormats(
			vulkan_state.physicalDevice, vulkan_state.surface)[1];

	auto queueFamilies = VK::getQueueFamilies(vulkan_state.physicalDevice);

	vulkan_state.device = VK::createDevice(vulkan_state.physicalDevice,
										   queueFamilies);
	VkDevice &device = vulkan_state.device;

	allocateVertexBuffer();
	allocateUniformBuffer();

	vulkan_state.uniformDescriptorLayout = VK::createDescriptorSetLayout(device,
																		 SPAN(VK::MVP::bindings));
	VkDescriptorPoolSize poolSize{.type = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, .descriptorCount = 1};
	vulkan_state.uniformDescriptorPool = VK::createDescriptorPool(device,
																  SPAN_E(poolSize));
	vulkan_state.uniformDescriptor = VK::allocateDescriptorSets(device,
																vulkan_state.uniformDescriptorPool,
																SPAN_E(vulkan_state.uniformDescriptorLayout))[0];

	VkDescriptorBufferInfo descriptorBufferInfo{
			.buffer = vulkan_state.uniformBuffer,
			.offset = 0,
			.range = sizeof(VK::MVP),
	};

	std::cout << " a " << vulkan_state.uniformDescriptor << std::endl;

	VkWriteDescriptorSet writeDescriptorSet{
			.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			.pNext = nullptr,
			.dstSet = vulkan_state.uniformDescriptor,
			.dstBinding = 0,
			.dstArrayElement = 0,
			.descriptorCount = 1,
			.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			.pImageInfo = nullptr,
			.pBufferInfo = &descriptorBufferInfo,
			.pTexelBufferView = nullptr};

	vkUpdateDescriptorSets(device, 1, &writeDescriptorSet, 0, nullptr);


	vulkan_state.imagesInSwapchain = VK::getImageCountInSwapchain(
			VK::getSurfaceCapabilities(vulkan_state.physicalDevice,
									   vulkan_state.surface))
			.getMinimum();

	std::cout << std::hex
			  << VK::getSurfaceCapabilities(vulkan_state.physicalDevice,
											vulkan_state.surface).supportedUsageFlags
			  << std::endl;


	for (auto &queueFamily: queueFamilies) {
		queueFamily.createQueues(device);
	}
	vulkan_state.queue = queueFamilies[0].queues[0];


	auto shaderCodeVert = readShader("vert.spv");
	auto shaderCodeFrag = readShader("frag.spv");
	vulkan_resource.vertShaderModule = VK::createShader(device, shaderCodeVert);
	vulkan_resource.fragShaderModule = VK::createShader(device, shaderCodeFrag);
	vulkan_state.imageAvailable = VK::createSemaphore(device);
	vulkan_state.renderingDone = VK::createSemaphore(device);


	{

		auto commandPool = VK::CommandPool<VK_QUEUE_GRAPHICS_BIT>(device,
																  queueFamilies[0]);
		vulkan_state.commandPool = &commandPool;


		initDynamic();
		createSwapchainDynamicPipeline();


		display.addExposeEvent(state.window, draw);
		display.addButtonPressedEvent(state.window, quit);
		display.addKeyPressedEvent(state.window, changeColorspace);
		display.showWindow(state.window);
		display.runEventLoop();


		vkDeviceWaitIdle(device);

		VK::freeCommandBuffers(vulkan_state.commandPool,
							   SPAN(vulkan_state.commandBuffers));
	}

	VK::destroySemaphore(device, vulkan_state.imageAvailable);
	VK::destroySemaphore(device, vulkan_state.renderingDone);

	VK::destroyFramebuffers(device, vulkan_state.framebuffers);

	VK::destroyPipeline(device, vulkan_state.pipeline);

	VK::destroyRenderPass(device, vulkan_state.renderPass);
	VK::destroyPipelineLayout(device, vulkan_state.pipelineLayout);
	VK::destroyShader(device, vulkan_resource.vertShaderModule);
	VK::destroyShader(device, vulkan_resource.fragShaderModule);


	VK::destroyImageView(device, vulkan_state.swapchainImageViews);
	VK::destroySwapchain(device, vulkan_state.swapchain);

	VK::freeMemory(device, vulkan_state.vertexBufferMemory);
	VK::destroyBuffer(device, vulkan_state.vertexBuffer);
	VK::freeMemory(device, vulkan_state.indexBufferMemory);
	VK::destroyBuffer(device, vulkan_state.indexBuffer);
	VK::freeMemory(device, vulkan_state.uniformBufferMemory);
	VK::destroyBuffer(device, vulkan_state.uniformBuffer);

	VK::destroyDevice(device);

	X::X_Display::destroySurface(instance, vulkan_state.surface);
	VK::destroyInstance(instance);

	// TODO VK::queueCanPresent results in SEGV in XCloseDisplay
	return 0;
}
