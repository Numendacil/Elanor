# -*- coding: utf-8 -*-
from PIL import Image, ImageDraw, ImageFont, ImageColor
import numpy as np
from decimal import Decimal, ROUND_HALF_UP
from math import radians, tan
from os import path

_round = lambda f, r=ROUND_HALF_UP: int(Decimal(str(f)).quantize(Decimal("0"), rounding=r))
rgb = lambda r, g, b: (r, g, b)

upper_font_path = path.join(path.dirname(__file__), 'NotoSansCJKSC-Black.ttf')
lower_font_path = path.join(path.dirname(__file__), 'NotoSerifCJKSC-Black.ttf')


def get_gradient_2d(start, stop, width, height, is_horizontal=False):
	if is_horizontal:
		return np.tile(np.linspace(start, stop, width), (height, 1))
	else:
		return np.tile(np.linspace(start, stop, height), (width, 1)).T


def getTextWidth(text, font, width=100, height=500, recursive=False):
	step = 100
	img = Image.new("L", (width, height))
	draw = ImageDraw.Draw(img)
	draw.text((0, 0), text, font=font, fill=255)
	box = img.getbbox()
	if box[2] < width - step or (recursive and box[2] == width - step):
		return box[2]
	else:
		return getTextWidth(text=text, font=font, width=width+step, height=height, recursive=True)


def get_gradient_3d(width, height, start_list, stop_list, is_horizontal_list=(False, False, False)):
	result = np.zeros((height, width, len(start_list)), dtype=float)
	for i, (start, stop, is_horizontal) in enumerate(zip(start_list, stop_list, is_horizontal_list)):
		result[:, :, i] = get_gradient_2d(start, stop, width, height, is_horizontal)
	return result


def createLinearGradient(steps, width, height, size=1, center=0.5):
	margin_up=_round(height*(center-size/2))
	margin_down=_round(height*(1-center-size/2))
	result = np.zeros((0, width, len(steps[0])), dtype=float)
	for i, k in enumerate(steps.keys()):
		if k == 0:
			array = get_gradient_3d(width, _round(margin_up), steps[k], steps[k])
			result = np.vstack([result, array])
			continue
		pk = list(steps.keys())[i-1]
		h = _round(height*size*(k-pk))
		array = get_gradient_3d(width, h, steps[pk], steps[k])
		result = np.vstack([result, array])
		if k==1:
			array = get_gradient_3d(width, _round(margin_down), steps[k], steps[k])
			result = np.vstack([result, array])
			continue
	return result


def genBaseImage(width=1500, height=500):

	k=0.50   #渐变色缩放系数，不应大于1
	c=0.47    #渐变色中心位置
	
	lowerSilverArray = createLinearGradient({
		0: rgb(0, 15, 36),
		0.10: rgb(255, 255, 255),
		0.18: rgb(55, 58, 59),
		0.25: rgb(55, 58, 59),
		0.5: rgb(200, 200, 200),
		0.75: rgb(55, 58, 59),
		0.85: rgb(25, 20, 31),
		0.91: rgb(240, 240, 240),
		0.95: rgb(166, 175, 194),
		1: rgb(50, 50, 50)
	}, width=width, height=height, size=k, center=c + 0.1)
	
	goldArray = createLinearGradient({
		0: rgb(253, 241, 0),
		0.25: rgb(245, 253, 187),
		0.4: rgb(255, 255, 255),
		0.75: rgb(253, 219, 9),
		0.9: rgb(127, 53, 0),
		1: rgb(243, 196, 11)
	}, width=width, height=height, size=k, center=c)
	
	strokeRedArray = createLinearGradient({
		0: rgb(255, 100, 0),
		0.5: rgb(123, 0, 0),
		0.51: rgb(240, 0, 0),
		1: rgb(5, 0, 0)
	}, width=width, height=height, size=k, center=c)
	
	redArray = createLinearGradient({
		0: rgb(230, 0, 0),
		0.5: rgb(123, 0, 0),
		0.51: rgb(240, 0, 0),
		1: rgb(5, 0, 0)
	}, width=width, height=height, size=k, center=c)
	
	silver2Array = createLinearGradient({
		0: rgb(245, 246, 248),
		0.15: rgb(255, 255, 255),
		0.35: rgb(195, 213, 220),
		0.5: rgb(160, 190, 201),
		0.51: rgb(160, 190, 201),
		0.52: rgb(196, 215, 222),
		1.0: rgb(255, 255, 255)
	}, width=width, height=height, size=k, center=c)
	
	navyArray = createLinearGradient({
		0: rgb(16,25,58),
		0.03: rgb(255, 255, 255),
		0.08: rgb(16, 25, 58),
		0.2: rgb(16, 25, 58),
		1: rgb(16, 25, 58)
	}, width=width, height=height, size=k, center=c)

	rainbowArray = createLinearGradient({
		0: ImageColor.getrgb("red"),
		0.25: ImageColor.getrgb("yellow"),
		0.50: ImageColor.getrgb("green"),
		0.75: ImageColor.getrgb("blue"),
		1: ImageColor.getrgb("violet")
	}, width=width, height=height, size=k * 0.9, center=c + 0.03)
	
	result = {
		"lowerSilver": Image.fromarray(np.uint8(lowerSilverArray)).crop((0, 0, width, height)),
		"gold": Image.fromarray(np.uint8(goldArray)).crop((0, 0, width, height)),
		"red": Image.fromarray(np.uint8(redArray)).crop((0, 0, width, height)),
		"strokeRed": Image.fromarray(np.uint8(strokeRedArray)).crop((0, 0, width, height)),
		"silver2": Image.fromarray(np.uint8(silver2Array)).crop((0, 0, width, height)),
		"strokeNavy": Image.fromarray(np.uint8(navyArray)).crop((0, 0, width, height)),  # Width: 7
		"rainbow": Image.fromarray(np.uint8(rainbowArray)).crop((0, 0, width, height)),
		"baseStrokeBlack": Image.new("RGBA", (width, height), rgb(0, 0, 0)).crop((0, 0, width, height)),  # Width: 17
		"strokeBlack": Image.new("RGBA", (width, height), rgb(16, 25, 58)).crop((0, 0, width, height)),  # Width: 17
		"strokeWhite": Image.new("RGBA", (width, height), rgb(221, 221, 221)).crop((0, 0, width, height)),  # Width: 8
		"baseStrokeWhite": Image.new("RGBA", (width, height), rgb(255, 255, 255)).crop((0, 0, width, height))  # Width: 8
	}
	for k in result.keys():
		result[k].putalpha(255)
	return result


