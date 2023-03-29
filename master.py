#!/usr/bin/env python3

import os
import re
import csv
import copy
import time
import collections
import tempfile
import subprocess
from pathlib import Path
import numpy as np
import matplotlib.pyplot as plt
from PIL import Image
# https://scikit-image.org/docs/dev/api/skimage.restoration.html#skimage.restoration.estimate_sigma
from skimage.restoration import estimate_sigma


RENDERTIME_REGEX = re.compile(r"INFO: Render Time: ([0-9.]+) \(~(\d+) FPS\)")
RENDERER_REGEX = re.compile(r"\t(\w+)\(([0-9.]+)\)")

FIG_SIZES = (10, 7)
FIG_ROUNDING = 1

EXECPATH = os.path.join("build", "RayTrace")
OUT_PATH = os.path.join("out", "master_results")
REF_PATH = os.path.join("out", "ref_images")
SCENE_PATH = os.path.join("res", "scene")
CAMERA_PATH = os.path.join("res", "camera")
RENDERER_PATH = os.path.join("res", "renderer")
DEFAULT_CAMERA_PATH = os.path.join(CAMERA_PATH, "default.camera")
LABYRINTH_CAMERA_PATH = os.path.join(CAMERA_PATH, "labyrinth.camera")
CLOSE_CAMERA_PATH = os.path.join(CAMERA_PATH, "close.camera")

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

SCENE_LABELS = {
	"cornell_box": "empty Cornell Box",
	# "cornell_box_5": "Cornell Box 5 times",
	"cornell_box_with_blocks": "Cornell Box",
	# "cornell_box_with_blocks_dancing": " dancing Cornell Box",
	"cornell_box_with_ball": "Cornell Box glass",
	"cornell_box_with_blocks_and_ball": "Cornell Box light ball",
	"cornell_box_with_blocks_big_light": "Cornell Box big light",
	"labyrinth": "labyrinth",
	"red_ball_room": "red ball room",
	"white_room": "white room",
}

SCENE_CAMERAS = {
	"cornell_box": CLOSE_CAMERA_PATH,
	# "cornell_box_5": CLOSE_CAMERA_PATH,
	"cornell_box_with_blocks": CLOSE_CAMERA_PATH,
	# "cornell_box_with_blocks_dancing": CLOSE_CAMERA_PATH,
	"cornell_box_with_ball": CLOSE_CAMERA_PATH,
	"cornell_box_with_blocks_and_ball": CLOSE_CAMERA_PATH,
	"cornell_box_with_blocks_big_light": CLOSE_CAMERA_PATH,
	"labyrinth": LABYRINTH_CAMERA_PATH,
	"red_ball_room": CLOSE_CAMERA_PATH,
	"white_room": DEFAULT_CAMERA_PATH,
}

def get_scene_labels(scenes):
	return tuple([SCENE_LABELS[scene] for scene in scenes])


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

RENDERER_LABELS = {
	"ShadowTracer": "Basic ray tracer",
	"path_tracer": "Path tracer",
	"bidirectional_path_tracer": "Bidirectional path tracer",
	"Bitterli2020": "ReSTIR",
	"Bitterli2020_bidirectional_path_tracer": "ReSTIR path tracer (own)",
	"Majercik2019": "DDGI",
	"Majercik2022": "DDGI + ReSTIR",
	"Majercik2022_bidirectional_path_tracer": "DDGI ReSTIR path tracer (own)",
}

RENDERER_COLORS = {
	"ShadowTracer": "skyblue",
	"path_tracer": "cyan",
	"bidirectional_path_tracer": "blue",
	"Bitterli2020": "lime",
	"Bitterli2020_bidirectional_path_tracer": "green",
	"Majercik2019": "yellow",
	"Majercik2022": "orange",
	"Majercik2022_bidirectional_path_tracer": "red",
}

def get_renderer_labels(renderers):
	return tuple([RENDERER_LABELS[renderer] for renderer in renderers])

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
	img_paths = os.path.join(REF_PATH, "png")
	sw_files = {}
	for scene in SCENE_NAMES:
		path = os.path.join(img_paths, "sw_bpt_renderer_{}.png".format(scene))
		with Image.open(path) as im:
			sw_files[scene] = np.array(im)
	return sw_files

