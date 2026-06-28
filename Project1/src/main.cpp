#ifndef NDEBUG
# pragma comment(lib, "Gallium_x64_Debug.lib")
#else
# pragma comment(lib, "Gallium_x64_Release.lib")
#endif
#pragma comment(lib, "vulkan-1.lib")

#include <gallium/platform/platform.h>
#include <gallium/platform/input.h>
#include <gallium/platform/vfs.h>

#include <gallium/gpu/device.h>
#include <gallium/gpu/commandbuffer.h>
#include <gallium/gpu/descriptorregistry.h>
#include <gallium/gpu/buffer.h>
#include <gallium/gpu/image.h>
#include <gallium/gpu/shadermodule.h>
#include <gallium/gpu/shadermodule_reflection.h>
#include <gallium/gpu/computepipeline.h>

#include <vector>
#include <cstdint>
#include <cstddef>
#include <cmath>
#include <random>

struct Sphere
{
	glm::vec3 center;
	float     radius;
	glm::vec3 albedo;
	float     fuzz;
	uint32_t  matType;
	float     refractionIndex;
	uint32_t  _pad0, _pad1;
};
static_assert(sizeof(Sphere)                    == 48, "Sphere doit faire 48 octets (layout std430)");
static_assert(offsetof(Sphere, albedo)          == 16, "albedo doit etre a l'offset 16");
static_assert(offsetof(Sphere, fuzz)            == 28, "fuzz doit etre a l'offset 28");
static_assert(offsetof(Sphere, matType)         == 32, "matType doit etre a l'offset 32");
static_assert(offsetof(Sphere, refractionIndex) == 36, "refractionIndex doit etre a l'offset 36");

