#include "c_img.h"
#include "seamcarving.h"
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <stdint.h>

//step one: energy calculation, calculate the energy of a pixel (more energy = less likely to be included as part of seam
//step two: find a vertical seam of pixels with the lowest energy using dynamic programming
//step three: remove the seam from the image

// PART ONE: DUAL-GRADED ENERGY FUNCTION
uint8_t get_wrapped_pixel(struct rgb_img *im, int y, int x, int col) {
    // if y is negative, then go to the bottom of the image
    //if y is greater than height, go to the top 
    if (y < 0) {
        y = im->height - 1;
    } else if (y >= im->height) 
        y = 0;
    //if x is negative, left of image go to right
    // if x is greater than the height wrap around to the left
    if (x < 0) {
        x = im->width - 1;
    } else if (x >= im->width) {
        x = 0;
    }
    return get_pixel(im, y, x, col);
}

uint8_t pixel_energy(struct rgb_img *im, int y, int x) {
    int Rx = get_wrapped_pixel(im, y, x + 1, 0) - get_wrapped_pixel(im, y, x - 1, 0);
    int Gx = get_wrapped_pixel(im, y, x + 1, 1) - get_wrapped_pixel(im, y, x - 1, 1);
    int Bx = get_wrapped_pixel(im, y, x + 1, 2) - get_wrapped_pixel(im, y, x - 1, 2);

    int Ry = get_wrapped_pixel(im, y + 1, x, 0) - get_wrapped_pixel(im, y - 1, x, 0);
    int Gy = get_wrapped_pixel(im, y + 1, x, 1) - get_wrapped_pixel(im, y - 1, x, 1);
    int By = get_wrapped_pixel(im, y + 1, x, 2) - get_wrapped_pixel(im, y - 1, x, 2);

    int delta_x_squared = Rx * Rx + Gx * Gx + Bx * Bx;
    int delta_y_squared = Ry * Ry + Gy * Gy + By * By;

    return (uint8_t)(sqrt(delta_x_squared + delta_y_squared) / 10);
}

void calc_energy(struct rgb_img *im, struct rgb_img **grad) {
    // Create new image to store the energy 
    create_img(grad, im->height, im->width);

    // Iterate over each pixel in the input image
    for (size_t y = 0; y < im->height; y++) {
        for (size_t x = 0; x < im->width; x++) {
            // Calculate energy for the current pixel
            uint8_t energy = pixel_energy(im, y, x);
            set_pixel(*grad, y, x, energy, energy, energy);
        }
    }
}

//PART TWO: COST ARRAY 
// keep in mind the input into this function is the gradient image so it has to be calc_energy before this function can be called 
void dynamic_seam(struct rgb_img *grad, double **best_arr){
    // contains the minimum cost of a seam from top of grad to point(i, j)
    *best_arr = (double *)malloc(grad->height * grad->width * sizeof(double)); // create an array of size height * width to store minimum cost to reach each pixel
    
    // the first row should just be the same as the first row of the gradient image
    //initializing the gradient image to the first row of the best array
    for (int x = 0; x < grad->width; x++) {
        (*best_arr)[x] = get_pixel(grad, 0, x, 0);
    }

    // iterate over the remaining rows to calculate the minimum cost of a seam to reach each pixel

    //iterating through the height of the image 
    for (int y =1; y <grad->height; y++) {
        //iterating from left to rest of the image 
        for (int x = 0; x < grad->width; x++) {
            //initialize the min value to the pixel above the current pixel
            double min_val = (*best_arr)[((y - 1) * grad->width) + x]; // ((y - 1) * grad->width) this gets you to the right row, + x gets you to the right column 
            
            // check both top left and top right pixel to see if they are less than current min_val, if so the reset the min_val
            if (x > 0 && (*best_arr)[((y - 1) * grad->width) + x - 1] < min_val) { // x > 0 makes sure not on left edge 
                min_val = (*best_arr)[((y - 1) * grad->width) + x - 1];
            }
            if (x < grad->width - 1 && (*best_arr)[((y - 1) * grad->width) + x + 1] < min_val) { // x < grad->width - 1 makes sure not on right edge
                min_val = (*best_arr)[((y - 1) * grad->width) + x + 1];
            }
            // now the min for that specific pixel has been computed, so add it to the current pixel to track it 
            (*best_arr)[(y * grad->width) + x] = min_val + get_pixel(grad, y, x, 0);                                                               
        }
    }

}

//PART THREE: RECOVER THE SEAM 
void recover_path(double *best, int height, int width, int **path) {
    // Allocate memory for the path
    *path = (int *)malloc(height * sizeof(int));

    // find the min value in the bottom row 
    double min_val = best[(height - 1) * width]; // get the first value in the last row
    int min_idx = 0;
    for (int x = 1; x < width; x++) {
        if (best[(height - 1) * width + x] < min_val) {
            min_val = best[(height - 1) * width + x];
            min_idx = x;
        }
    }
    (*path)[height - 1] = min_idx; //set the last element in the path array to the min_idx identified for the last row

    // Backtrack to find the path
    //height - 2 because we already set the last element in the path array (so you start at the second last row)
    for (int y = height - 2; y >= 0; y--) {
        int x = (*path)[y + 1]; // get the x value of the pixel in the row below
        double min_val = best[y * width + x]; // set the temporary min value to the value of the pixel right above 
        int min_idx = x; // set the temporary min index to the x value of the pixel right above

        // check to make sure not on the left edge and if the pixel to the left is less than the current min_val, then reset the min_val and min_idx
        if (x > 0 && best[y * width + x - 1] < min_val) {
            min_val = best[y * width + x - 1];
            min_idx = x - 1;
        }
        // check to make sure not on the right edge and if the pixel to the right is less than the current min_val, then reset the min_val and min_idx
        if (x < width - 1 && best[y * width + x + 1] < min_val) {
            min_val = best[y * width + x + 1];
            min_idx = x + 1;
        }
        (*path)[y] = min_idx;
    }
}

// PART FOUR: REMOVE THE SEAM
void remove_seam(struct rgb_img *src, struct rgb_img **dest, int *path) {
    // create the new image with a smaller width 
    create_img(dest, src->height, src->width - 1);

    // iterating based on original x and y height and width, but copying to new image based on new height and widths
    // copy pixels from dest to new, and if the pixel is part of the path don't include it 
    for (int y = 0; y < src->height; y++) {
        int dest_x = 0; // keep track of the x value in the new image
        // iterate the width of the new image
        for (int x = 0; x < src->width; x++) {
            // check if the index is not on the identified path, if not then copy the pixel to the new image
            if (x != path[y]) {
                set_pixel(*dest, y, dest_x, get_pixel(src, y, x, 0), get_pixel(src, y, x, 1), get_pixel(src, y, x, 2));
                dest_x++; // move to the next pixel of the new image
            }
        }
    }

}


int main() {
    struct rgb_img *im;
    struct rgb_img *cur_im;
    struct rgb_img *grad;
    double *best;
    int *path;

    read_in_img(&im, "HJoceanSmall.bin");
    
    for(int i = 0; i < 5; i++){
        printf("i = %d\n", i);
        calc_energy(im,  &grad);
        dynamic_seam(grad, &best);
        recover_path(best, grad->height, grad->width, &path);
        remove_seam(im, &cur_im, path);

        char filename[200];
        sprintf(filename, "img%d.bin", i);
        write_img(cur_im, filename);


        destroy_image(im);
        destroy_image(grad);
        free(best);
        free(path);
        im = cur_im;
    }
    destroy_image(im);

    return 0;
}
