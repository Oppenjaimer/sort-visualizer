/*** INCLUDES ***/

#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <time.h>

#include "SDL.h"

/*** DATA ***/

enum algorithm {
    BUBBLE_SORT,
    QUICK_SORT
};

struct config {
    int width;
    int height;
    float scale;
    int delay;
    enum algorithm algorithm;
};

struct config C;

/*** UTILS ***/

/**
 * Generate a random number in the range [0, n), with n <= RAND_MAX.
*/
int randint(int n) {
    if ((n - 1) == RAND_MAX) return rand();

    int end = RAND_MAX / n;
    end *= n;

    int r;
    while ((r = rand()) >= end);
    return r % n;
}

/**
 * Print array in the form [1, 2, 3, 4, ...].
*/
void print_arr(int *arr) {
    printf("[");
    for (int i = 0; i < C.width; i++) {
        if (i == C.width - 1) printf("%d]\n", arr[i]);
        else printf("%d, ", arr[i]);
    }
}

/**
 * Swap arr[i] and arr[j].
*/
void swap(int *arr, int i, int j) {
    int tmp = arr[i];
    arr[i] = arr[j];
    arr[j] = tmp;
}

/*** SDL DISPLAY ***/

/**
 * Draw current state of the sorting process.
*/
void draw_state(int *arr, SDL_Renderer *renderer, int green, int red) {
    for (int i = 0; i < C.width; i++) {
        if (i == green) SDL_SetRenderDrawColor(renderer, 0, 255, 0, 255);
        else if (i == red) SDL_SetRenderDrawColor(renderer, 255, 0, 0, 255);
        else SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);

        SDL_RenderDrawLine(renderer, i, C.height - 1, i, C.height - arr[i]);
    }
}

/**
 * Clear screen and draw current state.
*/
void update_screen(int *arr, SDL_Renderer *renderer, int i, int j) {
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    SDL_RenderClear(renderer);

    draw_state(arr, renderer, i, j);

    SDL_RenderPresent(renderer);
    SDL_Delay(C.delay);
}

/*** SORTING ALGORITHMS ***/

/**
 * Bubble sort algorithm.
*/
void bubble_sort(int *arr, SDL_Renderer *renderer) {
    for (int i = 0; i < C.width; i++) {
        for (int j = i; j < C.width; j++) {
            if (arr[i] > arr[j]) swap(arr, i, j);
            update_screen(arr, renderer, i, j);
        }
    }
}

/**
 * Quick sort helper function.
 * Partitions array in two sub-arrays determined by the pivot.
*/
int partition(int *arr, int start, int end, SDL_Renderer *renderer) {
    int pivot = arr[end];
    int i = start - 1;

    for (int j = start; j <= end - 1; j++) {
        if (arr[j] < pivot) {
            i++;
            swap(arr, i, j);
        }

        update_screen(arr, renderer, i, j);
    }

    i++;
    swap(arr, i, end);

    return i;
}

/**
 * Quick sort algorithm.
*/
void quick_sort(int *arr, int start, int end, SDL_Renderer *renderer) {
    if (end <= start) return;

    int pivot = partition(arr, start, end, renderer);
    quick_sort(arr, start, pivot - 1, renderer);
    quick_sort(arr, pivot + 1, end, renderer);
}

/*** MAIN ***/

