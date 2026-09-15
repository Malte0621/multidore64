#!/usr/bin/env python3
"""
MultiDore 64 D64 Disk Image Creator.

Bundles a main PRG and extra files (sound files, data files, secondary PRGs)
into a standard Commodore 64 1541 D64 disk image.

Usage:
    python3 tools/make_d64.py <output.d64> <disk_name> <file1[:cbm_name]> [file2[:cbm_name]] ...

Example:
    python3 tools/make_d64.py dist/main.d64 multidore64 dist/main.prg:main src/song.bin:song.bin
"""

import sys
import os
import shutil
import subprocess

SECTORS_PER_TRACK = [
    0,
    21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, 21, # 1-17
    19, 19, 19, 19, 19, 19, 19,                                         # 18-24
    18, 18, 18, 18, 18, 18,                                             # 25-30
    17, 17, 17, 17, 17                                                  # 31-35
]

def track_offset(track, sector):
    off = 0
    for t in range(1, track):
        off += SECTORS_PER_TRACK[t] * 256
    return off + sector * 256

class D64Builder:
    def __init__(self, disk_name="MULTIDORE64", disk_id="64"):
        self.data = bytearray(174848)
        self.disk_name = disk_name[:16]
        self.disk_id = disk_id[:2]
        self._init_bam()

    def _init_bam(self):
        bam_off = track_offset(18, 0)
        self.data[bam_off + 0] = 18
        self.data[bam_off + 1] = 1
        self.data[bam_off + 2] = 0x41  # DOS version 'A'

        for t in range(1, 36):
            num_sec = SECTORS_PER_TRACK[t]
            t_off = bam_off + 4 + (t - 1) * 4
            if t == 18:
                self.data[t_off + 0] = 0
                self.data[t_off + 1] = 0
                self.data[t_off + 2] = 0
                self.data[t_off + 3] = 0
            else:
                self.data[t_off + 0] = num_sec
                mask = (1 << num_sec) - 1
                self.data[t_off + 1] = mask & 0xFF
                self.data[t_off + 2] = (mask >> 8) & 0xFF
                self.data[t_off + 3] = (mask >> 16) & 0xFF

        name_bytes = self.disk_name.upper().encode('ascii', errors='replace')[:16].ljust(16, b'\xa0')
        self.data[bam_off + 144 : bam_off + 160] = name_bytes
        self.data[bam_off + 160 : bam_off + 162] = b'\xa0\xa0'
        id_bytes = self.disk_id.upper().encode('ascii', errors='replace')[:2].ljust(2, b'\xa0')
        self.data[bam_off + 162 : bam_off + 164] = id_bytes
        self.data[bam_off + 164] = 0xA0
        self.data[bam_off + 165 : bam_off + 167] = b'2A'
        self.data[bam_off + 167 : bam_off + 171] = b'\xa0\xa0\xa0\xa0'

        # Directory track 18 sector 1
        dir_off = track_offset(18, 1)
        self.data[dir_off + 0] = 0x00
        self.data[dir_off + 1] = 0xFF

    def _allocate_sector(self, preferred_track=1):
        tracks_order = list(range(1, 18)) + list(range(19, 36))
        if preferred_track in tracks_order:
            idx = tracks_order.index(preferred_track)
            tracks_order = tracks_order[idx:] + tracks_order[:idx]

        bam_off = track_offset(18, 0)
        for t in tracks_order:
            t_off = bam_off + 4 + (t - 1) * 4
            free_cnt = self.data[t_off + 0]
            if free_cnt > 0:
                mask = self.data[t_off + 1] | (self.data[t_off + 2] << 8) | (self.data[t_off + 3] << 16)
                for s in range(SECTORS_PER_TRACK[t]):
                    if mask & (1 << s):
                        mask &= ~(1 << s)
                        self.data[t_off + 0] = free_cnt - 1
                        self.data[t_off + 1] = mask & 0xFF
                        self.data[t_off + 2] = (mask >> 8) & 0xFF
                        self.data[t_off + 3] = (mask >> 16) & 0xFF
                        return (t, s)
        raise RuntimeError("Disk image is full!")

    def add_file(self, cbm_name, content, file_type=0x82):
        chunks = []
        pos = 0
        while pos < len(content):
            chunks.append(content[pos : pos + 254])
            pos += 254
        if not chunks:
            chunks = [b'']

        sectors = []
        last_t = 1
        for _ in chunks:
            t, s = self._allocate_sector(last_t)
            sectors.append((t, s))
            last_t = t

        for i, chunk in enumerate(chunks):
            t, s = sectors[i]
            off = track_offset(t, s)
            if i + 1 < len(chunks):
                next_t, next_s = sectors[i + 1]
                self.data[off + 0] = next_t
                self.data[off + 1] = next_s
                self.data[off + 2 : off + 2 + len(chunk)] = chunk
            else:
                self.data[off + 0] = 0x00
                self.data[off + 1] = len(chunk) + 1
                self.data[off + 2 : off + 2 + len(chunk)] = chunk

        first_t, first_s = sectors[0]
        num_blocks = len(chunks)
        self._add_dir_entry(cbm_name, file_type, first_t, first_s, num_blocks)

    def _add_dir_entry(self, cbm_name, file_type, first_t, first_s, num_blocks):
        dir_t = 18
        dir_s = 1
        while True:
            dir_off = track_offset(dir_t, dir_s)
            for entry_idx in range(8):
                entry_off = dir_off + 2 + entry_idx * 32
                if self.data[entry_off + 0] == 0:  # empty slot
                    self.data[entry_off + 0] = file_type
                    self.data[entry_off + 1] = first_t
                    self.data[entry_off + 2] = first_s
                    name_bytes = cbm_name.upper().encode('ascii', errors='replace')[:16].ljust(16, b'\xa0')
                    self.data[entry_off + 3 : entry_off + 19] = name_bytes
                    self.data[entry_off + 28] = num_blocks & 0xFF
                    self.data[entry_off + 29] = (num_blocks >> 8) & 0xFF
                    return
            next_t = self.data[dir_off + 0]
            next_s = self.data[dir_off + 1]
            if next_t == 0:
                if dir_s + 1 < 19:
                    new_s = dir_s + 1
                    self.data[dir_off + 0] = 18
                    self.data[dir_off + 1] = new_s
                    dir_s = new_s
                    new_dir_off = track_offset(18, new_s)
                    self.data[new_dir_off + 0] = 0x00
                    self.data[new_dir_off + 1] = 0xFF
                else:
                    raise RuntimeError("Directory is full!")
            else:
                dir_t = next_t
                dir_s = next_s

