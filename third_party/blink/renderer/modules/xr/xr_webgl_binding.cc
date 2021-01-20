// Copyright (c) Facebook, Inc. and its affiliates.
// Copyright 2020 The Chromium Authors. All rights reserved.
// Use of this source code is governed by a BSD-style license that can be
// found in the LICENSE file.

#include "third_party/blink/renderer/modules/xr/xr_webgl_binding.h"

#include "third_party/blink/renderer/modules/webgl/webgl_framebuffer.h"
#include "third_party/blink/renderer/modules/webgl/webgl_rendering_context_base.h"
#include "third_party/blink/renderer/modules/webgl/webgl_texture.h"
#include "third_party/blink/renderer/modules/webgl/webgl_unowned_texture.h"
#include "third_party/blink/renderer/modules/xr/xr_cube_layer.h"
#include "third_party/blink/renderer/modules/xr/xr_cube_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_cube_map.h"
#include "third_party/blink/renderer/modules/xr/xr_cylinder_layer.h"
#include "third_party/blink/renderer/modules/xr/xr_cylinder_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_equirect_layer.h"
#include "third_party/blink/renderer/modules/xr/xr_equirect_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_frame.h"
#include "third_party/blink/renderer/modules/xr/xr_light_probe.h"
#include "third_party/blink/renderer/modules/xr/xr_projection_layer.h"
#include "third_party/blink/renderer/modules/xr/xr_quad_layer.h"
#include "third_party/blink/renderer/modules/xr/xr_quad_layer_init.h"
#include "third_party/blink/renderer/modules/xr/xr_reference_space.h"
#include "third_party/blink/renderer/modules/xr/xr_render_state.h"
#include "third_party/blink/renderer/modules/xr/xr_session.h"
#include "third_party/blink/renderer/modules/xr/xr_space.h"
#include "third_party/blink/renderer/modules/xr/xr_utils.h"
#include "third_party/blink/renderer/modules/xr/xr_view.h"
#include "third_party/blink/renderer/modules/xr/xr_viewer_pose.h"
#include "third_party/blink/renderer/modules/xr/xr_viewport.h"
#include "third_party/blink/renderer/modules/xr/xr_webgl_layer.h"
#include "third_party/blink/renderer/modules/xr/xr_webgl_sub_image.h"
#include "third_party/blink/renderer/platform/bindings/exception_state.h"
#include "third_party/blink/renderer/platform/graphics/gpu/extensions_3d_util.h"

