/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#include "gpupixel/filter/face_reshape_filter.h"
#include <algorithm>
#include "core/gpupixel_context.h"

namespace gpupixel {

namespace {
float ClampLevel(float value, float min_value, float max_value) {
  return std::max(min_value, std::min(value, max_value));
}
}  // namespace

const std::string kGPUPixelThinFaceFragmentShaderString =
#if defined(GPUPIXEL_GLES_SHADER)
    R"(
 precision highp float;
)"
#endif
    R"(
 varying vec2 textureCoordinate;
 uniform sampler2D inputImageTexture;

 uniform int hasFace;
 uniform float facePoints[106 * 2];

 uniform float aspectRatio;
 uniform float thinFaceDelta;
 uniform float bigEyeDelta;
 uniform float chinDelta;
 uniform float jawDelta;
 uniform float cheekboneDelta;
 uniform float foreheadDelta;
 uniform float headSizeDelta;
 uniform float overallFaceScaleDelta;
 uniform float eyeDistanceDelta;
 uniform float eyeAngleDelta;
 uniform float doubleEyelidDelta;
 uniform float brightenEyesDelta;
 uniform float noseSlimDelta;
 uniform float noseBridgeDelta;
 uniform float noseLengthDelta;
 uniform float noseTipDelta;
 uniform float lipSizeDelta;
 uniform float mouthWidthDelta;
 uniform float smileDelta;
 uniform float lipShapeDelta;
 uniform float browHeightDelta;
 uniform float browShapeDelta;
 uniform float browThicknessDelta;

 vec2 landmark(int index) {
     return vec2(facePoints[index * 2], facePoints[index * 2 + 1]);
 }

 vec2 aspectPoint(vec2 point) {
     return vec2(point.x, point.y / aspectRatio);
 }

 float aspectDistance(vec2 pointA, vec2 pointB) {
     return distance(aspectPoint(pointA), aspectPoint(pointB));
 }

 vec2 curveWarp(vec2 textureCoord, vec2 originPosition, vec2 targetPosition, float delta) {
     vec2 direction = (targetPosition - originPosition) * delta;
     float radius = max(aspectDistance(targetPosition, originPosition), 0.001);
     float ratio = aspectDistance(textureCoord, originPosition) / radius;
     ratio = clamp(1.0 - ratio, 0.0, 1.0);
     return textureCoord - direction * ratio;
 }

 vec2 curveWarpRadius(vec2 textureCoord, vec2 originPosition, vec2 direction, float radius, float delta) {
     float safeRadius = max(radius, 0.001);
     float ratio = aspectDistance(textureCoord, originPosition) / safeRadius;
     ratio = clamp(1.0 - ratio, 0.0, 1.0);
     ratio = ratio * ratio * (3.0 - 2.0 * ratio);
     return textureCoord - direction * delta * ratio;
 }

 vec2 localScale(vec2 textureCoord, vec2 center, float radius, float delta) {
     float safeRadius = max(radius, 0.001);
     float ratio = aspectDistance(textureCoord, center) / safeRadius;
     float mask = 1.0 - smoothstep(0.0, 1.0, ratio);
     float scale = clamp(1.0 - delta * mask, 0.72, 1.28);
     return center + (textureCoord - center) * scale;
 }

 vec2 enlargeEye(vec2 textureCoord, vec2 originPosition, float radius, float delta) {
     float weight = aspectDistance(textureCoord, originPosition) / max(radius, 0.001);
     weight = 1.0 - (1.0 - weight * weight) * delta;
     weight = clamp(weight, 0.0, 1.0);
     return originPosition + (textureCoord - originPosition) * weight;
 }

 float distanceToSegment(vec2 point, vec2 startPoint, vec2 endPoint) {
     vec2 pointA = aspectPoint(point);
     vec2 startA = aspectPoint(startPoint);
     vec2 endA = aspectPoint(endPoint);
     vec2 pa = pointA - startA;
     vec2 ba = endA - startA;
     float h = clamp(dot(pa, ba) / max(dot(ba, ba), 0.0001), 0.0, 1.0);
     return length(pa - ba * h);
 }

 float lineMask(vec2 point, vec2 startPoint, vec2 endPoint, float width) {
     float dist = distanceToSegment(point, startPoint, endPoint);
     return 1.0 - smoothstep(width * 0.35, width, dist);
 }

 float ellipseMask(vec2 point, vec2 center, float radiusX, float radiusY) {
     vec2 pointA = aspectPoint(point);
     vec2 centerA = aspectPoint(center);
     vec2 radius = vec2(max(radiusX, 0.001), max(radiusY / aspectRatio, 0.001));
     vec2 normalized = (pointA - centerA) / radius;
     return 1.0 - smoothstep(0.55, 1.0, length(normalized));
 }

 vec2 thinFace(vec2 currentCoordinate) {
     vec2 faceIndexs[9];
     faceIndexs[0] = vec2(3.0, 44.0);
     faceIndexs[1] = vec2(29.0, 44.0);
     faceIndexs[2] = vec2(7.0, 45.0);
     faceIndexs[3] = vec2(25.0, 45.0);
     faceIndexs[4] = vec2(10.0, 46.0);
     faceIndexs[5] = vec2(22.0, 46.0);
     faceIndexs[6] = vec2(14.0, 49.0);
     faceIndexs[7] = vec2(18.0, 49.0);
     faceIndexs[8] = vec2(16.0, 49.0);

     for (int i = 0; i < 9; i++) {
         int originIndex = int(faceIndexs[i].x);
         int targetIndex = int(faceIndexs[i].y);
         currentCoordinate = curveWarp(currentCoordinate, landmark(originIndex), landmark(targetIndex), thinFaceDelta);
     }
     return currentCoordinate;
 }

 vec2 bigEye(vec2 currentCoordinate) {
     vec2 faceIndexs[2];
     faceIndexs[0] = vec2(74.0, 72.0);
     faceIndexs[1] = vec2(77.0, 75.0);

     for (int i = 0; i < 2; i++) {
         int originIndex = int(faceIndexs[i].x);
         int targetIndex = int(faceIndexs[i].y);
         vec2 originPoint = landmark(originIndex);
         vec2 targetPoint = landmark(targetIndex);
         float radius = aspectDistance(targetPoint, originPoint) * 5.0;
         currentCoordinate = enlargeEye(currentCoordinate, originPoint, radius, bigEyeDelta);
     }
     return currentCoordinate;
 }

 vec2 faceShape(vec2 currentCoordinate) {
     vec2 faceCenter = landmark(46);
     vec2 leftFace = landmark(0);
     vec2 rightFace = landmark(32);
     vec2 chin = landmark(16);
     float faceWidth = max(aspectDistance(leftFace, rightFace), 0.12);
     float faceHeight = max(abs(chin.y - landmark(43).y), 0.12);
     float faceRadius = max(faceWidth * 0.72, faceHeight / aspectRatio);

     currentCoordinate = localScale(currentCoordinate, faceCenter, faceRadius * 1.18, overallFaceScaleDelta);
     currentCoordinate = localScale(currentCoordinate, faceCenter, faceRadius * 1.28, -headSizeDelta);
     currentCoordinate = thinFace(currentCoordinate);

     currentCoordinate = curveWarp(currentCoordinate, landmark(5), faceCenter, jawDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(9), faceCenter, jawDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(12), landmark(49), jawDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(20), landmark(49), jawDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(23), faceCenter, jawDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(27), faceCenter, jawDelta);

     currentCoordinate = curveWarp(currentCoordinate, landmark(3), faceCenter, cheekboneDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(6), landmark(46), cheekboneDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(26), landmark(46), cheekboneDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(29), faceCenter, cheekboneDelta);

     currentCoordinate = curveWarpRadius(currentCoordinate, chin, vec2(0.0, faceHeight * 0.32), faceWidth * 0.32, chinDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(14), chin, max(chinDelta, 0.0) * 0.45);
     currentCoordinate = curveWarp(currentCoordinate, landmark(18), chin, max(chinDelta, 0.0) * 0.45);

     vec2 foreheadDirection = vec2(0.0, -faceHeight * 0.24);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(33), foreheadDirection, faceWidth * 0.24, foreheadDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(37), foreheadDirection, faceWidth * 0.24, foreheadDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(38), foreheadDirection, faceWidth * 0.24, foreheadDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(42), foreheadDirection, faceWidth * 0.24, foreheadDelta);

     return currentCoordinate;
 }

 vec2 eyeShape(vec2 currentCoordinate) {
     vec2 leftEyeCenter = landmark(74);
     vec2 rightEyeCenter = landmark(77);
     float eyeGap = max(abs(rightEyeCenter.x - leftEyeCenter.x), 0.04);
     float leftEyeWidth = max(aspectDistance(landmark(52), landmark(55)), 0.03);
     float rightEyeWidth = max(aspectDistance(landmark(58), landmark(61)), 0.03);
     float eyeRadius = max(leftEyeWidth, rightEyeWidth) * 2.5;

     currentCoordinate = bigEye(currentCoordinate);
     currentCoordinate = curveWarpRadius(currentCoordinate, leftEyeCenter, vec2(-eyeGap * 0.22, 0.0), eyeRadius, eyeDistanceDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, rightEyeCenter, vec2(eyeGap * 0.22, 0.0), eyeRadius, eyeDistanceDelta);

     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(52), vec2(0.0, -eyeGap * 0.10), eyeRadius * 0.64, eyeAngleDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(61), vec2(0.0, -eyeGap * 0.10), eyeRadius * 0.64, eyeAngleDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(55), vec2(0.0, eyeGap * 0.045), eyeRadius * 0.52, eyeAngleDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(58), vec2(0.0, eyeGap * 0.045), eyeRadius * 0.52, eyeAngleDelta);

     return currentCoordinate;
 }

 vec2 noseShape(vec2 currentCoordinate) {
     vec2 noseCenter = landmark(46);
     float noseWidth = max(aspectDistance(landmark(80), landmark(83)), 0.03);
     float noseHeight = max(abs(landmark(51).y - landmark(44).y), 0.05);

     currentCoordinate = curveWarp(currentCoordinate, landmark(80), noseCenter, noseSlimDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(82), landmark(49), noseSlimDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(81), noseCenter, noseSlimDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(83), landmark(49), noseSlimDelta);

     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(49), vec2(0.0, noseHeight * 0.34), noseWidth * 1.65, noseLengthDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(51), vec2(0.0, noseHeight * 0.26), noseWidth * 1.45, noseLengthDelta);

     currentCoordinate = curveWarp(currentCoordinate, landmark(82), landmark(49), noseTipDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(83), landmark(49), noseTipDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(49), vec2(0.0, -noseHeight * 0.10), noseWidth * 1.25, noseTipDelta);

     currentCoordinate = curveWarp(currentCoordinate, landmark(78), landmark(44), noseBridgeDelta * 0.45);
     currentCoordinate = curveWarp(currentCoordinate, landmark(79), landmark(44), noseBridgeDelta * 0.45);

     return currentCoordinate;
 }

 vec2 mouthShape(vec2 currentCoordinate) {
     vec2 leftCorner = landmark(84);
     vec2 rightCorner = landmark(90);
     vec2 mouthCenter = (landmark(84) + landmark(90) + landmark(87) + landmark(93)) * 0.25;
     float mouthWidth = max(aspectDistance(leftCorner, rightCorner), 0.05);
     float mouthHeight = max(abs(landmark(93).y - landmark(87).y), 0.03);

     currentCoordinate = localScale(currentCoordinate, mouthCenter, mouthWidth * 0.78, lipSizeDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, leftCorner, vec2(-mouthWidth * 0.22, 0.0), mouthWidth * 0.45, mouthWidthDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, rightCorner, vec2(mouthWidth * 0.22, 0.0), mouthWidth * 0.45, mouthWidthDelta);

     currentCoordinate = curveWarpRadius(currentCoordinate, leftCorner, vec2(0.0, -mouthHeight * 1.28), mouthWidth * 0.38, smileDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, rightCorner, vec2(0.0, -mouthHeight * 1.28), mouthWidth * 0.38, smileDelta);

     currentCoordinate = curveWarp(currentCoordinate, landmark(96), mouthCenter, lipShapeDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(100), mouthCenter, lipShapeDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(103), mouthCenter, lipShapeDelta);
     currentCoordinate = curveWarp(currentCoordinate, landmark(101), mouthCenter, lipShapeDelta);

     return currentCoordinate;
 }

 vec2 browShape(vec2 currentCoordinate) {
     vec2 leftEyeCenter = landmark(74);
     vec2 rightEyeCenter = landmark(77);
     float eyeGap = max(abs(rightEyeCenter.x - leftEyeCenter.x), 0.04);
     vec2 heightDirection = vec2(0.0, -eyeGap * 0.14);

     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(33), heightDirection, eyeGap * 0.34, browHeightDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(36), heightDirection, eyeGap * 0.34, browHeightDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(39), heightDirection, eyeGap * 0.34, browHeightDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(42), heightDirection, eyeGap * 0.34, browHeightDelta);

     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(35), heightDirection, eyeGap * 0.22, browShapeDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(40), heightDirection, eyeGap * 0.22, browShapeDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(33), -heightDirection * 0.42, eyeGap * 0.18, browShapeDelta);
     currentCoordinate = curveWarpRadius(currentCoordinate, landmark(42), -heightDirection * 0.42, eyeGap * 0.18, browShapeDelta);

     return currentCoordinate;
 }

 vec3 applyLocalColor(vec3 color, vec2 currentCoordinate) {
     vec2 leftEyeCenter = landmark(74);
     vec2 rightEyeCenter = landmark(77);
     float eyeWidth = max(aspectDistance(landmark(52), landmark(55)), aspectDistance(landmark(58), landmark(61)));
     float eyeMask = max(ellipseMask(currentCoordinate, leftEyeCenter, eyeWidth * 1.22, eyeWidth * 0.62),
                         ellipseMask(currentCoordinate, rightEyeCenter, eyeWidth * 1.22, eyeWidth * 0.62));

     vec3 brightEye = min(color * 1.08 + vec3(0.035), vec3(1.0));
     color = mix(color, brightEye, eyeMask * brightenEyesDelta);

     vec2 lidOffset = vec2(0.0, -eyeWidth * 0.36);
     float eyelidMask = max(lineMask(currentCoordinate, landmark(52) + lidOffset, landmark(55) + lidOffset, eyeWidth * 0.10),
                            lineMask(currentCoordinate, landmark(58) + lidOffset, landmark(61) + lidOffset, eyeWidth * 0.10));
     color = mix(color, color * 0.86, eyelidMask * doubleEyelidDelta);

     float bridgeWidth = max(aspectDistance(landmark(80), landmark(83)), 0.03);
     float bridgeMask = lineMask(currentCoordinate, landmark(44), landmark(49), bridgeWidth * 0.22);
     vec3 bridgeColor = min(color * 1.06 + vec3(0.025), vec3(1.0));
     color = mix(color, bridgeColor, bridgeMask * noseBridgeDelta);

     float browWidth = max(abs(landmark(42).x - landmark(33).x), 0.08);
     float browMask = max(lineMask(currentCoordinate, landmark(33), landmark(37), browWidth * 0.045),
                          lineMask(currentCoordinate, landmark(38), landmark(42), browWidth * 0.045));
     float browAmount = abs(browThicknessDelta);
     vec3 browColor = browThicknessDelta >= 0.0
                          ? color * (1.0 - 0.38 * browAmount)
                          : min(color * (1.0 + 0.24 * browAmount), vec3(1.0));
     color = mix(color, browColor, browMask * browAmount);

     return color;
 }

 void main() {
     vec2 positionToUse = textureCoordinate;

     if (hasFace == 1) {
         positionToUse = faceShape(positionToUse);
         positionToUse = eyeShape(positionToUse);
         positionToUse = noseShape(positionToUse);
         positionToUse = mouthShape(positionToUse);
         positionToUse = browShape(positionToUse);
     }

     vec4 color = texture2D(inputImageTexture, positionToUse);
     if (hasFace == 1) {
         color.rgb = applyLocalColor(color.rgb, textureCoordinate);
     }
     gl_FragColor = color;
 }
)";