def build_with_c1541(c1541_path, output_d64, disk_name, files):
    cmd = [c1541_path, "-format", f"{disk_name},64", "d64", output_d64]
    for host_path, cbm_name in files:
        cmd.extend(["-write", host_path, cbm_name])
    res = subprocess.run(cmd, capture_output=True, text=True, errors="replace")
    if res.returncode != 0:
        raise RuntimeError(f"c1541 failed: {res.stderr}")

def main():
    if len(sys.argv) < 4:
        print("Usage: make_d64.py <output.d64> <disk_name> <file1[:cbm_name]> [file2[:cbm_name]] ...")
        sys.exit(1)

    output_d64 = sys.argv[1]
    disk_name = sys.argv[2]
    raw_files = sys.argv[3:]

    files = []
    for item in raw_files:
        if ':' in item and not (len(item) >= 2 and item[1] == ':' and '\\' in item):
            parts = item.split(':', 1)
            host_path = parts[0]
            cbm_name = parts[1]
        else:
            host_path = item
            cbm_name = os.path.basename(item)
            if cbm_name.lower().endswith('.prg'):
                cbm_name = cbm_name[:-4]
        files.append((host_path, cbm_name))

    os.makedirs(os.path.dirname(os.path.abspath(output_d64)), exist_ok=True)

    # Check if c1541 is available
    c1541 = shutil.which("c1541") or shutil.which("c1541.exe")
    if c1541:
        try:
            build_with_c1541(c1541, output_d64, disk_name, files)
            print(f"Created {output_d64} using c1541 ({len(files)} files)")
            return
        except Exception as e:
            print(f"Notice: c1541 failed ({e}), falling back to internal builder...")

    # Pure Python fallback
    builder = D64Builder(disk_name=disk_name)
    for host_path, cbm_name in files:
        with open(host_path, "rb") as f:
            content = f.read()
        builder.add_file(cbm_name, content, file_type=0x82)

    with open(output_d64, "wb") as f:
        f.write(builder.data)
    print(f"Created {output_d64} using internal builder ({len(files)} files)")

if __name__ == "__main__":
    main()
