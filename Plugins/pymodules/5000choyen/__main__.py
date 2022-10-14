import sys
from .generator import genImage
from PIL import Image
import argparse

parser = argparse.ArgumentParser()

parser.add_argument("line1")
parser.add_argument("line2")
parser.add_argument("--rupper", action="store_true")
parser.add_argument("--rlower", action="store_true")

args = parser.parse_args()

image = genImage(
	word_a=args.line1, 
	word_b=args.line2, 
	rainbow_upper=args.rupper,
	rainbow_lower=args.rlower
)

image.resize((int(image.size[0] * 0.75), int(image.size[1] * 0.75)), Image.Resampling.LANCZOS)

image.save(sys.stdout.buffer, "jpeg")