FaceReshapeFilter::FaceReshapeFilter() {}

FaceReshapeFilter::~FaceReshapeFilter() {}

std::shared_ptr<FaceReshapeFilter> FaceReshapeFilter::Create() {
  auto ret = std::shared_ptr<FaceReshapeFilter>(new FaceReshapeFilter());
  gpupixel::GPUPixelContext::GetInstance()->SyncRunWithContext([&] {
    if (ret && !ret->Init()) {
      ret.reset();
    }
  });
  return ret;
}

bool FaceReshapeFilter::Init() {
  if (!InitWithFragmentShaderString(kGPUPixelThinFaceFragmentShaderString)) {
    return false;
  }

  RegisterProperty("thin_face", 0.0f,
                   "Face slim/V-line level.",
                   [this](float& val) { SetFaceSlimLevel(val); });
  RegisterProperty("face_slim", 0.0f,
                   "Face slim/V-line level.",
                   [this](float& val) { SetFaceSlimLevel(val); });
  RegisterProperty("big_eye", 0.0f,
                   "Eye enlarge level.",
                   [this](float& val) { SetEyeZoomLevel(val); });
  RegisterProperty("eye_enlarge", 0.0f,
                   "Eye enlarge level.",
                   [this](float& val) { SetEyeZoomLevel(val); });
  RegisterProperty("chin_adjust", 0.0f,
                   "Chin length/shape adjustment.",
                   [this](float& val) { SetChinLevel(val); });
  RegisterProperty("jaw_adjust", 0.0f,
                   "Jaw width adjustment.",
                   [this](float& val) { SetJawLevel(val); });
  RegisterProperty("cheekbone_adjust", 0.0f,
                   "Cheekbone width/height adjustment.",
                   [this](float& val) { SetCheekboneLevel(val); });
  RegisterProperty("forehead_adjust", 0.0f,
                   "Forehead height adjustment.",
                   [this](float& val) { SetForeheadLevel(val); });
  RegisterProperty("head_size_adjust", 0.0f,
                   "Head size adjustment.",
                   [this](float& val) { SetHeadSizeLevel(val); });
  RegisterProperty("overall_face_scale", 0.0f,
                   "Overall face scale adjustment.",
                   [this](float& val) { SetOverallFaceScaleLevel(val); });
  RegisterProperty("eye_distance", 0.0f,
                   "Eye distance adjustment.",
                   [this](float& val) { SetEyeDistanceLevel(val); });
  RegisterProperty("eye_angle", 0.0f,
                   "Eye angle adjustment.",
                   [this](float& val) { SetEyeAngleLevel(val); });
  RegisterProperty("double_eyelid", 0.0f,
                   "Double eyelid emphasis.",
                   [this](float& val) { SetDoubleEyelidLevel(val); });
  RegisterProperty("brighten_eyes", 0.0f,
                   "Eye brightening level.",
                   [this](float& val) { SetBrightenEyesLevel(val); });
  RegisterProperty("nose_slim", 0.0f,
                   "Nose slimming level.",
                   [this](float& val) { SetNoseSlimLevel(val); });
  RegisterProperty("nose_bridge_height", 0.0f,
                   "Nose bridge emphasis.",
                   [this](float& val) { SetNoseBridgeLevel(val); });
  RegisterProperty("nose_length", 0.0f,
                   "Nose length adjustment.",
                   [this](float& val) { SetNoseLengthLevel(val); });
  RegisterProperty("nose_tip_refine", 0.0f,
                   "Nose tip refinement.",
                   [this](float& val) { SetNoseTipLevel(val); });
  RegisterProperty("lip_size", 0.0f,
                   "Lip size adjustment.",
                   [this](float& val) { SetLipSizeLevel(val); });
  RegisterProperty("mouth_width", 0.0f,
                   "Mouth width adjustment.",
                   [this](float& val) { SetMouthWidthLevel(val); });
  RegisterProperty("smile_adjust", 0.0f,
                   "Smile corner adjustment.",
                   [this](float& val) { SetSmileLevel(val); });
  RegisterProperty("lip_shape_refine", 0.0f,
                   "Lip shape refinement.",
                   [this](float& val) { SetLipShapeLevel(val); });
  RegisterProperty("brow_height", 0.0f,
                   "Brow height adjustment.",
                   [this](float& val) { SetBrowHeightLevel(val); });
  RegisterProperty("brow_shape", 0.0f,
                   "Brow arch/shape adjustment.",
                   [this](float& val) { SetBrowShapeLevel(val); });
  RegisterProperty("brow_thickness", 0.0f,
                   "Brow thickness/color adjustment.",
                   [this](float& val) { SetBrowThicknessLevel(val); });

  std::vector<float> defaut;
  RegisterProperty("face_landmark", defaut,
                   "The face landmark of filter.",
                   [this](std::vector<float> val) { SetFaceLandmarks(val); });

  return true;
}

