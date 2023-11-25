#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "image8bit.h"

// Function to apply blur operation to an image
void applyBlur(const char *inputFileName, int dx, int dy) {
    Image img = ImageLoad(inputFileName);
    if (img == NULL) {
        fprintf(stderr, "Error loading image: %s\n", inputFileName);
        return;
    }

    // Apply blur operation
    ImageBlur(img, dx, dy);

    // Generate output file name
    char outputFileName[256];
    snprintf(outputFileName, sizeof(outputFileName), "blurred_%s", inputFileName);

    // Save the blurred image
    if (ImageSave(img, outputFileName) == 0) {
        fprintf(stderr, "Error saving blurred image: %s\n", outputFileName);
    }

    // Destroy the image
    ImageDestroy(&img);
}

int main(int argc, char *argv[]) {
    // Check for the correct number of command-line arguments
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <input_file_list> <blur_dx> <blur_dy>\n", argv[0]);
        return 1;
    }

    // Retrieve command-line arguments
    const char *inputFileList = argv[1];
    int blurDx = atoi(argv[2]);
    int blurDy = atoi(argv[3]);

    FILE *file = fopen(inputFileList, "r");
    if (file == NULL) {
        fprintf(stderr, "Error opening image list file: %s\n", inputFileList);
        return 1;
    }

    char line[256];
    while (fgets(line, sizeof(line), file) != NULL) {
        // Remove trailing newline character
        line[strcspn(line, "\n")] = 0;

        // Assume each line in the file contains an image file name
        applyBlur(line, blurDx, blurDy);
    }

    fclose(file);
    return 0;
}
