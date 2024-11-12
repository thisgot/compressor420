from PIL import Image

def bytes_to_mb(bytes_value):
    return bytes_value / (1024 * 1024)

def compress420(pixels, width, height):
    y_channel = []
    cr_channel = []
    cb_channel = []
    
    for r, g, b in pixels:
        y = int(0.299 * r + 0.587 * g + 0.114 * b)
        cb = int((b - y) * 0.564 + 128)
        cr = int((r - y) * 0.713 + 128)
        y_channel.append(y)
        cb_channel.append(cb)
        cr_channel.append(cr)

    cr_420 = []
    cb_420 = []

    for i in range(0, height, 2):
        for j in range(0, width, 2):
            #indices do blcoo 2x2
            index1 = i * width + j
            index2 = i * width + (j + 1)
            index3 = (i + 1) * width + j
            index4 = (i + 1) * width + (j + 1)
            
            avg_cr = (
                cr_channel[index1] +
                cr_channel[index2] +
                cr_channel[index3] +
                cr_channel[index4]
            ) // 4
            
            avg_cb = (
                cb_channel[index1] +
                cb_channel[index2] +
                cb_channel[index3] +
                cb_channel[index4]
            ) // 4

            cr_420.append(avg_cr)
            cb_420.append(avg_cb)

    return y_channel, cr_420, cb_420

def decompress420(y_channel, cr_420, cb_420, width, height):
    pixels = [0] * (width * height)
    chroma_index = 0

    for i in range(0, height, 2):
        for j in range(0, width, 2):
            cr = cr_420[chroma_index]
            cb = cb_420[chroma_index]
            chroma_index += 1
            
            for dy in range(2):
                for dx in range(2):
                    y_index = (i + dy) * width + (j + dx)
                    y = y_channel[y_index]
                    
                    r = int(y + 1.402 * (cr - 128))
                    g = int(y - 0.344136 * (cb - 128) - 0.714136 * (cr - 128))
                    b = int(y + 1.772 * (cb - 128))
                    r = max(0, min(255, r))
                    g = max(0, min(255, g))
                    b = max(0, min(255, b))
                    pixels[y_index]=(r, g, b)

    return pixels

def pixels_to_image(pixels, width, height):
    image = Image.new("RGB", (width, height))
    image.putdata(pixels)
    return image

filepath = input("nome da imagem:");

image = Image.open(filepath)
width, height = image.size

#converte para matriz de pixels, junta cada 3 itens (cores) em 1 vetor
pixels = list(image.getdata())
print(f"Total raw size: {bytes_to_mb(len(pixels*3)):.2f} MB")

#converte de vetor rgb para YCrCb
y_channel, cr_420, cb_420 = compress420(pixels, width, height)
print(f"Total compressed raw size: {bytes_to_mb(len(y_channel)+len(cr_420)+len(cb_420)):.2f} MB")

#reconverte de YCrCb para vetor de pixels rgb
decompressed_pixels = decompress420(y_channel, cr_420, cb_420, width, height)

#salva a imagem que foi comprimida e descomprimida
image = pixels_to_image(decompressed_pixels, width, height)
image.save("output_py.jpg")
input("...");