void FaceReshapeFilter::SetFaceLandmarks(std::vector<float> landmarks) {
  if (landmarks.size() < 106 * 2) {
    has_face_ = false;
    return;
  }

  face_landmarks_ = landmarks;
  has_face_ = true;
}

bool FaceReshapeFilter::DoRender(bool updateSinks) {
  float aspect = (float)framebuffer_->GetWidth() / framebuffer_->GetHeight();
  filter_program_->SetUniformValue("aspectRatio", aspect);

  filter_program_->SetUniformValue("thinFaceDelta", thin_face_delta_);
  filter_program_->SetUniformValue("bigEyeDelta", big_eye_delta_);
  filter_program_->SetUniformValue("chinDelta", chin_delta_);
  filter_program_->SetUniformValue("jawDelta", jaw_delta_);
  filter_program_->SetUniformValue("cheekboneDelta", cheekbone_delta_);
  filter_program_->SetUniformValue("foreheadDelta", forehead_delta_);
  filter_program_->SetUniformValue("headSizeDelta", head_size_delta_);
  filter_program_->SetUniformValue("overallFaceScaleDelta",
                                   overall_face_scale_delta_);
  filter_program_->SetUniformValue("eyeDistanceDelta", eye_distance_delta_);
  filter_program_->SetUniformValue("eyeAngleDelta", eye_angle_delta_);
  filter_program_->SetUniformValue("doubleEyelidDelta", double_eyelid_delta_);
  filter_program_->SetUniformValue("brightenEyesDelta", brighten_eyes_delta_);
  filter_program_->SetUniformValue("noseSlimDelta", nose_slim_delta_);
  filter_program_->SetUniformValue("noseBridgeDelta", nose_bridge_delta_);
  filter_program_->SetUniformValue("noseLengthDelta", nose_length_delta_);
  filter_program_->SetUniformValue("noseTipDelta", nose_tip_delta_);
  filter_program_->SetUniformValue("lipSizeDelta", lip_size_delta_);
  filter_program_->SetUniformValue("mouthWidthDelta", mouth_width_delta_);
  filter_program_->SetUniformValue("smileDelta", smile_delta_);
  filter_program_->SetUniformValue("lipShapeDelta", lip_shape_delta_);
  filter_program_->SetUniformValue("browHeightDelta", brow_height_delta_);
  filter_program_->SetUniformValue("browShapeDelta", brow_shape_delta_);
  filter_program_->SetUniformValue("browThicknessDelta",
                                   brow_thickness_delta_);

  filter_program_->SetUniformValue("hasFace", has_face_);
  if (has_face_) {
    int landmark_float_count =
        std::min(static_cast<int>(face_landmarks_.size()), 106 * 2);
    filter_program_->SetUniformValue("facePoints", face_landmarks_.data(),
                                     landmark_float_count);
  }
  return Filter::DoRender(updateSinks);
}