namespace blink {

namespace {

const char kSessionEnded[] = "XRSession has already ended.";
const char kWrongLayout[] = "Invalid layout passed to layer.";
const char kWrongLayerTypeForViewSubImage[] =
    "Invalid layer type passed to getViewSubImage.";
const char kWrongLayerTypeForSubImage[] =
    "Invalid layer type passed to getSubImage.";
const char kWrongEyeType[] = "Invalid eye type passed to getSubImage.";

const char kLayersNotRequested[] =
    "XRSession was not created with the layers feature";

const char kNotStereoLayer[] = "Called getViewSubImage on a mono layer";
const char kNotMonoLayer[] = "Called getSubImage on a stereo layer";
const char kNotActiveFrame[] = "Called getSubImage with inactive frame";
const char kMixingLayers[] =
    "Called getSubImage with objects from different states";

const double kFramebufferMinScale2 = 0.2;

// Because including base::ClampToRange would be a dependency violation
double ClampToRange2(const double value, const double min, const double max) {
  return std::min(std::max(value, min), max);
}

const char kOnlyMonoOrStereoSupported[] =
    "Only 'mono' or 'stereo' layout is supported for texture arrays and cube "
    "maps";
const char kRequestedTextureIsTooLarge[] = "Requested texture is too large";
const char kCantFitStereoInOneTexture[] =
    "Can't fit stereo layout into a single texture; consider texture arrays";
const char kStaticLayer[] =
    "Static layer can be updated only when needsRedraw is signalled";


Vector<GLenum> GetSupportedCompressedFormats() {
  Vector<GLenum> formats {
    //
    // S3TC/DXT/BC
    //
    // The following compressed format block is disabled due to the following
    // error on import: <validate_surface_metadata:2300>: Metadata format is not
    // recognized 0 From Qualcomm: S3TC might not be supported for ES.
  #if 0
    GL_COMPRESSED_RGB_S3TC_DXT1_EXT, // line through 3D space, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_S3TC_DXT1_EXT, // line through 3D space plus 1-bit alpha, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_S3TC_DXT3_EXT, // line through 3D space plus line through 1D space, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_S3TC_DXT5_EXT, // line through 3D space plus 4-bit alpha, 4x4 blocks, unsigned normalized
  #endif

    //
    // S3TC SRGB/DXT/BC
    //
  #if 0
    GL_COMPRESSED_SRGB_S3TC_DXT1_EXT, // line through 3D space, 4x4 blocks, sRGB
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT1_EXT, // line through 3D space plus 1-bit alpha, 4x4 blocks, sRGB
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT3_EXT, // line through 3D space plus line through 1D space, 4x4 blocks, sRGB
    GL_COMPRESSED_SRGB_ALPHA_S3TC_DXT5_EXT, // line through 3D space plus 4-bit alpha, 4x4 blocks, sRGB
  #endif

    //
    // LATC, not available in webgl
    //
    // GL_COMPRESSED_LUMINANCE_LATC1_EXT, // line through 1D space, 4x4 blocks, unsigned normalized
    // GL_COMPRESSED_LUMINANCE_ALPHA_LATC2_EXT, // two lines through 1D space, 4x4 blocks, unsigned normalized
    // GL_COMPRESSED_SIGNED_LUMINANCE_LATC1_EXT, // line through 1D space, 4x4 blocks, signed normalized
    // GL_COMPRESSED_SIGNED_LUMINANCE_ALPHA_LATC2_EXT, // two lines through 1D space, 4x4 blocks, signed normalized

    //
    // RGTC
    //
  #if 0
    GL_COMPRESSED_RED_RGTC1, // line through 1D space, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_RG_RGTC2, // two lines through 1D space, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_SIGNED_RED_RGTC1, // line through 1D space, 4x4 blocks, signed normalized
    GL_COMPRESSED_SIGNED_RG_RGTC2, // two lines through 1D space, 4x4 blocks, signed normalized
  #endif

    //
    // BPTC
    //
  #if 0
    GL_COMPRESSED_RGB_BPTC_UNSIGNED_FLOAT, // 3-component, 4x4 blocks, unsigned floating-point
    GL_COMPRESSED_RGB_BPTC_SIGNED_FLOAT, // 3-component, 4x4 blocks, signed floating-point
    GL_COMPRESSED_RGBA_BPTC_UNORM, // 4-component, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_SRGB_ALPHA_BPTC_UNORM, // 4-component, 4x4 blocks, sRGB
  #endif

    //
    // ETC1
    //
    GL_ETC1_RGB8_OES,  // 3-component ETC1, 4x4 blocks, unsigned normalized

    //
    // ETC
    //
    GL_COMPRESSED_RGB8_ETC2,  // 3-component ETC2, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_RGB8_PUNCHTHROUGH_ALPHA1_ETC2,  // 4-component ETC2 with 1-bit alpha, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_RGBA8_ETC2_EAC,  // 4-component ETC2, 4x4 blocks, unsigned normalized

    GL_COMPRESSED_SRGB8_ETC2,  // 3-component ETC2, 4x4 blocks, sRGB
    GL_COMPRESSED_SRGB8_PUNCHTHROUGH_ALPHA1_ETC2,  // 4-component ETC2 with 1-bit alpha, 4x4 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ETC2_EAC,  // 4-component ETC2, 4x4 blocks, sRGB

    GL_COMPRESSED_R11_EAC,  // 1-component ETC, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_RG11_EAC,  // 2-component ETC, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_SIGNED_R11_EAC,  // 1-component ETC, 4x4 blocks, signed normalized
    GL_COMPRESSED_SIGNED_RG11_EAC,  // 2-component ETC, 4x4 blocks, signed normalized

    //
    // PVRTC
    //
  #if 0
    GL_COMPRESSED_RGB_PVRTC_2BPPV1_IMG,  // 3-component PVRTC, 16x8 blocks, unsigned normalized
    GL_COMPRESSED_RGB_PVRTC_4BPPV1_IMG,  // 3-component PVRTC,  8x8 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_PVRTC_2BPPV1_IMG,  // 4-component PVRTC, 16x8 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_PVRTC_4BPPV1_IMG,  // 4-component PVRTC,  8x8 blocks, unsigned normalized
  #endif

  #if 0
    // PVRTC2, not available in webgl
    // GL_COMPRESSED_RGBA_PVRTC_2BPPV2_IMG,  // 4-component PVRTC,  8x4 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_PVRTC_4BPPV2_IMG,  // 4-component PVRTC,  4x4 blocks, unsigned normalized

    // PVRTC_SRGB, not available in webgl
    // GL_COMPRESSED_SRGB_PVRTC_2BPPV1_EXT,  // 3-component PVRTC, 16x8 blocks, sRGB
    // GL_COMPRESSED_SRGB_PVRTC_4BPPV1_EXT,  // 3-component PVRTC,  8x8 blocks, sRGB
    // GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV1_EXT, // 4-component PVRTC, 16x8 blocks, sRGB
    // GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV1_EXT, // 4-component PVRTC,  8x8 blocks, sRGB

    // PVRT2 + PVRTC_SRGB
    // GL_COMPRESSED_SRGB_ALPHA_PVRTC_2BPPV2_IMG, // 4-component PVRTC,  8x4 blocks, sRGB
    // GL_COMPRESSED_SRGB_ALPHA_PVRTC_4BPPV2_IMG, // 4-component PVRTC,  4x4 blocks, sRGB
  #endif

    //
    // ASTC
    //
    GL_COMPRESSED_RGBA_ASTC_4x4_KHR,  // 4-component ASTC, 4x4 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_5x4_KHR,  // 4-component ASTC, 5x4 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_5x5_KHR,  // 4-component ASTC, 5x5 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_6x5_KHR,  // 4-component ASTC, 6x5 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_6x6_KHR,  // 4-component ASTC, 6x6 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_8x5_KHR,  // 4-component ASTC, 8x5 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_8x6_KHR,  // 4-component ASTC, 8x6 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_8x8_KHR,  // 4-component ASTC, 8x8 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_10x5_KHR,  // 4-component ASTC, 10x5 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_10x6_KHR,  // 4-component ASTC, 10x6 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_10x8_KHR,  // 4-component ASTC, 10x8 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_10x10_KHR,  // 4-component ASTC, 10x10 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_12x10_KHR,  // 4-component ASTC, 12x10 blocks, unsigned normalized
    GL_COMPRESSED_RGBA_ASTC_12x12_KHR,  // 4-component ASTC, 12x12 blocks, unsigned normalized

    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4_KHR,  // 4-component ASTC, 4x4 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4_KHR,  // 4-component ASTC, 5x4 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5_KHR,  // 4-component ASTC, 5x5 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5_KHR,  // 4-component ASTC, 6x5 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6_KHR,  // 4-component ASTC, 6x6 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x5_KHR,  // 4-component ASTC, 8x5 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x6_KHR,  // 4-component ASTC, 8x6 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_8x8_KHR,  // 4-component ASTC, 8x8 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x5_KHR,  // 4-component ASTC, 10x5 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x6_KHR,  // 4-component ASTC, 10x6 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x8_KHR,  // 4-component ASTC, 10x8 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_10x10_KHR,  // 4-component ASTC, 10x10 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x10_KHR,  // 4-component ASTC, 12x10 blocks, sRGB
    GL_COMPRESSED_SRGB8_ALPHA8_ASTC_12x12_KHR,  // 4-component ASTC, 12x12 blocks, sRGB

    // ASTC for 3D textures
    // GL_COMPRESSED_RGBA_ASTC_3x3x3_OES, // 4-component ASTC, 3x3x3 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_ASTC_4x3x3_OES, // 4-component ASTC, 4x3x3 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_ASTC_4x4x3_OES, // 4-component ASTC, 4x4x3 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_ASTC_4x4x4_OES, // 4-component ASTC, 4x4x4 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_ASTC_5x4x4_OES, // 4-component ASTC, 5x4x4 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_ASTC_5x5x4_OES, // 4-component ASTC, 5x5x4 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_ASTC_5x5x5_OES, // 4-component ASTC, 5x5x5 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_ASTC_6x5x5_OES, // 4-component ASTC, 6x5x5 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_ASTC_6x6x5_OES, // 4-component ASTC, 6x6x5 blocks, unsigned normalized
    // GL_COMPRESSED_RGBA_ASTC_6x6x6_OES, // 4-component ASTC, 6x6x6 blocks, unsigned normalized

    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_3x3x3_OES, // 4-component ASTC, 3x3x3 blocks, sRGB
    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x3x3_OES, // 4-component ASTC, 4x3x3 blocks, sRGB
    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x3_OES, // 4-component ASTC, 4x4x3 blocks, sRGB
    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_4x4x4_OES, // 4-component ASTC, 4x4x4 blocks, sRGB
    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x4x4_OES, // 4-component ASTC, 5x4x4 blocks, sRGB
    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x4_OES, // 4-component ASTC, 5x5x4 blocks, sRGB
    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_5x5x5_OES, // 4-component ASTC, 5x5x5 blocks, sRGB
    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x5x5_OES, // 4-component ASTC, 6x5x5 blocks, sRGB
    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x5_OES, // 4-component ASTC, 6x6x5 blocks, sRGB
    // GL_COMPRESSED_SRGB8_ALPHA8_ASTC_6x6x6_OES, // 4-component ASTC, 6x6x6 blocks, sRGB

    //
    // ATC
    //
  #if 0
    GL_ATC_RGB_AMD, // 3-component, 4x4 blocks, unsigned normalized
    GL_ATC_RGBA_EXPLICIT_ALPHA_AMD, // 4-component, 4x4 blocks, unsigned normalized
    GL_ATC_RGBA_INTERPOLATED_ALPHA_AMD, // 4-component, 4x4 blocks, unsigned normalized
  #endif
  };

  return formats;
}


