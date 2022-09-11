#!/bin/bash

pushd build
cmake ..
make -j 8
popd


# build/RayTrace res/renderer/full_monte_carlo.renderer res/scene/cornell_box_with_blocks.scene 1280 720 out/cornell_box_blocks.ppm 30
build/RayTrace res/renderer/full_monte_carlo.renderer res/scene/cornell_box_with_blocks.scene 1280 720
# build/RayTrace res/renderer/full_monte_carlo.renderer res/scene/cornell_box_with_ball.scene 1280 720
# build/RayTrace res/renderer/full_monte_carlo.renderer res/scene/cornell_box_with_blocks_and_ball.scene 1280 720


# build/RayTrace res/renderer/ddgi.renderer res/scene/cornell_box_with_blocks.scene 1280 720 out/ddgi.ppm 5