def genImage(word_a="5000兆円", word_b="欲しい!", default_width=1500, height=500,
			 bg="white", default_base=None, rainbow_upper = False, rainbow_lower = False):
	# width = max_width

	k=0.7     #字体缩放系数

	alpha = (0, 0, 0, 0)
	leftmargin = 30
	upmargin = 20
	font_upper = ImageFont.truetype(upper_font_path, _round(height*0.35*k)+ upmargin)
	font_lower = ImageFont.truetype(lower_font_path, _round(height*0.37*k)+ upmargin)
	subset = max(getTextWidth(word_a, font_upper, width=default_width,
								   height=_round(height/2)) - 100, 200)

	# Prepare Width
	upper_width = max([default_width,
					  getTextWidth(word_a, font_upper, width=default_width,
								   height=_round(height/2))]) + 300
	lower_width = max([default_width,
					   getTextWidth(word_b, font_lower, width=default_width,
									height=_round(height/2))]) + 300

	# Prepare base - Upper (if required)
	if default_width == upper_width:
		upper_base = default_base
	else:
		upper_base = genBaseImage(width=upper_width+leftmargin, height=_round(height/2)+ upmargin)

	# Prepare base - lower (if required)
	lower_base = genBaseImage(width=lower_width+leftmargin, height=_round(height/2)+ upmargin)
	# if default_width == lower_width:
	#     lower_base = default_base
	# else:

	# Prepare mask - Upper
	upper_mask_base = Image.new("L", (upper_width+leftmargin, _round(height/2)+ upmargin), 0)

	mask_img_upper = list()
	if rainbow_upper:
		upper_data =  [
			[
				(4, 4), (4, 4), (0, 0), (0, 0), (2, -3), (0, -3), (0, -3), (0, -3),
				(0, 0), (0, 0), (2, -3), (0, -3), (0, -3), (0, -3)
			],
			[
				22, 20, 16, 10, 6, 6, 3, 0, 16, 10, 6, 6, 3, 0
			],
			[
				"baseStrokeBlack",
				"rainbow",
				"baseStrokeBlack",
				"rainbow",
				"baseStrokeBlack",
				"baseStrokeWhite",
				"rainbow",
				"red",
				"baseStrokeBlack",
				"gold",
				"baseStrokeBlack",
				"baseStrokeWhite",
				"strokeRed",
				"red",
			]
		]
	else:
		upper_data = [
			[
				(4, 4), (4, 4), (0, 0), (0, 0), (2, -3), (0, -3), (0, -3), (0, -3)
			],
			[
				22, 20, 16, 10, 6, 6, 3, 0
			],
			[
				"baseStrokeBlack",
				"lowerSilver",
				"baseStrokeBlack",
				"gold",
				"baseStrokeBlack",
				"baseStrokeWhite",
				"strokeRed",
				"red",
			]
		]
	for pos, stroke, color in zip(upper_data[0], upper_data[1], upper_data[2]):
		mask_img_upper.append(upper_mask_base.copy())
		mask_draw_upper = ImageDraw.Draw(mask_img_upper[-1])
		mask_draw_upper.text((pos[0]+leftmargin, pos[1]+ upmargin), word_a,
							 font=font_upper, fill=255,
							 stroke_width=_round(stroke*height/500))
		

	# Prepare mask - lower
	lower_mask_base = Image.new("L", (lower_width+leftmargin, _round(height/2)+ upmargin), 0)
	mask_img_lower = list()
	if rainbow_lower:
		lower_data = [
			[
				(5, 2), (5, 2), (0, 0), (0, 0), (0, 0), (0, -3)
			], [
				22, 19, 17, 8, 7, 0
			], [
				"baseStrokeBlack",
				"lowerSilver",
				"strokeBlack",
				"strokeWhite",
				"strokeNavy",
				"rainbow"
			]
		]
	else:
		lower_data = [
			[
				(5, 2), (5, 2), (0, 0), (0, 0), (0, 0), (0, -3)
			], [
				22, 19, 17, 8, 7, 0
			], [
				"baseStrokeBlack",
				"lowerSilver",
				"strokeBlack",
				"strokeWhite",
				"strokeNavy",
				"silver2"
			]
		]
	for pos, stroke, color in zip(lower_data[0], lower_data[1], lower_data[2]):
		mask_img_lower.append(lower_mask_base.copy())
		mask_draw_lower = ImageDraw.Draw(mask_img_lower[-1])
		mask_draw_lower.text((pos[0]+leftmargin, pos[1]+ upmargin), word_b,
							  font=font_lower, fill=255,
							  stroke_width=_round(stroke*height/500))

	# Draw text - Upper
	img_upper = Image.new("RGBA", (upper_width, _round(height/2)), alpha)

	for i, (pos, stroke, color) in enumerate(zip(upper_data[0], upper_data[1], upper_data[2])):
		img_upper_part = Image.new("RGBA", (upper_width+leftmargin, _round(height/2)+ upmargin), alpha)
		img_upper_part.paste(upper_base[color], (0, 0), mask=mask_img_upper[i])
		img_upper.alpha_composite(img_upper_part)

	# Draw text - lower
	img_lower = Image.new("RGBA", (lower_width+leftmargin, _round(height/2)), alpha)
	for i, (pos, stroke, color) in enumerate(zip(lower_data[0], lower_data[1], lower_data[2])):
		img_lower_part = Image.new("RGBA", (lower_width+leftmargin, _round(height/2)+ upmargin), alpha)
		img_lower_part.paste(lower_base[color], (0, 0), mask=mask_img_lower[i])
		img_lower.alpha_composite(img_lower_part)

	#img_upper.save("./uptemp.png")
	#img_lower.save("./downtemp.png")
	# tilt image
	tiltres = list()
	angle = 24
	for img in [img_upper, img_lower]:
		dist = img.height * tan(radians(angle))
		data = (1, tan(radians(angle)), -dist, 0, 1, 0)
		imgc = img.crop((0, 0, img.width+dist, img.height))
		imgt = imgc.transform(imgc.size, Image.AFFINE, data, Image.BILINEAR)
		tiltres.append(imgt)

	# finish
	previmg = Image.new("RGBA", (max([upper_width, lower_width]) + leftmargin + subset + 100, height + upmargin + 30), (255,255,255,0))
	# previmg.paste(tiltres[0], (0, 0))
	# previmg.paste(tiltres[1], (subset, _round(height/2)))
	previmg.alpha_composite(tiltres[0], (0, 50), (0, 0))
	if upper_width> lower_width+subset:
		previmg.alpha_composite(tiltres[1], (upper_width + subset - lower_width, _round(height/2) + 15), (0, 0))
	else:
		previmg.alpha_composite(tiltres[1], (subset, _round(height/2) + 15), (0, 0))
	#previmg.save("./test1.png")
	croprange = previmg.getbbox()
	img = previmg.crop(croprange)
	final_image = Image.new("RGB", (img.size[0] + 100, img.size[1] + 30), bg)
	final_image.paste(img, (50, 15))

	return final_image


#genImage(word_a="怎么还没到五一", word_b="我不想上班了").save("./temp.png")
