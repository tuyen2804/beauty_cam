/*
 * GPUPixel
 *
 * Created by PixPark on 2021/6/24.
 * Copyright © 2021 PixPark. All rights reserved.
 */

#pragma once

#include "gpupixel/filter/filter.h"

namespace gpupixel {
class GPUPIXEL_API FaceReshapeFilter : public Filter {
 public:
  static std::shared_ptr<FaceReshapeFilter> Create();
  FaceReshapeFilter();
  ~FaceReshapeFilter();

  bool Init();
  bool DoRender(bool updateSinks = true) override;

  void SetFaceSlimLevel(float level);
  void SetEyeZoomLevel(float level);
  void SetFaceLandmarks(std::vector<float> landmarks);

  void SetChinLevel(float level);
  void SetJawLevel(float level);
  void SetCheekboneLevel(float level);
  void SetForeheadLevel(float level);
  void SetHeadSizeLevel(float level);
  void SetOverallFaceScaleLevel(float level);
  void SetEyeDistanceLevel(float level);
  void SetEyeAngleLevel(float level);
  void SetDoubleEyelidLevel(float level);
  void SetBrightenEyesLevel(float level);
  void SetNoseSlimLevel(float level);
  void SetNoseBridgeLevel(float level);
  void SetNoseLengthLevel(float level);
  void SetNoseTipLevel(float level);
  void SetLipSizeLevel(float level);
  void SetMouthWidthLevel(float level);
  void SetSmileLevel(float level);
  void SetLipShapeLevel(float level);
  void SetBrowHeightLevel(float level);
  void SetBrowShapeLevel(float level);
  void SetBrowThicknessLevel(float level);

 private:
  float thin_face_delta_ = 0.0f;
  float big_eye_delta_ = 0.0f;
  float chin_delta_ = 0.0f;
  float jaw_delta_ = 0.0f;
  float cheekbone_delta_ = 0.0f;
  float forehead_delta_ = 0.0f;
  float head_size_delta_ = 0.0f;
  float overall_face_scale_delta_ = 0.0f;
  float eye_distance_delta_ = 0.0f;
  float eye_angle_delta_ = 0.0f;
  float double_eyelid_delta_ = 0.0f;
  float brighten_eyes_delta_ = 0.0f;
  float nose_slim_delta_ = 0.0f;
  float nose_bridge_delta_ = 0.0f;
  float nose_length_delta_ = 0.0f;
  float nose_tip_delta_ = 0.0f;
  float lip_size_delta_ = 0.0f;
  float mouth_width_delta_ = 0.0f;
  float smile_delta_ = 0.0f;
  float lip_shape_delta_ = 0.0f;
  float brow_height_delta_ = 0.0f;
  float brow_shape_delta_ = 0.0f;
  float brow_thickness_delta_ = 0.0f;

  std::vector<float> face_landmarks_;
  int has_face_ = 0;
};

}  // namespace gpupixel