 Vector<GLenum> GetSupportedColorTextureSwapChainFormatsV1() {
  Vector<GLenum> formats {
    GL_RGB,
    GL_RGBA,

    // EXT_sRGB
    GL_SRGB,  // 3-component, 8-bit sRGB
    GL_SRGB_ALPHA_EXT,  // 4-component, 8-bit sRGB

    // EXT_color_buffer_half_float
    // Enables the use of GL_HALF_FLOAT on glTexImage2D type
    // for the internal formats GL_RGB/GL_RGBA (16-bit)

    // WEBGL_color_buffer_float
    // Enables the use of GL_FLOAT on glTexImage2D type
    // for the internal formats GL_RGB/GL_RGBA (32-bit)
  };

  Vector<GLenum> compressed = GetSupportedCompressedFormats();
  formats.AppendRange(compressed.begin(), compressed.end());

  return formats;
}


Vector<GLenum> GetSupportedColorTextureSwapChainFormatsV2() {
  Vector<GLenum> formats {
    //
    // 8 bits per component
    //
    GL_SRGB8,  // 3-component, 8-bit sRGB
    GL_SRGB8_ALPHA8,  // 4-component, 8-bit sRGB, renderable

    GL_R8,  // 1-component, 8-bit unsigned normalized, renderable
    GL_RG8,  // 2-component, 8-bit unsigned normalized, renderable
    // NOTE: T66970556: RGB8 usage on Qualcomm is currently not working as
    // expected.
    GL_RGB8,  // 3-component, 8-bit unsigned normalized, renderable
    GL_RGBA8,  // 4-component, 8-bit unsigned normalized, renderable

    GL_R8_SNORM,  // 1-component, 8-bit signed normalized
    GL_RG8_SNORM,  // 2-component, 8-bit signed normalized
    GL_RGB8_SNORM,  // 3-component, 8-bit signed normalized
    GL_RGBA8_SNORM,  // 4-component, 8-bit signed normalized

    GL_R8UI,  // 1-component, 8-bit unsigned integer
    GL_RG8UI,  // 2-component, 8-bit unsigned integer
    GL_RGB8UI,  // 3-component, 8-bit unsigned integer
    GL_RGBA8UI,  // 4-component, 8-bit unsigned integer

    GL_R8I,  // 1-component, 8-bit signed integer
    GL_RG8I,  // 2-component, 8-bit signed integer
    GL_RGB8UI,  // 3-component, 8-bit unsigned integer
    GL_RGBA8I,  // 4-component, 8-bit signed integer

    //
    // 16 bits per component
    //
    GL_R16UI, // 1-component, 16-bit unsigned integer
    GL_RG16UI, // 2-component, 16-bit unsigned integer
    GL_RGB16UI, // 3-component, 16-bit unsigned integer
    GL_RGBA16UI, // 4-component, 16-bit unsigned integer

    GL_R16I, // 1-component, 16-bit signed integer
    GL_RG16I, // 2-component, 16-bit signed integer
    GL_RGB16I, // 3-component, 16-bit signed integer
    GL_RGBA16I, // 4-component, 16-bit signed integer

    GL_R16F,  // 1-component, 16-bit floating-point
    GL_RG16F,  // 2-component, 16-bit floating-point
    GL_RGB16F,  // 2-component, 16-bit floating-point
    GL_RGBA16F,  // 4-component, 16-bit floating-point

    //
    // 32 bits per component
    //
    GL_R32UI,  // 1-component, 32-bit unsigned integer
    GL_RG32UI,  // 2-component, 32-bit unsigned integer
    GL_RGB32UI,  // 3-component, 32-bit unsigned integer
    GL_RGBA32UI,  // 4-component, 32-bit unsigned integer

    GL_R32I,  // 1-component, 32-bit signed integer
    GL_RG32I,  // 2-component, 32-bit signed integer
    GL_RGB32I,  // 3-component, 32-bit signed integer
    GL_RGBA32I,  // 4-component, 32-bit signed integer

    GL_R32F,  // 1-component, 32-bit floating-point
    GL_RG32F,  // 2-component, 32-bit floating-point
    GL_RGB32F,  // 2-component, 32-bit floating-point
    GL_RGBA32F,  // 4-component, 32-bit floating-point

    //
    // Packed
    //
    GL_RGBA4,  // 4-component 4:4:4:4, unsigned normalized
    GL_RGB5_A1,  // 4-component 5:5:5:1, unsigned normalized
    GL_RGB565, // 3-component 5:6:5, unsigned normalized
    GL_RGB10_A2,  // 4-component 10:10:10:2,  unsigned normalized
    GL_RGB10_A2UI,  // 4-component 10:10:10:2,  unsigned integer
    GL_R11F_G11F_B10F,  // 3-component 11:11:10, floating-point
    GL_RGB9_E5,  // 3-component/exp 9:9:9/5, floating-point
  };

  static Vector<GLenum> compressed = GetSupportedCompressedFormats();
  formats.AppendRange(compressed.begin(), compressed.end());

  return formats;
}

Vector<GLenum> GetSupportedDepthTextureSwapChainFormatsV1() {
  Vector<GLenum> formats {
    // WEBGL_depth_texture
    GL_DEPTH_COMPONENT,
    GL_DEPTH_STENCIL,
  };

  return formats;
}

Vector<GLenum> GetSupportedDepthTextureSwapChainFormatsV2() {
  Vector<GLenum> formats {
    GL_DEPTH_COMPONENT16,
    GL_DEPTH_COMPONENT24,
    GL_DEPTH_COMPONENT32F,
    GL_DEPTH24_STENCIL8,
    GL_STENCIL_INDEX8,

    // The following depth formats trigger a validation error on import:
    //<validate_surface_metadata:2514>: Metadata has flagbuffer for some levels
    // but not
    // others {level 0, plane 1}
    // According to Qualcomm, this is a mistake in the validation code, but
    // should otherwise work. In practice, this has not been the case:
    // vkAllocateMemory fails with 'unknown' error when allocating memory for
    // one of these types.
  #if 0
    GL_DEPTH32F_STENCIL8,
  #endif
  };

  return formats;
}
}  // namespace

XRWebGLBinding* XRWebGLBinding::Create(XRSession* session,
                                       const XRWebGLRenderingContext& context,
                                       ExceptionState& exception_state) {
  if (session->ended()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "Cannot create an XRWebGLBinding for an "
                                      "XRSession which has already ended.");
    return nullptr;
  }