void FaceReshapeFilter::SetFaceSlimLevel(float level) {
  thin_face_delta_ = ClampLevel(level, -0.15f, 0.15f);
}

void FaceReshapeFilter::SetEyeZoomLevel(float level) {
  big_eye_delta_ = ClampLevel(level, -0.35f, 0.35f);
}

void FaceReshapeFilter::SetChinLevel(float level) {
  chin_delta_ = ClampLevel(level, -0.12f, 0.12f);
}

void FaceReshapeFilter::SetJawLevel(float level) {
  jaw_delta_ = ClampLevel(level, -0.12f, 0.12f);
}

void FaceReshapeFilter::SetCheekboneLevel(float level) {
  cheekbone_delta_ = ClampLevel(level, -0.10f, 0.10f);
}

void FaceReshapeFilter::SetForeheadLevel(float level) {
  forehead_delta_ = ClampLevel(level, -0.10f, 0.10f);
}

void FaceReshapeFilter::SetHeadSizeLevel(float level) {
  head_size_delta_ = ClampLevel(level, -0.16f, 0.16f);
}

void FaceReshapeFilter::SetOverallFaceScaleLevel(float level) {
  overall_face_scale_delta_ = ClampLevel(level, -0.18f, 0.18f);
}

void FaceReshapeFilter::SetEyeDistanceLevel(float level) {
  eye_distance_delta_ = ClampLevel(level, -0.14f, 0.14f);
}

