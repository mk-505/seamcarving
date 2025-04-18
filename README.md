# ✂️ Seam Carving — Content-Aware Image Resizing in C

This project implements **seam carving**, a content-aware image resizing algorithm that intelligently removes vertical seams of least importance from an image. By preserving areas of high energy (visual significance), this technique avoids distorting key visual elements — unlike standard resizing methods that scale everything uniformly.

---

## 🧠 What is Seam Carving?

Seam carving resizes images by iteratively removing **seams** — paths of connected pixels from top to bottom — with the **lowest cumulative energy**. These seams represent the least important visual information.

This project supports:
- Dual-gradient energy calculation (using color gradients in both x and y directions)
- Dynamic programming for optimal seam selection
- Seam backtracking and removal
- Outputting updated images after each iteration

---

## 🗂️ Directory Overview

```
📁 project/
├── seamcarving.c        # Main algorithm logic
├── seamcarving.h        # Function declarations
├── c_img.c / c_img.h    # Binary image read/write and helper functions
├── HJoceanSmall.bin     # Input image (custom binary RGB format)
├── img0.bin-img4.bin    # Output images after seam removal
├── 3x4.bin, 6x5.bin     # Additional test images
└── test                 # Compiled executable
```

---

## 🔧 How It Works

### 🌀 1. Energy Function (Dual Gradient)
Each pixel's "energy" is calculated using the gradient of RGB values in both the x and y directions — sharp transitions mean higher energy.

```c
energy = sqrt((Rx² + Gx² + Bx²) + (Ry² + Gy² + By²)) / 10
```

Pixels with high energy are less likely to be removed.

---

### 🧮 2. Dynamic Programming for Seam Cost
A 2D `best_arr` array stores the minimum cumulative energy to reach each pixel from the top. For every pixel, we only consider its top-left, top, and top-right neighbors.

---

### 🔙 3. Seam Backtracking
Starting from the pixel with the lowest cost in the bottom row, we trace the seam path upwards, always selecting the minimum of the three possible parent pixels.

---

### 🧼 4. Seam Removal
One pixel per row is removed along the seam path, forming a new image with width reduced by 1. This process is repeated for 5 iterations, creating 5 resized images.

---

## 💻 How to Compile and Run

```bash
gcc seamcarving.c c_img.c -o test -lm
./test
```

- Input: `HJoceanSmall.bin`
- Output: `img0.bin`, `img1.bin`, ..., `img4.bin`

Each output image will be 1 pixel narrower than the previous.

---

## 📷 Input/Output Format

- Custom `.bin` format (RGB images)
- Functions in `c_img.c` handle binary I/O
- Images are stored as 3 bytes per pixel: R, G, B

---

## 📌 Use Cases

- Smart image resizing for responsive web design
- Content-aware cropping
- Non-uniform scaling in photo editors

---

## 🧪 Sample Output Progression

| Iteration | Output File  | Description                      |
|-----------|--------------|----------------------------------|
| 0         | `img0.bin`   | Original image with 1 seam removed |
| 1         | `img1.bin`   | 2 seams removed                  |
| 4         | `img4.bin`   | Final image, 5 seams removed     |

---

## 🧑‍💻 Author

Made with <3 by **Manroop Kalsi**  
