#include "init.h"
#include "defer.h"
#include "sdl_exception.h"
#include "sdl_wgpu.h"
#include "state.h"
#include "wgpu_exception.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_stdinc.h>
#include <SDL3/SDL_video.h>
#include <array>
#include <cassert>
#include <cstddef>
#include <iostream>
#include <string>
#include <webgpu/wgpu.h>
namespace jmauto {
void Init() {
  if (!SDL_Init(SDL_INIT_VIDEO)) {
    throw SDLException("init sdl");
  }
  state->releaser.Push([] {
    std::cout << "quit sdl\n";
    SDL_Quit();
  });
  state->window = SDL_CreateWindow("auto", 1280, 720, SDL_WINDOW_HIDDEN);
  if (!state->window) {
    throw SDLException("create window");
  }
  state->releaser.Push([] {
    std::cout << "destroy window\n";
    SDL_DestroyWindow(state->window);
  });
  wgpuSetLogLevel(WGPULogLevel_Warn);
  wgpuSetLogCallback(
      [](WGPULogLevel level, WGPUStringView message, void *userdata) {
        const char *levelstr = "";
        switch (level) {
        case WGPULogLevel_Off:
          levelstr = "Off";
          break;
        case WGPULogLevel_Error:
          levelstr = "Error";
          break;
        case WGPULogLevel_Warn:
          levelstr = "Warn";
          break;
        case WGPULogLevel_Info:
          levelstr = "Info";
          break;
        case WGPULogLevel_Debug:
          levelstr = "Debug";
          break;
        case WGPULogLevel_Trace:
          levelstr = "Trace";
          break;
        case WGPULogLevel_Force32:
          levelstr = "Force32";
          break;
        }
        std::cout << "wgpu [" << levelstr << "] -> "
                  << std::string(message.data, message.length) << '\n';
      },
      nullptr);
  state->instance = wgpuCreateInstance(nullptr);
  if (!state->instance) {
    throw WGPUException("create instance");
  }
  state->releaser.Push([] {
    std::cout << "release wpgu instance\n";
    wgpuInstanceRelease(state->instance);
  });
  state->surface = GetSurface();
  state->releaser.Push([] {
    std::cout << "release surface\n";
    wgpuSurfaceRelease(state->surface);
  });
  {
    WGPURequestAdapterCallbackInfo callbackinfo = {};
    callbackinfo.callback = [](WGPURequestAdapterStatus status,
                               WGPUAdapter adapter, WGPUStringView message,
                               WGPU_NULLABLE void *userdata1,
                               WGPU_NULLABLE void *userdata2) {
      if (status != WGPURequestAdapterStatus_Success) {
        std::cerr << "adapter request failed\n"
                  << "status " << status << '\n'
                  << "message: " << std::string(message.data, message.length);
      }
      state->adapter = adapter;
      state->releaser.Push([] {
        wgpuAdapterRelease(state->adapter);
        std::cout << "release adapter\n";
      });
    };
    WGPURequestAdapterOptions options = {};
    options.compatibleSurface = state->surface;
    options.powerPreference = WGPUPowerPreference_HighPerformance;
    wgpuInstanceRequestAdapter(state->instance, &options, callbackinfo);
  }
  {
    WGPUDeviceDescriptor descriptor = {};
    descriptor.deviceLostCallbackInfo.callback =
        [](WGPUDevice const *device, WGPUDeviceLostReason reason,
           WGPUStringView message, WGPU_NULLABLE void *userdata1,
           WGPU_NULLABLE void *userdata2) {
          std::cerr << "device lost\n"
                    << "reason " << reason << '\n'
                    << "message: " << std::string(message.data, message.length)
                    << '\n';
        };
    descriptor.uncapturedErrorCallbackInfo.callback =
        [](WGPUDevice const *device, WGPUErrorType type, WGPUStringView message,
           WGPU_NULLABLE void *userdata1, WGPU_NULLABLE void *userdata2) {
          const char *typestr = "";
          switch (type) {
          case WGPUErrorType_NoError:
            typestr = "NoError";
            break;
          case WGPUErrorType_Validation:
            typestr = "Validation";
            break;
          case WGPUErrorType_OutOfMemory:
            typestr = "OutOfMemory";
            break;
          case WGPUErrorType_Internal:
            typestr = "Internal";
            break;
          case WGPUErrorType_Unknown:
            typestr = "Unknown";
            break;
          case WGPUErrorType_Force32:
            typestr = "Force32";
            break;
          }
          std::cerr << "uncaptured wgpu [" << typestr << "] -> "
                    << std::string(message.data, message.length) << '\n';
        };
    WGPURequestDeviceCallbackInfo callbackinfo = {};
    callbackinfo.callback = [](WGPURequestDeviceStatus status,
                               WGPUDevice device, WGPUStringView message,
                               WGPU_NULLABLE void *userdata1,
                               WGPU_NULLABLE void *userdata2) {
      if (status != WGPURequestDeviceStatus_Success) {
        std::cerr << "request device error\n"
                  << "status " << status << '\n'
                  << "message: " << std::string(message.data, message.length)
                  << '\n';
      }
      state->device = device;
      state->releaser.Push([] {
        std::cout << "release device\n";
        wgpuDeviceRelease(state->device);
      });
    };
    wgpuAdapterRequestDevice(state->adapter, &descriptor, callbackinfo);
  }
  state->queue = wgpuDeviceGetQueue(state->device);
  if (!state->queue) {
    throw WGPUException("get queue");
  }
  {
    size_t length;
    const char *data = (const char *)SDL_LoadFile("wgsl/render.wgsl", &length);
    if (!data) {
      throw SDLException("load file");
    }
    // state->releaser.Push([data]() {
    //   SDL_free((void *)data);
    //   std::cout << "free code data\n";
    // });
    Defer free_code([data] {
      SDL_free((void *)data);
      std::cout << "free code data\n";
    });
    WGPUShaderSourceWGSL source = {};
    source.code.length = length;
    source.code.data = data;
    source.chain.sType = WGPUSType_ShaderSourceWGSL;
    WGPUShaderModuleDescriptor descriptor = {};
    descriptor.nextInChain = &source.chain;
    state->render_module =
        wgpuDeviceCreateShaderModule(state->device, &descriptor);
    if (!state->render_module) {
      throw WGPUException("create shader module");
    }
    state->releaser.Push([] {
      wgpuShaderModuleRelease(state->render_module);
      std::cout << "release shader module\n";
    });
  }
  {
    WGPUPipelineLayoutDescriptor descriptor = {};
    descriptor.label = {"render pipeline layout", WGPU_STRLEN};
    state->pipeline_layout =
        wgpuDeviceCreatePipelineLayout(state->device, &descriptor);
    if (!state->pipeline_layout) {
      throw WGPUException("create pipeline layout");
    }
    state->releaser.Push([] {
      wgpuPipelineLayoutRelease(state->pipeline_layout);
      std::cout << "release pipeline layout\n";
    });
  }
  state->releaser.Push([] {
    wgpuSurfaceCapabilitiesFreeMembers(state->capabilities);
    std::cout << "free capabilities members\n";
  });
  if (wgpuSurfaceGetCapabilities(state->surface, state->adapter,
                                 &state->capabilities) != WGPUStatus_Success) {
    throw WGPUException("get surface capabilities");
  }
  {
    WGPUColorTargetState color_target = {};
    color_target.format = state->capabilities.formats[0];
    color_target.writeMask = WGPUColorWriteMask_All;

    WGPUFragmentState fragment_state = {};
    fragment_state.module = state->render_module;
    fragment_state.entryPoint = {"fs_main", WGPU_STRLEN};
    fragment_state.targetCount = 1;
    fragment_state.targets = &color_target;

    WGPURenderPipelineDescriptor descriptor = {};
    descriptor.label = {"render pipeline", WGPU_STRLEN};
    descriptor.layout = state->pipeline_layout;
    descriptor.vertex.module = state->render_module;
    descriptor.vertex.entryPoint = {"vs_main", WGPU_STRLEN};
    descriptor.fragment = &fragment_state;
    descriptor.primitive.topology = WGPUPrimitiveTopology_TriangleList;
    descriptor.multisample.count = 1;
    descriptor.multisample.mask = 0xFFFFFFFF;

    state->render_pipeline =
        wgpuDeviceCreateRenderPipeline(state->device, &descriptor);
    if (!state->render_pipeline) {
      throw WGPUException("create render pipeline");
    }
    state->releaser.Push([] {
      wgpuRenderPipelineRelease(state->render_pipeline);
      std::cout << "release render pipeline\n";
    });
  }
  {
    int w, h;
    if (!SDL_GetWindowSizeInPixels(state->window, &w, &h)) {
      throw SDLException("get window size");
    }
    state->config.device = state->device;
    state->config.usage = WGPUTextureUsage_RenderAttachment;
    state->config.format = state->capabilities.formats[0];
    state->config.presentMode = WGPUPresentMode_Fifo;
    state->config.alphaMode = state->capabilities.alphaModes[0];
    state->config.width = w;
    state->config.height = h;
    wgpuSurfaceConfigure(state->surface, &state->config);
  }
  {
    WGPUTextureDescriptor descriptor = {
        .usage =
            WGPUTextureUsage_StorageBinding | WGPUTextureUsage_TextureBinding,
        .dimension = WGPUTextureDimension_2D,
        .size = {State::WIDTH, State::HEIGHT, 1},
        .format = WGPUTextureFormat_RGBA32Float,
        .mipLevelCount = 1,
        .sampleCount = 1,
    };
    state->texture = wgpuDeviceCreateTexture(state->device, &descriptor);
    if (!state->texture) {
      throw WGPUException("create texture");
    }
    state->releaser.Push([] { wgpuTextureRelease(state->texture); });
  }
  {
    WGPUTextureViewDescriptor descriptor = {
        .format = WGPUTextureFormat_RGBA32Float,
        .dimension = WGPUTextureViewDimension_2D,
        .baseMipLevel = 0,
        .mipLevelCount = 1,
        .baseArrayLayer = 0,
        .arrayLayerCount = 1,
        .aspect = WGPUTextureAspect_All,
    };
    state->view = wgpuTextureCreateView(state->texture, &descriptor);
    if (!state->view) {
      throw WGPUException("create view");
    }
    state->releaser.Push([] { wgpuTextureViewRelease(state->view); });
  }
}
} // namespace jmauto