OFFLINE_SCENES = load_sw_files()

def load_icons():
	img_paths = os.path.join(REF_PATH, "icons")
	sw_files = {}
	for scene in SCENE_NAMES:
		path = os.path.join(img_paths, "sw_bpt_renderer_{}.png".format(scene))
		with Image.open(path) as im:
			sw_files[scene] = np.array(im)
	return sw_files

ICONS = load_icons()


def cut_image(image, cut_parts, full_image_path, border_size=3):
	full_image = copy.deepcopy(image)

	for part in cut_parts:
		start = part[0]
		size = part[1]
		cut_image_path = part[2]

		sub_image = [[(0, 0, 0)] * size[1] for _ in range(size[0])]
		for dx in range(size[0]):
			for dy in range(size[1]):
				x = start[0] + dx
				y = start[1] + dy
				sub_image[dx][dy] = image[x][y]

				xborder = (dx < border_size) or (dx >= size[0] - border_size)
				yborder = (dy < border_size) or (dy >= size[1] - border_size)
				if xborder or yborder:
					full_image[x][y] = (255, 0, 255)

		sub_image = np.array(sub_image, dtype=np.uint8).reshape(size[0], size[1], 3)
		img = Image.fromarray(sub_image)
		img.save(cut_image_path)

	img = Image.fromarray(full_image)
	img.save(full_image_path)


def compile_renderer(renderer_data):
	result = renderer_data["name"] + "\n"
	for key, param in renderer_data["params"].items():
		result += "\t{}({})\n".format(key, param)
	return result


def convert_to_latex(elem, digits=1):
	if type(elem) == float:
		elem = round(elem, digits)
	if type(elem) != str:
		elem = str(elem)

	elem = elem.replace("_", "\\_")

	return elem

def compile_latex_table(label, caption, header, content, digits=1):
	caption = convert_to_latex(caption, digits)
	header = [convert_to_latex(h, digits) for h in header]
	content = [[convert_to_latex(r, digits) for r in row] for row in content]

	table = "\\begin{table}[h]\n"
	table += "\\caption{" + caption + "}\n"
	table += "\\label{" + label + "}\n"

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
	ppm_path = os.path.join(REF_PATH, "ppm")
	png_path = os.path.join(REF_PATH, "png")

	image_files = []
	for file in os.listdir(ppm_path):
		name = Path(file).stem
		image_files.append({
			"ppm": os.path.join(ppm_path, name + ".ppm"),
			"png": os.path.join(png_path, name + ".png")
		})

	for f in image_files:
		with Image.open(f["ppm"]) as im:
			im.save(f["png"])

def generate_icons():
	png_path = os.path.join(REF_PATH, "png")
	icons_path = os.path.join(REF_PATH, "icons")

	image_files = []
	for file in os.listdir(png_path):
		image_files.append({
			"png": os.path.join(png_path, file),
			"icons": os.path.join(icons_path, file)
		})

	height = 180
	width = int((height * 16) / 9)
	
	for f in image_files:
		with Image.open(f["png"]) as im:
			im = im.resize((width, height))
			im.save(f["icons"])

# def convert_1_5_files():
# 	sw_files = []
# 	for name in ["1", "5"]:
# 		sw_files.append({
# 			"ppm": os.path.join("out", "full_scene_1_5_test_{}.ppm".format(name)),
# 			"png": os.path.join("out", "ref_images", "full_scene_1_5_test_{}.png".format(name))
# 		})

# 	for f in sw_files:
# 		with Image.open(f["ppm"]) as im:
# 			im.save(f["png"])

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

	for name, renderer in RENDERERS.items():
		path = os.path.join(base_path, name)
		if not os.path.exists(path):
			os.makedirs(path)

		results = []

		for scene_name, scene_path in SCENES.items():
			out_path = os.path.join(path, "{}_test".format(scene_name))
			camera = SCENE_CAMERAS[scene_name]
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

		test_header = ["scene", "diff", "avg time", "avg fps"]
		
		csv_path = os.path.join(path, "test.csv")
		with open(csv_path, "w") as csvfile:
			writer = csv.writer(csvfile)
			writer.writerow(test_header)
			writer.writerows(results)