  if (!session->immersive()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "Cannot create an XRWebGLBinding for an "
                                      "inline XRSession.");
    return nullptr;
  }

  WebGLRenderingContextBase* webgl_context =
      webglRenderingContextBaseFromUnion(context);

  if (webgl_context->isContextLost()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      "Cannot create an XRWebGLBinding with a "
                                      "lost WebGL context.");
    return nullptr;
  }

  if (!webgl_context->IsXRCompatible()) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kInvalidStateError,
        "WebGL context must be marked as XR compatible in order to "
        "use with an immersive XRSession");
    return nullptr;
  }

  return MakeGarbageCollected<XRWebGLBinding>(
      session, webgl_context, context.IsWebGL2RenderingContext());
}

XRWebGLBinding::XRWebGLBinding(XRSession* session,
                               WebGLRenderingContextBase* webgl_context,
                               bool webgl2)
    : session_(session), webgl_context_(webgl_context), webgl2_(webgl2) {}

WebGLTexture* XRWebGLBinding::getReflectionCubeMap(
    XRLightProbe* light_probe,
    ExceptionState& exception_state) {
  GLenum internal_format, format, type;

  if (webgl_context_->isContextLost()) {
    exception_state.ThrowDOMException(
        DOMExceptionCode::kInvalidStateError,
        "Cannot get reflection cube map with a lost context.");
    return nullptr;
  }

  // Determine the internal_format, format, and type that will be passed to
  // glTexImage2D for each possible light probe reflection format. The formats
  // will differ depending on whether we're using WebGL 2 or WebGL 1 with
  // extensions.
  switch (light_probe->ReflectionFormat()) {
    case XRLightProbe::kReflectionFormatRGBA16F:
      if (!webgl2_ && !webgl_context_->ExtensionsUtil()->IsExtensionEnabled(
                          "GL_OES_texture_half_float")) {
        exception_state.ThrowDOMException(
            DOMExceptionCode::kInvalidStateError,
            "WebGL contexts must have the OES_texture_half_float extension "
            "enabled "
            "prior to calling getReflectionCubeMap with a format of "
            "\"rgba16f\". "
            "This restriction does not apply to WebGL 2.0 contexts.");
        return nullptr;
      }

      internal_format = webgl2_ ? GL_RGBA16F : GL_RGBA;
      format = GL_RGBA;
      // Surprisingly GL_HALF_FLOAT and GL_HALF_FLOAT_OES have different values.
      type = webgl2_ ? GL_HALF_FLOAT : GL_HALF_FLOAT_OES;
      break;

    case XRLightProbe::kReflectionFormatSRGBA8:
      bool use_srgb =
          webgl2_ ||
          webgl_context_->ExtensionsUtil()->IsExtensionEnabled("GL_EXT_sRGB");

      if (use_srgb) {
        internal_format = webgl2_ ? GL_SRGB8_ALPHA8 : GL_SRGB_ALPHA_EXT;
      } else {
        internal_format = GL_RGBA;
      }

      format = webgl2_ ? GL_RGBA : internal_format;
      type = GL_UNSIGNED_BYTE;
      break;
  }

  XRCubeMap* cube_map = light_probe->getReflectionCubeMap();
  if (!cube_map) {
    return nullptr;
  }

  WebGLTexture* texture = MakeGarbageCollected<WebGLTexture>(webgl_context_);
  cube_map->updateWebGLEnvironmentCube(webgl_context_, texture, internal_format,
                                       format, type);

  return texture;
}

