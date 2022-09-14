#!/usr/bin/env python3

import copy
import os
import re
import csv
import subprocess
from PIL import Image


EXECPATH = os.path.join("build", "RayTrace")
OUT_PATH = os.path.join("out")
IMAGE_WIDTH = 1280
IMAGE_HEIGHT = 720
MAX_RUNS = 10

LIGHT_RAY_COUNTS = [100, 250, 500, 750, 1000]
LIGHT_JUMP_COUNTS = [1, 2, 3, 4]
VISION_JUMP_COUNTS = [2, 3, 4, 5]
LIGHT_COLLECTION_DISTANCES = [1.0, 3.0, 5.0, 8.0]
VISION_RAY_COUNTS = [3, 10, 20]
COLLECTION_DISTANCE_SHRINK_FACTORS = [5.0, 7.5, 10.0, 15.0]
LIGHT_COLLECTION_COUNTS = [5, 10, 20, 30]

RENDERTIME_REGEX = re.compile(r"INFO: Render Time: ([0-9.]+) \(~(\d+) FPS\)")
RUNCATEGORY = os.path.join("presentation")
RENDERER_PATH = os.path.join("res", "renderer")
SCENE_PATH = os.path.join("res", "scene")

RENDERERS = [
	"monte_carlo",
	"monte_carlo_gauss",
	"monte_carlo_median",
	"monte_carlo_gauss_median",
	"monte_carlo_median_gauss",
	"ddgi",
	"ddgi_gauss",
	"ddgi_median",
	"ddgi_gauss_median",
	"ddgi_median_gauss"
]

SCENES = [
	"cornell_box",
	"cornell_box_with_blocks",
	"cornell_box_with_ball",
	"cornell_box_with_blocks_and_ball"
]


def loadCSV(path):
	try:
		with open(path) as f:
			return [row for row in csv.DictReader(f, skipinitialspace=True)]
	except FileNotFoundError:
		return []

def saveCSV(path, data):
	keys = data[0].keys()

	with open(path, "w", newline="") as output_file:
		dict_writer = csv.DictWriter(output_file, keys)
		dict_writer.writeheader()
		dict_writer.writerows(data)


def ray_trace(renderer_path, scene_path, dirpath, outname):
	outimage_path_ppm = os.path.join(dirpath, "{}.ppm".format(outname))
	outimage_path_png = os.path.join(dirpath, "{}.png".format(outname))
	args = [EXECPATH, renderer_path, scene_path, str(IMAGE_WIDTH), str(IMAGE_HEIGHT), outimage_path_ppm, str(MAX_RUNS)]
	process = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)

	times = []
	for line in process.stdout.split("\n"):
		match = re.match(RENDERTIME_REGEX, line)
		if match:
			times.append({
				"seconds": match.group(1),
				"fps": match.group(2)
			})

	with Image.open(outimage_path_ppm) as im:
		im.save(outimage_path_png)
	os.remove(outimage_path_ppm)

	return times


def run_monte_carlo_renderers(out_path, run_category):
	dirpath = os.path.join(out_path, run_category)
	if not os.path.exists(dirpath):
		os.mkdir(dirpath)

	results_path = os.path.join(out_path, "{}.csv".format(run_category))
	results = loadCSV(results_path)
	done = []
	for result in results:
		rcopy = copy.deepcopy(result)
		rcopy.pop("average_time")
		rcopy.pop("average_fps")
		done.append(rcopy)

	for renderer in RENDERERS:
		for scene in SCENES:
			current = dict(
				renderer=renderer, scene=scene
			)
			current_key = {k:str(v) for k,v in current.items()}
			already_done = current_key in done
			print("Current:", current_key, "(skipping)" if already_done else "")
			if already_done: continue

			name = "{renderer}___{scene}".format(**current)
			
			renderer_path = os.path.join(RENDERER_PATH, "{}.renderer".format(renderer))
			scene_path = os.path.join(SCENE_PATH, "{}.scene".format(scene))
			data = ray_trace(renderer_path, scene_path, dirpath, name)

			data_path = os.path.join(dirpath, "{}.csv".format(name))
			saveCSV(data_path, data)

			current["average_time"] = sum([float(x["seconds"]) for x in data]) / len(data)
			current["average_fps"] = sum([float(x["fps"]) for x in data]) / len(data)
			results.append(current)
			saveCSV(results_path, results)


def main():
	if not os.path.exists(OUT_PATH):
		os.mkdir(OUT_PATH)

	run_monte_carlo_renderers(OUT_PATH, RUNCATEGORY)


if __name__ == "__main__":
	main()
