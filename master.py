#!/usr/bin/env python3

import os
import re
import csv
import copy
import time
import collections
import tempfile
import subprocess
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
# https://scikit-image.org/docs/dev/api/skimage.restoration.html#skimage.restoration.estimate_sigma
from skimage.restoration import estimate_sigma


RENDERTIME_REGEX = re.compile(r"INFO: Render Time: ([0-9.]+) \(~(\d+) FPS\)")
RENDERER_REGEX = re.compile(r"\t(\w+)\(([0-9.]+)\)")

EXECPATH = os.path.join("build", "RayTrace")
OUT_PATH = os.path.join("out", "master_results")
SCENE_PATH = os.path.join("res", "scene")
CAMERA_PATH = os.path.join("res", "camera")
RENDERER_PATH = os.path.join("res", "renderer")
DEFAULT_CAMERA_PATH = os.path.join(CAMERA_PATH, "default.camera")
LABYRINTH_CAMERA_PATH = os.path.join(CAMERA_PATH, "labyrinth.camera")

SCENE_NAMES = [
	"cornell_box",
	# "cornell_box_5",
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

RENDERER_NAMES = [
	"ShadowTracer",
	"path_tracer",
	"bidirectional_path_tracer",
	"Bitterli2020",
	"Bitterli2020_bidirectional_path_tracer",
	"Majercik2019",
	"Majercik2022",
	"Majercik2022_bidirectional_path_tracer"
]

RENDERER_PATHS = {name: os.path.join(RENDERER_PATH, "{}.renderer".format(name)) for name in RENDERER_NAMES}

def load_renderer(path):
	params = {}

	with open(path) as file:
		name = file.readline().strip()
		for line in file:
			if line.startswith("#"): continue
			match = re.match(RENDERER_REGEX, line)
			if match:
				params[match.group(1)] = match.group(2)
	
	return {
		"name": name,
		"params": params
	}

RENDERERS = {name: load_renderer(path) for name, path in RENDERER_PATHS.items()}

def load_sw_files():
	sw_files = {}
	for scene in SCENE_NAMES:
		path = os.path.join("out", "sw_renderer_{}.ppm".format(scene))
		with Image.open(path) as im:
			sw_files[scene] = np.array(im)
	return sw_files

OFFLINE_SCENES = load_sw_files()


def compile_renderer(renderer_data):
	result = renderer_data["name"] + "\n"
	for key, param in renderer_data["params"].items():
		result += "\t{}({})\n".format(key, param)
	return result


def convert_to_latex(elem, digits=3):
	if type(elem) == float:
		elem = round(elem, digits)
	if type(elem) != str:
		elem = str(elem)

	elem = elem.replace("_", "\\_")

	return elem

def compile_latex_table(label, caption, header, content, digits=3):
	caption = convert_to_latex(caption, digits)
	header = [convert_to_latex(h, digits) for h in header]
	content = [[convert_to_latex(r, digits) for r in row] for row in content]

	table = "\\begin{table}[h]\n"
	table += "\\label{" + label + "}\n"
	table += "\\caption{" + caption + "}\n"

	ls = "|".join(["l"] * len(header))
	table += "\\begin{tabular}{" + ls + "}\n"

	table += " & ".join(["\\textbf{" + h + "}" for h in header])
	table += " \\\\ \\hline\n"

	table += " \\\\\n".join([" & ".join([str(r) for r in row]) for row in content])
	table += "\n"

	table += "\\end{tabular}\n"
	table += "\\end{table}\n"

	return table

def run(renderer_data, scene_path, result_path, camera_path=DEFAULT_CAMERA_PATH, size=(1920, 1080), max_runs=30):
	outimage_path_ppm = os.path.join("{}.ppm".format(str(result_path)))
	outimage_path_png = os.path.join("{}.png".format(str(result_path)))

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
			path = os.path.join(OUT_PATH, "bpt_test_v{}_l{}".format(vi, li))
			bpt = copy.deepcopy(base_bpt)
			bpt["params"]["visionJumpCount"] = vi
			bpt["params"]["lightJumpCount"] = li

			times, image = run(bpt, SCENES["cornell_box"], path, max_runs=3)

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
		path = os.path.join(OUT_PATH, "bpt_test_r{}".format(ri))
		bpt = copy.deepcopy(base_bpt)
		bpt["params"]["raysPerPixelCount"] = ri

		times, image = run(bpt, SCENES["cornell_box"], path, max_runs=3)

		diff_path = os.path.join(OUT_PATH, "bpt_test_r{}_diff.png".format(ri))
		diff = compare(image, OFFLINE_SCENES["cornell_box"], diff_path)
		avg_time = get_average_time(times)
		results.append([ri, diff, avg_time])
	
	csv_path = os.path.join(OUT_PATH, "bpt_test_r.csv")
	with open(csv_path, "w") as csvfile:
		writer = csv.writer(csvfile)
		writer.writerow(["rays per pixel", "diff", "avg time"])
		writer.writerows(results)

def all_scenes_algorithms_test():
	base_path = os.path.join(OUT_PATH, "all_scenes_algorithms_test")
	if not os.path.exists(base_path):
		os.makedirs(base_path)

	result_only_tex = []
	all_tex = []

	for name, renderer in RENDERERS.items():
		path = os.path.join(base_path, name)
		if not os.path.exists(path):
			os.makedirs(path)

		results = []
		results_tex = []

		for scene_name, scene_path in SCENES.items():
			out_path = os.path.join(path, "{}_test".format(scene_name))
			camera = DEFAULT_CAMERA_PATH
			if scene_name == "labyrinth": camera = LABYRINTH_CAMERA_PATH

			times, image = run(renderer, scene_path, out_path, camera_path=camera)

			diff_path = os.path.join(path, "{}_diff.png".format(scene_name))
			diff = compare(image, OFFLINE_SCENES[scene_name], diff_path)
			avg_time = get_average_time(times)
			results.append([scene_name, diff, avg_time, 1.0/avg_time])

			times_header = ["seconds", "fps"]
			times_body = [[t["seconds"], t["fps"]] for t in times]

			times_path = os.path.join(path, "{}_times.csv".format(scene_name))
			with open(times_path, "w") as csvfile:
				writer = csv.writer(csvfile)
				writer.writerow(times_header)
				writer.writerows(times_body)

			times_tex_label = "tab:frame_results_{}_{}".format(name, scene_name)
			times_tex_caption = "Frame by frame results of renderer {} in scene {}".format(name, scene_name)
			times_tex = compile_latex_table(times_tex_label, times_tex_caption, times_header, times_body)
			results_tex.append(times_tex)

		test_header = ["scene", "diff", "avg time", "avg fps"]
		
		csv_path = os.path.join(path, "test.csv")
		with open(csv_path, "w") as csvfile:
			writer = csv.writer(csvfile)
			writer.writerow(test_header)
			writer.writerows(results)

		test_tex_label = "tab:results_overview_{}".format(name)
		test_tex_caption = "Result data for renderer {}".format(name)
		test_tex = ""
		# test_tex += "\\section{" + convert_to_latex(name) + "}\n"
		# test_tex += "\\label{sec:comparison_results:" + name + "}\n\n"
		test_tex += compile_latex_table(test_tex_label, test_tex_caption, test_header, results)
		result_only_tex.append(test_tex)
		test_tex += "\n\n"
		test_tex += "\n\n".join(results_tex)
		all_tex.append(test_tex)

	full_tex = "\n\n".join(all_tex)
	tex_path = os.path.join(base_path, "comparison_results.tex")
	with open(tex_path, "w") as texfile:
		texfile.write(full_tex)

	result_tex = "\n\n".join(result_only_tex)
	result_tex_path = os.path.join(base_path, "comparison_results_only.tex")
	with open(result_tex_path, "w") as texfile:
		texfile.write(result_tex)

# "ShadowTracer",
# "path_tracer",
# "bidirectional_path_tracer",
# "Bitterli2020",
# "Bitterli2020_bidirectional_path_tracer",
# "Majercik2019",
# "Majercik2022",
# "Majercik2022_bidirectional_path_tracer"

def all_scenes_algorithms_test_diagrams():
	renderer_lists = {
		"path_tracers": ["path_tracer", "bidirectional_path_tracer"],
		"shadow_tracers": ["ShadowTracer", "Bitterli2020", "Majercik2019"],
		"bitterlis": ["Bitterli2020", "Bitterli2020_bidirectional_path_tracer"],
		"majercicks": ["Majercik2019", "Majercik2022", "Majercik2022_bidirectional_path_tracer"],
		"bitterli_vs_majercik": ["Bitterli2020", "Majercik2019", "Majercik2022"]
	}

	for name, use_renderers in renderer_lists.items():
		scenes_algorithms_test_diagrams(name, use_renderers)

def scenes_algorithms_test_diagrams(file_name, use_renderers):
	base_path = os.path.join(OUT_PATH, "all_scenes_algorithms_test")

	# renderers = tuple(RENDERERS.keys())
	renderers = tuple(use_renderers)
	scene_data = collections.defaultdict(lambda: list())
	renderer_data = collections.defaultdict(lambda: list())
	for name in renderers:
		path = os.path.join(base_path, name)
		csv_path = os.path.join(path, "test.csv")

		with open(csv_path) as csvfile:
			for row in csv.DictReader(csvfile, skipinitialspace=True):
				value = float(round(float(row["diff"]), 2))
				scene_data[row["scene"]].append(value)
				renderer_data[name].append(value)

	scenes = tuple(scene_data.keys())

	# labels = renderers
	# data = {name: tuple(d) for name, d in scene_data.items()}

	labels = scenes
	data = {name: tuple(d) for name, d in renderer_data.items()}
	
	x = np.arange(len(labels))
	width = 1 / (len(data) + 2)
	multiplier = 0

	fig, ax = plt.subplots(layout="constrained")

	for name, renderer_data in data.items():
		offset = width * multiplier
		rects = ax.bar(x + offset, renderer_data, width, label=name)
		ax.bar_label(rects, padding=5)
		multiplier += 1

	plt.xticks(rotation=45)
	ax.set_ylabel("difference value")
	ax.set_title("differences by scene and renderer")
	ax.set_xticks(x + width, labels)
	ax.legend(loc='upper left', ncols=1)
	ax.set_ylim(0, 50)

	output_path = os.path.join(base_path, "{}.svg".format(file_name))
	fig.set_size_inches(14.5, 7.5)
	fig.savefig(output_path)


def compare_relative(image_1, image_5):
	total_percent = 0.0
	total = 0
	for r1, r5 in zip(image_1, image_5):
		for i1, i5 in zip(r1, r5):
			value1 = float(sum(i1) / 3.0)
			value5 = float(sum(i5) / 3.0)
			if value1 != 0.0:
				total += 1
				total_percent += value5 / value1

	return total_percent / float(total)


def renderer_1_5_test():
	base_path = os.path.join(OUT_PATH, "renderer_1_5_test")
	if not os.path.exists(base_path):
		os.makedirs(base_path)

	cornell_box_1_path = os.path.join(SCENE_PATH, "cornell_box.scene")
	cornell_box_5_path = os.path.join(SCENE_PATH, "cornell_box_5.scene")
	results = []

	for name, renderer in RENDERERS.items():
		path = os.path.join(base_path, name)
		if not os.path.exists(path):
			os.makedirs(path)

		out_1_path = os.path.join(path, "cornell_box_1_test")
		out_5_path = os.path.join(path, "cornell_box_5_test")

		times_1, image_1 = run(renderer, cornell_box_1_path, out_1_path)
		times_5, image_5 = run(renderer, cornell_box_5_path, out_5_path)

		diff = compare_relative(image_1, image_5)
		avg_time_1 = get_average_time(times_1)
		avg_time_5 = get_average_time(times_5)
		results.append([name, diff, avg_time_1, 1.0/avg_time_1, avg_time_5, 1.0/avg_time_5])

		times_header = ["seconds", "fps"]
		times_1_body = [[t["seconds"], t["fps"]] for t in times_1]
		times_5_body = [[t["seconds"], t["fps"]] for t in times_5]

		times_1_path = os.path.join(path, "cornell_box_1_times.csv")
		with open(times_1_path, "w") as csvfile:
			writer = csv.writer(csvfile)
			writer.writerow(times_header)
			writer.writerows(times_1_body)

		times_5_path = os.path.join(path, "cornell_box_5_times.csv")
		with open(times_5_path, "w") as csvfile:
			writer = csv.writer(csvfile)
			writer.writerow(times_header)
			writer.writerows(times_5_body)

	test_header = ["renderer", "diff", "avg time cb1", "avg fps cb1", "avg time cb5", "avg fps cb5"]
	
	csv_path = os.path.join(base_path, "test.csv")
	with open(csv_path, "w") as csvfile:
		writer = csv.writer(csvfile)
		writer.writerow(test_header)
		writer.writerows(results)

def renderer_1_5_test_diagrams_latex():
	base_path = os.path.join(OUT_PATH, "renderer_1_5_test")

	headers = None
	full_table = []
	renderers = []
	scene_data = []
	csv_path = os.path.join(base_path, "test.csv")

	with open(csv_path) as csvfile:
		for row in csv.DictReader(csvfile, skipinitialspace=True):
			if headers is None:
				headers = list(row.keys())
			full_table.append([(x if i == "renderer" else float(x)) for i, x in row.items()])
			renderers.append(row["renderer"])
			value = float(round(float(row["diff"]) * 100.0, 1))
			scene_data.append(value)

	renderer_names = tuple(renderers)
	diffs = tuple(scene_data)
	
	fig, ax = plt.subplots(layout="constrained")

	plt.xticks(rotation=45)
	ax.set_ylabel("equalness (%)")
	ax.set_title("similarity between one and five cornellbox scene")
	ax.set_ylim(0, 110)
	ax.bar(renderer_names, diffs)

	output_path = os.path.join(base_path, "test.svg")
	fig.set_size_inches(14.5, 7.5)
	fig.savefig(output_path)

	result_tex = compile_latex_table("tab:results_overview_1_5_test", "Results of the one and five cornell box test", headers, full_table)
	tex_path = os.path.join(base_path, "test.tex")
	with open(tex_path, "w") as texfile:
		texfile.write(result_tex)


def main():
	print("Hello Master run!")

	# convert_sw_files()
	# bpt_test_vl()
	# bpt_test_r()

	# all_scenes_algorithms_test()
	# all_scenes_algorithms_test_diagrams()

	# renderer_1_5_test()
	renderer_1_5_test_diagrams_latex()


if __name__ == "__main__":
	main()
