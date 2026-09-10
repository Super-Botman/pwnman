from pwn import pack, unpack, xor
from random import randbytes

def crc32(message: bytes) -> int:
    crc = 0xFFFFFFFF
    for byte in message:
        crc ^= byte
        for _ in range(8):
            mask = -(crc & 1)          # 0 or 0xFFFFFFFF (as -1, works fine in Python)
            crc = (crc >> 1) ^ (0xEDB88320 & mask)
    return ~crc & 0xFFFFFFFF           # ~ in Python gives negative; mask back to 32-bit

file = open("./test.db", "wb")
header = b"B0T\x00"
header += pack(0, 32+64) # padding

def eentry(username, password):
    usize = len(username)
    psize = len(password)
    entry = pack(usize, 16) + pack(psize, 16)
    entry += username + randbytes(64-usize)
    entry += password + randbytes(64-psize)
    sig = crc32(entry)
    return pack(sig, 32) + entry

def entry(title, username, password, key):
    tsize = len(title)
    header = pack(tsize, 16) + pack(0, 48) # 48bits reserved for later use
    header += title.ljust(64, b'\x00')

    dentry = eentry(username, password)
    return header + xor(dentry, key)

master = b"superpass"
entries = entry(b"proton", b"botman", b"pass1", master)
for i in range(100):
    entries += entry(f"google{i}".encode(), b"botman", b"pass2", master)
content = header+entries
file.write(content)