void FaceReshapeFilter::SetEyeAngleLevel(float level) {
  eye_angle_delta_ = ClampLevel(level, -0.14f, 0.14f);
}

void FaceReshapeFilter::SetDoubleEyelidLevel(float level) {
  double_eyelid_delta_ = ClampLevel(level, 0.0f, 1.0f);
}

void FaceReshapeFilter::SetBrightenEyesLevel(float level) {
  brighten_eyes_delta_ = ClampLevel(level, 0.0f, 1.0f);
}

void FaceReshapeFilter::SetNoseSlimLevel(float level) {
  nose_slim_delta_ = ClampLevel(level, -0.12f, 0.12f);
}

void FaceReshapeFilter::SetNoseBridgeLevel(float level) {
  nose_bridge_delta_ = ClampLevel(level, 0.0f, 1.0f);
}

void FaceReshapeFilter::SetNoseLengthLevel(float level) {
  nose_length_delta_ = ClampLevel(level, -0.12f, 0.12f);
}

void FaceReshapeFilter::SetNoseTipLevel(float level) {
  nose_tip_delta_ = ClampLevel(level, -0.12f, 0.12f);
}

void FaceReshapeFilter::SetLipSizeLevel(float level) {
  lip_size_delta_ = ClampLevel(level, -0.22f, 0.22f);
}

void FaceReshapeFilter::SetMouthWidthLevel(float level) {
  mouth_width_delta_ = ClampLevel(level, -0.16f, 0.16f);
}

void FaceReshapeFilter::SetSmileLevel(float level) {
  smile_delta_ = ClampLevel(level, -0.12f, 0.12f);
}

void FaceReshapeFilter::SetLipShapeLevel(float level) {
  lip_shape_delta_ = ClampLevel(level, -0.12f, 0.12f);
}

void FaceReshapeFilter::SetBrowHeightLevel(float level) {
  brow_height_delta_ = ClampLevel(level, -0.14f, 0.14f);
}

void FaceReshapeFilter::SetBrowShapeLevel(float level) {
  brow_shape_delta_ = ClampLevel(level, -0.14f, 0.14f);
}

void FaceReshapeFilter::SetBrowThicknessLevel(float level) {
  brow_thickness_delta_ = ClampLevel(level, -1.0f, 1.0f);
}

}  // namespace gpupixel