def all_scenes_algorithms_test_diagrams():
	renderer_lists = {
		"path_tracers": ["ShadowTracer", "path_tracer", "bidirectional_path_tracer"],
		"bidirectionals": ["bidirectional_path_tracer", "Bitterli2020", "Bitterli2020_bidirectional_path_tracer", "Majercik2022_bidirectional_path_tracer"],
		"bitterlis": ["ShadowTracer", "Bitterli2020", "Bitterli2020_bidirectional_path_tracer"],
		"shadow_tracers": ["Bitterli2020", "Majercik2022", "Bitterli2020_bidirectional_path_tracer", "Majercik2022_bidirectional_path_tracer"],
		"majerciks": ["Majercik2019", "Majercik2022", "Majercik2022_bidirectional_path_tracer"],
		"bitterli_vs_majercik": ["Bitterli2020", "Majercik2019", "Majercik2022"],
		"all": ["ShadowTracer", "path_tracer", "bidirectional_path_tracer", "Bitterli2020", "Bitterli2020_bidirectional_path_tracer", "Majercik2019", "Majercik2022", "Majercik2022_bidirectional_path_tracer"]
	}

	for name, use_renderers in renderer_lists.items():
		scenes_algorithms_test_diagrams(name, use_renderers)

	base_path = os.path.join(OUT_PATH, "all_scenes_algorithms_test")
	scenes_data = collections.defaultdict(lambda: list())
	result_tex = []
	for name in RENDERERS.keys():
		path = os.path.join(base_path, name)
		csv_path = os.path.join(path, "test.csv")

		results_converter = {
			"scene":    lambda v: SCENE_LABELS[v],
			"diff":     lambda v: float(v),
			"avg time": lambda v: float(v) * 1000,
			"avg fps":  lambda v: float(v),
		}

		results = []
		with open(csv_path) as csvfile:
			for row in csv.DictReader(csvfile, skipinitialspace=True):
				value = float(round(float(row["avg time"]) * 1000, FIG_ROUNDING))
				scenes_data[name].append(value)
				results.append([results_converter[k](v) for k, v in row.items()])

		test_header = ["Scene name", "diff value", "Avg time (ms)", "Avg \\ac{fps}"]
		test_tex_label = "tab:results_overview_{}".format(name)
		test_tex_caption = "Result data for {}".format(RENDERER_LABELS[name])
		test_tex = compile_latex_table(test_tex_label, test_tex_caption, test_header, results)
		result_tex.append(test_tex)

	scene_names = get_scene_labels(SCENES.keys())
	scenes_data = {name: tuple(data) for name, data in scenes_data.items()}
	fig, ax = plt.subplots()

	for name, data in scenes_data.items():
		label = RENDERER_LABELS[name]
		color = RENDERER_COLORS[name]
		ax.plot(scene_names, data, label=label, linewidth=3, color=color)

	ax.plot(scene_names, [33.3] * len(scene_names), label="30 FPS line", linewidth=3, color="gray", linestyle="dashed")
	ax.plot(scene_names, [16.6] * len(scene_names), label="60 FPS line", linewidth=3, color="black", linestyle="dashed")

	plt.xticks(rotation=45)
	ax.set_ylabel("average rendering time per frame (ms)")
	ax.set_title("rendertime by renderer and scene")
	ax.legend()
	ax.set_ylim(0, 85)
	fig.tight_layout(rect=(0, 0, 1, 1))

	for index, name in enumerate(SCENES.keys()):
		size=0.08

		img = ICONS[name]
		x = index * 0.107 + 0.12
		y = 0.035
		tmp_ax = fig.add_axes([x, y, size, size])
		tmp_ax.axison = False
		tmp_ax.imshow(img)

	output_path = os.path.join(base_path, "rendering_times.svg")
	fig.set_size_inches(*FIG_SIZES)
	fig.savefig(output_path)

	result_tex = "\n\n".join(result_tex)
	result_tex_path = os.path.join(base_path, "comparison_results_only.tex")
	with open(result_tex_path, "w") as texfile:
		texfile.write(result_tex)


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
				value = float(round(float(row["diff"]), FIG_ROUNDING))
				scene_data[row["scene"]].append(value)
				renderer_data[name].append(value)

	scenes = get_scene_labels(scene_data.keys())

	# labels = renderers
	# data = {name: tuple(d) for name, d in scene_data.items()}

	labels = scenes
	data = {name: tuple(d) for name, d in renderer_data.items()}
	
	x = np.arange(len(labels))
	width = 1 / (len(data) + 2)
	label_pos = x + width * (len(data) - 1) / 2
	multiplier = 0

	fig, ax = plt.subplots()

	for name, renderer_data in data.items():
		label = RENDERER_LABELS[name]
		color = RENDERER_COLORS[name]
		offset = width * multiplier
		rects = ax.bar(x + offset, renderer_data, width, label=label, color=color)
		ax.bar_label(rects, padding=5, rotation=90)
		multiplier += 1

	plt.xticks(rotation=45)
	ax.set_ylabel("difference value")
	ax.set_title("differences by scene and renderer")
	ax.set_xticks(label_pos, labels)
	ax.legend(loc='upper left', ncols=1)
	ax.set_ylim(0, 60)
	fig.tight_layout(rect=(0, 0, 1, 1))

	for index, name in enumerate(scene_data.keys()):
		size=0.08

		img = ICONS[name]
		x = index * 0.107 + 0.12
		y = 0.035
		tmp_ax = fig.add_axes([x, y, size, size])
		tmp_ax.axison = False
		tmp_ax.imshow(img)

	output_path = os.path.join(base_path, "{}.svg".format(file_name))
	fig.set_size_inches(*FIG_SIZES)
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

	full_table = []
	renderers = []
	scene_data = []
	colors = []
	csv_path = os.path.join(base_path, "test.csv")

	with open(csv_path) as csvfile:
		for row in csv.DictReader(csvfile, skipinitialspace=True):
			full_table.append([(RENDERER_LABELS[x] if i == "renderer" else float(x)) for i, x in row.items()][:2])
			renderers.append(row["renderer"])
			value = float(round(float(row["diff"]) * 100.0, 1))
			scene_data.append(value)
			colors.append(RENDERER_COLORS[row["renderer"]])

	renderer_names = get_renderer_labels(renderers)
	diffs = tuple(scene_data)
	
	fig, ax = plt.subplots(layout="constrained")

	plt.xticks(rotation=45)
	ax.set_ylabel("equalness from one to five cornell boxes (%)")
	ax.set_title("similarity between one and five cornellbox scene")
	ax.set_ylim(0, 110)
	bars = ax.bar(renderer_names, diffs)
	for i, c in enumerate(colors):
		bars[i].set_color(c)

	output_path = os.path.join(base_path, "test.svg")
	fig.set_size_inches(*FIG_SIZES)
	fig.savefig(output_path)

	headers = ["Renderer name", "difference between one and five cornell boxes (\\%)"]
	result_tex = compile_latex_table("tab:results_overview_1_5_test", "Results of the one and five cornell box test", headers, full_table)
	tex_path = os.path.join(base_path, "test.tex")
	with open(tex_path, "w") as texfile:
		texfile.write(result_tex)


