#!/bin/bash

set -e

pushd build
cmake ..
make -j 12
popd


# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_blocks_big_light.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_blocks_dancing.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/red_ball_room.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/white_room.scene 1920 1080

# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/path_tracer.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera
# build/RayTrace res/renderer/path_tracer.renderer res/scene/red_ball_room.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/path_tracer.renderer res/scene/white_room.scene 1920 1080

# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/default.camera out/ms_comparison_pt_1.ppm 3
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/default.camera out/ms_comparison_pt_5.ppm 3

# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/red_ball_room.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/white_room.scene 1920 1080

# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/default.camera out/ms_comparison_bpt_1.ppm 3
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/default.camera out/ms_comparison_bpt_5.ppm 3

# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/default.camera out/comparison_cbwb_hwr.ppm 3
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/default.camera out/comparison_cbwb_swr.ppm


# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/red_ball_room.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/white_room.scene 1920 1080

# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/default.camera out/ms_comparison_st_1.ppm 3
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/default.camera out/ms_comparison_st_5.ppm 3

# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/full_scene_1_5.camera out/full_scene_1_5_test_1.ppm 3
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/full_scene_1_5.camera out/full_scene_1_5_test_5.ppm 3


# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks_dancing.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/red_ball_room.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/white_room.scene 1920 1080

# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box.scene 1920 1080 res/camera/default.camera out/ms_comparison_m2019_1.ppm 3
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/default.camera out/ms_comparison_m2019_5.ppm 3


# build/RayTrace res/renderer/Majercik2022.renderer res/scene/cornell_box.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022.renderer res/scene/cornell_box_with_blocks_dancing.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022.renderer res/scene/cornell_box_with_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera
# build/RayTrace res/renderer/Majercik2022.renderer res/scene/red_ball_room.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022.renderer res/scene/white_room.scene 1920 1080

# build/RayTrace res/renderer/Majercik2022.renderer res/scene/cornell_box.scene 1920 1080 res/camera/default.camera out/ms_comparison_m2022_1.ppm 3
# build/RayTrace res/renderer/Majercik2022.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/default.camera out/ms_comparison_m2022_5.ppm 3


# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/cornell_box_with_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera
# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/red_ball_room.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/white_room.scene 1920 1080

# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/default.camera out/ms_comparison_m2022_bpt_1.ppm 3
# build/RayTrace res/renderer/Majercik2022_bidirectional_path_tracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/default.camera out/ms_comparison_m2022_bpt_5.ppm 3


# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks_dancing.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks_big_light.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/red_ball_room.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/white_room.scene 1920 1080

# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box.scene 1920 1080 res/camera/default.camera out/ms_comparison_b2020_1.ppm 3
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/default.camera out/ms_comparison_b2020_5.ppm 3

# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera out/b2020_comparison_st.ppm 3
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera out/b2020_comparison_b2020.ppm 3

# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box.scene 1920 1080 res/camera/full_scene_1_5.camera out/full_scene_1_5_test_1_b2020.ppm 3
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/full_scene_1_5.camera out/full_scene_1_5_test_5_b2020.ppm 3


# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/cornell_box_with_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/red_ball_room.scene 1920 1080 res/camera/close.camera
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/white_room.scene 1920 1080

# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/cornell_box.scene 1920 1080 res/camera/default.camera out/ms_comparison_b2020_bpt_1.ppm 3
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/cornell_box_5.scene 1920 1080 res/camera/default.camera out/ms_comparison_b2020_bpt_5.ppm 3

# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera out/labyrinth_bpt.ppm 3
# build/RayTrace res/renderer/Bitterli2020_bidirectional_path_tracer.renderer res/scene/labyrinth.scene 1920 1080 res/camera/labyrinth.camera out/labyrinth_b2020_bpt.ppm 3


# build/RayTrace res/renderer/meta_normal_vector.renderer res/scene/cornell_box_with_blocks.scene 1920 1080
# build/RayTrace res/renderer/meta_position.renderer res/scene/cornell_box_with_blocks.scene 1920 1080
# build/RayTrace res/renderer/meta_distance.renderer res/scene/cornell_box_with_blocks.scene 1920 1080
# build/RayTrace res/renderer/meta_weights.renderer res/scene/cornell_box_with_blocks.scene 1920 1080
# build/RayTrace res/renderer/meta_normal_vector.renderer res/scene/labyrinth.scene 1920 1080

# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box.scene 1920 1080
# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box_5.scene 1920 1080
# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box_with_blocks.scene 1920 1080
# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box_with_ball.scene 1920 1080
# build/RayTrace res/renderer/phong_shader.renderer res/scene/labyrinth.scene 1920 1080
# build/RayTrace res/renderer/phong_shader.renderer res/scene/red_ball_room.scene 1920 1080
# build/RayTrace res/renderer/phong_shader.renderer res/scene/white_room.scene 1920 1080


# build/SoftwareRenderer res/renderer/SoftwareRendererPathTracer.renderer res/scene/cornell_box.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_pt_renderer_cornell_box.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererPathTracer.renderer res/scene/cornell_box_5.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_pt_renderer_cornell_box_5.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererPathTracer.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_pt_renderer_cornell_box_with_blocks.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererPathTracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_pt_renderer_cornell_box_with_blocks_big_light.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererPathTracer.renderer res/scene/cornell_box_with_ball.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_pt_renderer_cornell_box_with_ball.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererPathTracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_pt_renderer_cornell_box_with_blocks_and_ball.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererPathTracer.renderer res/scene/labyrinth.scene 1920 1080 15 res/camera/labyrinth.camera out/ref_images/ppm/sw_pt_renderer_labyrinth.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererPathTracer.renderer res/scene/red_ball_room.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_pt_renderer_red_ball_room.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererPathTracer.renderer res/scene/white_room.scene 1920 1080 15 res/camera/default.camera out/ref_images/ppm/sw_pt_renderer_white_room.ppm

# build/SoftwareRenderer res/renderer/SoftwareRendererBidirectionalPathTracer.renderer res/scene/cornell_box.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_bpt_renderer_cornell_box.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererBidirectionalPathTracer.renderer res/scene/cornell_box_5.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_bpt_renderer_cornell_box_5.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererBidirectionalPathTracer.renderer res/scene/cornell_box_with_blocks.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_bpt_renderer_cornell_box_with_blocks.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererBidirectionalPathTracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_bpt_renderer_cornell_box_with_blocks_big_light.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererBidirectionalPathTracer.renderer res/scene/cornell_box_with_ball.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_bpt_renderer_cornell_box_with_ball.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererBidirectionalPathTracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_bpt_renderer_cornell_box_with_blocks_and_ball.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererBidirectionalPathTracer.renderer res/scene/labyrinth.scene 1920 1080 15 res/camera/labyrinth.camera out/ref_images/ppm/sw_bpt_renderer_labyrinth.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererBidirectionalPathTracer.renderer res/scene/red_ball_room.scene 1920 1080 15 res/camera/close.camera out/ref_images/ppm/sw_bpt_renderer_red_ball_room.ppm
# build/SoftwareRenderer res/renderer/SoftwareRendererBidirectionalPathTracer.renderer res/scene/white_room.scene 1920 1080 15 res/camera/default.camera out/ref_images/ppm/sw_bpt_renderer_white_room.ppm