WebGLTexture* XRWebGLBinding::getCameraImage(XRFrame* frame, XRView* view) {
  // Verify that frame is currently active.
  if (!frame->IsActive()) {
    return nullptr;
  }

  if (frame != view->frame()) {
    return nullptr;
  }

  XRWebGLLayer* base_layer = view->session()->renderState()->baseLayer();
  DCHECK(base_layer);

  base::Optional<gpu::MailboxHolder> camera_image_mailbox_holder =
      base_layer->CameraImageMailboxHolder();

  if (!camera_image_mailbox_holder) {
    return nullptr;
  }

  GLuint texture_id = base_layer->CameraImageTextureId();

  // This resource is owned by the renderer, and is freed OnFrameEnd();
  WebGLUnownedTexture* texture = MakeGarbageCollected<WebGLUnownedTexture>(
      webgl_context_, texture_id, GL_TEXTURE_2D);
  return texture;
}

double XRWebGLBinding::nativeProjectionScaleFactor() const {
  return 1.0f;
}

XRWebGLBinding::XRViewportPair XRWebGLBinding::createViewportPair(
    const IntSize& desired_size,
    int x_mult,
    int y_mult,
    bool as_texture_array) {
  XRViewport* left_viewport;
  XRViewport* right_viewport;

  if (x_mult != 1 || y_mult != 1) {
    const double x_vp_mult = 1.0 / x_mult;
    const double y_vp_mult = 1.0 / y_mult;
    // stereo left-to-right or top-to-bottom
    left_viewport =
        MakeGarbageCollected<XRViewport>(0, 0, desired_size.Width() * x_vp_mult,
                                         desired_size.Height() * y_vp_mult);
    right_viewport = MakeGarbageCollected<XRViewport>(
        desired_size.Width() * (1.0f - x_vp_mult),
        desired_size.Height() * (1.0f - y_vp_mult),
        desired_size.Width() * x_vp_mult, desired_size.Height() * y_vp_mult);
  } else {
    left_viewport = MakeGarbageCollected<XRViewport>(0, 0, desired_size.Width(),
                                                     desired_size.Height());
    right_viewport = as_texture_array ? left_viewport : nullptr;
  }

  return XRViewportPair{left_viewport, right_viewport};
}

template <class T>
bool XRWebGLBinding::createTextures(TexturesInfo* out_info,
                                    ScriptState* script_state,
                                    T* initializer,
                                    bool is_static,
                                    ExceptionState& exception_state) {
  DCHECK(out_info);
  out_info->as_texture_array = initializer->textureType() == "texture-array";
  const GLenum gl_texture_style =
      out_info->as_texture_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
  if ((initializer->layout() == XRCompositionLayer::kLayoutDefault ||
       initializer->layout() == XRCompositionLayer::kLayoutStereo) &&
      !out_info->as_texture_array) {
    // Automatic detection of layout.
    // By default we choose left-right stereo, unless width * 2 >
    // webgl_context_->max_texture_size()
    if (initializer->viewPixelWidth() * 2 <=
        webgl_context_->max_texture_size()) {
      initializer->setLayout(XRCompositionLayer::kLayoutStereoLeftRight);
    } else {
      if (initializer->viewPixelHeight() * 2 <=
          webgl_context_->max_texture_size()) {
        initializer->setLayout(XRCompositionLayer::kLayoutStereoTopBottom);
      } else {
        exception_state.ThrowDOMException(DOMExceptionCode::kNotSupportedError,
                                          kCantFitStereoInOneTexture);
        return false;
      }
    }
  }

  int x_mult = 1;
  int y_mult = 1;
  if (!out_info->as_texture_array) {
    if (initializer->layout() != XRCompositionLayer::kLayoutMono) {
      if (initializer->layout() == XRCompositionLayer::kLayoutStereoLeftRight) {
        x_mult = 2;
      } else if (initializer->layout() ==
                 XRCompositionLayer::kLayoutStereoTopBottom) {
        y_mult = 2;
      } else {
        NOTREACHED() << "Unexpected stereo layout";
      }
    }
  } else if (out_info->as_texture_array) {
    // only mono or stereo
    if (initializer->layout() != XRCompositionLayer::kLayoutMono &&
        initializer->layout() != XRCompositionLayer::kLayoutStereo &&
        initializer->layout() != XRCompositionLayer::kLayoutDefault) {
      exception_state.ThrowDOMException(DOMExceptionCode::kNotSupportedError,
                                        kOnlyMonoOrStereoSupported);
      return false;
    }
  }

  // check if texture is whitin supported dimensions.
  if (initializer->viewPixelWidth() * x_mult >
          webgl_context_->max_texture_size() ||
      initializer->viewPixelHeight() * y_mult >
          webgl_context_->max_texture_size()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kNotSupportedError,
                                      kRequestedTextureIsTooLarge);
    return false;
  }

  const IntSize desired_size(initializer->viewPixelWidth() * x_mult,
                             initializer->viewPixelHeight() * y_mult);

  VLOG(1) << __func__ << " size: " << desired_size.Width() << " x "
          << desired_size.Height();

  out_info->color_texture =
      session_->GetWebGLResourceManager()->CreateTextureSwapChain(
          webgl_context_, gl_texture_style, desired_size, initializer->alpha(),
          false /* foveation */, true /* can_discard_depth */, is_static);

  auto viewport_pair = createViewportPair(desired_size, x_mult, y_mult,
                                          out_info->as_texture_array);
  out_info->left_viewport = viewport_pair.left_viewport;
  out_info->right_viewport = viewport_pair.right_viewport;

  out_info->depth_stencil_texture = nullptr;
  if (initializer->depth() || initializer->stencil()) {
    out_info->depth_stencil_texture =
        session_->GetWebGLResourceManager()->CreateMatchedDepthStencilTexture(
            webgl_context_, out_info->color_texture, initializer->stencil());

    if (!out_info->depth_stencil_texture) {
      exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                        kSessionEnded);
      return false;
    }
  }
  return true;
}