def cut_images():
	result_path = os.path.join(OUT_PATH, "all_scenes_algorithms_test")
	cuts_path = os.path.join(OUT_PATH, "cuts")
	glas_ball_cuts = os.path.join(cuts_path, "glas_ball")

	for name in RENDERER_NAMES:
		scene_path = os.path.join(result_path, name, "cornell_box_with_ball_test.png")
		full_img_path = os.path.join(glas_ball_cuts, "{}_full.png".format(name))
		top_cut_img_path = os.path.join(glas_ball_cuts, "{}_top_cut.png".format(name))
		bottom_cut_img_path = os.path.join(glas_ball_cuts, "{}_bottom_cut.png".format(name))

		with Image.open(scene_path) as im:
			scene = np.array(im)
		
		cut_parts = [
			((100, 750), (200, 200), top_cut_img_path),
			((750, 750), (300, 400), bottom_cut_img_path),
		]
		cut_image(scene, cut_parts, full_img_path)


def main():
	print("Hello Master run!")

	# convert_sw_files()
	# generate_icons()
	# convert_1_5_files()
	
	# bpt_test_vl()
	# bpt_test_r()

	# all_scenes_algorithms_test()
	all_scenes_algorithms_test_diagrams()

	# renderer_1_5_test()
	# renderer_1_5_test_diagrams_latex()

	# cut_images()


if __name__ == "__main__":
	main()
