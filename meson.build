project('lnl', 'c',
  version: '0.0.2',
  meson_version: '>=1.1.0',
  default_options: [
    'c_std=c99',
    'warning_level=2',
  ]
)

nasm = find_program('nasm')
# qemu = find_program('qemu-system-i386') TODO

c_flags = [
  '-ffreestanding',
  '-nostdlib',
  '-fno-pie',
  '-no-pie',
  '-fno-stack-protector',
  '-fno-builtin',
  '-m32',
  '-mno-sse',
  '-mno-sse2',
  '-mno-mmx',
  '-mno-3dnow',
  '-Wall',
  '-Wextra',
]

link_flags = [
  '-m32',
  '-nostdlib',
  '-no-pie',
  '-T', meson.current_source_dir() / 'src/linker.ld',
]

# Build bootloader
boot_bin = custom_target('boot.bin',
  input: 'src/boot.asm',
  output: 'boot.bin',
  command: [nasm, '-f', 'bin', '@INPUT@', '-o', '@OUTPUT@'],
)

# Build kernel entry (assembly)
kernel_entry_o = custom_target('kernel_entry.o',
  input: 'src/kernel_entry.asm',
  output: 'kernel_entry.o',
  command: [nasm, '-f', 'elf32', '@INPUT@', '-o', '@OUTPUT@'],
)

# Build interrupts (assembly)
interrupts_o = custom_target('interrupts.o',
  input: 'src/interrupts.asm',
  output: 'interrupts.o',
  command: [nasm, '-f', 'elf32', '@INPUT@', '-o', '@OUTPUT@'],
)

sources = files(
  'src/kernel.c',
  'src/keyboard.c',
  'src/cursor.c',
  'src/timer.c',
  'src/vga.c',
  'src/vesa.c',
  'src/font.c',
  'src/framebuffer.c',
  'src/LNLISP/lnlisp.c',
  'src/LNLISP/sexparser.c',
)

# Include directories
inc = include_directories('src')

# Build kernel binary
kernel_elf = executable('kernel.elf',
  [sources],
  objects: [kernel_entry_o, interrupts_o],
  c_args: c_flags,
  link_args: link_flags,
  link_depends: files('src/linker.ld'),
  include_directories: inc,
)

# Extract flat binary from ELF
kernel_bin = custom_target('kernel.bin',
  input: kernel_elf,
  output: 'kernel.bin',
  command: [
    'objcopy',
    '-O', 'binary',
    '@INPUT@',
    '@OUTPUT@'
  ],
  build_by_default: true,
)

# Create OS image (bootloader + kernel)
os_img = custom_target('os.img',
  input: [boot_bin, kernel_bin],
  output: 'os.img',
  command: [
    'sh', '-c',
    'cat "$1" "$2" > "$3" && truncate -s 1440K "$3"',
    'sh',        # $0
    '@INPUT0@',  # $1
    '@INPUT1@',  # $2
    '@OUTPUT@',  # $3
  ],
  build_by_default: true,
)

run_target('run',
  command: [
    'qemu-system-i386',
    '-drive', 'format=raw,file=' + meson.current_build_dir() / 'os.img',
    '-display', 'sdl,gl=off',
  ],
  depends: os_img,
)

run_target('debug',
  command: [
    'qemu-system-i386',
    '-drive', 'format=raw,file=' + meson.current_build_dir() / 'os.img',
    '-display', 'sdl,gl=off',
    '-monitor', 'stdio',
    '-d', 'int,cpu_reset,guest_errors'
  ],
  depends: os_img,
)
