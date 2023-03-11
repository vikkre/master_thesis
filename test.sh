#!/bin/bash

set -e

pushd build
cmake ..
make -j 12
popd


# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_blocks.scene 1280 720 res/camera/default.camera out/cornell_box_blocks.ppm 30
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/labyrinth.scene 1280 720
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/photon_mapper.renderer res/scene/white_room.scene 1280 720

# build/RayTrace res/renderer/photon_mapper_gauss.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/photon_mapper_median.renderer res/scene/cornell_box_with_ball.scene 1280 720

# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_5.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/labyrinth.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/white_room.scene 1280 720

# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box.scene 1280 720 res/camera/default.camera out/ms_comparison_pt_1.ppm 3
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_5.scene 1280 720 res/camera/default.camera out/ms_comparison_pt_5.ppm 3

# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_5.scene 1280 720
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/labyrinth.scene 1280 720
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/white_room.scene 1280 720

# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box.scene 1280 720 res/camera/default.camera out/ms_comparison_bpt_1.ppm 3
# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_5.scene 1280 720 res/camera/default.camera out/ms_comparison_bpt_5.ppm 3

# build/RayTrace res/renderer/bidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1280 720 res/camera/default.camera out/comparison_cbwb_hwr.ppm 3
build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/cornell_box_with_blocks.scene 1280 720 res/camera/default.camera out/comparison_cbwb_swr.ppm


# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box.scene 1280 720 res/camera/default.camera out/shadow_tracer_cornell_box.ppm 30
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_5.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/white_room.scene 1280 720

# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box.scene 1280 720 res/camera/default.camera out/ms_comparison_st_1.ppm 3
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_5.scene 1280 720 res/camera/default.camera out/ms_comparison_st_5.ppm 3


# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks.scene 1280 720 res/camera/default.camera out/ddgi.ppm 5
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_5.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/white_room.scene 1280 720

# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box.scene 1280 720 res/camera/default.camera out/ms_comparison_m2019_1.ppm 3
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_5.scene 1280 720 res/camera/default.camera out/ms_comparison_m2019_5.ppm 3


# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_5.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/white_room.scene 1280 720

# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box.scene 1280 720 res/camera/default.camera out/ms_comparison_b2020_1.ppm 3
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_5.scene 1280 720 res/camera/default.camera out/ms_comparison_b2020_5.ppm 3

# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera out/b2020_comparison_st.ppm 3
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera out/b2020_comparison_b2020.ppm 3


# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera
# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/white_room.scene 1280 720

# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/cornell_box.scene 1280 720 res/camera/default.camera out/ms_comparison_b2020c_1.ppm 3
# build/RayTrace res/renderer/Bitterli2020Custom.renderer res/scene/cornell_box_5.scene 1280 720 res/camera/default.camera out/ms_comparison_b2020c_5.ppm 3


# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera out/shadowtracer_labyrinth.ppm 5
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera out/bitterli2020_labyrinth.ppm 5

# build/RayTrace res/renderer/meta_normal_vector.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/meta_position.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/meta_distance.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/meta_weights.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/meta_normal_vector.renderer res/scene/labyrinth.scene 1280 720

# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box_5.scene 1280 720
# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/phong_shader.renderer res/scene/labyrinth.scene 1280 720
# build/RayTrace res/renderer/phong_shader.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/phong_shader.renderer res/scene/white_room.scene 1280 720

# build/SoftwareRenderer res/renderer/SoftwareRenderer.rendl_box_with_blocks.scene 1280 720 res/camera/default.camera out/comparison_cbwb_hwr.ppm 3
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/cornell_derer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera out/sw_renderer_labyrinth.ppm
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/red_ball_room.scene 1280 720 res/camera/default.camera out/sw_renderer_red_ball_room.ppm
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/white_room.scene 1280 720 res/camera/default.camera out/sw_renderer_white_room.ppm
