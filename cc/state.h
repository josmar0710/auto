#include "releaser.h"
#include <SDL3/SDL.h>
#include <webgpu/wgpu.h>
#pragma once
namespace jmauto {
class State {
public:
  static constexpr Uint32 WIDTH = 512;
  static constexpr Uint32 HEIGHT = 512;
  bool must_run;
  SDL_Window *window;
  WGPUInstance instance;
  WGPUSurface surface;
  WGPUAdapter adapter;
  WGPUDevice device;
  WGPUQueue queue;
  WGPUShaderModule render_module;
  WGPUPipelineLayout pipeline_layout;
  WGPUSurfaceCapabilities capabilities;
  WGPURenderPipeline render_pipeline;
  WGPUSurfaceConfiguration config;
  WGPUTexture texture;
  WGPUTextureView view;
  Releaser releaser;
};
extern State *state;
} // namespace jmauto
