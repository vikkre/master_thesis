#!/usr/bin/env python3

import os
import re
import csv
import copy
import time
import tempfile
import subprocess
import numpy as np
from PIL import Image
# https://scikit-image.org/docs/dev/api/skimage.restoration.html#skimage.restoration.estimate_sigma
from skimage.restoration import estimate_sigma

EXECPATH = os.path.join("build", "RayTrace")
OUT_PATH = os.path.join("out", "master_results")
SCENE_PATH = os.path.join("res", "scene")
CAMERA_PATH = os.path.join("res", "camera")
DEFAULT_CAMERA_PATH = os.path.join(CAMERA_PATH, "default.camera")

SCENE_NAMES = [
	"cornell_box",
	"cornell_box_5",
	"cornell_box_with_blocks",
	# "cornell_box_with_blocks_dancing",
	"cornell_box_with_ball",
	"cornell_box_with_blocks_and_ball",
	"cornell_box_with_blocks_big_light",
	"labyrinth",
	"red_ball_room",
	"white_room",
]

SCENES = {name: os.path.join(SCENE_PATH, "{}.scene".format(name)) for name in SCENE_NAMES}

def load_sw_files():
	sw_files = {}
	for scene in SCENE_NAMES:
		path = os.path.join("out", "sw_renderer_{}.ppm".format(scene))
		with Image.open(path) as im:
			sw_files[scene] = np.array(im)
	return sw_files

OFFLINE_SCENES = load_sw_files()

RENDERTIME_REGEX = re.compile(r"INFO: Render Time: ([0-9.]+) \(~(\d+) FPS\)")


def compile_renderer(renderer_data):
	result = renderer_data["name"] + "\n"
	for key, param in renderer_data["params"].items():
		result += "\t{}({})\n".format(key, param)
	return result

def run(renderer_data, scene_path, result_name, camera_path=DEFAULT_CAMERA_PATH, size=(1920, 1080), max_runs=30):
	outimage_path_ppm = os.path.join(OUT_PATH, "{}.ppm".format(result_name))
	outimage_path_png = os.path.join(OUT_PATH, "{}.png".format(result_name))

	tmp_renderer = tempfile.NamedTemporaryFile(suffix=".renderer")
	tmp_renderer.write(compile_renderer(renderer_data).encode("ascii"))
	tmp_renderer.flush()

	args = [EXECPATH,
		tmp_renderer.name, scene_path,
		str(size[0]), str(size[1]),
		camera_path, outimage_path_ppm, str(max_runs)
	]

	process = subprocess.run(args, stdout=subprocess.PIPE, stderr=subprocess.STDOUT, universal_newlines=True)

	tmp_renderer.close()
	
	if (process.returncode != 0):
		print("args: ", args)
		print(process.stdout)
		raise RuntimeError("Returncode was not 0")

	times = []
	for line in process.stdout.split("\n"):
		match = re.match(RENDERTIME_REGEX, line)
		if match:
			times.append({
				"seconds": float(match.group(1)),
				"fps": float(match.group(2))
			})

	with Image.open(outimage_path_ppm) as im:
		im.save(outimage_path_png)
		im_array = np.array(im)
	os.remove(outimage_path_ppm)

	return times, im_array

def get_sigma(image):
	im_array = np.array(image)
	sigmas = estimate_sigma(im_array, channel_axis=-1)
	return (sigmas[0] + sigmas[1] + sigmas[2]) / 3

def get_average_time(times):
	avg = 0.0
	for t in times:
		avg += t["seconds"]
	return avg / len(times)

def compare(img1, img2, diffpath, size=(1920, 1080)):
	diff_data = []
	total_error = 0
	for r1, r2 in zip(img1, img2):
		for i1, i2 in zip(r1, r2):
			error = 0
			for i in range(3):
				e = abs(int(i1[i]) - int(i2[i]))
				e = float(e) / 255.0
				e = e*e
				e = e * 255.0
				error += e
			error /= 3.0
			total_error += error
			error = int(error) * 3
			diff_data.append((error, int(error / 2), error))

	img = Image.new("RGB", size)
	img.putdata(diff_data)
	img.save(diffpath)

	return total_error / float(size[0] * size[1])

def convert_sw_files():
	sw_files = []
	for scene in SCENE_NAMES:
		sw_files.append({
			"ppm": os.path.join("out", "sw_renderer_{}.ppm".format(scene)),
			"png": os.path.join("out", "ref_images", "sw_renderer_{}.png".format(scene))
		})

	for f in sw_files:
		with Image.open(f["ppm"]) as im:
			im.save(f["png"])