bool XRWebGLBinding::IsValidXRSpace(XRSession* session,
                                    XRSpace* space,
                                    bool allow_viewer_and_non_reference_space) {
  if (!space) {
    return false;
  }

  if (space->session() != session) {
    return false;
  }

  if (!allow_viewer_and_non_reference_space) {
    if (!space->GetWrapperTypeInfo()->IsSubclass(
            XRReferenceSpace::GetStaticWrapperTypeInfo())) {
      return false;
    }

    XRReferenceSpace* ref_space = static_cast<XRReferenceSpace*>(space);
    if (ref_space->GetType() ==
        device::mojom::blink::XRReferenceSpaceType::kViewer) {
      return false;
    }
  }

  return true;
}

XRProjectionLayer* XRWebGLBinding::createProjectionLayer(
    ScriptState* script_state,
    XRProjectionLayerInit* initializer,
    ExceptionState& exception_state) {
  if (session_->ended()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  // bool want_depth_buffer = initializer->depth();
  // bool want_stencil_buffer = initializer->stencil();
  bool want_alpha_channel = initializer->alpha();
  bool as_texture_array = initializer->textureType() == "texture-array";
  GLenum gl_texture_style =
      as_texture_array ? GL_TEXTURE_2D_ARRAY : GL_TEXTURE_2D;
  double framebuffer_scale = 1.0;

  double max_scale = std::max(session_->NativeFramebufferScale(), 1.0);

  framebuffer_scale = ClampToRange2(initializer->scaleFactor(),
                                    kFramebufferMinScale2, max_scale);

  DoubleSize framebuffers_size = session_->DefaultFramebufferSize(
      (initializer->textureType() == "texture-array"));

  IntSize desired_size(framebuffers_size.Width() * framebuffer_scale,
                       framebuffers_size.Height() * framebuffer_scale);

  VLOG(1) << __func__ << " size: " << desired_size.Width() << " x "
          << desired_size.Height();

  WebGLTexture* color_texture =
      session_->GetWebGLResourceManager()->CreateTextureSwapChain(
          webgl_context_, gl_texture_style, desired_size, want_alpha_channel,
          true /* foveation */, true /* can_discard_depth */,
          false /* is_static */);

  XRViewport *left_viewport, *right_viewport;
  if (gl_texture_style == GL_TEXTURE_2D) {
    left_viewport = MakeGarbageCollected<XRViewport>(
        0, 0, desired_size.Width() * 0.5, desired_size.Height());
    right_viewport = MakeGarbageCollected<XRViewport>(
        desired_size.Width() * 0.5, 0, desired_size.Width() * 0.5,
        desired_size.Height());
  } else if (gl_texture_style == GL_TEXTURE_2D_ARRAY) {
    left_viewport = MakeGarbageCollected<XRViewport>(0, 0, desired_size.Width(),
                                                     desired_size.Height());
    right_viewport = MakeGarbageCollected<XRViewport>(
        0, 0, desired_size.Width(), desired_size.Height());
  } else {
    DCHECK(0) << "Unsupported texture target (" << gl_texture_style << ")";
  }

  WebGLTexture* depth_stencil_texture = nullptr;
  if (initializer->depth() || initializer->stencil()) {
    depth_stencil_texture =
        session_->GetWebGLResourceManager()->CreateMatchedDepthStencilTexture(
            webgl_context_, color_texture, initializer->stencil());

    if (!depth_stencil_texture) {
      exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                        kSessionEnded);
      return nullptr;
    }
  }
  return MakeGarbageCollected<XRProjectionLayer>(
      session_, webgl_context_, color_texture, depth_stencil_texture,
      left_viewport, right_viewport, as_texture_array);
}

