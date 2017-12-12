#!/usr/bin/env python

from PIL import Image
import sys


def convert(img):
	return img.convert('RGB')


def load_raw_img(f_in, size):
	f = open(f_in, 'rb')
	return convert(Image.frombuffer('RGB', size, f.read(), 'raw', 'RGB', 0, 1))


def main(argv):
	try:
		i = argv[1]
	except:
		print "Usage: "
		exit(1)

	s = i.split("_")[1].split(".")[0]
	w = int(s.split("x")[0])
	h = int(s.split("x")[1])

	img = load_raw_img(i, (w, h))
	img.show()


main(sys.argv)