def bpt_test_vl():
	bpt_vision = [1, 8]
	bpt_light = [0, 8]
	base_bpt = {
		"name": "BidirectionalPathTracer",
		"params": {
			"visionJumpCount": 5,
			"lightJumpCount": 5,
			"maxDepth": 5,
			"raysPerPixelCount": 3
		}
	}

	results = []

	for vi in range(bpt_vision[0], bpt_vision[1]):
		for li in range(bpt_light[0], bpt_light[1]):
			name = "bpt_test_v{}_l{}".format(vi, li)
			bpt = copy.deepcopy(base_bpt)
			bpt["params"]["visionJumpCount"] = vi
			bpt["params"]["lightJumpCount"] = li

			times, image = run(bpt, SCENES["cornell_box"], name, max_runs=3)

			diff_path = os.path.join(OUT_PATH, "bpt_test_v{}_l{}_diff.png".format(vi, li))
			diff = compare(image, OFFLINE_SCENES["cornell_box"], diff_path)
			avg_time = get_average_time(times)
			results.append([vi, li, diff, avg_time])
	
	csv_path = os.path.join(OUT_PATH, "bpt_test.csv")
	with open(csv_path, "w") as csvfile:
		writer = csv.writer(csvfile)
		writer.writerow(["vision jump count", "light jump count", "diff", "avg time"])
		writer.writerows(results)

def bpt_test_r():
	rays = [1, 8]
	base_bpt = {
		"name": "BidirectionalPathTracer",
		"params": {
			"visionJumpCount": 5,
			"lightJumpCount": 5,
			"maxDepth": 5,
			"raysPerPixelCount": 3
		}
	}

	results = []

	for ri in range(rays[0], rays[1]):
		name = "bpt_test_r{}".format(ri)
		bpt = copy.deepcopy(base_bpt)
		bpt["params"]["raysPerPixelCount"] = ri

		times, image = run(bpt, SCENES["cornell_box"], name, max_runs=3)

		diff_path = os.path.join(OUT_PATH, "bpt_test_r{}_diff.png".format(ri))
		diff = compare(image, OFFLINE_SCENES["cornell_box"], diff_path)
		avg_time = get_average_time(times)
		results.append([ri, diff, avg_time])
	
	csv_path = os.path.join(OUT_PATH, "bpt_test_r.csv")
	with open(csv_path, "w") as csvfile:
		writer = csv.writer(csvfile)
		writer.writerow(["rays per pixel", "diff", "avg time"])
		writer.writerows(results)

def bitterli2020_test():
	base_bitterli2020 = {
		"name": "Bitterli2020",
		"params": {
			"visionJumpCount": 3,
			"candidateCount": 64,
			"sampleCount": 8
		}
	}

	results = []

	for scene_name, scene_path in SCENES.items():
		name = "bitterli2020_test_{}".format(scene_name)

		times, image = run(base_bitterli2020, scene_path, name, max_runs=3)

		diff_path = os.path.join(OUT_PATH, "bitterli2020_test_{}_diff.png".format(scene_name))
		diff = compare(image, OFFLINE_SCENES[scene_name], diff_path)
		avg_time = get_average_time(times)
		results.append([scene_name, diff, avg_time])
	
	csv_path = os.path.join(OUT_PATH, "bitterli2020_test.csv")
	with open(csv_path, "w") as csvfile:
		writer = csv.writer(csvfile)
		writer.writerow(["scene", "diff", "avg time"])
		writer.writerows(results)

def majercik2019_test():
	rays = [1, 8]
	base_bpt = {
		"name": "BidirectionalPathTracer",
		"params": {
			"visionJumpCount": 5,
			"lightJumpCount": 5,
			"maxDepth": 5,
			"raysPerPixelCount": 3
		}
	}

	results = []

	for ri in range(rays[0], rays[1]):
		name = "bpt_test_r{}".format(ri)
		bpt = copy.deepcopy(base_bpt)
		bpt["params"]["raysPerPixelCount"] = ri

		times, image = run(bpt, SCENES["cornell_box"], name, max_runs=3)

		diff_path = os.path.join(OUT_PATH, "bpt_test_r{}_diff.png".format(ri))
		diff = compare(image, OFFLINE_SCENES["cornell_box"], diff_path)
		avg_time = get_average_time(times)
		results.append([ri, diff, avg_time])
	
	csv_path = os.path.join(OUT_PATH, "bpt_test_r.csv")
	with open(csv_path, "w") as csvfile:
		writer = csv.writer(csvfile)
		writer.writerow(["rays per pixel", "diff", "avg time"])
		writer.writerows(results)


def main():
	print("Hello Master run!")

	# convert_sw_files()
	# bpt_test_vl()
	# bpt_test_r()

	bitterli2020_test()
	# majercik2019_test()


if __name__ == "__main__":
	main()
