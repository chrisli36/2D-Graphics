from random import randint
import random as rand
import math

for i in range(500):
    meatballColors = (("0.71372549019","0.34901960784","0.26666666666"), 
                      ("0.71372549019","0.34901960784","0.26666666666"), 
                      ("0.71372549019","0.34901960784","0.26666666666"), 
                      ("0.71372549019","0.34901960784","0.26666666666"), 
                      ("0.71372549019","0.34901960784","0.26666666666"), 
                      ("0.7725490196","0.47843137254","0.41176470588"), 
                      ("0.7725490196","0.47843137254","0.41176470588"), 
                      ("0.82745098039","0.60784313725","0.56078431372"), 
                      ("0.8862745098","0.74117647058","0.70588235294"))
    
    x, y = rand.uniform(0, 256), rand.uniform(0, 256)
    while (x - 128)**2 + (y - 128)**2 > 128**2:
        x, y = rand.uniform(0, 256), rand.uniform(0, 256)
    dist = math.sqrt((x - 128)**2 + (y - 128)**2)
    r1 = rand.uniform(0, 128 - dist)
    r2 = rand.uniform(0, 128 - dist)

    l, t, r, b = x - r1, y - r2, x + r1, y + r2
    if i % 2:
        red, green, blue = rand.choice(meatballColors)
        alpha = rand.uniform(0, 0.5 * (1 - dist / 256))
    else:
        red, green, blue, alpha = rand.uniform(0, 1), rand.uniform(0, 1), rand.uniform(0, 1), rand.uniform(0, 0.2)
    print("\tcanvas->fillRect(GRect {{ {}f, {}f, {}f, {}f }}, GColor{{ {}f, {}f, {}f, {}f }});".format(l, t, r, b, red, green, blue, alpha))