#include "loop.h"
#include "defer.h"
#include "sdl_exception.h"
#include "state.h"
#include "wgpu_exception.h"
#include <SDL3/SDL.h>
#include <SDL3/SDL_video.h>
namespace jmauto {
void HandleEvents() {
  SDL_Event event;
  while (SDL_PollEvent(&event)) {
    switch (event.type) {
    case SDL_EVENT_QUIT:
      state->must_run = false;
      break;
    }
  }
}
void Draw() {
  WGPUSurfaceTexture surface_texture;
  wgpuSurfaceGetCurrentTexture(state->surface, &surface_texture);
  switch (surface_texture.status) {
  case WGPUSurfaceGetCurrentTextureStatus_SuccessOptimal:
  case WGPUSurfaceGetCurrentTextureStatus_SuccessSuboptimal:
    // All good, could handle suboptimal here
    break;
  case WGPUSurfaceGetCurrentTextureStatus_Timeout:
  case WGPUSurfaceGetCurrentTextureStatus_Outdated:
  case WGPUSurfaceGetCurrentTextureStatus_Lost: {
    // Skip this frame, and re-configure surface.
    if (surface_texture.texture != NULL) {
      wgpuTextureRelease(surface_texture.texture);
    }
    int width, height;
    if (!SDL_GetWindowSizeInPixels(state->window, &width, &height)) {
      throw SDLException("get window size");
    }
    state->config.width = width;
    state->config.height = height;
    wgpuSurfaceConfigure(state->surface, &state->config);
    return;
  }
  case WGPUSurfaceGetCurrentTextureStatus_OutOfMemory:
  case WGPUSurfaceGetCurrentTextureStatus_DeviceLost:
  case WGPUSurfaceGetCurrentTextureStatus_Error:
  case WGPUSurfaceGetCurrentTextureStatus_Force32:
    // Fatal error
    if (surface_texture.texture != NULL) {
      wgpuTextureRelease(surface_texture.texture);
    }
    throw WGPUException("wrong surface texture status");
  }
  if (!surface_texture.texture) {
    throw WGPUException("null surface texture");
  }
  Defer release_texture(
      [surface_texture] { wgpuTextureRelease(surface_texture.texture); });
  WGPUTextureView frame =
      wgpuTextureCreateView(surface_texture.texture, nullptr);
  if (!frame) {
    throw WGPUException("null frame from create texture view");
  }
  Defer release_frame([frame] { wgpuTextureViewRelease(frame); });
  WGPUCommandEncoder command_encoder;
  {
    WGPUCommandEncoderDescriptor descriptor = {};
    descriptor.label = {"command encoder", WGPU_STRLEN};
    command_encoder =
        wgpuDeviceCreateCommandEncoder(state->device, &descriptor);
    if (!command_encoder) {
      throw WGPUException("create command encoder");
    }
  }
  Defer release_ce(
      [command_encoder] { wgpuCommandEncoderRelease(command_encoder); });
  {
    WGPURenderPassEncoder render_pass_encoder;
    {
      WGPURenderPassColorAttachment attachment = {};
      attachment.view = frame;
      attachment.loadOp = WGPULoadOp_Clear;
      attachment.storeOp = WGPUStoreOp_Store;
      attachment.depthSlice = WGPU_DEPTH_SLICE_UNDEFINED;
      attachment.clearValue = {0.3, 0.2, 0.1, 1.0};

      WGPURenderPassDescriptor descriptor = {};
      descriptor.label = {"render pass encoder", WGPU_STRLEN};
      descriptor.colorAttachmentCount = 1;
      descriptor.colorAttachments = &attachment;
      render_pass_encoder =
          wgpuCommandEncoderBeginRenderPass(command_encoder, &descriptor);
      if (!render_pass_encoder) {
        throw WGPUException("begin render pass");
      }
    }
    Defer release_rp([render_pass_encoder] {
      wgpuRenderPassEncoderEnd(render_pass_encoder);
      wgpuRenderPassEncoderRelease(render_pass_encoder);
    });
    wgpuRenderPassEncoderSetPipeline(render_pass_encoder,
                                     state->render_pipeline);
    wgpuRenderPassEncoderDraw(render_pass_encoder, 3, 1, 0, 0);
  }
  WGPUCommandBuffer command_buffer;
  {
    WGPUCommandBufferDescriptor descriptor = {};
    descriptor.label = {"command buffer", WGPU_STRLEN};
    command_buffer = wgpuCommandEncoderFinish(command_encoder, &descriptor);
    if (!command_buffer) {
      throw WGPUException(
          "finish command encoder, null command buffer returned");
    }
  }
  Defer release_cb(
      [command_buffer] { wgpuCommandBufferRelease(command_buffer); });
  wgpuQueueSubmit(state->queue, 1, (const WGPUCommandBuffer[]){command_buffer});
  wgpuSurfacePresent(state->surface);
}
void Loop() {
  HandleEvents();
  Draw();
}
} // namespace jmauto
