#!/bin/bash

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

# build/RayTrace res/renderer/unidirectional_path_tracer.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/unidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/unidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720
# build/RayTrace res/renderer/unidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/unidirectional_path_tracer.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/unidirectional_path_tracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/unidirectional_path_tracer.renderer res/scene/labyrinth.scene 1280 720
# build/RayTrace res/renderer/unidirectional_path_tracer.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/unidirectional_path_tracer.renderer res/scene/white_room.scene 1280 720


# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/white_room.scene 1280 720


# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks.scene 1280 720 res/camera/default.camera out/ddgi.ppm 5
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/labyrinth.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/white_room.scene 1280 720

# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/red_ball_room.scene 1280 720
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/white_room.scene 1280 720

# build/RayTrace res/renderer/ShadowTracer.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera out/shadowtracer_labyrinth.ppm 5
# build/RayTrace res/renderer/Bitterli2020.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera out/bitterli2020_labyrinth.ppm 5

# build/RayTrace res/renderer/meta_normal_vector.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/meta_position.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/meta_distance.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/meta_weights.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/meta_normal_vector.renderer res/scene/labyrinth.scene 1280 720

# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box_with_blocks.scene 720 720
# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box_with_ball.scene 720 720
# build/RayTrace res/renderer/phong_shader.renderer res/scene/labyrinth.scene 720 720

# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/cornell_box.scene 1280 720 res/camera/default.camera out/sw_renderer_cornell_box.ppm
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/cornell_box_with_blocks.scene 1280 720 res/camera/default.camera out/sw_renderer_cornell_box_with_blocks.ppm
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/cornell_box_with_blocks_big_light.scene 1280 720 res/camera/default.camera out/sw_renderer_cornell_box_with_blocks_big_light.ppm
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/cornell_box_with_ball.scene 1280 720 res/camera/default.camera out/sw_renderer_cornell_box_with_ball.ppm
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720 res/camera/default.camera out/sw_renderer_cornell_box_with_blocks_and_ball.ppm
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/labyrinth.scene 1280 720 res/camera/labyrinth.camera out/sw_renderer_labyrinth.ppm
build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/red_ball_room.scene 1280 720 res/camera/default.camera out/sw_renderer_red_ball_room.ppm
# build/SoftwareRenderer res/renderer/SoftwareRenderer.renderer res/scene/white_room.scene 1280 720 res/camera/default.camera out/sw_renderer_white_room.ppm