XRQuadLayer* XRWebGLBinding::createQuadLayer(ScriptState* script_state,
                                             XRQuadLayerInit* initializer,
                                             ExceptionState& exception_state,
                                             HTMLVideoElement* media) {
  if (!session_->IsFeatureEnabled(device::mojom::XRSessionFeature::LAYERS)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kNotSupportedError,
                                      kLayersNotRequested);
    return nullptr;
  }

  if (session_->ended()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (!IsValidXRSpace(session_, initializer->space(), true)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (initializer->layout() == XRCompositionLayer::kLayoutDefault) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kWrongLayout);
    return nullptr;
  }

  TexturesInfo tex_info = {};
  if (!createTextures(&tex_info, script_state, initializer,
                      initializer->isStatic(), exception_state)) {
    return nullptr;
  }

  auto* layer = MakeGarbageCollected<XRQuadLayer>(
      session_, initializer->space(), webgl_context_, tex_info.color_texture,
      tex_info.depth_stencil_texture, tex_info.left_viewport,
      tex_info.right_viewport, tex_info.as_texture_array, media);

  layer->setWidth(initializer->width());
  layer->setHeight(initializer->height());
  if (initializer->transform()) {
    layer->setTransform(initializer->transform());
  }

  return layer;
}

XRCylinderLayer* XRWebGLBinding::createCylinderLayer(
    ScriptState* script_state,
    XRCylinderLayerInit* initializer,
    ExceptionState& exception_state,
    HTMLVideoElement* media) {
  if (!session_->IsFeatureEnabled(device::mojom::XRSessionFeature::LAYERS)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kNotSupportedError,
                                      kLayersNotRequested);
    return nullptr;
  }

  if (session_->ended()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (!IsValidXRSpace(session_, initializer->space(), true)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (initializer->layout() == XRCompositionLayer::kLayoutDefault) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kWrongLayout);
    return nullptr;
  }

  TexturesInfo tex_info = {};
  if (!createTextures(&tex_info, script_state, initializer,
                      initializer->isStatic(), exception_state)) {
    return nullptr;
  }

  auto* layer = MakeGarbageCollected<XRCylinderLayer>(
      session_, initializer->space(), webgl_context_, tex_info.color_texture,
      tex_info.depth_stencil_texture, tex_info.left_viewport,
      tex_info.right_viewport, tex_info.as_texture_array, media);

  layer->setRadius(initializer->radius());
  layer->setCentralAngle(initializer->centralAngle());
  layer->setAspectRatio(initializer->aspectRatio());
  if (initializer->transform()) {
    layer->setTransform(initializer->transform());
  }

  return layer;
}

XREquirectLayer* XRWebGLBinding::createEquirectLayer(
    ScriptState* script_state,
    XREquirectLayerInit* initializer,
    ExceptionState& exception_state,
    HTMLVideoElement* media) {
  if (!session_->IsFeatureEnabled(device::mojom::XRSessionFeature::LAYERS)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kNotSupportedError,
                                      kLayersNotRequested);
    return nullptr;
  }

  if (session_->ended()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (!IsValidXRSpace(session_, initializer->space(), false)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (initializer->layout() == XRCompositionLayer::kLayoutDefault) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kWrongLayout);
    return nullptr;
  }

  TexturesInfo tex_info = {};
  if (!createTextures(&tex_info, script_state, initializer,
                      initializer->isStatic(), exception_state)) {
    return nullptr;
  }

  auto* layer = MakeGarbageCollected<XREquirectLayer>(
      session_, initializer->space(), webgl_context_, tex_info.color_texture,
      tex_info.depth_stencil_texture, tex_info.left_viewport,
      tex_info.right_viewport, tex_info.as_texture_array, media);

  layer->setRadius(initializer->radius());
  layer->setCentralHorizontalAngle(initializer->centralHorizontalAngle());
  layer->setUpperVerticalAngle(initializer->upperVerticalAngle());
  layer->setLowerVerticalAngle(initializer->lowerVerticalAngle());
  if (initializer->transform()) {
    layer->setTransform(initializer->transform());
  }

  return layer;
}