/**
 * Da main loop.
 * Parses arguments, nitializes the array along with SDL and its components.
 * It also executes the sorting algorithm, listens for SDL events and performs
 * the final cleanup.
*/
int main(int argc, char **argv) {
    C.width = 200;
    C.height = 150;
    C.scale = 5;
    C.delay = 0;
    C.algorithm = BUBBLE_SORT;

    int opt;
    while ((opt = getopt(argc, argv, ":w:h:s:d:")) != -1) {
        switch (opt) {
            case 'w':
                C.width = atoi(optarg);
                if (C.width <= 0) {
                    fprintf(stderr, "Invalid width value\n");
                    return 1;
                }
                break;
            case 'h':
                C.height = atoi(optarg);
                if (C.height <= 0) {
                    fprintf(stderr, "Invalid height value\n");
                    return 1;
                }
                break;
            case 's':
                C.scale = atof(optarg);
                if (C.scale <= 0) {
                    fprintf(stderr, "Invalid scale value\n");
                    return 1;
                }
                break;
            case 'd':
                C.delay = atoi(optarg);
                if (C.delay < 0) {
                    fprintf(stderr, "Invalid delay value\n");
                    return 1;
                }
                break;
            case ':':
                fprintf(stderr, "Option requires a value\n");
                return 1;
            case '?':
                fprintf(stderr, "Unknown option\n");
                return 1;
        }
    }

    int args_len = 0;
    int arg_pos;
    for (; optind < argc; optind++) {
        if (args_len == 0) arg_pos = optind;
        args_len++;
    }
    
    if (args_len > 1) {
        fprintf(stderr, "Too many arguments provided (use `help` to see usage)");
        return 1;
    } else if (args_len == 1) {
        if (strcmp(argv[arg_pos], "help") == 0) {
            printf("Usage: sort-visualizer [-w WIDTH] [-h HEIGHT] [-s SCALE] [-d DELAY] [ALGORITHM]\n");
            printf("\n");
            printf("Default values:\n");
            printf("  WIDTH -- 200\n");
            printf("  HEIGHT -- 150\n");
            printf("  SCALE -- 5\n");
            printf("  DELAY -- 0\n");
            printf("\n");
            printf("Available algorithms:\n");
            printf("  Bubble sort (default) -- bs\n");
            printf("  Quick sort -- qs\n");
            return 0;
        } else if (strcmp(argv[arg_pos], "bs") == 0) {
            C.algorithm = BUBBLE_SORT;
        } else if (strcmp(argv[arg_pos], "qs") == 0) {
            C.algorithm = QUICK_SORT;
        } else {
            fprintf(stderr, "Unknown algorithm (use `help` to see available ones)\n");
            return 1;
        }
    }

    srand(time(NULL));

    // Numbers in range [1, C.height - 1]
    int arr[C.width];
    for (int i = 0; i < C.width; i++) {
        arr[i] = randint(C.height - 1) + 1;
    }

    if (SDL_Init(SDL_INIT_VIDEO) != 0) {
        SDL_Log("Unable to initialize SDL subsystems: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Window *window = SDL_CreateWindow("Sort Visualizer", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, C.width * C.scale, C.height * C.scale, 0);
    if (!window) {
        SDL_Log("Unable to create window: '%s'\n", SDL_GetError());
        return 1;
    }

    SDL_Renderer *renderer = SDL_CreateRenderer(window, -1, SDL_RENDERER_ACCELERATED);
    if (!renderer) {
        SDL_Log("Unable to create renderer: '%s'\n", SDL_GetError());
        return 1;
    }

    if (SDL_RenderSetScale(renderer, C.scale, C.scale) != 0) {
        SDL_Log("Unable to set render scale: '%s'\n", SDL_GetError());
        return 1;
    }

    clock_t begin = clock();

    switch (C.algorithm) {
        case BUBBLE_SORT:
            bubble_sort(arr, renderer);
            break;
        case QUICK_SORT:
            quick_sort(arr, 0, C.width - 1, renderer);
            break;
    }

    clock_t end = clock();

    print_arr(arr);
    printf("Elapsed time: %f seconds\n", (double)(end - begin) / CLOCKS_PER_SEC);

    int running = 1;
    while (running) {
        SDL_Event event;
        while (SDL_PollEvent(&event)) {
            switch (event.type) {
                case SDL_QUIT:
                    running = 0;
                    break;
                
                default:
                    break;
            }
        }
    }

    SDL_DestroyRenderer(renderer);
    SDL_DestroyWindow(window);
    SDL_Quit();

    return 0;
}