#!/bin/bash

pushd build
cmake ..
make -j 8
popd


# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1280 720 out/cornell_box_blocks.ppm 30
build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks_dancing.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720
# build/RayTrace res/renderer/path_tracer.renderer res/scene/cornell_box_gray.scene 1280 720


# build/RayTrace res/renderer/path_tracer_gauss.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/path_tracer_median.renderer res/scene/cornell_box_with_ball.scene 1280 720


# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks.scene 1280 720 out/ddgi.ppm 5
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/Majercik2019.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/Majercik2019_median.renderer res/scene/cornell_box_with_ball.scene 1280 720

# build/RayTrace res/renderer/meta_normal_vector.renderer res/scene/cornell_box_with_blocks.scene 1280 720 out/meta_normal_vector.ppm 5
# build/RayTrace res/renderer/meta_position.renderer res/scene/cornell_box_with_blocks.scene 1280 720 out/meta_position.ppm 5
# build/RayTrace res/renderer/meta_distance.renderer res/scene/cornell_box_with_blocks.scene 1280 720 out/meta_distance.ppm 5
# build/RayTrace res/renderer/meta_weights.renderer res/scene/cornell_box_with_blocks.scene 1280 720 out/meta_weights.ppm 5

# build/RayTrace res/renderer/phong_shader.renderer res/scene/cornell_box_with_blocks.scene 720 720
