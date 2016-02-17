import numpy
import sh
import numpy.linalg

# Read the data from the files
# This was created setting DUMP_FRAME = 1 in iosvideo.mm and resize_ref.c and forcing the resize_method to NONE or DIV2
bgra = open("bgra_data2.txt").read().strip("\n").split(" ")
y = open("yuv_data2.txt").read().split("\n")[0].strip(" ").split(" ")
crcb = open("yuv_data2.txt").read().split("\n")[2].strip(" ").split(" ")

bgra = map(lambda x : float(x), bgra)
y = map(lambda x : float(x), y)

crcb = map(lambda x : float(x), crcb)
cr = crcb[::2]
cb = crcb[1::2]

bgra = numpy.array(bgra)
y = numpy.array(y)

def optimize_(reference, inputData, center = [0.5,0.5,0.5], steps = 40, scale = 1.0, mask = [1.0, 1.0, 1.0]):
    """reference : the observed data
       inputData : the input matrix to be multiplied by a conversion vector
       center    : the center of the zone to look for the best conversion vector
       steps     : the number of steps
       scale     : the scale factor (search smaller zone around center)
       mask      : the mask to be applied to the conversion vector"""

    min_norm = 10000000
    R = steps / 2

    for i in range(1): #-R, R + 1):
        for j in range(-R, R + 1):
            for k in range(-R, R + 1):
#                conv = [i * scale / float(R) + center[0], j * scale / float(R) + center[1], k * scale / float(R) + center[2]]
                conv = [1.0, j * scale / float(R) + center[1], k * scale / float(R) + center[2]]

                for i in range(len(mask)):
                    conv[i] *= mask[i]

                result = inputData[::3] * conv[0] + inputData[1::3] * conv[1] + inputData[2::3] * conv[2]
                l2norm = numpy.linalg.norm(result - reference)

                if l2norm < min_norm:
                    min_norm = l2norm
                    final_conv = conv
#                    print final_conv, l2norm
                    best_result = result
    return final_conv, best_result, min_norm

# Search in a smaller and smaller zone using optimize_ function
def optimize(reference, inputData, mask):
    r_conv = [0.5, 0.5, 0.5]
    scale = 2.0
    for i in range(5):
        r_conv, best_result, min_norm = optimize_(reference, inputData, center = r_conv, scale = scale, mask = mask)
        scale *= 0.1
    return r_conv, best_result, min_norm

# Prepare the data
inputData = numpy.zeros(1920 * 3)
inputData[::3] = y
shift = numpy.ones(1920 / 2) * 128.0
# Duplicate cr cb data as it is half resolution
inputData[1::6] = cr - shift
inputData[4::6] = cr - shift
inputData[2::6] = cb - shift
inputData[5::6] = cb - shift

# Search mask because we already know which factor should be nulls (see http://en.wikipedia.org/wiki/YCbCr#ITU-R_BT.601_conversion )
masks = [[1,0,1],[1,1,1],[1,1,0]]

for i in range(0, 3):
    reference = bgra[2 - i::4]
    r_conv, best_result, min_norm = optimize(reference, inputData, masks[i])

    print min_norm
    # Display the best conversion vector
    print numpy.array(numpy.multiply(r_conv, 16384), dtype = 'i')

    # Write the plot with the original and computed data : should be around the y=x line
    # You can plot it with gnuplot using : plot 'plot_X.txt'
    f  = open("plot_%d.txt" % i, "wb")
    for i in range(1920):
        f.write("%s %s\n" % (reference[i], best_result[i]))

    f.close()