int main(int argc, char** argv)
{
	auto platform = std::make_unique<ga::platform::Platform>(ga::platform::PlatformDesc {
		.appName              = "Compute playground",
		.appVersion           = { 1, 0, 0 },
		.requestedSurfaceSize = { 1600, 900 }
	});
	auto gpu = std::make_unique<ga::gpu::Device>(*platform);

	auto outputImage = std::make_unique<ga::gpu::Image>(*gpu, ga::gpu::ImageDesc {
		.type   = ga::gpu::EImageType::Image2D,
		.format = ga::gpu::EFormat::R8G8B8A8_UNorm,
		.width  = 1600,
		.height = 900,
		.usage  = ga::gpu::EImageUsage::Sampled | ga::gpu::EImageUsage::Storage | ga::gpu::EImageUsage::TransferSrc | ga::gpu::EImageUsage::TransferDst
	});
	outputImage->SetDescriptorIndex(gpu->GetDescriptorRegistry().Register(*outputImage));

	std::vector<Sphere> spheres;
	{
		std::mt19937 rng;
		std::uniform_real_distribution<float> uni(0.0f, 1.0f);
		auto rnd  = [&] { return uni(rng); };
		// Returns a random real in [min,max).
		auto rndr = [&](float lo, float hi) { return lo + (hi - lo) * uni(rng); };

		spheres.push_back({ glm::vec3(0.0f, -1000.0f, 0.0f), 1000.0f, glm::vec3(0.5f), 0.0f, 0u, 0.0f });

		for (int a = -11; a < 11; a++)
		for (int b = -11; b < 11; b++)
		{
			float     choose = rnd();
			float     cx = a + 0.9f * rnd();
			float     cz = b + 0.9f * rnd();
			glm::vec3 center(cx, 0.2f, cz);
			if (glm::length(center - glm::vec3(4.0f, 0.2f, 0.0f)) <= 0.9f)
				continue;

			if (choose < 0.8f) {          // diffuse
				float r = rnd() * rnd();
				float g = rnd() * rnd();
				float bl = rnd() * rnd();
				spheres.push_back({ center, 0.2f, glm::vec3(r, g, bl), 0.0f, 0u, 0.0f });
			} else if (choose < 0.95f) {  // metal
				float r  = rndr(0.5f, 1.0f);
				float g  = rndr(0.5f, 1.0f);
				float bl = rndr(0.5f, 1.0f);
				float fz = rndr(0.0f, 0.5f);
				spheres.push_back({ center, 0.2f, glm::vec3(r, g, bl), fz, 1u, 0.0f });
			} else {                      // glass
				spheres.push_back({ center, 0.2f, glm::vec3(1.0f), 0.0f, 2u, 1.5f });
			}
		}

		spheres.push_back({ glm::vec3( 0.0f, 1.0f, 0.0f), 1.0f, glm::vec3(1.0f),             0.0f, 2u, 1.5f });
		spheres.push_back({ glm::vec3(-4.0f, 1.0f, 0.0f), 1.0f, glm::vec3(0.4f, 0.2f, 0.1f), 0.0f, 0u, 0.0f });
		spheres.push_back({ glm::vec3( 4.0f, 1.0f, 0.0f), 1.0f, glm::vec3(0.7f, 0.6f, 0.5f), 0.0f, 1u, 0.0f });
	}
	auto sceneBuffer = std::make_unique<ga::gpu::Buffer>(*gpu, ga::gpu::EBufferUsage::StorageBuffer, spheres);

	auto shader = platform->Vfs().Open("/shaders/compute.slang.spv")
		.and_then([](auto f) { return f.ReadBytes(); })
		.transform([&gpu](auto bytes) { return std::make_unique<ga::gpu::ShaderModule>(*gpu, bytes.data(), bytes.size()); })
		.value();

	auto pipeline = std::make_unique<ga::gpu::ComputePipeline>(*gpu, ga::gpu::ComputePipelineInfo {
		.stage = { *shader, ga::gpu::EShaderStage::Compute, "cs_main" }
	});

	auto pushConstants = shader->GetStruct("PushConstants")->Instantiate();
	pushConstants["imageSize"]   = glm::uvec2(outputImage->Size());
	pushConstants["outputId"]    = outputImage->GetDescriptorIndex().slot;
	pushConstants["sceneId"]         = sceneBuffer->GetDescriptorIndex().slot;
	pushConstants["sphereCount"]     = uint32_t(spheres.size());
	pushConstants["samplesPerPixel"] = uint32_t(50);   // Count of random samples for each pixel
	pushConstants["maxDepth"]        = uint32_t(50);   // Maximum number of ray bounces into scene

	{
		glm::uvec2 dims = glm::uvec2(outputImage->Size());

		float     vfov          = 20.0f;                          // Vertical view angle (field of view)
		glm::vec3 lookfrom      = glm::vec3(13.0f, 2.0f, 3.0f);   // Point camera is looking from
		glm::vec3 lookat        = glm::vec3( 0.0f, 0.0f, 0.0f);   // Point camera is looking at
		glm::vec3 vup           = glm::vec3( 0.0f, 1.0f, 0.0f);   // Camera-relative "up" direction
		float     defocus_angle = 0.6f;                           // Variation angle of rays through each pixel
		float     focus_dist    = 10.0f;                          // Distance from camera lookfrom point to plane of perfect focus

		glm::vec3 camera_center = lookfrom;   // Camera center

		// Determine viewport dimensions.
		float     theta         = glm::radians(vfov);
		float     h             = std::tan(theta / 2.0f);
		float     viewport_h    = 2.0f * h * focus_dist;
		float     viewport_w    = viewport_h * (float(dims.x) / float(dims.y));

		// Calculate the u,v,w unit basis vectors for the camera coordinate frame.
		glm::vec3 w = glm::normalize(lookfrom - lookat);
		glm::vec3 u = glm::normalize(glm::cross(vup, w));
		glm::vec3 v = glm::cross(w, u);        // Camera frame basis vectors

		// Calculate the vectors across the horizontal and down the vertical viewport edges.
		glm::vec3 viewport_u = viewport_w * u;    // Vector across viewport horizontal edge
		glm::vec3 viewport_v = viewport_h * -v;   // Vector down viewport vertical edge

		// Calculate the horizontal and vertical delta vectors to the next pixel.
		glm::vec3 pixel_delta_u = viewport_u / float(dims.x);   // Offset to pixel to the right
		glm::vec3 pixel_delta_v = viewport_v / float(dims.y);   // Offset to pixel below

		// Calculate the location of the upper left pixel.
		glm::vec3 viewport_upper_left = camera_center
			- (focus_dist * w)
			- viewport_u / 2.0f
			- viewport_v / 2.0f;
		glm::vec3 pixel00_loc = viewport_upper_left + 0.5f * (pixel_delta_u + pixel_delta_v);   // Location of pixel 0, 0

		// Calculate the camera defocus disk basis vectors.
		float     defocus_radius = focus_dist * std::tan(glm::radians(defocus_angle / 2.0f));
		glm::vec3 defocus_disk_u = u * defocus_radius;   // Defocus disk horizontal radius
		glm::vec3 defocus_disk_v = v * defocus_radius;   // Defocus disk vertical radius

		pushConstants["cameraCenter"] = camera_center;
		pushConstants["pixel00Loc"]   = pixel00_loc;
		pushConstants["pixelDeltaU"]  = pixel_delta_u;
		pushConstants["pixelDeltaV"]  = pixel_delta_v;
		pushConstants["defocusDiskU"] = defocus_disk_u;
		pushConstants["defocusDiskV"] = defocus_disk_v;
	}

	{
		auto cb = gpu->AcquireCommandBuffer();
		cb->Record([&](const ga::gpu::CommandEncoder& encoder) {
			encoder.Transition(*outputImage, ga::gpu::EImageLayout::General);
			encoder.Compute({ *pipeline }, [&](const ga::gpu::ComputeEncoder& compute) {
				glm::uvec2 numWorkgroups = glm::uvec2(outputImage->Size()) / glm::uvec2(16) + glm::uvec2(1);
				compute.PushConstants(pushConstants);
				compute.Dispatch({ numWorkgroups, 1 });
			});
			encoder.Transition(*outputImage, ga::gpu::EImageLayout::TransferSrcOptimal);
		}, ga::gpu::ECommandBufferUsage::OneTimeSubmit);
		gpu->SubmitAndWait(cb);
		gpu->ReleaseCommandBuffer(cb);
	}

	do
	{
		platform->PollEvents();
		auto acquiredImage = gpu->AcquireNextImage();

		auto cb = gpu->AcquireCommandBuffer();
		cb->Record([&](const ga::gpu::CommandEncoder& encoder) {
			encoder.Transition(acquiredImage.image, ga::gpu::EImageLayout::TransferDstOptimal);
			encoder.Transfer([&](const ga::gpu::TransferEncoder& transfer) {
				transfer.BlitImage(*outputImage, acquiredImage.image, {}, {}, { 1600u, 900u, 1u });
			});
			encoder.Transition(acquiredImage.image, ga::gpu::EImageLayout::PresentSrc);
		}, ga::gpu::ECommandBufferUsage::OneTimeSubmit);
		gpu->Submit(cb, { acquiredImage.presentComplete }, acquiredImage.renderComplete);
		gpu->ReleaseCommandBuffer(cb);

		gpu->Present({ acquiredImage.renderComplete });
	} while (!platform->IsExitRequested());

	gpu->WaitIdle();
	return 0;
}