XRCubeLayer* XRWebGLBinding::createCubeLayer(ScriptState* script_state,
                                             XRCubeLayerInit* initializer,
                                             ExceptionState& exception_state) {
  if (!session_->IsFeatureEnabled(device::mojom::XRSessionFeature::LAYERS)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kNotSupportedError,
                                      kLayersNotRequested);
    return nullptr;
  }

  if (session_->ended()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (initializer->viewPixelWidth() != initializer->viewPixelHeight()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (!IsValidXRSpace(session_, initializer->space(), false)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (initializer->layout() != XRCompositionLayer::kLayoutMono &&
      initializer->layout() != XRCompositionLayer::kLayoutStereo) {
    exception_state.ThrowDOMException(DOMExceptionCode::kNotSupportedError,
                                      kOnlyMonoOrStereoSupported);
    return nullptr;
  }

  const IntSize desired_size(initializer->viewPixelWidth(),
                             initializer->viewPixelHeight());

  VLOG(1) << __func__ << " size: " << desired_size.Width() << " x "
          << desired_size.Height();

  WebGLTexture* color_textures[2] = {};

  color_textures[0] =
      session_->GetWebGLResourceManager()->CreateTextureSwapChain(
          webgl_context_, GL_TEXTURE_CUBE_MAP, desired_size,
          initializer->alpha(), false /* foveation */,
          true /* can_discard_depth */, initializer->isStatic());

  XRViewport* left_viewport = MakeGarbageCollected<XRViewport>(
      0, 0, desired_size.Width(), desired_size.Height());
  XRViewport* right_viewport;
  if (initializer->layout() == XRCompositionLayer::kLayoutStereo) {
    color_textures[1] =
        session_->GetWebGLResourceManager()->CreateTextureSwapChain(
            webgl_context_, GL_TEXTURE_CUBE_MAP, desired_size,
            initializer->alpha(), false /* foveation */,
            true /* can_discard_depth */, initializer->isStatic());
    right_viewport = left_viewport;
  } else {
    right_viewport = nullptr;
  }

  auto* layer = MakeGarbageCollected<XRCubeLayer>(
      session_, initializer->space(), webgl_context_, color_textures[0],
      color_textures[1],
      /*left depth_stencil_texture*/ nullptr,
      /*right depth_stencil_texture*/ nullptr, left_viewport, right_viewport);

  if (initializer->orientation()) {
    layer->setOrientation(initializer->orientation());
  }

  return layer;
}

XRWebGLSubImage* XRWebGLBinding::getSubImage(XRCompositionLayer* layer,
                                             XRFrame* frame,
                                             const String& eye,
                                             ExceptionState& exception_state) {
  if (session_->ended()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (frame->session() != session_) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kNotMonoLayer);
    return nullptr;
  }

  if (!frame->IsActive()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kNotActiveFrame);
    return nullptr;
  }

  if ((session_ != layer->session()) || (webgl_context_ != layer->context()) ||
      (session_ != frame->session())) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kMixingLayers);
    return nullptr;
  }

  if (layer->IsProjectionLayer()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kWrongLayerTypeForSubImage);
    return nullptr;
  }

  if (layer->IsStatic() && !layer->needsRedraw()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kStaticLayer);
    return nullptr;
  }

  XRView::XREye eyeEnum = XRView::kEyeNone;
  int offset = 0;
  if (eye == "left") {
    eyeEnum = XRView::kEyeLeft;
  } else if (eye == "right") {
    offset = 1;
    eyeEnum = XRView::kEyeRight;
  }

  if ((layer->layout() == "stereo" && eyeEnum == XRView::kEyeNone) ||
      (layer->layout() == "mono" && eyeEnum != XRView::kEyeNone)) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kWrongEyeType);
    return nullptr;
  }

  session_->AddXRLayer(layer);

  XRWebGLSubImage* subimage = nullptr;
  auto it = subimage_cache_.find(layer);
  if (it != subimage_cache_.end()) {
    subimage = it->value;
  }

  // if subimage wasn't in the cache because this is the first call or it was
  // gc'ed
  if (subimage == nullptr) {
    subimage = MakeGarbageCollected<XRWebGLSubImage>();
    subimage_cache_.insert(layer, subimage);
  }

  bool isSideBySide = layer->layout() != "stereo" && layer->layout() != "mono";

  subimage->assign(
      layer->getTextureForEye(eyeEnum),
      layer->getDepthStencilTextureForEye(eyeEnum),
      layer->getViewportForEye(isSideBySide ? XRView::kEyeNone : eyeEnum),
      offset);

  return subimage;
}

XRWebGLSubImage* XRWebGLBinding::getViewSubImage(
    XRCompositionLayer* layer,
    XRView* view,
    ExceptionState& exception_state) {
  if (session_->ended()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kSessionEnded);
    return nullptr;
  }

  if (!layer->stereo()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kNotStereoLayer);
    return nullptr;
  }

  XRFrame* frame = view->frame();

  if (frame->session() != session_) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kNotMonoLayer);
    return nullptr;
  }

  if (!frame->IsActive()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kNotActiveFrame);
    return nullptr;
  }

  if (!layer->IsProjectionLayer()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kWrongLayerTypeForViewSubImage);
    return nullptr;
  }

  if ((session_ != layer->session()) || (webgl_context_ != layer->context()) ||
      (session_ != frame->session())) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kMixingLayers);
    return nullptr;
  }

  if (layer->IsStatic() && !layer->needsRedraw()) {
    exception_state.ThrowDOMException(DOMExceptionCode::kInvalidStateError,
                                      kStaticLayer);
    return nullptr;
  }

  session_->AddXRLayer(layer);

  auto it_for_view = subimage_cache_per_view_.find(view);
  if (it_for_view == subimage_cache_per_view_.end()) {
    subimage_cache_per_view_.insert(view,
                                    MakeGarbageCollected<LayerToSubImage>());
    it_for_view = subimage_cache_per_view_.find(view);
    DCHECK(it_for_view != subimage_cache_per_view_.end());
  }
  DCHECK(it_for_view->value);

  XRWebGLSubImage* subimage = nullptr;
  auto it_for_subimages = it_for_view->value->find(layer);
  if (it_for_subimages != it_for_view->value->end()) {
    subimage = it_for_subimages->value;
  }

  // if subimage wasn't in the cache because this is the first call or it was
  // gc'ed
  if (subimage == nullptr) {
    subimage = MakeGarbageCollected<XRWebGLSubImage>();
    it_for_view->value->insert(layer, subimage);
  }

  subimage->assign(layer->getTexture(view), layer->getDepthStencilTexture(view),
                   layer->getViewport(view), layer->getImageIndex(view));

  return subimage;
}

Vector<GLenum> XRWebGLBinding::supportedColorFormats() const {
  static Vector<GLenum> formats = GetSupportedColorTextureSwapChainFormatsV1();
  static Vector<GLenum> formats2 = GetSupportedColorTextureSwapChainFormatsV2();
  return webgl2_ ? formats2 : formats;
}

Vector<GLenum> XRWebGLBinding::supportedDepthStencilFormats() const {
  static Vector<GLenum> formats = GetSupportedDepthTextureSwapChainFormatsV1();
  static Vector<GLenum> formats2 = GetSupportedDepthTextureSwapChainFormatsV2();
  return webgl2_ ? formats2 : formats;
}

void XRWebGLBinding::Trace(Visitor* visitor) const {
  visitor->Trace(session_);
  visitor->Trace(subimage_cache_);
  visitor->Trace(subimage_cache_per_view_);
  visitor->Trace(webgl_context_);
  ScriptWrappable::Trace(visitor);
}

}  // namespace blink
