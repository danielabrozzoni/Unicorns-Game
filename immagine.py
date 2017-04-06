import os, sys
from PIL import Image

im = Image.open("/home/daniela/Scaricati/Frattali/ea9.jpg")
pix = im.load()
with open("immagine2.txt", "w") as out_file:
	for x in range(0,548):
		for y in range(0,27254):
			 pixel=pix[x,y]
      			 out_file.write(str(pixel)+'\n')