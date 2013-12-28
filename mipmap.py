from PIL import Image

def mipmap(d):
    im = Image.open('texture.png')
    mask = '\xff\x00\xff\xff'
    tiles = {}
    size = 16 / d
    for i in range(16):
        for j in range(16):
            x = j * 16
            y = i * 16
            tile = im.crop((x, y, x + 16, y + 16))
            has_mask = mask in tile.tostring()
            mode = Image.NEAREST if has_mask else Image.ANTIALIAS
            tile = tile.resize((size, size), mode)
            tiles[(i, j)] = tile
    im = Image.new('RGBA', (size * 16, size * 16))
    for i in range(16):
        for j in range(16):
            x = j * size
            y = i * size
            tile = tiles[(i, j)]
            im.paste(tile, (x, y))
    im.save('texture-%d.png' % d)

for d in [2, 4, 8, 16]:
    mipmap(d)
