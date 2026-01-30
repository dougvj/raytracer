# Raytracer

A multithreaded CPU ray tracer written in C with support for reflections,
diffuse lighting, path tracing with Monte Carlo hemisphere sampling, and real-time
SDL2 preview.

## Features

- Multithreaded rendering with configurable worker threads
- Sphere and plane primitives with checkerboard patterns
- Reflections with configurable bounce depth
- Diffuse lighting via direct illumination or Monte Carlo hemisphere sampling
- Real-time SDL2 window preview during rendering
- BMP frame output with multi-frame animation support
- GCC auto-vectorization for performance
- Intel Xeon Phi (MIC) cross-compilation support

## Building

Requires GCC, SDL2, and OpenGL development libraries.

```
# Arch Linux
sudo pacman -S sdl2

# Debian/Ubuntu
sudo apt install libsdl2-dev libgl-dev

# Build
make
```

## Usage

```
./raytrace --resolution 800x600 [options]
```

### Options

| Option | Description |
|---|---|
| `--resolution <W>x<H>` | Output resolution (required) |
| `--num-threads <N>` | Worker thread count (default: machine cores) |
| `--max-bounces <N>` | Maximum ray bounce depth (default: 8) |
| `--diffuse-samples <N>` | Hemisphere samples per hit for path tracing (0 = direct only) |
| `--num-spheres <N>` | Number of random spheres in the scene |
| `--num-frames <N>` | Number of frames to render |
| `--distance-per-frame <D>` | Camera distance step per frame |
| `--enable-window` | Show real-time SDL2 preview |
| `--window-resolution <W>x<H>` | Preview window size |
| `--output-directory <dir>` | Output directory for BMP frames |
| `--overwrite` | Overwrite existing frame files |
| `--random-seed <N>` | Set random seed for reproducibility |

### Examples

```
# Single frame with preview window
./raytrace --resolution 1920x1080 --enable-window

# 100-frame animation with path tracing
./raytrace --resolution 800x600 --num-frames 100 --diffuse-samples 16 \
    --output-directory render_output --overwrite
```

## License

Public domain.
