"""Extract a GameCube disc image (.iso/.gcm or .ciso) to <out>/sys and <out>/files, the layout the game reads (the same
output as the launcher's Extract). Python 3, standard library only; used by tools/build-scooby-linux.sh.

Usage: python3 gc_extract.py <image> <out dir>
"""
import os
import struct
import sys

WANTED_ID = b'G5DE78'  # Scooby-Doo! Unmasked (USA)


class Image:
    """Reads a plain image, or a CISO (0x8000-byte header: "CISO", block size LE, one byte per block, 1 = stored; then
    the stored blocks in order; blocks left out are all zero)."""

    def __init__(self, path):
        self.f = open(path, 'rb')
        head = self.f.read(0x8000)
        self.block = 0
        if head[:4] == b'CISO':
            self.block = struct.unpack('<I', head[4:8])[0]
            if self.block < 0x8000 or self.block % 0x8000:
                sys.exit('Unsupported CISO block size %#x.' % self.block)
            self.stored, k = [], 0
            for flag in head[8:0x8000]:
                self.stored.append(k if flag == 1 else -1)
                k += flag == 1

    def read(self, off, n):
        if not self.block:
            self.f.seek(off)
            data = self.f.read(n)
            return data + bytes(n - len(data))
        out = bytearray()
        while n > 0:
            i, o = divmod(off, self.block)
            take = min(n, self.block - o)
            idx = self.stored[i] if i < len(self.stored) else -1
            if idx < 0:
                out += bytes(take)
            else:
                self.f.seek(0x8000 + idx * self.block + o)
                data = self.f.read(take)
                out += data + bytes(take - len(data))
            off += take
            n -= take
        return bytes(out)


def main():
    if len(sys.argv) != 3:
        sys.exit(__doc__)
    image, out = Image(sys.argv[1]), sys.argv[2]
    read = image.read
    hdr = read(0, 0x440)
    if struct.unpack('>I', hdr[0x1C:0x20])[0] != 0xC2339F3D:
        sys.exit('Not a GameCube disc image (use a plain .iso/.gcm or a .ciso; .rvz and .nkit do not work).')
    if hdr[:6] != WANTED_ID:
        sys.exit('That is %s, not Scooby-Doo! Unmasked for the USA (%s).' % (hdr[:6].decode('latin1'), WANTED_ID.decode()))
    dol_off, fst_off, fst_size = struct.unpack('>III', hdr[0x420:0x42C])
    sysd, files = os.path.join(out, 'sys'), os.path.join(out, 'files')
    os.makedirs(sysd, exist_ok=True)
    os.makedirs(files, exist_ok=True)

    def write(name, data):
        with open(os.path.join(sysd, name), 'wb') as o:
            o.write(data)

    write('boot.bin', hdr)
    write('bi2.bin', read(0x440, 0x2000))
    ap = read(0x2440, 0x20)
    write('apploader.img', read(0x2440, 0x20 + struct.unpack('>I', ap[0x14:0x18])[0] + struct.unpack('>I', ap[0x18:0x1C])[0]))
    dh = read(dol_off, 0x100)
    size = max(struct.unpack('>I', dh[i * 4:i * 4 + 4])[0] + struct.unpack('>I', dh[0x90 + i * 4:0x94 + i * 4])[0]
               for i in range(18))
    write('main.dol', read(dol_off, size))
    fst = read(fst_off, fst_size)
    write('fst.bin', fst)
    count = struct.unpack('>I', fst[8:12])[0]
    strtab = count * 12

    def name(i):
        s = strtab + (struct.unpack('>I', fst[i * 12:i * 12 + 4])[0] & 0xFFFFFF)
        return fst[s:fst.index(b'\0', s)].decode('latin1')

    stack, n = [(count, files)], 0
    for i in range(1, count):
        while i >= stack[-1][0]:
            stack.pop()
        path = os.path.join(stack[-1][1], name(i))
        flags, off, sz = struct.unpack('>III', fst[i * 12:i * 12 + 12])
        if flags >> 24:
            os.makedirs(path, exist_ok=True)
            stack.append((sz, path))
            continue
        with open(path, 'wb') as o:
            p = 0
            while p < sz:
                chunk = read(off + p, min(1 << 22, sz - p))
                o.write(chunk)
                p += len(chunk)
        n += 1
    print('extracted %d files' % n)


if __name__ == '__main__':
    main()
